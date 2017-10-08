// fl2000_render.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose: Render Device Support
//

#include "fl2000_include.h"

int  bulk_transf_count=0;

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//

/*
 * push render_ctx to the bus, with dev_ctx->render.busy_list_lock held
 */
int
fl2000_render_with_busy_list_lock(
	struct dev_ctx * dev_ctx,
	struct render_ctx * render_ctx
	)
{
	struct list_head* const	free_list_head = &dev_ctx->render.free_list;
	int ret_val = 0;
	unsigned long flags;
//+++++URB add: static bool  sglist_ready=false;
	static bool sglist_ready=false;
        struct primary_surface* const surface = render_ctx->primary_surface;
        render_ctx->transfer_buffer = surface->render_buffer;
	render_ctx->transfer_buffer_length = surface->buffer_length;
//----------------------ebd if URB

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	if (!dev_ctx->monitor_plugged_in) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER,
			"WARNING Monitor is not attached.");
		/*
		 * put the render_ctx into free_list_head
		 */
		spin_lock_bh(&dev_ctx->render.free_list_lock);
		list_add_tail(&render_ctx->list_entry, free_list_head);
		spin_unlock_bh(&dev_ctx->render.free_list_lock);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->render.free_list_count++;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		goto exit;
	}

	/*
	 * put this render_ctx into busy_list
	 */
	list_add_tail(&render_ctx->list_entry, &dev_ctx->render.busy_list);
	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->render.busy_list_count++;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

//+++++ISO suspend +++++URB sglist_ready check
/*
//+++++URB: add check if  sglist_ready==true && surface->num_sgs>0
//--check if sglist is ready and surface->num_sgs>0
	if( sglist_ready ==true && surface->num_sgs>0){
		//--Now we can skip sglist preparation and go to fill urb 
		printk("----------  sglist_ready ==true && surface->num_sgs>0  -----------\n");
	        usb_init_urb(render_ctx->main_urb);
	        render_ctx->main_urb->num_sgs = surface->num_sgs;
        	render_ctx->main_urb->sg = &surface->sglist[0];
        	usb_fill_bulk_urb(
                	render_ctx->main_urb,
	                dev_ctx->usb_dev,
        	        dev_ctx->usb_pipe_bulk_out,
                	render_ctx->transfer_buffer,
	                render_ctx->transfer_buffer_length,
        	        fl2000_bulk_main_completion,
                	render_ctx);

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


	}
	else //call bulk_prepare_urb() and get sglist and store it in the surface
	{
		printk("--------sglist_ready =%d,  surface->num_sgs=%d  -----------\n", sglist_ready,surface->num_sgs);
		fl2000_bulk_prepare_urb(dev_ctx, render_ctx); //--original line
		sglist_ready = true;
	}
//---------------------------end of  URB
*/
// +++++-----ISO  test ----
 	fl2000_bulk_prepare_urb(dev_ctx, render_ctx); //--original line

//---------------------------end of  ISO

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	render_ctx->pending_count++;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

        //-------- check ISO pipe  ----
        if(!usb_pipe_endpoint(render_ctx->main_urb->dev, render_ctx->main_urb->pipe))
		printk("fl2000_render_with_busy_list_lock(): main_urb->pipe=0x%x,  ISO pipe unavailable!! .... -----------\n",render_ctx->main_urb->pipe);

	printk("fl2000_render_with_busy_list_lock()  start: usb_submit_urb() of  main_urb.... -----------\n");
	ret_val = usb_submit_urb(render_ctx->main_urb, GFP_ATOMIC);

	if (ret_val != 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] usb_submit_urb(%p) failed with %d!",
			render_ctx->main_urb,
			ret_val);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		render_ctx->pending_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

		/*
		 * remove this render_ctx from busy_list
		 */
		list_del(&render_ctx->list_entry);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->render.busy_list_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

		/*
		 * put the render_ctx into free_list_head
		 */
		spin_lock_bh(&dev_ctx->render.free_list_lock);
		list_add_tail(&render_ctx->list_entry, free_list_head);
		spin_unlock_bh(&dev_ctx->render.free_list_lock);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->render.free_list_count++;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

		if (-ENODEV == ret_val || -ENOENT == ret_val) {
			/*
			 * mark the fl2000 device gone
			 */
			dev_ctx->dev_gone = 1;
		}
	    goto exit;
	}




//+++++----- add bulk_transf_count, only when =2 to submit EOF_ZERO_LENGTH  urb, 
//when bulk_transf_count>2 AMD half bulk data, it will cause great leap on screen 

//+++++-----ISO check render_ctx->main_urb_finished
/*
while(render_ctx->main_urb_finished == false){
msleep(2);  !!!!!-MUST NOT SLEEP !!!
}
render_ctx->main_urb_finished =true;
*/
//-------------end of ISO check render_ctx->main_urb_finished

bulk_transf_count+=1;

if(bulk_transf_count>0)
{
  bulk_transf_count=0;

	if ((dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) &&
//+++++-----ISO for VR_TRANSFER_PIPE_BULK == dev_ctx->vr_params.transfer_pipe
//	    (VR_TRANSFER_PIPE_BULK == dev_ctx->vr_params.trasfer_pipe)) {
	    (VR_TRANSFER_PIPE_ISOCH == dev_ctx->vr_params.trasfer_pipe)) {
//---------end of ISO
		printk("fl2000_render_with_busy_list_lock()  start: usb_submit_urb() of  zero_length_urb.... -----------\n");
		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		render_ctx->pending_count++;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		ret_val = usb_submit_urb(
			render_ctx->zero_length_urb, GFP_ATOMIC);
		if (ret_val != 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"[ERR] zero_length_urb submit fails with %d.",
				ret_val);

			 // the main_urb is already schedule, we wait until
			 // the completion to move the render_ctx to free_list

			spin_lock_irqsave(&dev_ctx->count_lock, flags);
			render_ctx->pending_count--;
			spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
			if (-ENODEV == ret_val || -ENOENT == ret_val) {

				 // mark the fl2000 device gone
				dev_ctx->dev_gone = 1;
			}
			goto exit;
		}
	}

} // end of  (bulk_transf_count>1)

//+++++-----ISO  for test: add goto exit
//ret_val = -5;
//----------------end of ISO

exit:
    dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
    return ret_val;
}

int
fl2000_render_ctx_create(
	struct dev_ctx * dev_ctx
	)
{
	struct render_ctx * render_ctx;
	int		ret_val;
	uint32_t	i;
	unsigned long flags;

	ret_val = 0;
	for (i = 0; i < NUM_OF_RENDER_CTX; i++) {
		render_ctx = &dev_ctx->render.render_ctx[i];

		INIT_LIST_HEAD(&render_ctx->list_entry);
		render_ctx->dev_ctx = dev_ctx;
		render_ctx->pending_count = 0;

		render_ctx->main_urb = usb_alloc_urb(0, GFP_ATOMIC);
		if (!render_ctx->main_urb) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"no main_urb usb_alloc_urb?");
			ret_val = -ENOMEM;
			goto exit;
		}

		render_ctx->zero_length_urb = usb_alloc_urb(0, GFP_ATOMIC);
		if (!render_ctx->zero_length_urb) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"no zero_length_urb?" );
			ret_val = -ENOMEM;
			goto exit;
		}

		list_add_tail(&render_ctx->list_entry,
			&dev_ctx->render.free_list);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->render.free_list_count++;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
	}

exit:
    return ret_val;
}

void
fl2000_render_ctx_destroy(
    struct dev_ctx * dev_ctx
    )
{
	struct render_ctx * render_ctx;
	uint32_t 	i;
	unsigned long flags;

	for (i = 0; i < NUM_OF_RENDER_CTX; i++) {
		render_ctx = &dev_ctx->render.render_ctx[i];

		// It can be NULL in case of failed initialization.
		//
		if (render_ctx == NULL)
			break;

		if (render_ctx->main_urb) {
		    usb_free_urb( render_ctx->main_urb);
		    render_ctx->main_urb = NULL;
		}

		if (render_ctx->zero_length_urb) {
			usb_free_urb(render_ctx->zero_length_urb);
			render_ctx->zero_length_urb = NULL;
		}

		list_del(&render_ctx->list_entry);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->render.free_list_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

int
fl2000_render_create(struct dev_ctx * dev_ctx)
{
	int ret_val;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	INIT_LIST_HEAD(&dev_ctx->render.free_list);
	spin_lock_init(&dev_ctx->render.free_list_lock);
	dev_ctx->render.free_list_count = 0;

	INIT_LIST_HEAD(&dev_ctx->render.ready_list);
	spin_lock_init(&dev_ctx->render.ready_list_lock);
	dev_ctx->render.ready_list_count = 0;

	INIT_LIST_HEAD(&dev_ctx->render.busy_list);
	spin_lock_init(&dev_ctx->render.busy_list_lock);
	dev_ctx->render.busy_list_count = 0;

	ret_val = fl2000_render_ctx_create(dev_ctx);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] fl2000_render_ctx_create failed?");
		goto exit;
	}

	INIT_LIST_HEAD(&dev_ctx->render.surface_list);
	spin_lock_init(&dev_ctx->render.surface_list_lock);
	dev_ctx->render.surface_list_count = 0;

exit:
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] Initialize threads failed.");
		fl2000_render_destroy(dev_ctx);
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
	return ret_val;
}

void
fl2000_render_destroy(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	fl2000_render_ctx_destroy(dev_ctx);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
}

void fl2000_render_completion(struct render_ctx * render_ctx)
{
	struct dev_ctx * const dev_ctx = render_ctx->dev_ctx;
	int const urb_status = render_ctx->main_urb->status;
	unsigned long flags;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	/*
	 * remove this render_ctx from busy_list
	 */
	spin_lock_bh(&dev_ctx->render.busy_list_lock);
	list_del(&render_ctx->list_entry);
	spin_unlock_bh(&dev_ctx->render.busy_list_lock);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->render.busy_list_count--;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	/*
	 * put the render_ctx into free_list_head
	 */
	spin_lock_bh(&dev_ctx->render.free_list_lock);
	list_add_tail(&render_ctx->list_entry, &dev_ctx->render.free_list);
	spin_unlock_bh(&dev_ctx->render.free_list_lock);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->render.free_list_count++;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

//+++++printk
	printk("fl2000_render_completion: main_urb->iso_frame_desc[2]. status=%d   actual_length=%d  ------\n",\
	render_ctx->main_urb->iso_frame_desc[2].status,render_ctx->main_urb->iso_frame_desc[2].actual_length);
	if (urb_status < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"urb->status(%d) error", urb_status);
		dev_ctx->render.green_light = 0;
		if (urb_status == -ESHUTDOWN || urb_status == -ENOENT ||
		    urb_status == -ENODEV) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "mark device gone");
			dev_ctx->dev_gone = true;
		    }
		goto exit;
	}

//+++++ISO STOP fl2000_schedule_next_render() after URB completion in  fl2000_render_completion()
	fl2000_schedule_next_render(dev_ctx);
//-------------------end of ISO

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
}

void fl2000_render_completion_tasklet(unsigned long data)
{
	struct render_ctx * render_ctx = (struct render_ctx *) data;
	fl2000_render_completion(render_ctx);
}

/*
 * schedule a frame buffer for update.
 * the input frame_buffer should be pinned down or resident in kernel sapce
 */
void
fl2000_primary_surface_update(
	struct dev_ctx * 	dev_ctx,
	struct primary_surface* surface)
{
	struct list_head* const	free_list_head = &dev_ctx->render.free_list;
	struct list_head* const	ready_list_head = &dev_ctx->render.ready_list;
	struct render_ctx *	render_ctx;
	uint32_t		retry_count = 0;
	uint32_t		ready_count = 0;
	uint32_t		free_count = 0;
	unsigned long flags;

	might_sleep();

	dev_ctx->render.last_updated_surface = surface;
	dev_ctx->render.last_frame_num = surface->frame_num;

	if (dev_ctx->render.green_light == 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER, "green_light off");
		goto exit;
	}

retry:
	/*
	 * get render_ctx from free_list_head
	 */
	render_ctx = NULL;
	spin_lock_bh(&dev_ctx->render.free_list_lock);
	if (!list_empty(free_list_head)) {
		render_ctx = list_first_entry(
			free_list_head, struct render_ctx, list_entry);
		list_del(&render_ctx->list_entry);

		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->render.free_list_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
	}
	spin_unlock_bh(&dev_ctx->render.free_list_lock);

	if (render_ctx == NULL) {
		if (retry_count > 3) {
			dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER,
				"no render_ctx?");
			return;
		}
		retry_count++;
		msleep_interruptible(10);
		goto retry;
	}

	/*
	 * by now we have a render_ctx, initialize it and schedule it
	 */
	render_ctx->primary_surface = surface;

	//+++++COMP add fl2000_compression_gravity()-------------------
   	if(dev_ctx->vr_params.use_compression){
		uint32_t ncomp;
		if(surface->render_buffer == surface->shadow_buffer){  // Use shadow_buffer to render 
			 ncomp=fl2000_compression_gravity(
        			dev_ctx,//struct dev_ctx * dev_ctx,
		        	render_ctx,//struct render_ctx * render_ctx,
	        		surface->buffer_length,//size_t data_buffer_length,
		        	surface->system_buffer,//uint8_t * source,-----we self_swapped system_bffer in fl2000_ioctl.c
	        		surface->shadow_buffer, //uint8_t * target,-------------------------
		        	surface->buffer_length>>1//for it's in RGB_16, uint32_t num_of_pixels
	       	 	);
	 	}
		else if(surface->render_buffer == surface->system_buffer){ //Use system_buffer to render
			ncomp=fl2000_compression_gravity(
        			dev_ctx,//struct dev_ctx * dev_ctx,
		        	render_ctx,//struct render_ctx * render_ctx,
     		   		surface->buffer_length,//size_t data_buffer_length,
	        		surface->shadow_buffer,//uint8_t * source,-------we swapped system_buffer to shadow_buffer in fl2000_ioctl.c
	        		surface->system_buffer, //uint8_t * target,-------------------------
		        	surface->buffer_length>>1//for it's in RGB_16, uint32_t num_of_pixels
	       		 );
		 }
		surface->buffer_length = ncomp;//length aft. compression
		printk("=========== compressed_length  ncomp=%d     ==========\n",ncomp);
  	}
	////////----------------------------------------------end of COMP

	spin_lock_bh(&dev_ctx->render.ready_list_lock);
	list_add_tail(&render_ctx->list_entry, ready_list_head);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	ready_count = ++dev_ctx->render.ready_list_count;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	spin_unlock_bh(&dev_ctx->render.ready_list_lock);

	dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER,
		"render_ctx(%p) scheduled, free_count(%u)/ready_count(%u)",
		render_ctx, free_count, ready_count);

	/*
	 * schedule render context from ready_list
	 */
	fl2000_schedule_next_render(dev_ctx);

exit:
	return;
}

/*
 * schedule all render_ctx on ready_list. We schedule redundant frames if no
 * new frames are available.
 */
void
fl2000_schedule_next_render(struct dev_ctx * dev_ctx)
{
	struct list_head* const	free_list_head = &dev_ctx->render.free_list;
	struct list_head* const	ready_list_head = &dev_ctx->render.ready_list;
	struct list_head	staging_list;
	struct render_ctx *	render_ctx = NULL;
	uint32_t		ready_count = 0;
	unsigned long 		flags;
	int			ret_val;

	if (dev_ctx->render.green_light == 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER, "green_light off");
		goto exit;
	}

	/*
	 * step 1: take out all render_ctx on ready list, and put them to
	 * staging list
	 */
	INIT_LIST_HEAD(&staging_list);
	spin_lock_bh(&dev_ctx->render.ready_list_lock);
	while (!list_empty(ready_list_head)) {
		render_ctx = list_first_entry(
			ready_list_head, struct render_ctx, list_entry);
		list_del(&render_ctx->list_entry);
		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		ready_count = --dev_ctx->render.ready_list_count;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		list_add_tail(&render_ctx->list_entry, &staging_list);
	}
	ASSERT(ready_count == 0);
	spin_unlock_bh(&dev_ctx->render.ready_list_lock);

	/*
	 * step 2, schedule all render_ctx, with busy_list_lock held.
	 * this is critical path where we schedule redundant frames.
	 */
	spin_lock_bh(&dev_ctx->render.busy_list_lock);
reschedule:
	while (!list_empty(&staging_list)) {
		render_ctx = list_first_entry(
			&staging_list, struct render_ctx, list_entry);
		list_del(&render_ctx->list_entry);
		ret_val = fl2000_render_with_busy_list_lock(dev_ctx, render_ctx);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"usb_submit_urb failed %d, "
				"turn off green_light\n", ret_val);
			dev_ctx->render.green_light = false;
			break;
		}
	}

	/*
	 * schedule redundant frames.
	 */
	if (dev_ctx->render.busy_list_count < NUM_RENDER_ON_BUS &&
	    dev_ctx->render.green_light) {
		struct primary_surface* surface;

		spin_lock_bh(&dev_ctx->render.free_list_lock);
		if (!list_empty(free_list_head)) {
			render_ctx = list_first_entry(
				free_list_head, struct render_ctx, list_entry);
			list_del(&render_ctx->list_entry);

			spin_lock_irqsave(&dev_ctx->count_lock, flags);
			dev_ctx->render.free_list_count--;
			spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		}
		spin_unlock_bh(&dev_ctx->render.free_list_lock);

		/*
		 * preparing additional frame
		 */
		if (render_ctx != NULL) {
			surface = dev_ctx->render.last_updated_surface;
			render_ctx->primary_surface = surface;
			list_add_tail(&render_ctx->list_entry, &staging_list);
			goto reschedule;
		}
	}
	spin_unlock_bh(&dev_ctx->render.busy_list_lock);

exit:
	return;
}

void fl2000_render_start(struct dev_ctx * dev_ctx)
{
	dev_ctx->render.green_light = 1;
}

void fl2000_render_stop(struct dev_ctx * dev_ctx)
{
	uint32_t delay_ms = 0;

	might_sleep();
	dev_ctx->render.green_light = 0;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"busy_list_count(%u)", dev_ctx->render.busy_list_count);

	while (dev_ctx->render.busy_list_count != 0) {
		delay_ms += 10;
		DELAY_MS(10);
	}
	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"waited %u ms", delay_ms);

}

// eof: fl2000_render.c
//
