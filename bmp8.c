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
    img->dataSize = *(unsigned int *)&img->header[34];

    // Verify color depth
    if (img->colorDepth != 8) {
        printf("Error: Image is not 8-bit grayscale\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Allocate memory for image data
    img->data = (unsigned char *)malloc(img->dataSize);
    if (!img->data) {
        printf("Error: Could not allocate memory for image data\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Read image data
    if (fread(img->data, 1, img->dataSize, file) != img->dataSize) {
        printf("Error: Could not read image data\n");
        free(img->data);
        free(img);
        fclose(file);
        return NULL;
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

    // Write image data
    fwrite(img->data, 1, img->dataSize, file);

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
    int offset = kernelSize / 2;
    unsigned char *newData = (unsigned char *)malloc(img->dataSize);
    if (!newData) return;

    for (unsigned int y = offset; y < img->height - offset; y++) {
        for (unsigned int x = offset; x < img->width - offset; x++) {
            float sum = 0;
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    sum += img->data[(y + ky) * img->width + (x + kx)] * 
                           kernel[ky + offset][kx + offset];
                }
            }
            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;
            newData[y * img->width + x] = (unsigned char)sum;
        }
    }

    memcpy(img->data, newData, img->dataSize);
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