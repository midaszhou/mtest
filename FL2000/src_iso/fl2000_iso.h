#ifndef _FL2000_ISO_H_
#define _FL2000_ISO_H_

#define ISO_PACKET_SIZE (3*1024)

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
                              );

#endif
