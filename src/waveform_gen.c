// File for generation of waveforms from a Wavepond generator

#include <stdbool.h>

// Header file containing pre-defined waveform data
//#include "pn_code.h"

// Header files for the waveform generator ("wavepond")
#include "dax22000_lib_DLL64.h"

// These are static in case they change all instances can be updated from a
// single location
static const DWORD CardNum = 1; // Fixed, we only have 1 card
static const DWORD Chan = 1; // Fixed, our card only has one channel

void start_wvfm_gen(int pn_bit_len, int mod_freq)
{
  int i,x;
  double actual_frequency;
  DWORD NumPoints = pn_bit_len; // Not sure what this does, but must be modulo 16

  WORD wvfm_array[1024] = {0}; // Array to hold our waveform values, may need to make larger buffer if we need to alter our clock rate

  // Check if we need to have longer "bits" and a fictitious clock rate, or if
  // we can get away with setting the clock rate to be the actual user-desired
  // rate and just going from high to low (this is what is currently implemented)

  // Load our waveform array from the header file here:
  /*if (pn_bit_len == 32) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_32_bit[i];
    }
  } else if (pn_bit_len == 64) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_64_bit[i];
    }
  } else if (pn_bit_len == 128) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_128_bit[i];
    }
  } else if (pn_bit_len == 256) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_256_bit[i];
    }
  } else if (pn_bit_len == 512) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_512_bit[i];
    }
  } else if (pn_bit_len == 1024) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_1024_bit[i];
    }
  } */

  // Initialize the driver and controller, and set clock rate:
  x = DAx22000_Open(CardNum);
  x = DAx22000_Initialize(CardNum);

  actual_frequency = DAx22000_SetClkRate(CardNum, mod_freq);

  // Input our waveform:
  x = DAx22000_CreateSingleSegment(
    CardNum,
    Chan,
    NumPoints,
    0, // NumLoops, 0 -> continuous loop
    2047, // PAD_Val_Beg (0 <= value <= 4095)
    2047, // PAD_Val_eEnd (same ^)
    wvfm_array,
    1 // Trigger status, 1 lets us re-trigger later
  );

  // Now turn on the generator:
  DAx22000_Run(CardNum, true);


  return;
}

void stop_wvfm_gen() {
  // Stop output:
  DAx22000_Stop(CardNum);

  // Close driver:
  DAx22000_Close(CardNum);

  return;
}
