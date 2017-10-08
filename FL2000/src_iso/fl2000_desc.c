// fl2000_desc.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose:
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//


/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//
void fl2000_desc_init(struct dev_ctx * dev_ctx)
{
	memcpy(	&dev_ctx->usb_dev_desc,
		&dev_ctx->usb_dev->descriptor,
		sizeof(struct usb_device_descriptor));

//+++++	 change card_name = CARD_NAME_UNDEFINED to CARD_NAME_FL2000;
        //dev_ctx->card_name = CARD_NAME_UNDEFINED;
	dev_ctx->card_name=CARD_NAME_FL2000;

/*
	if (VID_FRESCO_LOGIC == dev_ctx->usb_dev_desc.idVendor &&
	    PID_FL2000 == dev_ctx->usb_dev_desc.idProduct &&
	    DEVICE_ID_FL2000DX == dev_ctx->usb_dev_desc.bcdDevice)
		dev_ctx->card_name = CARD_NAME_FL2000DX;
*/
}

// eof: f2l000_desc.c
//
