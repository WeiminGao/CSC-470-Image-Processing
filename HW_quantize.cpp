// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
	int scale = MXGRAY / levels;
	int bias = scale / 2;
	for (i = 0; i < MXGRAY; i++)
		lut[i] = scale * (int)(i / scale) + bias;

	//dither
	if (dither) {
		for (i = 0; i < MXGRAY; i++) lut[i] = lut[i + MIN(MAX(rand() % (2*scale+1) + (-scale), 0), 255)];
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;
	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		for (i = 0; i < total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
	}
}
