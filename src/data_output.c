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

// File to handle convoluting and outputting the data:

#include <stdio.h>

#include <gtk/gtk.h>

#include "acquire_data.h"
#include "data_output.h"

// May need to adjust to longer precision...
void write_line(FILE *filePtr, double x, double y)
{
  fprintf(filePtr, "%lf,%.15lf\n", x, y);
  return;
}

void output_data(int numPixels,
                 double xVals[],
                 double pixelValues[],
                 double pn_interp_fft[],
                 int iteration,
                 struct dataAcqParams *params)
{
g_print("Outputting data...\n");
  int i;
  // Get the information about which outputs / where they should go:
  struct dataOutputOpts *outputPtr = params->outputPtr;
  const gchar *baseFname = outputPtr->fname;
  const gchar *data_dir = outputPtr->data_dir;
  gchar *basePath = g_strjoin("/", data_dir, baseFname, NULL);
  gchar headerLines[500];
  sprintf(headerLines,
    "Data modulated at %d MHz with a PN code length of %d, and integrated for %d msec\nWavenumber (cm^-1), intensity\n",
     params->mod_freq, params->pn_bit_length, params->integrationTime);
g_print(baseFname);
g_print("\n");
g_print(data_dir);
g_print("\n");
g_print(basePath);
g_print("\n");

  // Output raw spectrometer data
  if (outputPtr->raw_data) {
    gchar iterText[10];
    sprintf(iterText, "%d", iteration);
    gchar *fullURI = g_strjoin(NULL, basePath, "_raw_", iterText, ".txt", NULL);
    gchar *fullPath = g_filename_from_uri(fullURI, NULL, NULL);

    FILE *outFile;
    outFile = fopen(fullPath, "w");

    for (i = 0; i < numPixels; i++) {
      write_line(outFile, xVals[i], pixelValues[i]);
    }

    g_free(fullPath);
    g_free(fullURI);
    fclose(outFile);
  }

  // Output our interpolated PN FFT
  if (outputPtr->pn_fft_data && iteration == 0) {
    // Only output this once, as it's constant as a function of iteration
    gchar *fullURI = g_strjoin(NULL, basePath, "_pn_fft.txt", NULL);
    gchar *fullPath = g_filename_from_uri(fullURI, NULL, NULL);
    FILE *outFile;
    outFile = fopen(fullPath, "w");
    fprintf(outFile, headerLines);

    for (i = 0; i < numPixels; i++) {
      write_line(outFile, xVals[i], pn_interp_fft[i]);
    }

    fclose(outFile);
    g_free(fullPath);
    g_free(fullURI);
  }

  // Output the point-wise multiplication of spectrum and PN FFT
  if (outputPtr->final_data) {
    gchar iterText[10];
    sprintf(iterText, "%d", iteration);
    gchar *fullURI = g_strjoin(NULL, basePath, "_final_", iterText, ".txt", NULL);
    gchar *fullPath = g_filename_from_uri(fullURI, NULL, NULL);

    FILE *outFile;
    outFile = fopen(fullPath, "w");
    fprintf(outFile, headerLines);

    for (i = 0; i < numPixels; i++) {
      double value = pixelValues[i] * pn_interp_fft[i]; // point-wise multiplication
      write_line(outFile, xVals[i], value);
    }
    g_free(fullPath);
    g_free(fullURI);
    fclose(outFile);
  }

  g_free(basePath);
  return;
}
