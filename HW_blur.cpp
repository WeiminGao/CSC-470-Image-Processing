// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	
	// Force kernel dimensions to be odd
	if (filterW % 2 == 0) filterW++;
	if (filterH % 2 == 0) filterH++;

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);
	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	int sum = 0;
	int i, j, x;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;
	// visit all image channels
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
	}
	
	//Constant filter (Static): filterW = 7
	/*
	for (j = 0; j < h; j++) {
		sum = p1[0 + (j*w)] + p1[1 + (j*w)] + p1[2 + (j*w)] + p1[3 + (j*w)] + p1[4 + (j*w)] + p1[5 + (j*w)] + p1[6 + (j*w)];
		for (i = 3; i < w - 3; i++) {
			p2[i + (j*w)] = sum / 7;
			sum += (p1[(i + 4) + (j*w)] - p1[(i - 3) + (j*w)]);
		}
	}
	*/

	//Dynamic filterW: using total
	/*
	for (i = 0; i < filterW; i++) sum += p1[i];
	for (i = filterW / 2; i < total - filterW / 2; i++) {
	p2[i] = sum / filterW;
	sum += (p1[(i + (filterW - filterW / 2))] - p1[(i - filterW / 2)]);
	}
	*/

	if (filterW > 2) {
		for (x = 0; x < h; x++) {

			
			int bufferW = w + filterW - 1;
			uchar* buffer = new uchar[bufferW];

			// Fill bufffer with padding (using pixel replication)
			int padding = (filterW - 1) / 2;
			int firstPixel = p1[0 + (x*w)];
			int lastPixel = p1[(w - 1) + (x*w)];
			for (i = 0; i < padding; i++) buffer[i] = firstPixel;
			for (i = (padding + w); i < bufferW; i++) buffer[i] = lastPixel;

			// Fill buffer with src pixels
			for (i = padding, j = 0; i < padding + w; i++, j += 1) buffer[i] = p1[j + (x*w)];

			sum = 0;
			for (i = 0; i < filterW; i++) sum += buffer[i];
			p2[0 + (x*w)] = sum / filterW;
			for (i = 1; i < w; i++) {
				sum += buffer[i + filterW - 1] - buffer[i - 1];
				p2[i + (x*w)] = sum / filterW;
			}
			delete[] buffer;
		}
	}
	else for (int ch = 0; ch < total; ch++)  IP_copyChannel(I1, ch, I2, ch);



	/* //Constant filter (Static): filterW = 7
	for (j = 0; j < w; j++) {
		sum = p1[0 + j] + p1[w + j] + p1[2 * w + j] + p1[3 * w + j] + p1[4 * w + j] + p1[5 * w + j] + p1[6 * w + j];
		for (i = 3; i < h - 3; i++) {
			p2[i * w + j] = MIN(MAX(sum / 7, 0), 255);
			sum += (p1[(i + 4)*w + j] - p1[(i - 3)*w + j]);
		}
	}
	*/
	
	/*  //Dynamic filterH:
	
	for (j = 0; j < w; j++) {
		
		sum = 0;
		for (i = 0; i < filterH; i++) sum += p1[i * w + j];
		for (i = filterH / 2; i < h - filterH / 2; i++) {
			p2[i * w + j] = sum / filterH;
			sum += (p1[(i + (filterH - filterH / 2))*w + j] - p1[(i - filterH / 2)*w + j]);
		}
	}
*/
	if (filterH > 2) {
		for (x = 0; x < w; x++) {

			int i, j;
			int bufferH = h + filterH - 1;
			uchar* buffer = new uchar[bufferH];

			// Fill bufffer with padding (using pixel replication)
			int padding = (filterH - 1) / 2;
			int firstPixel = p2[0 * w + x];
			int lastPixel = p2[(h - 1) * w + x];
			for (i = 0; i < padding; i++) buffer[i] = firstPixel;
			for (i = (padding + h); i < bufferH; i++) buffer[i] = lastPixel;

			// Fill buffer with src pixels
			for (i = padding, j = 0; i < padding + h; i++, j += 1) buffer[i] = p2[j * w + x];

			sum = 0;
			for (i = 0; i < filterH; i++) sum += buffer[i];
			p2[0 * w + x] = sum / filterH;
			for (i = 1; i < w; i++) {
				sum += buffer[i + filterH - 1] - buffer[i - 1];
				p2[i * w + x] = sum / filterH;
			}
			delete[] buffer;
		}
	}
	
}


