#ifndef DATA_OUTPUT
#define DATA_OUTPUT

#include "acquire_data.h"

// Note: check the state using (e.g.): if (dataCheckboxes->raw_data) {}
struct dataOuputOpts {
  int raw_data;
  int fft_data;
  int conv_data;
  int final_data;
  const char *fname;
  const char *data_dir;
};

void output_data(int numPixels, double wavelengths[], double pixelValues[],
                  int iteration, struct dataAcqParams *params);

#endif
