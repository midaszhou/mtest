// fl2000_bulk.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose: Companion File.
//

#ifndef _FL2000_BULK_H_
#define _FL2000_BULK_H_

//+++++ add fl2000_bulk_main_completion() and zero_length_completion() in bulk.h file that let render.c can see
void fl2000_bulk_main_completion(
        struct urb *urb
        );

void fl2000_bulk_zero_length_completion(
        struct urb *urb
        );
//-------------end of URB

void fl2000_bulk_prepare_urb(
	struct dev_ctx * dev_ctx,
	struct render_ctx * render_ctx);

#endif // _FL2000_BULK_H_

// eof: fl2000_bulk.h
//
