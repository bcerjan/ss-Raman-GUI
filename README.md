# ss-Raman-GUI
UI, data collection, and spectral math for perfoming spread spectrum (surface-enhanced) Raman measurements. Designed to implement the method described by [Lee et al](https://doi.org/10.1038/s41467-020-20413-8). Automates the process of controlling a function generator and spectrometer to collect data and performs the necessary FFT's and automatically processes the data produced.

# Usage
Most of the controls should be self-explanatory, but for clarity:
* Data Directory: Absolute path to where data files should be stored
* Data Filename: Name for the output files, note that suffixes will be automatically appended -- _raw, _pn_fft, _final with numbers for multiple scans (e.g. filename_raw_0.txt). Files will be automatically overwritten if the same filename is used for subsequent runs. Output data is comma separated in the format: Frequency (cm^-1), Intensity (counts or arb.)
* Select a Spectrometer: Select a spectrometer from the list to use for this measurement
* Re-scan for spectrometers: Re-scans ports for attached spectrometers (if you attach a spectrometer once the program has started)
* \# of Measurement Repetitions: For performing multiple measurements sequentially
* PN Bit Length: How many bits should be used to generate the pseudorandom noise sequence? Generally more is better, but the improvement saturates
* Integration Time: How long should the spectrometer acquire a spectrum for (in milliseconds)?
* Modulation Frequency: How fast should the function generator adjust the laser power (in MHz)? Generally want this as fast as your function generator / electro-optic modulator can handle
* File output options: 
  * Raw Data saves the raw data from the spectrometer, generally useful for diagnosing the system and making sure things are working as expected
  * PN FFT Data is the Fourier Transform of the PN noise sequence, also generally useful for diagnosing issues with the system or with the code
  * Finalized Data is the point-wise multiplication of the above, and is generally the "actual" output from the measurement
* Start Scan: Unsurprisingly, starts a measurement. Entries in the other choices are fixed at the time the scan starts and changes will not be honored. Changes to "Stop Scan" while a measurement is in progress, to end it early. Note that it can only end a measurement after a complete measurement (that is, this only stops early if you have more than one Measurement Repetition(s))
* Scan Progress: Progress bar for the whole measurement (including all repetitions). Should always overestimate how much time remains


# Compilation
The application is built using [GTK3](https://www.gtk.org/) for the UI and [FFTW](http://www.fftw.org/) to perform Fourier Transforms. For the specific equipment we use, we also need the DAx-22000 library from [Wavepond](https://www.chase-scientific.com/wavepond.html) which contains all the necessary pieces on it's own. We also have a QE-Pro from Ocean Insight and communicate with it using the [Seabreeze API](https://www.oceaninsight.com/globalassets/catalog-blocks-and-images/software-downloads-installers/javadocs-api/seabreeze/html/index.html). For compilation on Windows, I used [Mingw-w64](http://mingw-w64.org/doku.php). GTK and FFTW have native mingw-w64-x86 packages and the Wavepond library "just worked" for me, but compiling the Seabreeze library required a few extra steps. In particular:
1. Install the mingw-w64 `libusb` and `libusb-compat` and then moved the installed `usb.h` file up one level into the main include folder in your mingw location
1. Inside the Seabreeze `common.mk` file, made the following changes:
  1. Remove -Werror as there are some errors which can be safely ignored that I wasn't in a position to fix
  1. Modified the Mingw-32 block to link against the correct libraries: `-lusb, -lstdc++, -lwsock32, -lm` and the installed version of mingw64 `-L/path/to/msys64/ming64/lib`
  1. Added line `CFLAGS_BASE += -D _WINDOWS -D BUILD_DLL` to indicate that we are on Windows and are building the DLL
1. Modify the root Makefile under `lib/libseabreeze.$(SUFFIX): initialize` to reorder some of the arguments as the order matters for `gcc/g++`: `$(CPP) lib/*.o $(LFLAGS_LIB) -o $@` 

# Customization
The code is designed to be relatively easy to adopt for a different combination of spectrometer (currently uses an Ocean Insight QE-Pro) and function generator (Wavepond DAx-14000). To do this, the code in `spectrometer_functions.c` and `waveform_gen.c` are the only places that should need to be changed to use a different API. As long as the replacement files provide the functions specified in `spectrometer_functions.h` and `waveform_gen.h` you can rewrite those files as needed.
