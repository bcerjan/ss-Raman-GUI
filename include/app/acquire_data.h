// Header file for data acquisition functions
#ifndef ACQUIRE_DATA
#define ACQUIRE_DATA

struct dataAcqParams {
  int spectrometerIndex;
  int integrationTime; // in ms
  int measurement_reps; // in #
  int mod_freq; // in Hz
  struct dataOuputOpts *outputPtr;
  GtkWidget *progressBar;
  int timeoutID;
  int timeoutInterval;
  GtkWidget *btn; // Start/Stop button
  //struct dataAcqParams *self; // So we can free our struct from a thread
  GCancellable *cancellable;
};

void start_data_acq_async(gpointer            data,
                          GCancellable       *cancellable,
                          GAsyncReadyCallback callback,
                          gpointer            user_data);
int progressBar_timeout_cb(gpointer data);

#endif
