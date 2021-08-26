/**
    Copyright (c) 2021 Ben Cerjan
    This file is part of ss-Raman-GUI.
    ss-Raman-GUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    ss-Raman-GUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
    You should have received a copy of the GNU Affero General Public License
    along with ss-Raman-GUI.  If not, see <https://www.gnu.org/licenses/>.
**/

// File containing functions for interfacing with the spectrometer, if you are
// using a different spectrometer this should be the only file you need to change.
// You just need to make sure to expose the functions listed in
// spectromter_functions.h.

#include "gtk/gtk.h"
#include <stdio.h>
#include <stdlib.h>
#include "api/seabreezeapi/SeaBreezeAPI.h"

#include "spectrometer_functions.h"

// So we don't need to keep passing all of this around, make it static
static long currentSpecId = 0;
static long currentDeviceId = 0;
static long currentBufferId = 0;
static int numPixels = 0;

// These values are for electronic dark and nonlinearity correction:
#define MAX_DARK_PIXELS 32
#define DARK_PIXEL_BUF_SIZE 40 // Number of recent dark pixel results to store / average
#define MAX_NL_COEFFS 10
static int dark_pixel_count = 0;
static int dark_pixels[MAX_DARK_PIXELS];
static int dark_pixel_buf_pos = 0;
static int dark_pixel_buf_full = 0;
static double dark_pixel_ring_buf[DARK_PIXEL_BUF_SIZE] = {0.0};
static int num_nl_coeffs = 0;
static double nl_coeffs[MAX_NL_COEFFS] = {0.0};

void set_nl_coeff_data();
void set_edark_pixel_data();
void do_edark_correction(double values[]);
void do_nonlinearity_correction(double values[]);

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
  if (spectrometerId != currentDeviceId) { // only if this is a new spectrometer
    int error = 0;
    if (currentDeviceId != 0) { // close the previous one if this isn't initial startup
      close_spectrometer();
    }
  g_print("Opening device...\n");
    int numSpecs;
    long *specList;
    currentDeviceId = spectrometerId;
    sbapi_open_device(currentDeviceId, &error);

    numSpecs = sbapi_get_number_of_spectrometer_features(currentDeviceId, &error);
    specList = g_malloc0(numSpecs * sizeof(specList));
    numSpecs = sbapi_get_spectrometer_features(currentDeviceId, &error,
            specList, numSpecs);
    currentSpecId = specList[0]; // WE ARE ASSUMING ONLY ONE HERE!!!!!

    // We also assume only one data buffer:
    int number_of_data_buffers;
    long *data_buffer_ids;
    number_of_data_buffers = sbapi_get_number_of_data_buffer_features(currentDeviceId, &error);

    data_buffer_ids = g_malloc0(number_of_data_buffers*sizeof(data_buffer_ids));
    number_of_data_buffers = sbapi_get_data_buffer_features(currentDeviceId, &error,
            data_buffer_ids, number_of_data_buffers);
    currentBufferId = data_buffer_ids[0];

    set_edark_pixel_data();
    set_nl_coeff_data();
    g_free(specList);
    g_free(data_buffer_ids);
  }

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

  return count;
}

void get_spectrometer_name(long deviceId, char nameBuf[MAX_SPEC_NAME_LEN])
{
  sbapi_get_device_type(deviceId, NULL, nameBuf, MAX_SPEC_NAME_LEN-1);
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
  unsigned long usTime = (unsigned long )integrationTime * 1000;

  sbapi_spectrometer_set_integration_time_micros(currentDeviceId,
        currentSpecId, &error, usTime);

  return;
}

void clear_spectrometer_buffer()
{
  int error = 0;
  sbapi_data_buffer_clear(currentDeviceId, currentBufferId, &error);

  return;
}

int get_spectrum(double values[])
{
  int error = 0;
  int count = 0;
  count = sbapi_spectrometer_get_formatted_spectrum(currentDeviceId,
          currentSpecId, &error, values, numPixels);

  do_edark_correction(values);
  do_nonlinearity_correction(values);
  return count; // Return actual number of pixels in spectrum, though this is unused
}


//========================================================
// Private functions used only inside this file

void set_edark_pixel_data()
{
  int error = 0;
  // Get number of dark pixels
  dark_pixel_count = sbapi_spectrometer_get_electric_dark_pixel_count(currentDeviceId,
      currentSpecId, &error);
  // Fill dark_pixels with the indices of the dark pixels
  dark_pixel_count = sbapi_spectrometer_get_electric_dark_pixel_indices(currentDeviceId,
          currentSpecId, &error, dark_pixels, dark_pixel_count);
}

void set_nl_coeff_data()
{
  int error = 0;
  int num_nl_features;
  long *nl_feature_ids = 0;

  // Find how many detectors support NL correction (WE ASSUME 1!!!)
  num_nl_features = sbapi_get_number_of_nonlinearity_coeffs_features
                                (currentDeviceId, &error);
  nl_feature_ids = g_malloc0(num_nl_features*sizeof(nl_feature_ids));
  // Set the NL feature IDs
  num_nl_features = sbapi_get_nonlinearity_coeffs_features(currentDeviceId,
                      &error, nl_feature_ids, num_nl_features);
  // Now get the NL coefficients
  num_nl_coeffs = sbapi_nonlinearity_coeffs_get(currentDeviceId,
                    nl_feature_ids[0], &error, nl_coeffs, MAX_NL_COEFFS);
  g_free(nl_feature_ids);
  return;
}

// In-Place operation on pixel intensity values to account for electronic noise
void do_edark_correction(double values[])
{
  int i, maxBufPos;
  double baseline = 0.0;

  // Add new values to our ring buffer:
  for (i = 0; i < dark_pixel_count; i++) {
    dark_pixel_ring_buf[dark_pixel_buf_pos] = values[dark_pixels[i]];
    dark_pixel_buf_pos++;
    if (dark_pixel_buf_pos == DARK_PIXEL_BUF_SIZE) {
      // Reset our buffer and note that we have now filled it:
      dark_pixel_buf_pos = 0;
      dark_pixel_buf_full = 1;
    }
  }

  // Average over the buffer:
  if(dark_pixel_buf_full == 1) {
    maxBufPos = DARK_PIXEL_BUF_SIZE;
  } else {
    maxBufPos = dark_pixel_buf_pos;
  }
  // Build the baseline:
  for (i = 0; i < maxBufPos; i++) {
    baseline += dark_pixel_ring_buf[i];
  }

  baseline /= (double )maxBufPos; // average

  // Now correct our values:
  for (i = 0; i < numPixels; i++) {
    values[i] -= baseline;
  }

  return;
}


// In-place nonlinearity correction for the pixel values
void do_nonlinearity_correction(double values[])
{
  int i,j;
  double xpower,y;

  for (i = 0; i < numPixels; i++) {
    // N-th order polynomial correction based on how many terms we have:
    xpower = values[i];
    y = nl_coeffs[0];

    for (j = 1; j < num_nl_coeffs; j++) { // starts at 1 as we pre-load the 0 term above
      y += xpower*nl_coeffs[j];
      xpower *= values[i];
    }

    values[i] /= y;
  }

  return;
}
