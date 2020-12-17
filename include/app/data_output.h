#ifndef DATA_OUTPUT
#define DATA_OUTPUT

#include <gtk/gtk.h>

// Note: check the state using if (dataCheckboxes->raw_data) {}
struct dataCheckboxes {
  gboolean raw_data;
  gboolean fft_data;
  gboolean conv_data;
  gboolean final_data;
};

void output_data(char *data_dir, char *fname, struct dataCheckboxes *checkboxes);

#endif
