// File containing functions for controlling a serial-connected controller for
// laser power.

#include <gtk/gtk.h>

// How long could our URI potentially be (in characters)
#define MAX_URI_LENGTH 60
#define SENDING_STR_LEN 15 // we use a float which has a maximum of 9, so leave some room

void set_duty_cycle(gchar port_id, float percent)
{
  // first, format port number into a URI in a platform-dependent way
  const gchar portURI[MAX_URI_LENGTH] = generate_port_uri(port_id);

  // Now we need to write to our port:
  const gchar strToSend[SENDING_STR_LEN];
  sprintf(strToSend, "<%f>", percent); // use < and > as start/end markers for transmission

  // Open serial port:
  GIOChannel *serialCon;
  GError **error;
  gsize *bytes_written;

  serialCon = g_io_channel_new_file(portURI, "w", error);

  g_io_channel_write_chars(serialCon, strToSend, -1, bytes_written, error);

  g_io_channel_shutdown(serialCon, TRUE, error);

  return;

}


// function to generate a string containing URI for the serial port:
*gchar generate_port_uri(gchar port_id)
{
  /*
  port_id is something like: COM2 (Windows), ttyS0 (linux, potentially also
  ttySUSB0 or ttyUSB0), or tty.usbserial-A700dYoR on OSX. On Windows, our return
  should be like: "\\\\.\\COM2". On the *nix based ones, we need an output like
  "/dev/ttyUSB0" (or whatever)
  */
  gchar output[MAX_URI_LENGTH];

  #if defined _WIN32
    output = g_strjoin(NULL, "\\\\\\\\.\\\\", port_id, NULL);
    // we need 4 backslashes then 2 backslashes, but have to escape each one
    // making this mess
  #endif

  #if defined(__unix__) || defined(__unix) || \
        (defined(__APPLE__) && defined(__MACH__))

    output = g_strjoin(NULL, "/dev/", port_id, NULL);
  #endif

  return output;
}
