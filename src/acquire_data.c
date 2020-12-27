/*

File to handle main data acquisition tasks -- e.g. acquire raw spectrum, FFT
to time domain, perform convolution, IFFT back to frequency, and output data
as needed (automatically naming files for multiple runs)

*/

#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>


// Header files for Ocean Inisght Spectrometer
//#include "API INFO HERE"
//#include "ArrayTypes.h"

// Data output header file, contains struct definition
#include "data_output.h"
#include "acquire_data.h"


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

  return G_SOURCE_REMOVE;
}

int progressBar_timeout_cb(gpointer data)
{
  // First, get approximate total time we'll need:
  struct dataAcqParams *params = data;

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

void *start_data_acq(gpointer data)
{

  struct dataAcqParams *params = data;

  int i;
  int integrationTime = params->integrationTime;
  int measurement_reps = params->measurement_reps;
  GtkWidget *progressBar = params->progressBar;
  int spectrometerIndex = params->spectrometerIndex;

  //Wrapper wrapper = params->spectrometerWrapper; // Only create one of these, this might not be correct
  int numberOfPixels;
  //DoubleArray pixelArray;
  double *pixelValues, *wavelengths;

  // Cycle for each measurement repetition:
  for (i = 0; i < measurement_reps; i++) {
    // For testing, sleep for the requested integration time
    g_print("Performing a measurement...\n");
    g_usleep(integrationTime * 10e2); // in microseconds
    // Set integration time:
    /*
    wrapper.setIntegrationTime(spectrometerIndex, integrationTime*1000); // This is specified in microseconds, hence the *1000
    // Acquire spectrum:
    pixelArray = wrapper.getSpectrum(spectrometerIndex);
    // Get number of pixels:
    numberOfPixels = pixelArray.getLength();
    // Convert into an array of values
    pixelValues = pixelArray.getDoubleValues();
    wavelengths = wrapper.getWavelengths(spectrometerIndex); // Not totally sure this will work...
    */

    // We're now ready to output our data
  }

  // Set the integration time to something short so we're ready in case it
  // is adjusted for the next run:
  //wrapper.setIntegrationTime(spectrometerIndex, 10000); // set to 10 ms
  // If we reach here, we're done!
  gdk_threads_add_idle(complete_progressBar, params); // This also turns off the progress bar updates

  // Free the output options data:
  g_free(params->outputPtr);

  // Free the memory that called this run:
  g_free(params->self);

}

// Function to stop everything:
void *stop_data_acq(gpointer data)
{
  struct dataAcqParams *params = data;
  // Turn off bar updates
  g_source_remove(params->timeoutID);

  // Free struct for output:
  g_free(params->outputPtr);

  // And free the main struct:
  g_free(params->self);
}
