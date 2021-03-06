#include "fl2000_include.h"

 void usb_fill_iso_urb(
				struct urb *urb,
				struct usb_device *dev,
				unsigned int pipe,
				void *transfer_buffer,
				int buffer_length,
				int number_of_packets, // = buffer_length /wMaxPacketSize(3076byte in USB_VGA info.)
				int interval,              // time interval, see bInterval = 1 in USB_VGA info.
				usb_complete_t complete_fn,
				void *context
			      )
{
	int i;

	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->number_of_packets = number_of_packets;
	urb->interval = interval;
	urb->complete = complete_fn;
	urb->context = context;
//+++++ISO set urb->start_frame = 0; !!!NO EFFECT
	urb->start_frame = 0;

	 //---------set transfer flags ----------------
	//+++++ISO add URB_NO_INTERRUPT flag
//	urb->transfer_flags = URB_ISO_ASAP; //URB_NO_INTERRUPT | URB_ISO_ASAP;//ISO-ONLY, urb->start_frame is ignored then // URB_ZERO_PACKET is for BULK

	for(i=0;i<number_of_packets; i++){
		urb->iso_frame_desc[i].offset = i* ISO_PACKET_SIZE;//3076; //3076bytes max. for EP2 
		urb->iso_frame_desc[i].length = ISO_PACKET_SIZE;//3076;
		urb->iso_frame_desc[i].status = 0;
		urb->iso_frame_desc[i].actual_length = 0;
	}
}


