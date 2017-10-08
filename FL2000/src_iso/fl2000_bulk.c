// fl2000_bulk.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose: Bulk Pipe Support
//

#include "fl2000_include.h"

//change MERGE_ADJACENT_PAGES  from 1 to 0
#define	MERGE_ADJACENT_PAGES	1

/*
 * this routine is called typically from hard_irq context, as of the latest
 * xHCI implementation. We do our processing in lower IRQL context by launching
 * a tasklet.
 */
void fl2000_bulk_main_completion(
	struct urb *urb
	)
{
	struct render_ctx * const render_ctx = urb->context;
	struct dev_ctx * const dev_ctx = render_ctx->dev_ctx;
	unsigned long flags;
	uint32_t pending_count;

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	pending_count = --render_ctx->pending_count;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
        //+++++ add some printk in fl2000_bulk_main_completion
	printk("--------------- enter bulk_main_completion ------------\n");
	//+++++ISO put dev_ctx->main_urb_finished = true
//	render_ctx->main_urb_finished = true;
	//+++++ISO add usb_free_urb(urb) in fl2000_bulk_main_completion
	if(urb){
		usb_free_urb(urb);
		urb=NULL;
	}

	if (pending_count == 0) {
		if (in_irq()) {
			struct tasklet_struct * tasklet = &render_ctx->tasklet;
			printk("bulk_main_completion: in_irq() \n");
			tasklet_init(
				tasklet,
				fl2000_render_completion_tasklet,
				(unsigned long) render_ctx);
			tasklet_schedule(tasklet);
		} else {
			printk("bulk_main_completion: NOT in_irq() \n");
			fl2000_render_completion(render_ctx);
		}
	}

}

/*
 * this routine is called typically from hard_irq context, as of the latest
 * xHCI implementation. We do our processing in lower IRQL context by launching
 * a tasklet.
 */
void fl2000_bulk_zero_length_completion(
	struct urb *urb
	)
{
	struct render_ctx * const render_ctx = urb->context;
	struct dev_ctx * const dev_ctx = render_ctx->dev_ctx;
	uint32_t pending_count;
	unsigned long flags;

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	pending_count = --render_ctx->pending_count;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

        //+++++ add some printk in fl2000_bulk_zero_length_completion
	printk("--------------- enter bulk_zero_length_completion ------------\n");

	if (pending_count == 0) {
		if (in_irq()) {
			struct tasklet_struct * tasklet = &render_ctx->tasklet;

			printk("bulk_zero_length_completion: in_irq() \n");

			tasklet_init(
				tasklet,
				fl2000_render_completion_tasklet,
				(unsigned long) render_ctx);
			tasklet_schedule(tasklet);
		} else {
			fl2000_render_completion(render_ctx);
			printk("bulk_zero_length_completion: NOT in_irq() \n");
		}
	}

}

void fl2000_bulk_prepare_urb(
	struct dev_ctx * dev_ctx,
	struct render_ctx * render_ctx
	)
{
	struct primary_surface* const surface = render_ctx->primary_surface;
	struct scatterlist * const sglist = &surface->sglist[0];
	struct scatterlist * list_entry;
	unsigned int len = surface->buffer_length;
	unsigned int nr_pages = 0;
	unsigned int num_sgs = 0;
	unsigned int i;

//+++++ISO add int 'static bool isourb_read','number_of_packets' and 'int interval' for usb_fill_iso_urb()
	static bool isourb_ready = false;
        int number_of_packets; // = buffer_length /wMaxPacketSize(3076byte in USB_VGA info.)
        int interval;              // time interval, see bInterval = 1 in USB_VGA info.
//------------------------end of ISO


	render_ctx->transfer_buffer = surface->render_buffer;
	render_ctx->transfer_buffer_length = surface->buffer_length;

	list_entry = &sglist[0];
	if (surface->render_buffer == surface->system_buffer &&
	    surface->type == SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTENT) {
		nr_pages = surface->nr_pages;

		//+++++----- printk information added
		printk("-------------fl2000_bulk_prepare_urb WHEN surface->render_buffer == surface->system_buffer &&  SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTEN -----\n");

		dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			"surface->nr_pages(%u), start_offset(0x%x)",
			nr_pages, surface->start_offset);

		/*
		 * the buffer is from user mode address, where the start offset
		 * might not be zero
		 */
		sg_init_table(sglist, nr_pages);
		sg_set_page(
			list_entry,
			surface->pages[0],
			PAGE_SIZE - surface->start_offset,
			surface->start_offset);
		len -= PAGE_SIZE - surface->start_offset;
		num_sgs++;
		for (i = 1; i < nr_pages; i++) {
			struct page * pg = surface->pages[i];
#if (MERGE_ADJACENT_PAGES)
			struct page * prev_pg = surface->pages[i - 1];
			unsigned long this_pfn = page_to_pfn(pg);
			unsigned long prev_pfn = page_to_pfn(prev_pg);

			if (this_pfn != prev_pfn + 1) {
				list_entry = &sglist[num_sgs];
				num_sgs++;
				sg_set_page(list_entry, pg, 0, 0);
			}
#else
			list_entry = &sglist[num_sgs];
			num_sgs++;
			sg_set_page(list_entry, pg, 0, 0);
#endif

			/*
			 * update list_entry
			 */
			if (len > PAGE_SIZE) {
				list_entry->length += PAGE_SIZE;
				len -= PAGE_SIZE;
				}
			else {
				list_entry->length += len;
				dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
					"sglist[%u], len = 0x%x",
					num_sgs - 1, len);
			}
		}
	}

//+++++ disable else if() for VIRTUAL_CONTIGUOUS || PHYSICAL_CONTIGUOUS, in fl2000_bulk_prepare_urb()
/*
	else if (surface->render_buffer == surface->system_buffer &&
	         (surface->type == SURFACE_TYPE_VIRTUAL_CONTIGUOUS ||
		  surface->type == SURFACE_TYPE_PHYSICAL_CONTIGUOUS)) {

		//+++++ printk information added
		printk("-------------fl2000_bulk_prepare_urb with surface type VIRTURAL or PHYSICAL CONTIGUOUS -----\n");

		sg_init_table(sglist, 1);
		sg_set_page(
			list_entry,
			surface->first_page,
			len - surface->start_offset,
			surface->start_offset);
		num_sgs++;
		dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			"sglist[%u], len = 0x%x",
			num_sgs - 1, len);
	}
*/
	else {
		/*
		 * the buffer is allocated in kernel vmalloc space. the start
		 * offset should be zero.
		 */

		//+++++ printk information added
		printk("-------------fl2000_bulk_prepare_urb EXCEPT surface->render_buffer == surface->system_buffer &&  SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTEN -----\n");

		unsigned long start;
		unsigned long end;
		unsigned int start_offset;
		uint8_t * buf = surface->render_buffer;
		uint8_t * prev_buf;

		start = (unsigned long) buf & PAGE_MASK;
		end = (unsigned long) (buf + surface->buffer_length);
		start_offset = ((unsigned long) buf) & ~PAGE_MASK;
		nr_pages = (end - start + PAGE_SIZE - 1) >> PAGE_SHIFT;

		dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			"len(x%x), nr_pages(%u), start_offset(0x%x)",
			len, nr_pages, (unsigned int) start_offset);

		sg_init_table(sglist, nr_pages);
		sg_set_page(
			&sglist[0],
			vmalloc_to_page(buf),
			PAGE_SIZE - start_offset,
			start_offset);
		num_sgs++;
		prev_buf = buf;
		len -= PAGE_SIZE - start_offset;
		buf += PAGE_SIZE - start_offset;

		for (i = 1; i < nr_pages; i++) {
			struct page * pg = vmalloc_to_page(buf);
#if (MERGE_ADJACENT_PAGES)
			struct page * prev_pg = vmalloc_to_page(prev_buf);
			unsigned long this_pfn = page_to_pfn(pg);
			unsigned long prev_pfn = page_to_pfn(prev_pg);

			if (this_pfn != prev_pfn + 1) {
				list_entry = &sglist[num_sgs];
				num_sgs++;
				sg_set_page(list_entry, pg, 0, 0);
			}
#else
			list_entry = &sglist[num_sgs];
			num_sgs++;
			sg_set_page(list_entry, pg, 0, 0);
#endif

			/*
			 * update list_entry
			 */
			if (len > PAGE_SIZE) {
				list_entry->length += PAGE_SIZE;
				len -= PAGE_SIZE;
				prev_buf = buf;
				buf += PAGE_SIZE;
				}
			else {
				list_entry->length += len;
				prev_buf = buf;
				buf += len;
				dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
					"sglist[%u], len = 0x%x",
					num_sgs - 1, len);
			}
		}
	}
	sg_mark_end(list_entry);

	dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
		"------ num_sgs(%u) transfer_buffer_length:(%u)", num_sgs,render_ctx->transfer_buffer_length);

//+++++URB get surface->num_sgs
	surface->num_sgs = num_sgs;
//--------------end of URB

//+++++ISO in stead of usb_init_urb(), we try to use usb_alloc_urb(int iso_packets, gft_t mem_flags)
//	usb_init_urb(render_ctx->main_urb);
	number_of_packets = (render_ctx->transfer_buffer_length / ISO_PACKET_SIZE);//= buffer_length /wMaxPacketSize(3076byte in USB_VGA info.)

//-- MUST NOT !!!!! free old urb first
/*
        if (render_ctx->main_urb) {
                usb_free_urb( render_ctx->main_urb);
                render_ctx->main_urb = NULL;
        }
        if (render_ctx->zero_length_urb) {
                usb_free_urb(render_ctx->zero_length_urb);
                render_ctx->zero_length_urb = NULL;
        }
*/
//+++++-----ISO for test of number_of_packets=16;
 //number_of_packets=16;
//------------------end of ISO


	//--- alloc main_urb and zero_length_urb --- alloc urb each time !!!!!!!
//	if(isourb_ready == false){
		printk("-------start alloc main_urb and zero_lenth_urb:render_ctx->transfer_buffer_length=%d,  number_of_packets=%d ------------\n",render_ctx->transfer_buffer_length,number_of_packets);
		//------------ alloc render_ctx->main_urb
		// ????? urb already allocated in render_ctx_create() !!!!!!!
		render_ctx->main_urb = usb_alloc_urb(number_of_packets,GFP_ATOMIC);//GFP_ATOMIC);
		if(render_ctx->main_urb == NULL ){
			printk("----------   alloc render_ctx->main_urb fails!  -----------\n");
			return;
		}
		else 
			printk("----------   alloc render_ctx->main_urb succeeds! as urb(%p)  -----------\n",render_ctx->main_urb);
		//----------- alloc render_ctx->zero_length_urb
/*
	        if (dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) {
			// ????? urb already allocated in render_ctx_create() !!!!!!!
        	        render_ctx->zero_length_urb = usb_alloc_urb(1,GFP_ATOMIC);//GFP_ATOMIC);
                	if(render_ctx->zero_length_urb == NULL ){
                        	printk("----------   alloc render_ctx->zero_length_urb fails!  -----------\n");
	                        return;
        	        }
                	else
                        	printk("----------   alloc render_ctx->zero_length_urb succeeds! as urb(%p)  -----------\n",render_ctx->zero_length_urb);
		}
*/
		//----set token
		isourb_ready = true;
//	}

//-------------------end of ISO

//+++++ISO  force main_urb->num_sgs=0
	render_ctx->main_urb->num_sgs = 0;//num_sgs; !!!! for ISO tranfer num_sgs MUST be 0!!!
	render_ctx->main_urb->sg = sglist;
//------------end of ISO

//+++++ISO comment out usb_fill_bulk_urb()  for BULK and replaced with usb_fill_iso_urb()
/*
	usb_fill_bulk_urb(
		render_ctx->main_urb,
		dev_ctx->usb_dev,
		dev_ctx->usb_pipe_bulk_out,
		render_ctx->transfer_buffer,
		render_ctx->transfer_buffer_length,
		fl2000_bulk_main_completion,
		render_ctx);
*/

	interval = 1; //2^(1-1)*125us

	printk("-------start usb_fill_iso_urb() for DATA: number_of_packets=%d ------------\n",number_of_packets);
	usb_fill_iso_urb(
                render_ctx->main_urb,
                dev_ctx->usb_dev,
                dev_ctx->usb_pipe_iso_out,
                render_ctx->transfer_buffer,
                render_ctx->transfer_buffer_length, // change it, NO EFFECT !!!
		number_of_packets,
		interval,
                fl2000_bulk_main_completion,
                render_ctx);

//-------------- BULK  usb_fill_bulk_urb() -----------------

	if (dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) {
		usb_init_urb(render_ctx->zero_length_urb);
		usb_fill_bulk_urb(
			render_ctx->zero_length_urb,
			dev_ctx->usb_dev,
			dev_ctx->usb_pipe_bulk_out,
			NULL,
			0,
			fl2000_bulk_zero_length_completion,
			render_ctx);
	}


//-------------- ISOCH  usb_fill_iso_urb() -----------------
/*
	render_ctx->zero_length_urb->num_sgs = 0;
        if (dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) {
        	usb_fill_iso_urb(
	                render_ctx->zero_length_urb,
        	        dev_ctx->usb_dev,
                	dev_ctx->usb_pipe_iso_out,
			NULL,
			0,
        	        1,
                	interval,
			fl2000_bulk_zero_length_completion,
	                render_ctx);
	}
*/
//-----------------------------end of ISO
}

// eof: fl2000_bulk.c
//
