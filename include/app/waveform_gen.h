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

// File for defining constants for our waveform generator
#ifndef WAVEFORM_CONSTANTS
#define WAVEFORM_CONSTANTS

void start_wvfm_gen(int pn_bit_len, int mod_freq);
void stop_wvfm_gen();
unsigned long count_wvfm_gen();

#endif
