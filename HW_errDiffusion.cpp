#include <cstdlib>  // for malloc

void memoryFailure();
short *initializeErrorLUT(int, int);
void gammaCorrect(ImagePtr, double, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//
void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{
  // Apply gamma correction to input image
  ImagePtr Itmp;
  gammaCorrect(I1, gamma, Itmp);

  IP_copyImageHeader(I1, I2);
  int w = I1->width ();
  int h = I1->height();

  int ch, type, x, y;
  ChannelPtr<uchar> src, dst;

  // Buffer variables; some variables depend on the method.
  int numOfRows = method + 2, rowPadding = method + 1, bufferIndex = 0;
  int rowWidthPlusPadding = w + (2 * rowPadding);
  void *ptr;
  short *buffer, *in1, *in2, *in3;

  // Error calculation variables
  short error, *errorLUT, *lutTmpPtr;
  int threshold = MXGRAY / 2;

  // General variables
  int i, j, direction;

  errorLUT = initializeErrorLUT(method, threshold);

  // Allocate buffer
  ptr = std::malloc(sizeof(short) * numOfRows * rowWidthPlusPadding);
  if (ptr == NULL) memoryFailure();
  buffer = (short *)ptr;

  // For each channel
  for(ch = 0; IP_getChannel(Itmp, ch, src, type); ch++) {
    IP_getChannel(I2, ch, dst, type);

    // Insert row into buffer
    for(i = 0, j = rowPadding; i < w; i++, j++) buffer[((bufferIndex % numOfRows) * rowWidthPlusPadding) + j] = src[i];
    bufferIndex++;
    src += w;

    if (method) {
      // Insert another row into buffer
      for(i = 0, j = rowPadding; i < w; i++, j++) buffer[((bufferIndex % numOfRows) * rowWidthPlusPadding) + j] = src[i];
      bufferIndex++;
      src += w;
    }

    for(y = 0; y < h; y++) {
      // Insert row into buffer
      for(i = 0, j = rowPadding; i < w; i++, j++) buffer[((bufferIndex % numOfRows) * rowWidthPlusPadding) + j] = src[i];
      bufferIndex++;
      src += w;

      /* Set pixel traversal direction;
      1 means left-to-right. -1 means right-to-left.
      */
      direction = (serpentine && (y % 2)) ? -1 : 1;

      // Access rows in buffer
      if (direction == -1) {
        in1 = buffer + ((bufferIndex       % numOfRows) * rowWidthPlusPadding) + rowPadding + w - 1;
        in2 = buffer + (((bufferIndex + 1) % numOfRows) * rowWidthPlusPadding) + rowPadding + w - 1;
        if (method) {
          in3 = buffer + (((bufferIndex + 2) % numOfRows) * rowWidthPlusPadding) + rowPadding + w - 1;
        }
      }
      else {
        in1 = buffer + ((bufferIndex       % numOfRows) * rowWidthPlusPadding) + rowPadding;
        in2 = buffer + (((bufferIndex + 1) % numOfRows) * rowWidthPlusPadding) + rowPadding;
        if (method) {
          in3 = buffer + (((bufferIndex + 2) % numOfRows) * rowWidthPlusPadding) + rowPadding;
        }
      }

      for(x = 0; x < w; x++) {
        // Apply threshold
        *dst = (*in1 < threshold) ? 0 : MaxGray;

        // Compute error produced by thresholding
        error = *in1 - *dst;

        /* Spread error around to neightboring pixels;
        Indexes into error LUT.
        */
        if (!method) {
          lutTmpPtr = errorLUT + 4 * error;
          in1[ 1 * direction ] += *(lutTmpPtr    );
          in2[-1 * direction ] += *(lutTmpPtr + 1);
          in2[ 0 * direction ] += *(lutTmpPtr + 2);
          in2[ 1 * direction ] += *(lutTmpPtr + 3);
        }
        else {
          lutTmpPtr = errorLUT + 12*error;
          in1[ 1 * direction ] += *(lutTmpPtr    );
          in1[ 2 * direction ] += *(lutTmpPtr +  1);
          in2[-2 * direction ] += *(lutTmpPtr +  2);
          in2[-1 * direction ] += *(lutTmpPtr +  3);
          in2[ 0 * direction ] += *(lutTmpPtr +  4);
          in2[ 1 * direction ] += *(lutTmpPtr +  5);
          in2[ 2 * direction ] += *(lutTmpPtr +  6);
          in3[-2 * direction ] += *(lutTmpPtr +  7);
          in3[-1 * direction ] += *(lutTmpPtr +  8);
          in3[ 0 * direction ] += *(lutTmpPtr +  9);
          in3[ 1 * direction ] += *(lutTmpPtr + 10);
          in3[ 2 * direction ] += *(lutTmpPtr + 11);
        }

        // Advance pointers
        if (direction == -1) {
          in1--;
          in2--;
          if (method) in3--;
          dst--;
        }
        else {
          in1++;
          in2++;
          if (method) in3++;
          dst++;
        }
      } // end x loop

      if      (serpentine && (direction == -1)) dst += (w + 1);
      else if (serpentine && (direction ==  1)) dst += (w - 1);
    } // end y loop
  } // end channel loop

  // Deallocate buffer
  free(buffer);

  // Deallocate error LUT
  switch(method) {
    case 0: errorLUT -= 4*threshold; break;
    case 1: errorLUT -= 12*threshold; break;
  }
  free(errorLUT);
}// end HW_errDiffusion()


short *
initializeErrorLUT(int method, int threshold)
{
  void *ptr;
  short *lutTmpPtr, *errorLUT, i;

  switch(method) {
  // Floyd-Steinberg method
  case 0:
    // Allocate error LUT
    ptr = std::malloc(sizeof(short) * MXGRAY * 4);
    if (ptr == NULL) memoryFailure();
    errorLUT = (short *)ptr;

    // Fill error LUT
    for(i = -threshold, lutTmpPtr = errorLUT; i < threshold; i++, lutTmpPtr += 4) {
      *lutTmpPtr       = i * (7/16.);
      *(lutTmpPtr + 1) = i * (3/16.);
      *(lutTmpPtr + 2) = i * (5/16.);
      *(lutTmpPtr + 3) = i * (1/16.);
    }

    // Shift pointer to middle of LUT
    errorLUT += 4*threshold;
    break;

  // Jarvis-Judice-Ninke method
  case 1:
    // Allocate error LUT
    ptr = std::malloc(sizeof(short) * MXGRAY * 12);
    if (ptr == NULL) memoryFailure();
    errorLUT = (short *)ptr;

    // Fill error LUT
    for(i = -threshold, lutTmpPtr = errorLUT; i < threshold; i++, lutTmpPtr += 12) {
      *lutTmpPtr        = i * (7/48.);
      *(lutTmpPtr +  1) = i * (5/48.);
      *(lutTmpPtr +  2) = i * (3/48.);
      *(lutTmpPtr +  3) = i * (5/48.);
      *(lutTmpPtr +  4) = i * (7/48.);
      *(lutTmpPtr +  5) = i * (5/48.);
      *(lutTmpPtr +  6) = i * (3/48.);
      *(lutTmpPtr +  7) = i * (1/48.);
      *(lutTmpPtr +  8) = i * (3/48.);
      *(lutTmpPtr +  9) = i * (5/48.);
      *(lutTmpPtr + 10) = i * (3/48.);
      *(lutTmpPtr + 11) = i * (1/48.);
    }

    // Shift pointer to middle of LUT
    errorLUT += 12*threshold;
    break;
  } // end error LUT switch
  return errorLUT;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// gammaCorrect:
//
// Apply gamma correction to image I1.
// Save result in I2.
//
void
gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{
  IP_copyImageHeader(I1, I2);
  int w = I1->width ();
  int h = I1->height();
  int total = w * h;

  // init lookup table
  int i, lut[MXGRAY];
  double exponent = 1 / gamma;
  double tmp;
  for(i=0; i<MXGRAY; ++i) {
    /* Put current intensity value in range [0, 1], then raise value to
    the exponent, then scale the value to [0, 255]. */
    tmp = pow((double)i / MaxGray, exponent) * MaxGray;
    lut[i] = tmp;
  }

  int type;
  ChannelPtr<uchar> p1, p2, endd;

  // for each channel
  for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
    IP_getChannel(I2, ch, p2, type);

    // evaluate output: each input pixel indexes into lut[] to eval output
    for(endd = p1 + total; p1<endd;) *p2++ = lut[*p1++];
  }
}


// -------------------------------
// --- Memory Failure function ---
// -------------------------------
void
memoryFailure() {
  printf("Malloc failed\n");
  exit(1);
}
