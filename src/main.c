#include <gtk/gtk.h>


void on_window_main_destroy()
{
  gtk_main_quit();
}


void on_file_quit_activate()
{
  gtk_main_quit();
}

void spectrometer_scan_clicked_cb()
{
  g_print("Spectrometer_scan clicked\n");
}

void scan_button_clicked_cb()
{
  g_print("scan_button clicked\n");
}

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
  GFile  *cssFile = g_file_new_for_path("css/test.css");
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

  // Show the main window:
  gtk_widget_show(window);

  // Run the main loop:
  gtk_main();

  return 0;
}
