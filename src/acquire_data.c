/*

File to handle main data acquisition tasks -- e.g. acquire raw spectrum, FFT
to time domain, perform convolution, IFFT back to frequency, and output data
as needed (automatically naming files for multiple runs)

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>


// Header files for Ocean Inisght Spectrometer
//#include "API INFO HERE"
//#include "ArrayTypes.h"

// FFTW header file:
//#include "fftw3.h"

// Data output header file, contains struct definition
#include "data_output.h"
#include "acquire_data.h"

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
  //const char *text = "Start Scan";
  //gtk_button_set_label(params->btn, text);
  timeoutLoops = 1;
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

void free_data_acq_data(void *data)
{
  struct dataAcqParams *params = data;
  g_free(params->outputPtr);
  g_free(params);
  timeoutLoops = 1; // Reset our loop counter for next run
}

int data_acq(struct dataAcqParams *data)
{
  struct dataAcqParams *params = data;
  int i,j;
  int integrationTime = params->integrationTime;
  int measurement_reps = params->measurement_reps;
  int mod_freq = params->mod_freq;
  int pn_bit_len = params->pn_bit_length;
  GtkWidget *progressBar = params->progressBar;
  int spectrometerIndex = params->spectrometerIndex;

  //double speedC = 2.99792458e17; // In nm/sec

  // Set up initial data from the spectrometer:
  //Wrapper *wrapper = params->spectrometerWrapper; // Maybe not correct...
  //int numberOfPixels;
  //double *wavelengths, *frequencies;
  //DoubleArray pixelArray; // Storage for data from the spectrometer

  // Get wavelengths that our data will use:
  //pixelArray = wrapper.getSpectrum(spectrometerIndex); // This should be very fast as integration time is 10 ms, and we don't care about the actual data
  //wavelengths = wrapper.getWavelengths(spectrometerIndex); // (nm????) Not totally sure this will work...
  // need to get number of wavelengths here somehow...
  // numberOfPixels = pixelArray.getLength(); // Would be better if we could get length from wavelengths variable rather than running a scan like this...

  // Convert from nm to Hz (assuming it's in nm...)
  /*for (i = 0; i < numberOfPixels; i++) {
    // Assuming wavlenghts are in nm for the moment:
    frequencies[i] = speedC / wavelengths[i];
  }*/

  // Find highest frequency we care about and convert it to a sampling rate:
  //double maxFreq = frequencies[0]; // Or maybe frequencies[numberOfPixels-1], depends on ordering
  //double sampFreq = 2.01*maxFreq; // How fast we need to sample in time to get an FFT result that is meaningful (the extra 0.01 is for leeway...)


  // Cycle for each measurement repetition:
  for (i = 0; i < measurement_reps; i++) {
    // Check if we've been cancelled:
    if (g_cancellable_is_cancelled(params->cancellable)) {
      g_source_remove(params->timeoutID); // Turn off update for progressbar
      //wrapper.setIntegrationTime(spectrometerIndex, 10000); // Set integration time to 10 ms again

      // Free data that stays in this function:
      //g_free(pn_fft);
      //g_free(freq_fft);
      return 1; // Memory freeing function called automatically
    }

    // For testing, sleep for the requested integration time
    g_print("Performing a measurement...\n");
    g_usleep(integrationTime * 10e2); // in microseconds
    // Set integration time:
    /*
    wrapper.setIntegrationTime(spectrometerIndex, integrationTime*1000); // This is specified in microseconds, hence the *1000
    // Acquire spectrum:
    pixelArray = wrapper.getSpectrum(spectrometerIndex);

    // Convert into an array of values
    pixelValues = pixelArray.getDoubleValues();

    */

    // Add checking for saturated pixels???

    // We're now ready to process / output our data (if requested):
    // Export raw data if requested:
    //output_data_raw(wavelengths, pixelValues);

    // Then we need to convert from wavelength to frequency (for FFT interpretation):



  }

  // Set the integration time to something short so we're ready in case it
  // is adjusted for the next run:
  //wrapper.setIntegrationTime(spectrometerIndex, 10000); // set to 10 ms
  // If we reach here, we're done!
  gdk_threads_add_idle(complete_progressBar, params); // This also turns off the progress bar updates

  //free_data_acq_data(params);
  //g_free(pixelValues);
  //g_free(wavelengths);
  //g_free(frequencies);


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
printf("PBar Pointer Async: %p\n", params->progressBar);
  // Error if this is badly formatted:
  g_return_if_fail(cancellable == NULL | G_IS_CANCELLABLE(cancellable));
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
