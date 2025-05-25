#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp8.h"

t_bmp8 *bmp8_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }

    t_bmp8 *img = (t_bmp8 *)malloc(sizeof(t_bmp8));
    if (!img) {
        fclose(file);
        return NULL;
    }

    // Read header
    if (fread(img->header, 1, 54, file) != 54) {
        printf("Error: Could not read header\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read color table
    if (fread(img->colorTable, 1, 1024, file) != 1024) {
        printf("Error: Could not read color table\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Extract image information from header
    img->width = *(unsigned int *)&img->header[18];
    img->height = *(unsigned int *)&img->header[22];
    img->colorDepth = *(unsigned int *)&img->header[28];
    // Calculate row size with padding
    int row_padded = (img->width + 3) & (~3);
    img->dataSize = row_padded * img->height;

    // Verify color depth
    if (img->colorDepth != 8) {
        printf("Error: Image is not 8-bit grayscale\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Allocate memory for image data (only pixel data, not padding)
    img->data = (unsigned char *)malloc(img->width * img->height);
    if (!img->data) {
        printf("Error: Could not allocate memory for image data\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read image data row by row, skipping padding
    for (unsigned int y = 0; y < img->height; y++) {
        unsigned char row_buf[row_padded];
        if (fread(row_buf, 1, row_padded, file) != row_padded) {
            printf("Error: Could not read image data row\n");
            free(img->data);
            free(img);
            fclose(file);
            return NULL;
        }
        memcpy(&img->data[y * img->width], row_buf, img->width);
    }

    fclose(file);
    return img;
}

void bmp8_saveImage(const char *filename, t_bmp8 *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not create file %s\n", filename);
        return;
    }

    // Write header
    fwrite(img->header, 1, 54, file);

    // Write color table
    fwrite(img->colorTable, 1, 1024, file);

    // Calculate row size with padding
    int row_padded = (img->width + 3) & (~3);
    unsigned char pad[4] = {0, 0, 0, 0};

    // Write image data row by row, adding padding
    for (unsigned int y = 0; y < img->height; y++) {
        fwrite(&img->data[y * img->width], 1, img->width, file);
        fwrite(pad, 1, row_padded - img->width, file);
    }

    fclose(file);
}

void bmp8_free(t_bmp8 *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}

void bmp8_printInfo(t_bmp8 *img) {
    printf("Image Info:\n");
    printf("Width: %u\n", img->width);
    printf("Height: %u\n", img->height);
    printf("Color Depth: %u\n", img->colorDepth);
    printf("Data Size: %u\n", img->dataSize);
}

void bmp8_negative(t_bmp8 *img) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8 *img, int value) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        int newValue = img->data[i] + value;
        if (newValue > 255) newValue = 255;
        if (newValue < 0) newValue = 0;
        img->data[i] = (unsigned char)newValue;
    }
}

void bmp8_threshold(t_bmp8 *img, int threshold) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}

void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize) {
    if (!img || !img->data || !kernel || kernelSize % 2 == 0) return;

    int offset = kernelSize / 2;
    unsigned char *newData = (unsigned char *)malloc(img->width * img->height);
    if (!newData) {
        printf("Error: Could not allocate memory for filtered data\n");
        return;
    }

    // Copy original data to new buffer
    memcpy(newData, img->data, img->width * img->height);

    // Apply filter to each pixel (ignore padding, only process pixel data)
    for (unsigned int y = 0; y < img->height; y++) {
        for (unsigned int x = 0; x < img->width; x++) {
            float sum = 0.0f;
            for (int ky = 0; ky < kernelSize; ky++) {
                for (int kx = 0; kx < kernelSize; kx++) {
                    int pixelX = x + kx - kernelSize/2;
                    int pixelY = y + ky - kernelSize/2;
                    // Handle edge cases
                    if (pixelX < 0) pixelX = 0;
                    if (pixelY < 0) pixelY = 0;
                    if (pixelX >= (int)img->width) pixelX = img->width - 1;
                    if (pixelY >= (int)img->height) pixelY = img->height - 1;
                    sum += img->data[pixelY * img->width + pixelX] * kernel[ky][kx];
                }
            }
            // Clamp and store result
            int result = (int)(sum + 0.5f);
            if (result < 0) result = 0;
            if (result > 255) result = 255;
            newData[y * img->width + x] = (unsigned char)result;
        }
    }

    // Copy filtered data back to image
    memcpy(img->data, newData, img->width * img->height);
    free(newData);
}

unsigned int *bmp8_computeHistogram(t_bmp8 *img) {
    unsigned int *hist = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (!hist) return NULL;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        hist[img->data[i]]++;
    }

    return hist;
}

unsigned int *bmp8_computeCDF(unsigned int *hist) {
    unsigned int *cdf = (unsigned int *)malloc(256 * sizeof(unsigned int));
    if (!cdf) return NULL;

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + hist[i];
    }

    return cdf;
}

void bmp8_equalize(t_bmp8 *img, unsigned int *hist_eq) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = hist_eq[img->data[i]];
    }
} 