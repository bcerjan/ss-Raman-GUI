// GTK for GUI generation
#include <gtk/gtk.h>


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Header files for the waveform generator ("wavepond")
#include "dax22000_lib_DLL64.h"

// Variable to track if we're currently running a scan or not:
static int scan_running = 0;

// Both the "X" button and File->Quit should end the process
void on_window_main_destroy()
{
  gtk_main_quit();
}


void on_file_quit_activate()
{
  gtk_main_quit();
}

// Handlers for the various buttons:

// Re-scan for spectrometers:
void spectrometer_scan_clicked_cb()
{
  g_print("Spectrometer_scan clicked\n");
}

// We want to start or stop a real scan:
void scan_button_clicked_cb(GtkButton *button)
{
  g_print("scan_button clicked\n");
  if (scan_running == 0) { // We are not currently running a scan
    scan_running = 1;
    char *text = "Stop Scan";
    gtk_button_set_label(button, text);
  } else {
    scan_running = 0;
    char *text = "Start Scan";
    gtk_button_set_label(button, text);
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
  GtkWidget  *window;

  // Load custom CSS:
  GFile  *cssFile = g_file_new_for_path("css/progressBar.css");
  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_file(cssProvider, cssFile, NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  // Free CSS stuff:
  g_object_unref(cssFile);
  g_object_unref(cssProvider);

  // get UI from our .glade file:
  builder = gtk_builder_new_from_file("glade/appLayout_V1.glade");

  // Apply UI to our window
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

  // Hook up the signals from the UI to their associated functions:
  gtk_builder_connect_signals(builder, NULL);

  // Dereference builder as its job is done:
  g_object_unref(builder);

  // Set up waveform generator:
  DWORD NumCards = 0;
  NumCards = DAx22000_GetNumCards();
  printf("Number of Cards = %d\n", NumCards);

  // Show the main window:
  gtk_widget_show(window);

  // Run the main loop:
  gtk_main();

  // TODO: Add turn-off of spectrometers / function generator here

  return 0;
}
