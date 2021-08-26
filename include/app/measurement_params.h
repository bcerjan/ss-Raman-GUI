// Header file that contains experimental constants for the application
// Note that you will likely need to edit this before compilation
#ifndef MEASUREMENT_PARAMS
#define MEASUREMENT_PARAMS

#define MODULATION_OPTS     3      // Number of choices for modulation frequency
#define PN_CODE_LENGTH_OPTS 6
#define LASER_WAVELENGTH 638.318 // in nm

// Parameters for electro-optic modulator
#define WVFM_MAGNITUDE 850 // Dependent on your waveform generator and/or amplifier
                           // Ours accepts values 0-4095, but with the amplifier
                           // we need only a fraction of that (850 into 50 Ohms)


static const int mod_freqs[MODULATION_OPTS] = {100, 250, 500}; // In MHz
static const int pn_code_lengths[PN_CODE_LENGTH_OPTS] = {32, 64, 128, 256, 512, 1024};
// If you change pn_code_lengths, you will also need to update pn_code_generator.c
// and waveform_gen.c to use the new values. This is only necessary if you add
// or alter values, if you remove them it should still be fine.

#endif
