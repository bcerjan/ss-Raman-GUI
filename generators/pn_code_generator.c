/*
Code to generate our header files for the various bit-length PN signals.

The method used here is described here:
https://blog.kurttomlinson.com/posts/prbs-pseudo-random-binary-sequence

It might not be the most efficient way to do this, but I understand what it
does, and we're only going up to 10 bits for our longest sequence.

Wikipedia has a list of characteristic polynomials if at some point you want
to add more to this generation function.
(https://en.wikipedia.org/wiki/Linear-feedback_shift_register#Some_polynomials_for_maximal_LFSRs)

Note that the +1 term in the polynomials is dropped in our binary reperesentation,
but is used when we output the bit (using bit & 1)

Also generates binary data files with possible combinations of pn_bit_len and
allowed modulation frequencies FFT'd into the frequency domain.

*/

#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>


#include "fftw3.h"

#include "measurement_params.h"

/* Function to automate lfsr looping */
void lfsr_loop(int output[], int poly)
{
  int a = output[0];
  int i;

  for (i = 1;; i++) {
    int lsb = a & 1; // This is our output
    a = (a >> 1);
    if (lsb == 1) {
      a ^= poly;
    }

    //printf("%x\n", lsb);
    output[i] = lsb;
    if (a == output[0]) {
      //printf("period is %d\n", i);
      break;
    }
  }
}

/* Function to generate 32-bit PN code
   Characteristic Polynomial: x^5 + x^3 + 1 */
void pn_32_generate(int output[])
{
  int poly = 0b10100; // 0bXXXX indicates a binary string (rather than hex)
  lfsr_loop(output, poly);
  return;
}

/* Polynomial: x^6 + x^5 + 1 */
void pn_64_generate(int output[])
{
  int poly = 0b110000;
  lfsr_loop(output, poly);
  return;
}

/* Polynomial: x^7 + x^6 + 1 */
void pn_128_generate(int output[])
{
  int poly = 0b1100000;
  lfsr_loop(output, poly);
  return;
}

/* Polynomial: x^8 + x^6 + x^5 + x^4 + 1 */
void pn_256_generate(int output[])
{
  int poly = 0b10111000;
  lfsr_loop(output, poly);
  return;
}

/* Polynomial: x^9 + x^5 + 1 */
void pn_512_generate(int output[])
{
  int poly = 0b100010000;
  lfsr_loop(output, poly);
  return;
}

/* Polynomial: x^10 + x^7 + 1 */
void pn_1024_generate(int output[])
{
  int poly = 0b1001000000;
  lfsr_loop(output, poly);
  return;
}

// Function to loop and write the values for each pre-computed array
// needs a header line like: static const int pn_32_bit[32] = {\n ...
void write_pn_block(FILE *fPtr, int data[], int pn_bit_len)
{

  int i;
  for (i = 0; i < pn_bit_len - 1; i++) {
    fprintf(fPtr, "%x,\n", data[i]);
  }
  // Make sure there isn't a trailing comma (not that it really matters...)
  fprintf(fPtr, "%x\n", data[pn_bit_len-1]);
  // Close the array
  fprintf(fPtr, "};\n\n");

}

void generate_fft_data(int mod_freq, int pn_bit_len, int pn_bits[], double fft[])
{
  double speedC = 2.99792458e11; // In nm/microsecond

  double sampFreq = 2.0 * speedC / LASER_WAVELENGTH; // Our data will always be at lower frequencies than this

  // Now, we need to set up our pn code array with an appropriate number of sampeles/sec
  // To reduce the size of the arrays, we're using MHz as our default unit instead of Hz
  // (so usec (micro) instead of sec)

  double bit_duration = 1.0/((double ) mod_freq); // How long each bit is in microseconds
  double samps_per_bit = bit_duration * sampFreq;
  // Make sure this needs to be unsigned long (or if it needs to be long long...)
  unsigned long int isamps_per_bit = (unsigned long int )samps_per_bit; // This should always be a huge number without any decimals, so this cast shouldn't be an issue
  double total_time = (double )pn_bit_len * bit_duration;
  double total_samps = total_time * sampFreq;
  unsigned long int itotal_samps = (unsigned long int )total_samps;
  // Maybe pad this to a "good" number, but should be pretty close to start with

  // Convert our pn_bits to double:
  double *pn_temp_bit;
  g_malloc0(sizeof(*pn_temp_bit) * pn_bit_len);

  for (i = 0; i < pn_bit_len; i++) {
    pn_temp_bit[i] = (double )pn_bits[i];
  }

  double *high_res_pn;
  g_malloc0(sizeof(*high_res_pn) * itotal_samps);
printf("Value of total_samps: %f\n", total_samps);
printf("Value of itotal_samps: %u\n", itotal_samps);
  // Now prepare our array to be FT'd -- this should almost certainly be padded to a power of 2...
  for (i = 0; i < pn_bit_len; i++) {
    for (j = 0; j < isamps_per_bit; j++) {
      int idx = j + i*isamps_per_bit;
      high_res_pn[idx] = pn_temp_bit[i];
    }
  }

  g_free(pn_temp_bit);

  // Now prepare to do the FFT
  fftw_complex *pn_fft_out;
  fftw_plan p_r2c;

  num_cmp_elements = (int) floor((total_samps / 2.0)) + 1; // Number of elements in our output DFT arrays

  pn_fft_out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * num_cmp_elements);

  p_r2c = fftw_plan_dft_r2c_1d(itotal_samps, high_res_pn, pn_fft_out, FFTW_ESTIMATE);

  // Run the FFT:
  fftw_execute(p_r2c);

  // Get magnitudes from the FFT:
  double *pn_fft, *freq_fft;
  g_malloc0(sizeof(*pn_fft) * num_cmp_elements);
  g_malloc0(sizeof(*freq_fft) * num_cmp_elements);

  for (i = 0; i < num_cmp_elements; i++) {
    // Magnitude of each component:
    pn_fft[i] = sqrt( pow( abs( c_re(pn_fft_out[i]) ), 2) +
                      pow( abs( c_im(pn_fft_out[i]) ), 2) );

    freq_fft[i] = sampFreq * ((double ) i) / total_time; // Frequency of each point in the FFT
  }

  // Free data:
  g_free(high_res_pn);

  fftw_destroy_plan(p_r2c);

  fftw_free(pn_fft_out);

  return;
}

int main() {
  // Initialize our array at its maximum possible size:
  int seed = 0x1; // Probably don't change this
                  // so future results all use the same pn string (just in case)
                  // Any number that has num & 1 = 1 will work

  int output[1024] = {0};
  output[0] = seed & 1; // set to our seed value

  //char headerFname[100] = "../include/app/pn_code.h";
  char headerFname[100] = "pn_code.h"; // This is to satisfy CMake as it sort of hides this header...

  FILE *headerPtr;
  headerPtr = fopen(headerFname,"w");

  // Add blocking definitions:
  fprintf(headerPtr, "#ifndef PN_CODE\n#define PN_CODE\n");

  // Now add the 32-bit code:
  fprintf(headerPtr, "static const int pn_32_bit[32] = {\n");
  // generate the sequence:
  pn_32_generate(output);
  // and write it out:
  write_pn_block(headerPtr, output, 32);

  // And now we take an FFT:
  /*double fft[5] = {0.0};
  generate_fft_data(mod_freqs[0], 32, output, fft);*/

  // Now, we repeat this for each of the code lengths:
  fprintf(headerPtr, "static const int pn_64_bit[64] = {\n");
  pn_64_generate(output);
  write_pn_block(headerPtr, output, 64);

  fprintf(headerPtr, "static const int pn_128_bit[128] = {\n");
  pn_128_generate(output);
  write_pn_block(headerPtr, output, 128);

  fprintf(headerPtr, "static const int pn_256_bit[256] = {\n");
  pn_256_generate(output);
  write_pn_block(headerPtr, output, 256);

  fprintf(headerPtr, "static const int pn_512_bit[512] = {\n");
  pn_512_generate(output);
  write_pn_block(headerPtr, output, 512);

  fprintf(headerPtr, "static const int pn_1024_bit[1024] = {\n");
  pn_1024_generate(output);
  write_pn_block(headerPtr, output, 1024);

  // Closing tag for if
  fprintf(headerPtr, "\n#endif");

  fclose(headerPtr)l
  return 0;
}
