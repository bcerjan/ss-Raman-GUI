// File containing functions for controlling a serial-connected controller for
// laser power.

#include <gtk/gtk.h>
#include "libserialport.h"

#include "measurement_params.h"

#define SENDING_STR_LEN 15 // we use a float which has a maximum of 9, so leave some room

// Function to get list of strings identifying available ports
// usage:
// int length = 0;
// char *port_names = get_port_list(&length); // needs to be freed by caller
char ** get_port_list(int *len)
{
  struct sp_port **port_list;
  enum sp_return result = sp_list_ports(&port_list);

  if (result != SP_OK) {
    printf("sp_list_ports() failed!\n");
    return NULL;
  }

  // First count how many we found:
  int i,j;
  for (i = 0; port_list[i] != NULL; i++) {}
  *len = i;
  char **res;
  res = g_malloc(i * sizeof(*res));

  for (j = 0; j < i; j++) {
    struct sp_port *port = port_list[j];
    res[j] = g_strdup(sp_get_port_name(port));
  }

  sp_free_port_list(port_list);

  return res;
}

// Function to free list produced by the above function
void free_port_list(int len, char **list)
{
  int i;
  for (i = 0; i < len; i++) {
    g_free(list[i]);
  }
  g_free(list);

  return;

}

// Function to actually set a particular duty cycle:
void set_duty_cycle(gchar port_name, float percent)
{
  // String to write to our port:
  const gchar strToSend[SENDING_STR_LEN];
  sprintf(strToSend, "<%f>", percent); // use < and > as start/end markers for transmission

  // Open and configure the port:
  struct sp_port *port;
  sp_get_port_by_name(port_name, &port);
  sp_open(port, SP_MODE_READ_WRITE);

  // Set our port parameters (defined in measurement_params.h):
  sp_set_baudrate(port, SERIAL_BAUD_RATE);
  sp_set_bits(port, SERIAL_BITS);
  sp_set_parity(port, SERIAL_PARITY);
  sp_set_stopbits(port, SERIAL_STOP_BITS);
  sp_set_flowcontrol(port, SERIAL_FLOW_CONTROL);

  int strLen = strlen(strToSend);
  int timeout = 500; // 500 ms timeout
  sp_blocking_write(port, strToSend, strLen, timeout);

  // Read here to verify?

  sp_close(port);
  sp_free_port(port);

  return;

}
