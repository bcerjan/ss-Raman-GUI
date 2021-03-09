#ifndef SPECTROMTER_FUNCS
#define SPECTROMETER_FUNCS

#define MAX_SPECTROMETERS 10 // maximum number of spectrometers you might connect at once
#define MAX_SPEC_NAME_LEN 80 // number of characters in longest name allowed

void initialize_spectrometer_api();
int count_spectrometers();
int get_spectrometer_ids(long idArr[MAX_SPECTROMETERS],
                         int count);
void get_spectrometer_name(long deviceId, char nameBuf[MAX_SPEC_NAME_LEN]);
void shutdown_spectrometer_api();
void open_spectrometer(long spectrometerId);
void close_spectrometer();
void set_integration_time(int integrationTime);
void clear_spectrometer_buffer();
int get_spectrum(double values[]);
int count_spectrometer_pixels();
void get_wavelengths(double wavelengths[]);

#endif
