#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "waveform_gen.h"


int main()
{
	//======================================================
  // Set up waveform generator:
  unsigned long NumCards = 0;
  NumCards = count_wvfm_gen();
  printf("Number of Cards = %ld\n", NumCards);

	int mod_freq = 250; // MHz
	int pn_bit_len = 128;

	printf("Starting waveform generation, press ENTER to stop\n");
	start_wvfm_gen(pn_bit_len, mod_freq);
	fflush(stdout);
	// Wait for keyboard interrupt
	getchar();
	printf("Stopped waveform generation\n");

	// Then stop:
	stop_wvfm_gen();

	return 0;
}
