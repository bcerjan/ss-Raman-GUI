// Contains functions to perform FFT on PN code as well as do point-wise
// multiplication with an input spectrum. Has helper function to do
// interpolation to make sure the frequencies from the PN FFT match the data
// from the spectrometer.

#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "complex.h"
#include "fftw3.h"

const static unsigned long isamps_per_bit = 512; // Number of samples per bit in PN.
                                 // Keep as power of 2 to make FFT fast
                                 // Sort of arbitrary, but some testing in
                                 // MATLAB indicates this should be plenty

// Function to linearly interpolate between two points to a target x-value and
// return the predicted y-value
double interpolate_pts(double x_tar,
                       double x1,
                       double x2,
                       double y1,
                       double y2)
{
  double m; // slope of line
  m = (y2 - y1) / (x2 - x1);
  return (m*(x_tar - x1) + y1);
}

// Function to interpolate our FFT results to the same frequencies as our
// spectrometer measures at. Assume that spec_freqs and fft_freqs (and fft_pows)
// are already sorted into ascending order (spec_freqs[0] = fft_freqs[0] = 0)
void interpolate_fft_data(int numPixels, // in spectrometer
                          double spec_freqs[],
                          unsigned long int fft_length,
                          double fft_freqs[], // length of fft_length
                          double fft_pows[], // length of fft_length
                          double fft_interp[]) // output, length of numPixels
{
  int i;
  unsigned long int j, jStart;
  jStart = 1;

  double targetFreq, lastFreq, thisFreq, nextFreq;
  double lastFreqDiff, thisFreqDiff, nextFreqDiff;
  double tol = 0.05; // Tolerance for deciding that two frequencies are "equal"
                     // as a ratio of frequency difference / targetFreq
  lastFreq = 0.0;

  //fft_interp[0] = fft_pows[0]; // 0th element is always DC for both
  //targetFreq = spec_freqs0];


  for (i = 0; i < numPixels; i++) { // Start at 1, as 0 is always DC
    targetFreq = spec_freqs[i];
    for (j = jStart; j < fft_length - 1; j++) {
      thisFreq = fft_freqs[j];
      thisFreqDiff = targetFreq - thisFreq;
      if ( fabs(thisFreqDiff/targetFreq) < tol ) { // We're super close where we are, so just use that point
        jStart = j; // next time start farther along, as we're pre-sorted
        fft_interp[i] = fft_pows[j];
        break; // we don't need to keep looping, we found the target.
      } /* if statement */

      lastFreq = fft_freqs[j-1];
      lastFreqDiff = targetFreq - lastFreq;
      nextFreq = fft_freqs[j+1];
      nextFreqDiff = targetFreq - nextFreq;

      if ( lastFreqDiff > 0 && nextFreqDiff < 0 ) { // This means we're bracketing our target value
        // We now need to find which two values (of the three) are closest
        // "this" is always one, as it is definitionally between the other two

        if (thisFreqDiff < 0) { // means "this" > target > "last"
                                // so we use "this" and "last" for interpolation
          fft_interp[i] = interpolate_pts(targetFreq, thisFreq, lastFreq,
                                          fft_pows[j], fft_pows[j-1]);

        } else { // means "next" > target > "this"
          fft_interp[i] = interpolate_pts(targetFreq, thisFreq, nextFreq,
                                          fft_pows[j], fft_pows[j+1]);

        } /* if/else statement */

        jStart = j;
        break;
      } /* if statement */
    } /* j for loop */
  } /* i for loop */

  fclose(outfile);

}

unsigned long int calc_fft_length(int pn_bit_len)
{
  double total_samps = (double )pn_bit_len * (double )isamps_per_bit;
  unsigned long int fft_length;
  fft_length = (unsigned long int) floor((total_samps / 2.0)) + 1; // Number of elements in our output DFT arrays
  return fft_length;
}


void generate_pn_fft(int mod_freq, // in MHz
                     int pn_bit_len,
                     int pn_bits[1024],
                     unsigned long int fft_len, // Length of output arrays
                     double pn_fft_freq[], // Output
                     double pn_fft_pow[]) // Output
{
  int i;
  unsigned long int j;

  double bit_duration = 1.0/((double ) mod_freq); // How long each bit is in microseconds

  double total_time = (double )pn_bit_len * bit_duration; // in us

  unsigned long int itotal_samps = ((unsigned long int )pn_bit_len * isamps_per_bit);

  // High-resolution sampling of our PN code
  double *high_res_pn;
  high_res_pn = g_malloc0(sizeof(*high_res_pn) * pn_bit_len * isamps_per_bit);

  unsigned long int idx;
  for (i = 0; i < pn_bit_len; i++) {
    for (j = 0; j < isamps_per_bit; j++) {
      idx = j + i*isamps_per_bit;
      high_res_pn[idx] = (double )pn_bits[i];
    }
  }

  // Now prepare to do the FFT
  fftw_complex *pn_fft_out;
  fftw_plan p_r2c; // FFTW plan

  pn_fft_out = fftw_alloc_complex(sizeof(fftw_complex) * fft_len);
  p_r2c = fftw_plan_dft_r2c_1d(itotal_samps, high_res_pn, pn_fft_out, FFTW_ESTIMATE); // As we're only running one transform, use FFTW_ESTIMATE

  // Run the FFT:
  fftw_execute(p_r2c);

  double speedC = 2.99792458e4; // In cm/usec

  // Get magnitudes and frequencies from the FFT:
  for (i = 0; i < fft_len; i++) {
    // Magnitude of each component:
    pn_fft_pow[i] = cabs(pn_fft_out[i])/(double )fft_len; // The division is to normalize
    //pn_fft_freq[i] = (double )i / total_time; // Frequency in MHz
    pn_fft_freq[i] = (((double )i / total_time))/speedC; // Frequency in cm^-1
  }

  // Free data:
  fftw_destroy_plan(p_r2c);
  fftw_free(pn_fft_out);
  g_free(high_res_pn);

  return;
}
