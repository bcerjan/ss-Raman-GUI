// Header file that contains experimental constants for the application
// Note that you will likely need to edit this before compilation
#ifndef MEASUREMENT_PARAMS
#define MEASUREMENT_PARAMS

#define MODULATION_OPTS     3      // Number of choices for modulation frequency
#define PN_CODE_LENGTH_OPTS 6

static const int mod_freqs[MODULATION_OPTS] = {100, 250, 500}; // In MHz
static const int pn_code_lengths[PN_CODE_LENGTH_OPTS] = {32, 64, 128, 256, 512, 1024};

#endif
