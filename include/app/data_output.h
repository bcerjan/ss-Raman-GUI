#ifndef DATA_OUTPUT
#define DATA_OUTPUT

#include "acquire_data.h"

// Note: check the state using (e.g.): if (dataCheckboxes->raw_data) {}
struct dataOutputOpts {
  int raw_data;
  int pn_fft_data;
  int final_data;
  const char *fname;
  const char *data_dir;
};

void output_data(int numPixels,
                 double wavelengths[],
                 double pixelValues[],
                 double pn_interp_fft[],
                 int iteration,
                 struct dataAcqParams *params);

#endif
