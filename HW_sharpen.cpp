// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//

extern void HW_blur  (ImagePtr, int, int, ImagePtr);

void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
	IP_copyImageHeader(I1, I2);
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	//blurring orignal image
	HW_blur(I1, size, size, I2);

	ChannelPtr<uchar> p1, p2;
	int type;

	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
			IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
	}
	 for(int i=0; i<total; i++) {
		 //The difference between I1 and its blurred version is multiplied by factor and then added back to I1
		 //to yield the output image stored in I2.
		*p2 = MIN(MAX(factor * (*p1 - *p2) + *p1, 0), 255);
		*p1++;
		*p2++;
	}
  
}

