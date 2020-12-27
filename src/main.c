// GTK for GUI generation
#include <gtk/gtk.h>


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#include <pthread.h>

// Header files for the waveform generator ("wavepond")
#include "dax22000_lib_DLL64.h"

// Header files for Ocean Inisght Spectrometer
//#include "API INFO HERE"

// Headers from this project:
#include "data_output.h"
#include "waveform_gen.h"
#include "acquire_data.h"

// Variable to track if we're currently running a scan or not:
static int scan_running = 0;
static GThread *worker_tid;

// Struct to hold all of the widgets when we start or stop a scan
typedef struct {
  GtkWidget *data_dir_entry;
  GtkWidget *data_fname_entry;
  GtkWidget *spectrometer_comboBox;
  GtkWidget *num_meas_entry;
  GtkWidget *pn_bit_length_entry;
  GtkWidget *integration_time_entry;
  GtkWidget *mod_freq_comboBox;
  GtkWidget *raw_data_check;
  GtkWidget *fft_data_check;
  GtkWidget *conv_data_check;
  GtkWidget *final_data_check;
  GtkWidget *spectrometer_dialog;
  GtkWidget *progressBar;
} userInputWidgets; // Don't love using a typedef here...
                    // Seems to be required to use g_slice_new()

// Both the "X" button and File->Quit should end the process
void on_window_main_destroy()
{
  gtk_main_quit();
}


void on_file_quit_activate()
{
  gtk_main_quit();
}

// As well as quitting from the function generator dialog:
void on_function_generator_dialog_quit_clicked()
{
  gtk_main_quit();
}

// Handlers for the various buttons:

// Re-scan for the waveform generator:
void on_function_generator_dialog_btn_clicked(GtkWidget *popup)
{
  DWORD NumCards = 0;
  NumCards = DAx22000_GetNumCards();
  // Close the dialog if we found a card, if not do nothing.
  if (NumCards == 1) {
    gtk_widget_hide(popup);
  }
}

// Re-scan for spectrometers:
void spectrometer_scan_clicked_cb()
{
  g_print("Spectrometer_scan clicked\n");
}

// Close error dialog when no spectrometers selected:
void spectrometer_dialog_btn_clicked_cb(GtkWidget *dialog) {
  gtk_widget_hide(dialog);
}

// We want to start or stop a real scan:
void scan_button_clicked_cb(GtkButton *button,
                            userInputWidgets *uiWidgets)
{
  g_print("scan_button clicked\n");


  if (scan_running == 0) { // We are not currently running a scan
    // Check this first to make sure this scan has a spectrometer selected
    // Set up our spectrometer:
    int spectrometerIndex;
    spectrometerIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(uiWidgets->spectrometer_comboBox)) - 1; // Offset by 1 to account for our note in the box
    /*if (spectrometerIndex < 0) {
      gtk_widget_show(uiWidgets->spectrometer_dialog);
      return;
    }*/

    scan_running = 1;
    char *text = "Stop Scan";
    gtk_button_set_label(button, text);

    // Prepare to start wafeform generation:x
    int pn_bit_len;
    // Get number of bits in our pseudorandom noise sequence
    // entry 0 is 32 and they multiply by power of two after that, this works
    // by bit-shifting which produces multiplication by powers of 2.
    // See https://stackoverflow.com/questions/141525/what-are-bitwise-shift-bit-shift-operators-and-how-do-they-work
    pn_bit_len = 32 << gtk_combo_box_get_active(GTK_COMBO_BOX(uiWidgets->pn_bit_length_entry));

    // Get modulation frequency in MHz
    int mod_freq, mod_freq_ind;
    mod_freq_ind = gtk_combo_box_get_active(GTK_COMBO_BOX(uiWidgets->mod_freq_comboBox));
    if (mod_freq_ind == 0) {
      mod_freq = 100e6; // Hz
    } else if (mod_freq_ind == 1) {
      mod_freq = 250e6; // Hz
    } else if (mod_freq_ind == 2) {
      mod_freq = 500e6; // Hz
    }

    // Start the waveform generator:
    //start_wvfm_gen(pn_bit_len, mod_freq);

    //printf("Number of pn_repetitions = %d\n", pn_repetitions);

    // Note that minimum integration time is 8 ms
    int integrationTime = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(uiWidgets->integration_time_entry)); // milliseconds


    // Get number of measurements we want to do right now:
    int measurement_reps,i;
    measurement_reps = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(uiWidgets->num_meas_entry));


    // Prepare to output our data:
    char *data_dir;
    data_dir = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(uiWidgets->data_dir_entry));

    const char *fname;
    fname = gtk_entry_get_text(GTK_ENTRY(uiWidgets->data_fname_entry));

    // Figure out which data they want:
    struct dataOuputOpts *outputPtr = g_malloc(sizeof(*outputPtr));

    outputPtr->raw_data = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(uiWidgets->raw_data_check));
    outputPtr->fft_data = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(uiWidgets->fft_data_check));
    outputPtr->conv_data = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(uiWidgets->conv_data_check));
    outputPtr->final_data = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(uiWidgets->final_data_check));
    outputPtr->fname = fname;
    outputPtr->data_dir = data_dir;

    //outputPtr = &outputOpts;
    //g_free(data_dir);
    //g_free(fname);

    GtkWidget *progressBar = uiWidgets->progressBar;
    // Set our bar to 0%
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), 0.0);
    // Begin the acquisition
    // This does many things -- it records data (updating the progress bar),
    // processes that data, and outputs it (at the appropriate step(s)) as
    // requested.
    int timeoutInterval = 100;
    struct dataAcqParams *params = g_malloc(sizeof(*params));

    params->spectrometerIndex = spectrometerIndex;
    params->integrationTime = integrationTime;
    params->measurement_reps = measurement_reps;
    params->outputPtr = outputPtr;
    params->progressBar = progressBar;
    params->timeoutID = 0;
    params->timeoutInterval = timeoutInterval;
    params->self = params;

gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(params->progressBar), 0.33);
    GThread *tid;
    //int timeoutID;
    printf("Original pointer location: %p\n", (void *) progressBar);
    //pthread_t tid;
    // These lines are to ensure that our worker thread is "joinable" --
    // this means that we can specify that we want to wait for it before
    // letting the main thread move on.
    /*pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);*/
    // end joinable lines

    /*int rc;
    void *status;*/

    // Start updating the progress bar:
    params->timeoutID = gdk_threads_add_timeout(timeoutInterval, progressBar_timeout_cb,
                                        params);
    //params->timeoutID = g_timeout_add(timeoutInterval, progressBar_timeout_cb,
    //                                            params);

    //params->timeoutID = timeoutID;
    //rc = pthread_create(&tid, &attr, start_data_acq, (void *) &params); // Update progressBar in here
    tid = g_thread_new("acquire", start_data_acq, params);
    worker_tid = tid; // Store this in a global in case we need to cancel it later
    g_print("After thread creation...\n");
    g_thread_unref(tid);
    // Stop the waveform generator:
    //stop_wvfm_gen();

    // Ensure that our worker thread has finished:
    //pthread_attr_destroy(&attr); // Free memory
    //rc = pthread_join(tid, &status);
    //g_print("After re-joining...\n");
//g_usleep(3e6);
    // Reset our global variable and change the text back:
//    scan_running = 0;
//    text = "Start Scan";
//    gtk_button_set_label(button, text);
    //g_free(params);

    //g_source_remove(params->timeoutID);
  } else {
    scan_running = 0;
    char *text = "Start Scan";
    gtk_button_set_label(button, text);
    //stop_wvfm_gen();
    //stop_spec_acq();
    //clear_data(); // If needed
  }
}

// Someone wants to see the help documentation
void on_help_help_activate()
{
  g_print("Open help!\n");
}

int main(int    argc,
         char **argv)
{
  // Initialize GTK:
  gtk_init(&argc, &argv);

  // Prepare pointers to builder and window:
  GtkBuilder *builder;
  GtkWidget  *window, *dialog, *spectrometer_comboBox;
  userInputWidgets *uiWidgets = g_slice_new(userInputWidgets);

  // Load custom CSS:
  GFile  *cssFile = g_file_new_for_path("../css/progressBar.css");
  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_file(cssProvider, cssFile, NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  // Free CSS stuff:
  g_object_unref(cssFile);
  g_object_unref(cssProvider);

  // get UI from our .glade file:
  builder = gtk_builder_new_from_file("../glade/appLayout_V1.glade");

  // Apply UI to our window
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

  // And get our dialog:
  dialog = GTK_WIDGET(gtk_builder_get_object(builder, "function_generator_dialog"));

  // All of the widgets that we need to read from later on:

  uiWidgets->data_dir_entry = GTK_WIDGET(gtk_builder_get_object(builder, "dir_input"));
  uiWidgets->data_fname_entry = GTK_WIDGET(gtk_builder_get_object(builder, "fname_input"));
  uiWidgets->spectrometer_comboBox = GTK_WIDGET(gtk_builder_get_object(builder, "spectrometer_select"));
  uiWidgets->num_meas_entry = GTK_WIDGET(gtk_builder_get_object(builder, "measurement_repetitions"));
  uiWidgets->pn_bit_length_entry = GTK_WIDGET(gtk_builder_get_object(builder, "pn_bit_length"));
  uiWidgets->integration_time_entry = GTK_WIDGET(gtk_builder_get_object(builder, "integration_time"));
  uiWidgets->mod_freq_comboBox = GTK_WIDGET(gtk_builder_get_object(builder, "mod_freq_box"));
  uiWidgets->raw_data_check = GTK_WIDGET(gtk_builder_get_object(builder, "raw_data_save"));
  uiWidgets->fft_data_check = GTK_WIDGET(gtk_builder_get_object(builder, "fft_data_save"));
  uiWidgets->conv_data_check = GTK_WIDGET(gtk_builder_get_object(builder, "conv_data_save"));
  uiWidgets->final_data_check = GTK_WIDGET(gtk_builder_get_object(builder, "final_data_save"));
  uiWidgets->spectrometer_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "spectrometer_dialog"));
  uiWidgets->progressBar = GTK_WIDGET(gtk_builder_get_object(builder, "scan_progress_bar"));

  // Hook up the signals from the UI to their associated functions, and
  // pass our struct of data to be returned as "userdata" for when we need to
  // access lots of widgets instead of one or two
  gtk_builder_connect_signals(builder, uiWidgets);

  // Dereference builder as its job is done:
  g_object_unref(builder);

  // Set up waveform generator:
  DWORD NumCards = 0;
  NumCards = DAx22000_GetNumCards();
  printf("Number of Cards = %d\n", NumCards);
  /*if (NumCards == 0) {
    gtk_widget_show(dialog);
  }*/

  // Scan for spectrometer(s)
  /*
  Wrapper wrapper; // wrapper for spectrometer
  int numberOfSpectrometers,i;

  numberOfSpectrometers = wrapper.openAllSpectrometers();
  if (numberOfSpectrometers < 0) {
    wrapper.getLastException();
    // g_print or something here, maybe exit()...
  }

  // Add all spectrometers to the selection list:

  // Check if this should be from 1 or from 0...
  for (i = 1; i < numberOfSpectrometers + 1; i++) {
    char *name = wrapper.getName(i);
    char *id = wrapper.getSerialNumber(i);
    gtk_combo_box_text_append(GTK_COMBO_BOX(spectrometer_comboBox), id, name);
  }
  */

  // Show the main window:
  gtk_widget_show(window);

  // Run the main loop:
  gtk_main();

  //g_object_unref(dialog);
  g_slice_free(userInputWidgets, uiWidgets);

  // TODO: Add turn-off of spectrometers / function generator here
  //wrapper.closeAllSpectrometers(); // Turns off all spectrometers
  //stop_wvfm_gen();

  return 0;
}
