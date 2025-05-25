#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../bmp24.h"
#include "bmp8.h" // Need this for grayscale equalization functions

/*
 * bmp24.c
 * Author: Simon Hillel
 * Description: Implementation of 24-bit BMP image handling and processing functions.
 * This file provides functions for loading, saving, manipulating, and filtering 24-bit BMP images.
 * It is a core part of the image processing project, supporting color image operations.
 */

// Function declarations
void freeKernel(float **kernel, int size);

// --- Part 2: Allocation and Deallocation --- //

/**
 * Allocates a 2D array of t_pixel pointers for image pixel data.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @return Pointer to the allocated 2D pixel array, or NULL on failure.
 */
t_pixel ** bmp24_allocateDataPixels(int width, int height) {
    if (width <= 0 || height <= 0) return NULL;

    t_pixel ** pixels = (t_pixel **)malloc(height * sizeof(t_pixel *));
    if (!pixels) {
        printf("Error: Memory allocation failed for pixel rows\n");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel *)malloc(width * sizeof(t_pixel));
        if (!pixels[i]) {
            printf("Error: Memory allocation failed for pixel row %d\n", i);
            // Free previously allocated rows
            for (int j = 0; j < i; j++) {
                free(pixels[j]);
            }
            free(pixels);
            return NULL;
        }
        // Optional: Initialize pixels to black or zero
        // memset(pixels[i], 0, width * sizeof(t_pixel));
    }
    return pixels;
}

/**
 * Frees a 2D array of t_pixel pointers allocated for image pixel data.
 * @param pixels The 2D pixel array to free.
 * @param height The height of the image (number of rows).
 */
void bmp24_freeDataPixels(t_pixel ** pixels, int height) {
    if (!pixels) return;
    for (int i = 0; i < height; i++) {
        if (pixels[i]) {
            free(pixels[i]);
        }
    }
    free(pixels);
}

/**
 * Allocates a t_bmp24 structure and its pixel data for a 24-bit BMP image.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param colorDepth The color depth (should be 24 for 24-bit images).
 * @return Pointer to the allocated t_bmp24 structure, or NULL on failure.
 */
t_bmp24 * bmp24_allocate(int width, int height, int colorDepth) {
    if (width <= 0 || height <= 0) return NULL;

    t_bmp24 * img = (t_bmp24 *)malloc(sizeof(t_bmp24));
    if (!img) {
        printf("Error: Memory allocation failed for t_bmp24 structure\n");
        return NULL;
    }

    img->data = bmp24_allocateDataPixels(width, height);
    if (!img->data) {
        free(img);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;

    // Initialize headers (can be filled properly during load/save)
    memset(&img->header, 0, sizeof(t_bmp_header));
    memset(&img->header_info, 0, sizeof(t_bmp_info));

    return img;
}

/**
 * Frees a t_bmp24 structure and its associated pixel data.
 * @param img Pointer to the t_bmp24 structure to free.
 */
void bmp24_free(t_bmp24 * img) {
    if (img) {
        bmp24_freeDataPixels(img->data, img->height);
        free(img);
    }
}

// --- Part 2: File I/O Helpers (Provided in description) --- //

/**
 * Reads raw data from a file at a specified position.
 * @param position The position in the file to start reading from.
 * @param buffer The buffer to read data into.
 * @param size The size of each element to read.
 * @param n The number of elements to read.
 * @param file The file pointer.
 */
void file_rawRead (uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file) {
    if(fseek(file, position, SEEK_SET) != 0) {
        printf("Error: Failed to seek to position %u\n", position);
        // Optional: Add more robust error handling, maybe exit or return error code
    }
    if(fread(buffer, size, n, file) != n) {
       printf("Error: Failed to read %zu elements of size %u at position %u\n", n, size, position);
       // Optional: Add more robust error handling
    }
}

/**
 * Writes raw data to a file at a specified position.
 * @param position The position in the file to start writing to.
 * @param buffer The buffer to write data from.
 * @param size The size of each element to write.
 * @param n The number of elements to write.
 * @param file The file pointer.
 */
void file_rawWrite (uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file) {
    if(fseek(file, position, SEEK_SET) != 0) {
         printf("Error: Failed to seek to position %u\n", position);
         // Optional: Add more robust error handling
    }
    if(fwrite(buffer, size, n, file) != n) {
        printf("Error: Failed to write %zu elements of size %u at position %u\n", n, size, position);
        // Optional: Add more robust error handling
    }
}

// --- Part 2: Pixel Data Read/Write --- //

/**
 * Reads pixel data from a BMP file into a t_bmp24 structure.
 * Handles row padding and bottom-up storage.
 * @param image Pointer to the t_bmp24 structure.
 * @param file The file pointer.
 */
void bmp24_readPixelData(t_bmp24 * image, FILE * file) {
    if (!image || !image->data || !file) return;

    int width = image->width;
    int height = image->height;
    uint32_t dataOffset = image->header.offset;

    // Calculate padding per row
    // Each row must be a multiple of 4 bytes
    int row_padded_size = (width * 3 + 3) & (~3);
    int padding = row_padded_size - (width * 3);

    // BMP stores rows bottom-up
    fseek(file, dataOffset, SEEK_SET);
    for (int y = height - 1; y >= 0; y--) {
        // Read a row of pixel data (BGR)
        size_t read_count = fread(image->data[y], sizeof(t_pixel), width, file);
        if (read_count != width) {
            printf("Error: Failed to read pixel data for row %d. Expected %d, got %zu\n", y, width, read_count);
            // Handle error: maybe stop, or fill rest with black?
            // For now, just print error and continue
        }

        // Skip padding bytes if any
        if (padding > 0) {
            fseek(file, padding, SEEK_CUR);
        }
    }
}

/**
 * Writes pixel data from a t_bmp24 structure to a BMP file.
 * Handles row padding and bottom-up storage.
 * @param image Pointer to the t_bmp24 structure.
 * @param file The file pointer.
 */
void bmp24_writePixelData(t_bmp24 * image, FILE * file) {
    if (!image || !image->data || !file) return;

    int width = image->width;
    int height = image->height;
    uint32_t dataOffset = image->header.offset;

    // Calculate padding per row
    int row_padded_size = (width * 3 + 3) & (~3);
    int padding = row_padded_size - (width * 3);
    unsigned char pad_bytes[3] = {0, 0, 0}; // Buffer for padding bytes

    // BMP stores rows bottom-up
    fseek(file, dataOffset, SEEK_SET);
    for (int y = height - 1; y >= 0; y--) {
        // Write a row of pixel data (BGR)
        size_t write_count = fwrite(image->data[y], sizeof(t_pixel), width, file);
         if (write_count != width) {
            printf("Error: Failed to write pixel data for row %d. Expected %d, got %zu\n", y, width, write_count);
            // Handle error
        }

        // Write padding bytes if any
        if (padding > 0) {
            fwrite(pad_bytes, 1, padding, file);
        }
    }
}

// --- Placeholders for remaining functions --- //
// (Will be implemented in subsequent steps)

/**
 * Loads a 24-bit BMP image from a file.
 * Supports classic and extended BMP headers (40, 108, 124 bytes).
 * @param filename The path to the BMP file.
 * @return Pointer to the loaded t_bmp24 structure, or NULL on failure.
 */
t_bmp24 * bmp24_loadImage(const char * filename) {
    FILE * file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }

    // Read header fields individually to avoid structure packing issues
    t_bmp_header header;
    file_rawRead(BITMAP_MAGIC_OFFSET, &header.type, sizeof(header.type), 1, file);
    file_rawRead(BITMAP_SIZE_OFFSET, &header.size, sizeof(header.size), 1, file);
    file_rawRead(BITMAP_OFFSET_OFFSET, &header.offset, sizeof(header.offset), 1, file);
    // Note: Skipping reserved fields

    if (header.type != BMP_TYPE) {
        printf("Error: Not a valid BMP file (magic number mismatch)\n");
        fclose(file);
        return NULL;
    }

    // Read info header fields individually
    t_bmp_info header_info;
    file_rawRead(BITMAP_INFO_SIZE_OFFSET, &header_info.size, sizeof(header_info.size), 1, file);
    file_rawRead(BITMAP_WIDTH_OFFSET, &header_info.width, sizeof(header_info.width), 1, file);
    file_rawRead(BITMAP_HEIGHT_OFFSET, &header_info.height, sizeof(header_info.height), 1, file);
    file_rawRead(BITMAP_PLANES_OFFSET, &header_info.planes, sizeof(header_info.planes), 1, file);
    file_rawRead(BITMAP_DEPTH_OFFSET, &header_info.bits, sizeof(header_info.bits), 1, file);
    file_rawRead(BITMAP_COMPRESSION_OFFSET, &header_info.compression, sizeof(header_info.compression), 1, file);
    file_rawRead(BITMAP_SIZE_RAW_OFFSET, &header_info.imagesize, sizeof(header_info.imagesize), 1, file);
     // Note: Skipping resolution and color count fields for now

    // Basic validation
    if (header_info.size < BMP_INFOHEADER_SIZE) {
        printf("Error: Unsupported BMP info header size (%u)\n", header_info.size);
        fclose(file);
        return NULL;
    } else if (header_info.size > BMP_INFOHEADER_SIZE) {
        // Skip the extra header bytes
        fseek(file, header_info.size - BMP_INFOHEADER_SIZE, SEEK_CUR);
    }
    if (header_info.bits != 24) {
        printf("Error: Not a 24-bit BMP image (color depth is %u)\n", header_info.bits);
        fclose(file);
        return NULL;
    }
    if (header_info.compression != 0) {
        printf("Error: Compressed BMP files are not supported\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for the image structure
    t_bmp24 * img = bmp24_allocate(header_info.width, header_info.height, header_info.bits);
    if (!img) {
        fclose(file);
        return NULL;
    }

    // Copy header information into the structure
    img->header = header;
    img->header_info = header_info;
    img->width = header_info.width;
    img->height = header_info.height;
    img->colorDepth = header_info.bits;

    // Read pixel data
    bmp24_readPixelData(img, file);

    fclose(file);
    return img;
}

/**
 * Saves a 24-bit BMP image to a file.
 * @param img Pointer to the t_bmp24 structure to save.
 * @param filename The path to the output BMP file.
 */
void bmp24_saveImage(t_bmp24 * img, const char * filename) {
    if (!img || !img->data) {
        printf("Error: Cannot save NULL image\n");
        return;
    }

    FILE * file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Cannot create file %s\n", filename);
        return;
    }

    int width = img->width;
    int height = img->height;
    int row_padded_size = (width * 3 + 3) & (~3);
    uint32_t data_size = row_padded_size * height;
    uint32_t file_size = BMP_HEADER_SIZE + BMP_INFOHEADER_SIZE + data_size;
    uint32_t data_offset = BMP_HEADER_SIZE + BMP_INFOHEADER_SIZE;

    // Prepare headers
    t_bmp_header header = img->header; // Start with existing header info
    header.type = BMP_TYPE;
    header.size = file_size;
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offset = data_offset;

    t_bmp_info header_info = img->header_info; // Start with existing info
    header_info.size = BMP_INFOHEADER_SIZE;
    header_info.width = width;
    header_info.height = height;
    header_info.planes = 1;
    header_info.bits = 24;
    header_info.compression = 0;
    header_info.imagesize = data_size; // Size of raw bitmap data
    header_info.xresolution = 0; // Default resolution
    header_info.yresolution = 0;
    header_info.ncolors = 0; // Not using palette
    header_info.importantcolors = 0;

    // Write headers field by field
    file_rawWrite(BITMAP_MAGIC_OFFSET, &header.type, sizeof(header.type), 1, file);
    file_rawWrite(BITMAP_SIZE_OFFSET, &header.size, sizeof(header.size), 1, file);
    // reserved fields are skipped (assumed 0)
    file_rawWrite(BITMAP_OFFSET_OFFSET, &header.offset, sizeof(header.offset), 1, file);

    file_rawWrite(BITMAP_INFO_SIZE_OFFSET, &header_info.size, sizeof(header_info.size), 1, file);
    file_rawWrite(BITMAP_WIDTH_OFFSET, &header_info.width, sizeof(header_info.width), 1, file);
    file_rawWrite(BITMAP_HEIGHT_OFFSET, &header_info.height, sizeof(header_info.height), 1, file);
    file_rawWrite(BITMAP_PLANES_OFFSET, &header_info.planes, sizeof(header_info.planes), 1, file);
    file_rawWrite(BITMAP_DEPTH_OFFSET, &header_info.bits, sizeof(header_info.bits), 1, file);
    file_rawWrite(BITMAP_COMPRESSION_OFFSET, &header_info.compression, sizeof(header_info.compression), 1, file);
    file_rawWrite(BITMAP_SIZE_RAW_OFFSET, &header_info.imagesize, sizeof(header_info.imagesize), 1, file);
    file_rawWrite(BITMAP_XRES_OFFSET, &header_info.xresolution, sizeof(header_info.xresolution), 1, file);
    file_rawWrite(BITMAP_YRES_OFFSET, &header_info.yresolution, sizeof(header_info.yresolution), 1, file);
    file_rawWrite(BITMAP_NCOLORS_OFFSET, &header_info.ncolors, sizeof(header_info.ncolors), 1, file);
    file_rawWrite(BITMAP_IMPORTANTCOLORS_OFFSET, &header_info.importantcolors, sizeof(header_info.importantcolors), 1, file);

    // Write pixel data
    bmp24_writePixelData(img, file);

    fclose(file);
}

/**
 * Prints information about a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_printInfo(t_bmp24 * img) {
     if (!img) {
        printf("Error: Invalid image pointer\n");
        return;
    }

    printf("Image Info (24-bit BMP):\n");
    printf("  Width: %d\n", img->width);
    printf("  Height: %d\n", img->height);
    printf("  Color Depth: %d\n", img->colorDepth);
    printf("  File Size: %u bytes\n", img->header.size);
    printf("  Data Offset: %u bytes\n", img->header.offset);
    printf("  Compression: %u\n", img->header_info.compression);
    printf("  Image Data Size: %u bytes\n", img->header_info.imagesize);
}

// --- Part 2: Image Processing --- //

/**
 * Applies a negative filter to a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_negative(t_bmp24 * img) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel * p = &img->data[y][x];
            p->red = 255 - p->red;
            p->green = 255 - p->green;
            p->blue = 255 - p->blue;
        }
    }
}

/**
 * Converts a 24-bit BMP image to grayscale in-place.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_grayscale(t_bmp24 * img) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel * p = &img->data[y][x];
            // Calculate average - using integer arithmetic for simplicity
            // Could use floating point for slightly more accuracy: (p->red + p->green + p->blue) / 3.0
            uint8_t gray = (uint8_t)(((unsigned int)p->red + p->green + p->blue) / 3);
            p->red = gray;
            p->green = gray;
            p->blue = gray;
        }
    }
}

/**
 * Adjusts the brightness of a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 * @param value The brightness adjustment value (-255 to 255).
 */
void bmp24_brightness(t_bmp24 * img, int value) {
     if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel * p = &img->data[y][x];
            int newRed = p->red + value;
            int newGreen = p->green + value;
            int newBlue = p->blue + value;

            // Clamp values to 0-255 range
            p->red = (uint8_t)((newRed > 255) ? 255 : (newRed < 0 ? 0 : newRed));
            p->green = (uint8_t)((newGreen > 255) ? 255 : (newGreen < 0 ? 0 : newGreen));
            p->blue = (uint8_t)((newBlue > 255) ? 255 : (newBlue < 0 ? 0 : newBlue));
        }
    }
}

// --- Part 2: Convolution Filters --- //

/**
 * Clamps a float value to the range [0, 255] and returns as uint8_t.
 * @param value The value to clamp.
 * @return The clamped value as uint8_t.
 */
uint8_t clamp_uint8(float value) {
    if (value > 255.0f) return 255;
    if (value < 0.0f) return 0;
    return (uint8_t)round(value);
}

/**
 * Applies a convolution kernel to a pixel in a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 * @param tempData Temporary copy of the image data.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param kernel The convolution kernel.
 * @param kernelSize The size of the kernel (must be odd).
 * @return The resulting t_pixel after convolution.
 */
t_pixel bmp24_convolution(t_bmp24 * img, t_pixel ** tempData, int x, int y, float ** kernel, int kernelSize) {
    t_pixel result = {0, 0, 0};
    if (!img || !tempData || !kernel || kernelSize % 2 == 0) return result;

    int n = kernelSize / 2;
    float sum_red = 0.0f, sum_green = 0.0f, sum_blue = 0.0f;

    // Apply kernel
    for (int ky = -n; ky <= n; ky++) {
        for (int kx = -n; kx <= n; kx++) {
            int pixelX = x + kx;
            int pixelY = y + ky;

            // We are already iterating only over valid (non-edge) pixels in bmp24_applyFilter
            // so bounds checking here might be redundant if called from there.
            // However, if called directly, it might be needed.
            // Assuming called from bmp24_applyFilter which handles boundaries:
            t_pixel neighbor = tempData[pixelY][pixelX];
            float kernel_val = kernel[ky + n][kx + n];

            sum_red += neighbor.red * kernel_val;
            sum_green += neighbor.green * kernel_val;
            sum_blue += neighbor.blue * kernel_val;
        }
    }

    // Clamp results
    result.red = clamp_uint8(sum_red);
    result.green = clamp_uint8(sum_green);
    result.blue = clamp_uint8(sum_blue);

    return result;
}

/**
 * Applies a convolution filter to a 24-bit BMP image using a given kernel.
 * @param img Pointer to the t_bmp24 structure.
 * @param kernel The convolution kernel.
 * @param kernelSize The size of the kernel (must be odd).
 */
void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    if (!img || !img->data || !kernel || kernelSize % 2 == 0) {
        printf("Error: Invalid parameters for filter application\n");
        return;
    }

    int width = img->width;
    int height = img->height;
    int n = kernelSize / 2;

    // Allocate temporary data buffer to store original pixel values
    t_pixel **tempData = bmp24_allocateDataPixels(width, height);
    if (!tempData) {
        printf("Error: Failed to allocate temporary buffer for filtering\n");
        return;
    }

    // Copy original data to temporary buffer
    for (int y = 0; y < height; y++) {
        memcpy(tempData[y], img->data[y], width * sizeof(t_pixel));
    }

    // Apply convolution to each pixel
    for (int y = n; y < height - n; y++) {
        for (int x = n; x < width - n; x++) {
            float sum_red = 0.0f, sum_green = 0.0f, sum_blue = 0.0f;

            // Apply kernel
            for (int ky = -n; ky <= n; ky++) {
                for (int kx = -n; kx <= n; kx++) {
                    t_pixel neighbor = tempData[y + ky][x + kx];
                    float kernel_val = kernel[ky + n][kx + n];

                    sum_red += neighbor.red * kernel_val;
                    sum_green += neighbor.green * kernel_val;
                    sum_blue += neighbor.blue * kernel_val;
                }
            }

            // Clamp results
            img->data[y][x].red = clamp_uint8(sum_red);
            img->data[y][x].green = clamp_uint8(sum_green);
            img->data[y][x].blue = clamp_uint8(sum_blue);
        }
    }

    // Free temporary buffer
    bmp24_freeDataPixels(tempData, height);
}

// --- Kernel Creation/Freeing Functions --- //
// (Add comments for each kernel function and freeKernel)

/**
 * Creates a 3x3 box blur kernel for smoothing images.
 * @return Pointer to the allocated kernel, or NULL on failure.
 */
float ** createBoxBlurKernel() {
    float ** kernel = (float **)malloc(3 * sizeof(float *));
    if (!kernel) {
        printf("Error: Kernel allocation failed (rows)\n");
        return NULL;
    }
    for (int i = 0; i < 3; i++) {
        kernel[i] = (float *)malloc(3 * sizeof(float));
        if (!kernel[i]) {
            printf("Error: Kernel allocation failed (row %d)\n", i);
            freeKernel(kernel, i); // Free previously allocated rows
            return NULL;
        }
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = 1.0f / 9.0f;
        }
    }
    return kernel;
}

/**
 * Creates a 3x3 Gaussian blur kernel for soft blurring.
 * @return Pointer to the allocated kernel, or NULL on failure.
 */
float ** createGaussianBlurKernel() {
    float ** kernel = (float **)malloc(3 * sizeof(float *));
    if (!kernel) {
        printf("Error: Kernel allocation failed (rows)\n");
        return NULL;
    }
    float values[3][3] = {
        {1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f},
        {2.0f/16.0f, 4.0f/16.0f, 2.0f/16.0f},
        {1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f}
    };
    for (int i = 0; i < 3; i++) {
        kernel[i] = (float *)malloc(3 * sizeof(float));
        if (!kernel[i]) {
            printf("Error: Kernel allocation failed (row %d)\n", i);
            freeKernel(kernel, i);
            return NULL;
        }
        memcpy(kernel[i], values[i], 3 * sizeof(float));
    }
    return kernel;
}

/**
 * Creates a 3x3 outline kernel for edge detection.
 * @return Pointer to the allocated kernel, or NULL on failure.
 */
float ** createOutlineKernel() {
    float ** kernel = (float **)malloc(3 * sizeof(float *));
    if (!kernel) {
        printf("Error: Kernel allocation failed (rows)\n");
        return NULL;
    }
     float values[3][3] = {
        {-1.0f, -1.0f, -1.0f},
        {-1.0f,  8.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f}
    };
    for (int i = 0; i < 3; i++) {
        kernel[i] = (float *)malloc(3 * sizeof(float));
         if (!kernel[i]) {
            printf("Error: Kernel allocation failed (row %d)\n", i);
            freeKernel(kernel, i);
            return NULL;
        }
         memcpy(kernel[i], values[i], 3 * sizeof(float));
    }
    return kernel;
}

/**
 * Creates a 3x3 emboss kernel for embossing effect.
 * @return Pointer to the allocated kernel, or NULL on failure.
 */
float ** createEmbossKernel() {
    float ** kernel = (float **)malloc(3 * sizeof(float *));
    if (!kernel) {
        printf("Error: Kernel allocation failed (rows)\n");
        return NULL;
    }
    float values[3][3] = {
        {-2.0f, -1.0f,  0.0f},
        {-1.0f,  1.0f,  1.0f},
        { 0.0f,  1.0f,  2.0f}
    };
     for (int i = 0; i < 3; i++) {
        kernel[i] = (float *)malloc(3 * sizeof(float));
        if (!kernel[i]) {
            printf("Error: Kernel allocation failed (row %d)\n", i);
            freeKernel(kernel, i);
            return NULL;
        }
         memcpy(kernel[i], values[i], 3 * sizeof(float));
    }
    return kernel;
}

/**
 * Creates a 3x3 sharpen kernel for sharpening images.
 * @return Pointer to the allocated kernel, or NULL on failure.
 */
float ** createSharpenKernel() {
    float ** kernel = (float **)malloc(3 * sizeof(float *));
    if (!kernel) {
        printf("Error: Kernel allocation failed (rows)\n");
        return NULL;
    }
    float values[3][3] = {
        { 0.0f, -1.0f,  0.0f},
        {-1.0f,  5.0f, -1.0f},
        { 0.0f, -1.0f,  0.0f}
    };
     for (int i = 0; i < 3; i++) {
        kernel[i] = (float *)malloc(3 * sizeof(float));
        if (!kernel[i]) {
            printf("Error: Kernel allocation failed (row %d)\n", i);
            freeKernel(kernel, i);
            return NULL;
        }
         memcpy(kernel[i], values[i], 3 * sizeof(float));
    }
    return kernel;
}

/**
 * Frees a dynamically allocated convolution kernel.
 * @param kernel The kernel to free.
 * @param size The size of the kernel (number of rows).
 */
void freeKernel(float ** kernel, int size) {
    if (!kernel) return;
    for (int i = 0; i < size; i++) {
        if (kernel[i]) free(kernel[i]);
    }
    free(kernel);
}

// Apply specific filters by creating the kernel and calling bmp24_applyFilter

/**
 * Applies a box blur filter to a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_boxBlur(t_bmp24 * img) {
    float ** kernel = createBoxBlurKernel();
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

/**
 * Applies a Gaussian blur filter to a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_gaussianBlur(t_bmp24 * img) {
     float ** kernel = createGaussianBlurKernel();
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

/**
 * Applies an outline filter to a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_outline(t_bmp24 * img) {
     float ** kernel = createOutlineKernel();
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

/**
 * Applies an emboss filter to a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_emboss(t_bmp24 * img) {
     float ** kernel = createEmbossKernel();
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

/**
 * Applies a sharpen filter to a 24-bit BMP image.
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_sharpen(t_bmp24 * img) {
     float ** kernel = createSharpenKernel();
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

// --- Part 3: Histogram Equalization (Color) --- //

/**
 * Converts an RGB pixel to YUV color space.
 * @param p The RGB pixel.
 * @return The corresponding YUV pixel.
 */
t_yuv rgb_to_yuv(t_pixel p) {
    t_yuv yuv;
    double r = p.red;
    double g = p.green;
    double b = p.blue;

    yuv.y =  0.299 * r + 0.587 * g + 0.114 * b;
    yuv.u = -0.14713 * r - 0.28886 * g + 0.436 * b;
    yuv.v =  0.615 * r - 0.51499 * g - 0.10001 * b;

    return yuv;
}

/**
 * Converts a YUV pixel to RGB color space.
 * @param yuv The YUV pixel.
 * @return The corresponding RGB pixel.
 */
t_pixel yuv_to_rgb(t_yuv yuv) {
    t_pixel p;
    double y = yuv.y;
    double u = yuv.u;
    double v = yuv.v;

    double r = y + 1.13983 * v;
    double g = y - 0.39465 * u - 0.58060 * v;
    double b = y + 2.03211 * u;

    // Clamp results to 0-255
    p.red = clamp_uint8(r);
    p.green = clamp_uint8(g);
    p.blue = clamp_uint8(b);

    return p;
}

/**
 * Performs histogram equalization on a 24-bit BMP image (color version).
 * @param img Pointer to the t_bmp24 structure.
 */
void bmp24_equalize(t_bmp24 * img) {
    if (!img || !img->data) return;

    int width = img->width;
    int height = img->height;
    unsigned int numPixels = width * height;
    if (numPixels == 0) return;

    // 1. Convert RGB to YUV and compute Y histogram
    t_yuv ** yuv_data = (t_yuv **)malloc(height * sizeof(t_yuv *));
    unsigned int * y_hist = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (!yuv_data || !y_hist) {
        printf("Error: Memory allocation failed for YUV data or histogram\n");
        if (yuv_data) free(yuv_data); // Only free yuv_data if hist failed
        if (y_hist) free(y_hist);
        return;
    }

    int y_hist_allocated = 1; // Flag to track histogram allocation
    for (int y = 0; y < height; y++) {
        yuv_data[y] = (t_yuv *)malloc(width * sizeof(t_yuv));
        if (!yuv_data[y]) {
            printf("Error: Memory allocation failed for YUV row %d\n", y);
            // Free previously allocated rows and histogram
            for (int j = 0; j < y; j++) free(yuv_data[j]);
            free(yuv_data);
            free(y_hist);
            return;
        }
        for (int x = 0; x < width; x++) {
            yuv_data[y][x] = rgb_to_yuv(img->data[y][x]);
            uint8_t y_val = clamp_uint8(yuv_data[y][x].y); // Get Y component for histogram
            y_hist[y_val]++;
        }
    }

    // 2. Compute normalized cumulative histogram (CDF) for Y channel
    // We reuse the bmp8_computeCDF function here.
    unsigned int * hist_eq = bmp8_computeCDF(y_hist);
    free(y_hist); // Free the raw Y histogram
    if (!hist_eq) {
        printf("Error: Failed to compute CDF for Y channel\n");
         // Free YUV data
        for (int y = 0; y < height; y++) free(yuv_data[y]);
        free(yuv_data);
        return;
    }

    // 3. Apply equalization to Y component and convert back to RGB
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t original_y = clamp_uint8(yuv_data[y][x].y);
            yuv_data[y][x].y = (double)hist_eq[original_y]; // Apply equalization map
            img->data[y][x] = yuv_to_rgb(yuv_data[y][x]); // Convert back
        }
    }

    // 4. Free temporary data
    free(hist_eq);
    for (int y = 0; y < height; y++) {
        free(yuv_data[y]);
    }
    free(yuv_data);
} 