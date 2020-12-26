#ifndef DATA_OUTPUT
#define DATA_OUTPUT

// Note: check the state using: if (dataCheckboxes->raw_data) {}
struct dataOuputOpts {
  int raw_data;
  int fft_data;
  int conv_data;
  int final_data;
  char *fname;
  char *data_dir;
};

void output_data(char *data_dir, char *fname, struct dataCheckboxes *checkboxes);

#endif
