/*
 * bmp8.h
 * Author: Simon Hillel
 * Description: Header for 8-bit grayscale BMP image handling and processing functions.
 * Declares structures and functions for loading, saving, manipulating, and filtering 8-bit BMP images.
 * This file is a core part of the image processing project, supporting grayscale image operations.
 */
#ifndef BMP8_H
#define BMP8_H

#include <stdint.h>

// Structure for 8-bit grayscale BMP image
typedef struct {
    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char *data;
    unsigned int width;
    unsigned int height;
    unsigned int colorDepth;
    unsigned int dataSize;
} t_bmp8;

// Function prototypes
/**
 * Loads an 8-bit grayscale BMP image from a file.
 */
t_bmp8 *bmp8_loadImage(const char *filename);
/**
 * Saves an 8-bit grayscale BMP image to a file.
 */
void bmp8_saveImage(const char *filename, t_bmp8 *img);
/**
 * Frees a t_bmp8 structure and its associated pixel data.
 */
void bmp8_free(t_bmp8 *img);
/**
 * Prints information about an 8-bit grayscale BMP image.
 */
void bmp8_printInfo(t_bmp8 *img);
/**
 * Applies a negative filter to an 8-bit grayscale BMP image.
 */
void bmp8_negative(t_bmp8 *img);
/**
 * Adjusts the brightness of an 8-bit grayscale BMP image.
 */
void bmp8_brightness(t_bmp8 *img, int value);
/**
 * Applies a threshold filter to an 8-bit grayscale BMP image.
 */
void bmp8_threshold(t_bmp8 *img, int threshold);
/**
 * Applies a convolution filter to an 8-bit grayscale BMP image using a given kernel.
 */
void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize);

// Part 3: Histogram Equalization (Grayscale)
/**
 * Computes the histogram of an 8-bit grayscale BMP image.
 */
unsigned int *bmp8_computeHistogram(t_bmp8 *img);
/**
 * Computes the cumulative distribution function (CDF) from a histogram.
 */
unsigned int *bmp8_computeCDF(unsigned int *hist);
/**
 * Applies histogram equalization to an 8-bit grayscale BMP image.
 */
void bmp8_equalize(t_bmp8 *img, unsigned int *hist_eq);

#endif // BMP8_H 