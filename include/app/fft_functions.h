// Header file for fft_functions.c function exports
#ifndef FFT_FUNCTIONS
#define FFT_FUNCTIONS

void interpolate_fft_data(int numPixels, // in spectrometer
                          double spec_freqs[],
                          unsigned long int fft_length,
                          double fft_freqs[],
                          double fft_pows[],
                          double fft_interp[]); // output, length of numPixels

unsigned long int calc_fft_length(int pn_bit_len);


void generate_pn_fft(int mod_freq,
                     int pn_bit_len,
                     int pn_bits[1024],
                     unsigned long int fft_len,
                     double pn_fft_freq[],
                     double pn_fft_pow[]);

#endif
