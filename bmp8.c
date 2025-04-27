#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp8.h"

t_bmp8 * bmp8_loadImage(const char * filename) {
    FILE * file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }

    t_bmp8 * img = (t_bmp8 *)malloc(sizeof(t_bmp8));
    if (!img) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read header
    if (fread(img->header, 1, 54, file) != 54) {
        printf("Error: Invalid BMP file format\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Check if it's a valid BMP file
    if (img->header[0] != 'B' || img->header[1] != 'M') {
        printf("Error: Not a valid BMP file\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Extract image information from header
    img->width = *(unsigned int *)&img->header[18];
    img->height = *(unsigned int *)&img->header[22];
    img->colorDepth = *(unsigned short *)&img->header[28];
    img->dataSize = *(unsigned int *)&img->header[34];

    // Check if it's an 8-bit grayscale image
    if (img->colorDepth != 8) {
        printf("Error: Not an 8-bit grayscale image\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read color table
    if (fread(img->colorTable, 1, 1024, file) != 1024) {
        printf("Error: Invalid color table\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Allocate memory for image data
    img->data = (unsigned char *)malloc(img->dataSize);
    if (!img->data) {
        printf("Error: Memory allocation failed for image data\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read image data
    if (fread(img->data, 1, img->dataSize, file) != img->dataSize) {
        printf("Error: Failed to read image data\n");
        free(img->data);
        free(img);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return img;
}

void bmp8_saveImage(const char * filename, t_bmp8 * img) {
    FILE * file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Cannot create file %s\n", filename);
        return;
    }

    // Write header
    if (fwrite(img->header, 1, 54, file) != 54) {
        printf("Error: Failed to write header\n");
        fclose(file);
        return;
    }

    // Write color table
    if (fwrite(img->colorTable, 1, 1024, file) != 1024) {
        printf("Error: Failed to write color table\n");
        fclose(file);
        return;
    }

    // Write image data
    if (fwrite(img->data, 1, img->dataSize, file) != img->dataSize) {
        printf("Error: Failed to write image data\n");
        fclose(file);
        return;
    }

    fclose(file);
}

void bmp8_free(t_bmp8 * img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}

void bmp8_printInfo(t_bmp8 * img) {
    if (!img) {
        printf("Error: Invalid image pointer\n");
        return;
    }

    printf("Image Info:\n");
    printf("Width: %u\n", img->width);
    printf("Height: %u\n", img->height);
    printf("Color Depth: %u\n", img->colorDepth);
    printf("Data Size: %u\n", img->dataSize);
}

void bmp8_negative(t_bmp8 * img) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8 * img, int value) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        int newValue = img->data[i] + value;
        if (newValue > 255) newValue = 255;
        if (newValue < 0) newValue = 0;
        img->data[i] = (unsigned char)newValue;
    }
}

void bmp8_threshold(t_bmp8 * img, int threshold) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}

void bmp8_applyFilter(t_bmp8 * img, float ** kernel, int kernelSize) {
    if (!img || !img->data || !kernel || kernelSize % 2 == 0) return;

    int n = kernelSize / 2;
    unsigned char * tempData = (unsigned char *)malloc(img->dataSize);
    if (!tempData) return;

    memcpy(tempData, img->data, img->dataSize);

    for (unsigned int y = 1; y < img->height - 1; y++) {
        for (unsigned int x = 1; x < img->width - 1; x++) {
            float sum = 0.0f;
            for (int ky = -n; ky <= n; ky++) {
                for (int kx = -n; kx <= n; kx++) {
                    int pixelX = x + kx;
                    int pixelY = y + ky;
                    sum += tempData[pixelY * img->width + pixelX] * kernel[ky + n][kx + n];
                }
            }

            // Clamp the result to [0, 255]
            int result = (int)sum;
            if (result > 255) result = 255;
            if (result < 0) result = 0;
            img->data[y * img->width + x] = (unsigned char)result;
        }
    }

    free(tempData);
}

// --- Part 3: Histogram Equalization (Grayscale) --- //

unsigned int * bmp8_computeHistogram(t_bmp8 * img) {
    if (!img || !img->data) return NULL;

    unsigned int * histogram = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (!histogram) {
        printf("Error: Memory allocation failed for histogram\n");
        return NULL;
    }

    for (unsigned int i = 0; i < img->dataSize; i++) {
        histogram[img->data[i]]++;
    }

    return histogram;
}

unsigned int * bmp8_computeCDF(unsigned int * hist, unsigned int numPixels) {
    if (!hist || numPixels == 0) return NULL;

    unsigned int * cdf = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (!cdf) {
        printf("Error: Memory allocation failed for CDF\n");
        return NULL;
    }

    // Calculate cumulative histogram
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // Find the minimum non-zero CDF value
    unsigned int cdf_min = 0; // Initialize with 0
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) { // Find the first non-zero value
            cdf_min = cdf[i];
            break;
        }
    }

    // If all pixels have the same value (cdf_min == numPixels), or if cdf_min is 0 (empty image?)
    // Avoid division by zero. In this case, the equalization doesn't change the image.
    // Return a mapping where each intensity maps to itself or handle as appropriate.
    // Here, we just return the CDF calculated so far, which will result in no change later.
    if (numPixels - cdf_min == 0) {
        // Create identity mapping to avoid division by zero
        for (int i = 0; i < 256; i++) {
             cdf[i] = i;
        }
        return cdf;
    }

    // Calculate normalized cumulative histogram (hist_eq in description)
    unsigned int * hist_eq = (unsigned int *)malloc(256 * sizeof(unsigned int));
    if (!hist_eq) {
        printf("Error: Memory allocation failed for hist_eq\n");
        free(cdf);
        return NULL;
    }

    double scale_factor = 255.0 / (double)(numPixels - cdf_min);
    for (int i = 0; i < 256; i++) {
        if (cdf[i] == 0) { // If this intensity level doesn't exist, map it to 0
            hist_eq[i] = 0;
        } else {
            // Apply the formula: hist_eq[i] = round(((cdf[i] - cdf_min) / (N - cdf_min)) * 255)
            double normalized_value = round(((double)(cdf[i] - cdf_min)) * scale_factor);
            hist_eq[i] = (unsigned int)normalized_value;

            // Clamp just in case of rounding issues, although theoretically unnecessary if cdf_min is handled correctly
            if (hist_eq[i] > 255) hist_eq[i] = 255;
        }
    }

    free(cdf); // Free the intermediate CDF array
    return hist_eq;
}

void bmp8_equalize(t_bmp8 * img) {
    if (!img || !img->data) return;

    unsigned int numPixels = img->width * img->height;
    if (numPixels == 0) return; // Empty image

    unsigned int * hist = bmp8_computeHistogram(img);
    if (!hist) return;

    unsigned int * hist_eq = bmp8_computeCDF(hist, numPixels);
    if (!hist_eq) {
        free(hist);
        return;
    }

    // Apply the equalization mapping
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (unsigned char)hist_eq[img->data[i]];
    }

    free(hist);
    free(hist_eq);
} 