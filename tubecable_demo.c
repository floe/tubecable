/*
 * libtubecable - displaylink protocol reference implementation
 *
 * version 0.1.1 - added missing Huffman sequences
 *                 fixed 2 bugs in encoder 
 *                 June 5th, 2009
 *
 * version 0.1   - initial public release
 *                 May 30th, 2009
 *
 * written 2008/09 by floe at butterbrot.org
 * in cooperation with chrisly at platon42.de
 * this code is released as public domain.
 *
 * this is so experimental that the warranty shot itself.
 * so don't expect any.
 *
 */

#include "tubecable.h"

int main(int argc, char* argv[] ) {

	printf("\ndisplaylink userspace controller demo 0.1\n\n");
	printf("written 2008/09 by floe at butterbrot.org\n");
	printf("in cooperation with chrisly at platon42.de\n");
 	printf("this code is released as public domain.\n\n");
	printf("this is so experimental that the warranty shot itself.\n");
	printf("so don't expect any.\n\n");
	printf("(note: you can pass a 640x480 pixel RGB raw image file as parameter)\n\n");

	#define XRES 640
	#define YRES 480

	dl_cmdstream cs;
	create( &cs, 1024*1024 );

	// load huffman table
	dl_huffman_load_table( "tubecable_huffman.c" );

	usb_dev_handle* handle = usb_get_device_handle( 0x17E9, 0x01AE ); // DL-120
	if (!handle)
		handle = usb_get_device_handle( 0x17E9, 0x0141 ); // DL-160
	if (!handle)
		return 1;

	// startup control messages & decompressor table
	dl_init( handle );

	// default register set & offsets
	printf("setting default registers for 640x480@60Hz..\n");
	dl_reg_set_all( &cs, dl_reg_mode_640x480_60 );
	dl_reg_set_offsets( &cs, 0x000000, 0x000A00, 0x555555, 0x000500 );
	/*dl_set_offsets( &cs, 0x000000, 0x000A00, 0x555555, 0x000500 );
	dl_unknown_command( &cs );
	dl_set_offsets( &cs, 0x000000, 0x000A00, 0x555555, 0x000500 );*/
	dl_reg_set( &cs, DL_REG_BLANK_SCREEN, 0x00 ); // enable output
	dl_reg_set( &cs, DL_REG_SYNC, 0xFF );
	dl_cmd_sync( &cs );
	send( handle, &cs );

	sleep(1);

	//dl_dumpmem(handle,"dump1.log");

	// fill with a bunch of red
	printf("filling screen with red gradient..\n");
	dl_rle_word red = { 0x00, 0x0000 };
	for (int i = 0; i < YRES; i++) {
		dl_gfx_rle( &cs, i*XRES*2,      0x00, &red );
		dl_gfx_rle( &cs, i*XRES*2+512,  0x00, &red );
		dl_gfx_rle( &cs, i*XRES*2+1024, 0x00, &red );
		red.value = (i/15) << 11;
	}
	dl_cmd_sync( &cs );
	send( handle, &cs );

	sleep(1);

	// grr. I'm pretty sure that I'm right about the stride register,
	// but I can't get it to have any effect...
	/*dl_set_register( &cs, DL_REG_SYNC, 0x00 );
	dl_set_address(  &cs, DL_ADDR_FB16_STRIDE, X*2 );
	dl_set_address(  &cs, DL_ADDR_FB8_STRIDE, X );
	dl_set_register( &cs, DL_REG_SYNC, 0xFF );
	dl_sync_command( &cs );
	send( &cs, handle );*/

	if (argc >= 2) {

		// fill with an image
		printf("filling screen with an image..\n");
		#define X 640
		#define Y 480 //960

		uint8_t* image = read_rgb16(argv[1],X*Y);
		uint16_t* img16 = (uint16_t*)image;
		
		int myaddr = 0;
		int mypcnt = 0;
		int imgsize = X*Y;

		dl_reg_set( &cs, DL_REG_SYNC, 0xFF );
		while (mypcnt < imgsize) {
			int bs = (mypcnt == 0 ? DL_HUFFMAN_BLOCKSIZE-3 : DL_HUFFMAN_BLOCKSIZE );
			int res = dl_huffman_compress( &cs, myaddr, imgsize-mypcnt, img16+mypcnt, bs );
			mypcnt += res;
			myaddr += res*2;
		}

		FILE* foo = fopen( "out.bin", "w+" );
		fwrite(cs.buffer,cs.pos,1,foo);
		fclose(foo);

		/*for (int i = 0; i < (X*Y)/256; i++) {
			dl_gfx_write( &cs, i*256*2, 0x00, image+(i*256*2) );
			if ((i % 100) == 0) {
				dl_sync_command( &cs );
				send( &cs, handle );
			}
		}*/
		printf( "encoded %d bytes\n",cs.pos );
		dl_cmd_sync( &cs );
		send( handle, &cs );

		sleep(1);
	}

	// some vertical scrolling
	printf("doing vertical scrolling (why doesn't horizontal work?)..\n");
	for (int i = 0; i < YRES; i++) {
		dl_reg_set( &cs, DL_REG_SYNC, 0x00 );
		dl_reg_set_address(  &cs, DL_ADDR_FB16_START, i*XRES*2 );
		dl_reg_set( &cs, DL_REG_SYNC, 0xFF );
		dl_cmd_sync( &cs );
		send( handle, &cs );
		usleep(5000);
	}

	//dl_dumpmem(handle,"dump2.log");

	sleep(1);

	// some memcopy
	printf("doing bitblt..\n\n");
	dl_reg_set_offsets( &cs, 0x000000, 0x000A00, 0x555555, 0x000500 );
	for (int i = 0; i < 100; i++) {
		dl_gfx_copy( &cs, 0x500*(280+i)+320*2, 0x500*(380+i)+420*2, 100 );
	}
	dl_cmd_sync( &cs );
	send( handle, &cs );

	printf("goodbye.\n\n");
	usb_close( handle );
	destroy( &cs );
}

