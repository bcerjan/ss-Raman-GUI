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
