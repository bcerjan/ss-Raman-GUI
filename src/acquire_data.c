/*

File to handle main data acquisition tasks -- e.g. acquire raw spectrum, FFT
to time domain, perform convolution, IFFT back to frequency, and output data
as needed (automatically naming files for multiple runs)

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>


// Data output header file, contains struct definition
#include "data_output.h"
#include "acquire_data.h"
#include "fft_functions.h"
#include "waveform_gen.h"
#include "spectrometer_functions.h"
#include "measurement_params.h"

// PN code header:
#include "pn_code.h"


int timeoutLoops = 1; // global to track how many loops we've done for the progress bar


// Function to update the progress bar to a given fraction of fullness:
void update_progressBar(GtkWidget *progressBar,
                        double fraction)
{
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), fraction);
}

int complete_progressBar(gpointer data)
{
g_print("Inside complete_progressBar...\n");
  struct dataAcqParams *params = data;
  GtkWidget *progressBar;
  progressBar = params->progressBar;
  g_source_remove(params->timeoutID); // This turns off our progress bar updates
  update_progressBar(progressBar, 1.0);
  const char *text = "Start Scan";
  gtk_button_set_label(GTK_BUTTON(params->scan_btn), text);
  timeoutLoops = 1;

  g_free(params->outputPtr);
  g_free(params);
  return G_SOURCE_REMOVE;
}

int progressBar_timeout_cb(gpointer data)
{
  // First, get approximate total time we'll need:
  struct dataAcqParams *params = data;
//printf("PBar Pointer timeout: %p", params->progressBar);
//g_print("\n");
  int integrationTime = params->integrationTime;
  int measurement_reps = params->measurement_reps;
  int timeoutInterval = params->timeoutInterval;
  int timeElapsed = timeoutLoops*timeoutInterval;

  GtkWidget *progressBar;
  progressBar = params->progressBar;

  int timePadding = integrationTime; // The first result can come back from the
  // spectrometer at most 2 integrationTimes late -- 1 for a calibration and the
  // second for the real data
  int totalTime = (integrationTime * measurement_reps) + timePadding;

  double fraction = (double ) timeElapsed / (double ) totalTime;
  update_progressBar(progressBar, fraction);

  timeoutLoops++;

  return G_SOURCE_CONTINUE; // We keep calling this until we cancel it...
}


// We don't really use this to avoid race condition
void free_data_acq_data(void *data)
{
  timeoutLoops = 1; // Reset our loop counter for next run
  return;
}

int data_acq(struct dataAcqParams *data)
{
  struct dataAcqParams *params = data;
  int i, numPixels;
  int integrationTime = params->integrationTime;
  int measurement_reps = params->measurement_reps;
  long spectrometerId = params->spectrometerId;
  int mod_freq = params->mod_freq;
  int pn_bit_len = params->pn_bit_length;

  double speedC = 2.99792458e17; // In nm/sec
  double *wavelengths, *frequencies, *values;

  // Set up initial data from the spectrometer:
  open_spectrometer(spectrometerId); // this prepares static data as well for calibration(s)
  numPixels = count_spectrometer_pixels(); // Find out how big our data set will be

  wavelengths = g_malloc0(numPixels * sizeof(wavelengths));
  frequencies = g_malloc0(numPixels * sizeof(frequencies));
  values = g_malloc0(numPixels * sizeof(values));

  get_wavelengths(wavelengths);

  // Convert from nm to frequency shift (in Hz)
  for (i = 0; i < numPixels; i++) {
    // Assuming wavlengths are in nm:
    //frequencies[i] = speedC / wavelengths[numPixels - 1 - i]; // Need to reorder to be from 0 -> MaxFreq instead of the opposite
    frequencies[i] = (speedC / LASER_WAVELENGTH) - (speedC / wavelengths[i]);
  }


  // Generate PN FFT data for multiplication (if needed)
  unsigned long int fft_length;
  double *pn_fft_freq, *pn_fft_pow,*pn_interp_fft;

  if (params->outputPtr->final_data) {
    fft_length = calc_fft_length(pn_bit_len);
    pn_fft_freq = g_malloc0(sizeof(*pn_fft_freq) * fft_length); // As long as our output
    pn_fft_pow = g_malloc0(sizeof(*pn_fft_pow) * fft_length); // ""
    pn_interp_fft = g_malloc0(sizeof(*pn_interp_fft) * numPixels); // Same number of elements as Pixels

    // Get PN bits:
    int pn_bits[1024] = {0};
    if (pn_bit_len == 32) {
      for (i = 0; i < pn_bit_len; i++) {
        pn_bits[i] = pn_32_bit[i];
      }
    } else if (pn_bit_len == 64) {
      for (i = 0; i < pn_bit_len; i++) {
        pn_bits[i] = pn_64_bit[i];
      }
    } else if (pn_bit_len == 128) {
      for (i = 0; i < pn_bit_len; i++) {
        pn_bits[i] = pn_128_bit[i];
      }
    } else if (pn_bit_len == 256) {
      for (i = 0; i < pn_bit_len; i++) {
        pn_bits[i] = pn_256_bit[i];
      }
    } else if (pn_bit_len == 512) {
      for (i = 0; i < pn_bit_len; i++) {
        pn_bits[i] = pn_512_bit[i];
      }
    } else if (pn_bit_len == 1024) {
      for (i = 0; i < pn_bit_len; i++) {
        pn_bits[i] = pn_1024_bit[i];
      }
    }

    // This generates the FFT of the PN code for the current bit length and
    // modulation frequency.
    generate_pn_fft(mod_freq, pn_bit_len, pn_bits, fft_length,
                    pn_fft_freq, pn_fft_pow);

    // This interpolates our FFT of the PN code to the same frequencies as the
    // data from the spectrometer. We do this here so it's only done once no
    // matter how many measurement repetitions we do.
    interpolate_fft_data(numPixels, frequencies, fft_length, pn_fft_freq,
                         pn_fft_pow, pn_interp_fft);

    g_free(pn_fft_freq);
    g_free(pn_fft_pow);
  } /* if for final data */

  // Set the integration time for the measurements:
  set_integration_time(integrationTime);
g_print("About to take spectra...\n");
  // Cycle for each measurement repetition:
  for (i = 0; i < measurement_reps; i++) {
    // Check if we've been cancelled:
    if (g_cancellable_is_cancelled(params->cancellable)) {
      g_source_remove(params->timeoutID); // Turn off update for progressbar
      set_integration_time(15);

      // Free data that stays in this function:
      g_free(pn_interp_fft);
      g_free(wavelengths);
      g_free(frequencies);
      g_free(values);

      g_free(params->outputPtr);
      g_free(params);

      return 1; // Memory freeing function called automatically
    } /* if cancelled */

    // Take data!
    get_spectrum(values); // This automatically applies corrections
                          // (e.g. dark pixels and nonlinearity)

    // Add checking for saturated pixels???

    // We're now ready to process / output our data (if requested):
    // Export raw data if requested:
    output_data(numPixels, frequencies, values, pn_interp_fft, i, params);

  } /* i for loop */

  if (params->outputPtr->final_data) {
    g_free(pn_interp_fft); // Free if we allocated it
  }

  // Set the integration time to something short so we're ready in case it
  // is adjusted for the next run:
  //wrapper.setIntegrationTime(spectrometerIndex, 10000); // set to 10 ms
  // If we reach here, we're done!
  gdk_threads_add_idle(complete_progressBar, params); // This also turns off the progress bar updates


  //free_data_acq_data(params);
  g_free(values);
  g_free(wavelengths);
  g_free(frequencies);
  stop_wvfm_gen();

  return 0;
}

static void data_acq_cb(GTask    *task,
                        gpointer source_object,
                        gpointer task_data,
                        GCancellable *cancellable)
{
  struct dataAcqParams *params = task_data;
  int retval;

  // Handle Cancellation:
  if(g_task_return_error_if_cancelled(task))
  {
    return;
  }
g_print("Inside data_acq_cb...\n");
  // Run the actual function:
  retval = data_acq(params);

  g_task_return_int(task, retval);
}

void start_data_acq_async(gpointer            data, // Input data
                          GCancellable       *cancellable,
                          GAsyncReadyCallback callback,
                          gpointer            user_data)
{
g_print("Starting task...\n");
  GTask *task = NULL;
  struct dataAcqParams *params;
  params = (struct dataAcqParams *) data;
//printf("Btn Pointer Async: %p\n", params->scan_btn);
  // Error if this is badly formatted:
  g_return_if_fail( (cancellable == NULL) | G_IS_CANCELLABLE(cancellable) );
g_print("Set as cancellable???\n");
  task = g_task_new(NULL, cancellable, callback, user_data);
  g_task_set_source_tag(task, start_data_acq_async);

  g_task_set_return_on_cancel(task, FALSE);

  // Prepare our owned data:
  //params = g_malloc(sizeof(*params));
  //*params = *data; // Copy to our local version

  g_task_set_task_data(task, params, free_data_acq_data);

  // Run the acquisition in a worker thread:
  g_task_run_in_thread(task, data_acq_cb);
g_print("done starting task...\n");
  g_object_unref(task);
}
