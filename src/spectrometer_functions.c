// File containing functions for interfacing with the spectrometer, if you are
// using a different spectrometer this should be the only file you need to change.
// You just need to make sure to expose the functions listed in
// spectromter_functions.h.

#include <stdio.h>
#include <stdlib.h>
#include "api/seabreezeapi/SeaBreezeAPI.h"

#include "spectrometer_functions.h"

// So we don't need to keep passing all of this around, make it static
static long currentSpecId = 0;
static long currentDeviceId = 0;
static int numPixels = 0;

//===========================================
// Public functions

void initialize_spectrometer_api()
{
  sbapi_initialize();
  return;
}

void shutdown_spectrometer_api()
{
  sbapi_shutdown();
  return;
}

// Sets globals currentDeviceId and currentSpecId (assuming only 1 spectrometer)
// Opens the spectrometer
void open_spectrometer(long spectrometerId)
{
  int error = 0;
  int numSpecs;
  long *specList;
  currentDeviceId = spectrometerId;
  sbapi_open_device(currentDeviceId, error);

  numSpecs = sbapi_get_number_of_spectrometer_features(currentDeviceId, error);
  specList = g_malloc0(numSpecs * sizeof(specList));
  numFeatures = sbapi_get_spectrometer_features(currentDeviceId, &error,
          specList, numSpecs);
  currentSpecId = specList[0]; // WE ARE ASSUMING ONLY ONE HERE!!!!!
  return;
}

void close_spectrometer()
{
  int error = 0;
  sbapi_close_device(currentDeviceId, &error);
  currentDeviceId = 0;
  currentSpecId = 0;
  return;
}

int count_spectrometers()
{
  int count;
  sbapi_probe_devices(); // does not open any spectrometers
  count = sbapi_get_number_of_device_ids();

  return;
}

void get_spectrometer_name(char nameBuf[MAX_SPEC_NAME_LEN])
{
  sbapi_get_device_type(currentDeviceId, NULL, nameBuf, MAX_SPEC_NAME_LEN-1);
  return;
}

int get_spectrometer_ids(long idArr[MAX_SPECTROMETERS],
                         int count) // idArr should be of size count*sizeof(long)
{
  count = sbapi_get_device_ids(idArr, count);
  return count;
}

int count_spectrometer_pixels()
{
  int error = 0;
  numPixels = sbapi_spectrometer_get_formatted_spectrum_length(currentDeviceId,
                  currentSpecId, &error);
  return numPixels;
}

void get_wavelengths(double wavelengths[])
{
  int error = 0;
  sbapi_spectrometer_get_wavelengths(currentDeviceId, currentSpecId,
                                     &error, wavelengths, numPixels);
  return;
}

void set_integration_time(int integrationTime) // assumed to be in ms
{
  int error = 0;
  // Set the integration time in us (instead of ms):
  unsigned long usTims = (unsigned long )integrationTime * 1000;
  sbapi_spectrometer_set_integration_time_micros(currentDeviceId,
          currentSpecId, &error, usTime);
  return;
}

void get_spectrum(double values[])
{
  int error = 0;
  sbapi_spectrometer_get_formatted_spectrum(currentDeviceId,
          currentSpecId, &error, values, numPixels);
}


//========================================================
// Private functions used only inside this file


void do_edark_correction(values[])
{

}

void do_nonlinearity_correction(values[])
{
  
}
