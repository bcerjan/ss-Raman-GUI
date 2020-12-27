/*

File to handle main data acquisition tasks -- e.g. acquire raw spectrum, FFT
to time domain, perform convolution, IFFT back to frequency, and output data
as needed (automatically naming files for multiple runs)

*/

#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

// Include threading support:
//#include <pthread.h>

// Header files for Ocean Inisght Spectrometer
//#include "API INFO HERE"
//#include "ArrayTypes.h"

// Data output header file, contains struct definition
#include "data_output.h"
#include "acquire_data.h"

/*struct progressBar_data {
  int integrationTime;
  int measurement_reps;
  int timeoutLoops;
  int timeoutInterval;
  GtkWidget* progressBar;
};*/

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
  //data = (struct dataAcqParams *) threadarg;
  GtkWidget *progressBar;
  progressBar = params->progressBar;
  g_source_remove(params->timeoutID); // This turns off our progress bar updates
  update_progressBar(progressBar, 1.0);

  return G_SOURCE_REMOVE;
}

int progressBar_timeout_cb(gpointer data)
{
  g_print("Inside timeout zone...\n");
  // First, get approximate total time we'll need:
  struct dataAcqParams *params = data;
  //params = (struct dataAcqParams *) threadarg;
  int integrationTime = params->integrationTime;
  int measurement_reps = params->measurement_reps;
  int timeoutInterval = params->timeoutInterval;
  //int timeElapsed = (data->timeoutLoops)*timeoutInterval;
  int timeElapsed = timeoutLoops*timeoutInterval;

  GtkWidget *progressBar;
  progressBar = params->progressBar;
printf("Integration Time: %d\n", integrationTime);
printf("Timeout pointer location: %p", (void *) progressBar);
g_print("\n");
  int timePadding = integrationTime; // The first result can come back from the
  // spectrometer at most 2 integrationTimes late -- 1 for a calibration and the
  // second for the real data
  int totalTime = (integrationTime * measurement_reps) + timePadding;

  double fraction = (double ) timeElapsed / (double ) totalTime;
  //double fraction = 0.66;
  update_progressBar(progressBar, fraction);

  //data->timeoutLoops++; // is this allowed?
  timeoutLoops++;

  //update_progressBar(GTK_WIDGET(threadarg), 0.5);

  return G_SOURCE_CONTINUE; // We keep calling this until we cancel it...
}

void *start_data_acq(gpointer data)
{
  g_print("Inside the thread...\n");
  struct dataAcqParams *params = data;
  //params = (struct dataAcqParams *) threadarg;

  int i;
  int integrationTime = params->integrationTime;
  int measurement_reps = params->measurement_reps;
  GtkWidget *progressBar = params->progressBar;
  int spectrometerIndex = params->spectrometerIndex;

  // Start progress bar timeout function:
  /*int timeoutInterval = 100; // in ms
  int timeoutID;
  struct progressBar_data progressBar_d;
  progressBar_d.integrationTime = integrationTime;
  progressBar_d.measurement_reps = measurement_reps;
  progressBar_d.progressBar = progressBar;
  progressBar_d.timeoutInterval = timeoutInterval;
  progressBar_d.timeoutLoops = 0;*/
  //pthread_create(&progressBar_tid, NULL, progressBar_update_loop,
  //               (void *) &progressBar_d);
  //timeoutID = gdk_threads_add_timeout(timeoutInterval, progressBar_timeout_cb,
  //                                    &progressBar_d);
  /*gdk_threads_add_idle(progressBar_timeout_cb,
                                    progressBar);*/
g_usleep(1e6);
  printf("integrationTime: %d\n", integrationTime);
printf("Thread pointer location: %p", (void *) progressBar);
g_print("\n");
gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), 0.5);
  //Wrapper wrapper; // wrapper class for spectrometer
  int numberOfPixels;
  //DoubleArray pixelArray;
  double *pixelValues, *wavelengths;

  // Cycle for each measurement repetition:
  for (i = 0; i < measurement_reps; i++) {
    // FOR TESTING:
    //_sleep(integrationTime);
    //_sleep(5);
    g_print("Performing a measurement...\n");
    g_usleep(4e6); // in microseconds
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
  // If we reach here, we're done! kill the progress bar timeout, and set to
  // indicate it's 100%
  //g_source_remove(timeoutID);
  gdk_threads_add_idle(complete_progressBar, params);

  // Free the output options data:
  g_free(params->outputPtr);

  // Free the memory that called this run:
  g_free(params->self);
  //pthread_exit(NULL); // We're done acquiring data now
}
