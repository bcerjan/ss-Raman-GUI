// Header file that contains experimental constants for the application
// Note that you will likely need to edit this before compilation
#ifndef MEASUREMENT_PARAMS
#define MEASUREMENT_PARAMS

#define MODULATION_OPTS     3      // Number of choices for modulation frequency
#define PN_CODE_LENGTH_OPTS 6
#define LASER_WAVELENGTH 638.318 // in nm

static const int mod_freqs[MODULATION_OPTS] = {100, 250, 500}; // In MHz
static const int pn_code_lengths[PN_CODE_LENGTH_OPTS] = {32, 64, 128, 256, 512, 1024};
// If you change pn_code_lengths, you will also need to update pn_code_generator.c
// and waveform_gen.c to use the new values. This is only necessary if you add
// or alter values, if you remove them it should still be fine.

// Define parameters for the serial port controller here:
// These are the defaults for the Arduino board we used (9600 8N1):
#define SERIAL_BAUD_RATE 9600
#define SERIAL_PARITY 0 // Same as SP_PARITY_NONE
#define SERIAL_FLOW_CONTROL 0 // Same as SP_FLOWCONTROL_NONE
#define SERIAL_BITS 8
#define SERIAL_STOP_BITS 1
#define MAX_LASER_POWER 100 // % of total power we ever will need



#endif
