/**
    Copyright (c) 2021 Ben Cerjan
    This file is part of ss-Raman-GUI.
    ss-Raman-GUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    ss-Raman-GUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
    You should have received a copy of the GNU Affero General Public License
    along with ss-Raman-GUI.  If not, see <https://www.gnu.org/licenses/>.
**/

// File for generation of waveforms from a Wavepond generator
// This is the only file you need to change if you alter your waveform generator
// (e.g. to a different model or whatever)

#include <gtk/gtk.h>
#include <stdbool.h>
#include <math.h>

// Header file containing pre-defined waveform data
#include "pn_code.h"

// Header files for the waveform generator ("wavepond")
#include "dax22000_lib_DLL64.h"

// These are static in case they change all instances can be updated from a
// single location
static const DWORD CardNum = 1; // Fixed, we only have 1 card
static const DWORD Chan = 1; // Fixed, our card only has one channel

// Function to count how many waveform generators are attached
unsigned long count_wvfm_gen()
{
  DWORD NumCards;
  NumCards = DAx22000_GetNumCards();
  printf("Number of Cards = %ld\n", NumCards);
  return NumCards;
}

void start_wvfm_gen(int pn_bit_len, int mod_freq)
{
  int i,j,x;
  double actual_frequency, clk_rate;
  clk_rate = 2.0e9; // Set to 2 GHz clock rate. Note that mod_freq should be
                    // an even divisor of this number
  int isamps_per_bit = (int ) ceil( clk_rate / ((double) mod_freq * 1.0e6) ); // How many clock cycles long are our bits? (multiplication is to convert from MHz)
  DWORD NumPoints = pn_bit_len * isamps_per_bit;// Length of our waveform before it loops
                                                 // Needs to be modulo 16 (not an issue for us)

  WORD wvfm_array[1024] = {0}; // Array to hold our waveform values, may need to make larger buffer if we need to alter our clock rate

  // LIKELY NEED TO ADJUST THIS OR ADD A 0 POSITION OFFSET (OR BOTH)
  WORD offset = 2047; // Offset from "0" state to "1" state

  // Check if we need to have longer "bits" and a fictitious clock rate, or if
  // we can get away with setting the clock rate to be the actual user-desired
  // rate and just going from high to low (this is what is currently implemented)

  // Load our waveform array from the header file here:
  if (pn_bit_len == 32) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_32_bit[i] * offset;
    }
  } else if (pn_bit_len == 64) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_64_bit[i] * offset;
    }
  } else if (pn_bit_len == 128) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_128_bit[i] * offset;
    }
  } else if (pn_bit_len == 256) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_256_bit[i] * offset;
    }
  } else if (pn_bit_len == 512) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_512_bit[i] * offset;
    }
  } else if (pn_bit_len == 1024) {
    for (i = 0; i < pn_bit_len; i++) {
      wvfm_array[i] = pn_1024_bit[i] * offset;
    }
  }

  // High-resolution sampling of our PN code
  DWORD *high_res_pn;
  high_res_pn = g_malloc0(sizeof(*high_res_pn) * pn_bit_len * isamps_per_bit);

  unsigned long int idx;
  for (i = 0; i < pn_bit_len; i++) {
    for (j = 0; j < isamps_per_bit; j++) {
      idx = j + i*isamps_per_bit;
      high_res_pn[idx] = wvfm_array[i];
      }
    }

  // This seems to be necessary, or the waveform generator doesn't turn on...
  x = DAx22000_GetNumCards();

  // Initialize the driver and controller, and set clock rate:
  x = DAx22000_Open(CardNum);
  x = DAx22000_Initialize(CardNum);

  actual_frequency = DAx22000_SetClkRate(CardNum, (double )mod_freq * 1e6); // Need to convert from MHz to Hz

  // Input our waveform:
  x = DAx22000_CreateSingleSegment(
    CardNum,
    Chan,
    NumPoints,
    0, // NumLoops, 0 -> continuous loop
    high_res_pn[0], // PAD_Val_Beg (0 <= value <= 4095)
    wvfm_array[NumPoints-1], // PAD_Val_End (same ^)
    high_res_pn,
    1 // Trigger status, 1 lets us re-trigger later
  );

  // Now turn on the generator:
  DAx22000_Run(CardNum, true);
  g_free(high_res_pn);

  return;
}

void stop_wvfm_gen() {
  // Stop output:
  DAx22000_Stop(CardNum);

  // Close driver:
  DAx22000_Close(CardNum);

  return;
}
