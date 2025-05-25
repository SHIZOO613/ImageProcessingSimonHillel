/*
 * bmp24.h
 * Author: Simon Hillel
 * Description: Header for 24-bit BMP image handling and processing functions.
 * Declares structures and functions for loading, saving, manipulating, and filtering 24-bit BMP images.
 * This file is a core part of the image processing project, supporting color image operations.
 */
#ifndef BMP24_H
#define BMP24_H

#include <stdint.h>
#include <stdio.h>

// Offsets for the BMP header (matching project description)
#define BITMAP_MAGIC_OFFSET 0x00 // Corrected name
#define BITMAP_SIZE_OFFSET 0x02
#define BITMAP_OFFSET_OFFSET 0x0A

// Offsets for the BMP Info header (matching project description)
#define BITMAP_INFO_SIZE_OFFSET 0x0E // Start of InfoHeader is at 14 (0x0E)
#define BITMAP_WIDTH_OFFSET 0x12
#define BITMAP_HEIGHT_OFFSET 0x16
#define BITMAP_PLANES_OFFSET 0x1A
#define BITMAP_DEPTH_OFFSET 0x1C
#define BITMAP_COMPRESSION_OFFSET 0x1E
#define BITMAP_SIZE_RAW_OFFSET 0x22
#define BITMAP_XRES_OFFSET 0x26
#define BITMAP_YRES_OFFSET 0x2A
#define BITMAP_NCOLORS_OFFSET 0x2E
#define BITMAP_IMPORTANTCOLORS_OFFSET 0x32

// Magical number for BMP files
#define BMP_TYPE 0x4D42 // 'BM' in hexadecimal

// Header sizes
#define BMP_HEADER_SIZE 14
#define BMP_INFOHEADER_SIZE 40

// Constant for the color depth
#define DEFAULT_COLOR_DEPTH 24

// --- Structures --- //

// Structure for a single pixel (RGB)
// Using uint8_t from stdint.h for exact 8-bit unsigned integers
typedef struct {
    uint8_t blue;  // Note: BMP stores pixels typically in BGR order in the file
    uint8_t green;
    uint8_t red;
} t_pixel;

// Structure for YUV pixel (for histogram equalization)
typedef struct {
    double y;
    double u;
    double v;
} t_yuv;

// BMP File Header (14 bytes)
// Use __attribute__((packed)) to prevent padding issues when reading/writing structs directly
// Alternatively, read/write field by field using offsets and sizes.
// Reading field by field is generally safer across compilers/platforms.
// We will use field-by-field reading/writing based on offsets.
typedef struct {
    uint16_t type;        // Magic identifier 'BM'
    uint32_t size;        // File size in bytes
    uint16_t reserved1;   // Not used
    uint16_t reserved2;   // Not used
    uint32_t offset;      // Offset to image data in bytes from beginning of file
} t_bmp_header;

// BMP Info Header (BITMAPINFOHEADER, 40 bytes)
typedef struct {
    uint32_t size;            // Header size in bytes (40)
    int32_t  width;           // Image width in pixels
    int32_t  height;          // Image height in pixels
    uint16_t planes;          // Number of color planes (must be 1)
    uint16_t bits;            // Bits per pixel (1, 4, 8, 16, 24, 32)
    uint32_t compression;     // Compression type (0 = uncompressed)
    uint32_t imagesize;       // Image size in bytes (can be 0 for uncompressed)
    int32_t  xresolution;     // Horizontal resolution (pixels per meter)
    int32_t  yresolution;     // Vertical resolution (pixels per meter)
    uint32_t ncolors;         // Number of colors in palette (0 for 24-bit)
    uint32_t importantcolors; // Number of important colors (0 = all)
} t_bmp_info;

// Main structure for 24-bit BMP Image
typedef struct {
    t_bmp_header header;
    t_bmp_info header_info; // Renamed from info_header for consistency
    int width;          // Convenience copy of header_info.width
    int height;         // Convenience copy of header_info.height
    int colorDepth;     // Convenience copy of header_info.bits
    t_pixel **data;     // Pixel data as a 2D array (matrix)
} t_bmp24;

// --- Function Prototypes --- //

// Allocation and Deallocation
/**
 * Allocates a 2D array of t_pixel pointers for image pixel data.
 */
t_pixel ** bmp24_allocateDataPixels(int width, int height);
/**
 * Frees a 2D array of t_pixel pointers allocated for image pixel data.
 */
void bmp24_freeDataPixels(t_pixel ** pixels, int height);
/**
 * Allocates a t_bmp24 structure and its pixel data for a 24-bit BMP image.
 */
t_bmp24 * bmp24_allocate(int width, int height, int colorDepth);
/**
 * Frees a t_bmp24 structure and its associated pixel data.
 */
void bmp24_free(t_bmp24 * img);

// File I/O Helpers
/**
 * Reads raw data from a file at a specified position.
 */
void file_rawRead(uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file);
/**
 * Writes raw data to a file at a specified position.
 */
void file_rawWrite(uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file);

// Pixel Data Read/Write
/**
 * Reads pixel data from a BMP file into a t_bmp24 structure.
 */
void bmp24_readPixelData(t_bmp24 * image, FILE * file);
/**
 * Writes pixel data from a t_bmp24 structure to a BMP file.
 */
void bmp24_writePixelData(t_bmp24 * image, FILE * file);

// Loading and Saving
/**
 * Loads a 24-bit BMP image from a file.
 */
t_bmp24 * bmp24_loadImage(const char * filename);
/**
 * Saves a 24-bit BMP image to a file.
 */
void bmp24_saveImage(t_bmp24 * img, const char * filename);
/**
 * Prints information about a 24-bit BMP image.
 */
void bmp24_printInfo(t_bmp24 * img);

// Image Processing
/**
 * Applies a negative filter to a 24-bit BMP image.
 */
void bmp24_negative(t_bmp24 * img);
/**
 * Converts a 24-bit BMP image to grayscale in-place.
 */
void bmp24_grayscale(t_bmp24 * img);
/**
 * Adjusts the brightness of a 24-bit BMP image.
 */
void bmp24_brightness(t_bmp24 * img, int value);

// Convolution Filters
/**
 * Applies a convolution kernel to a pixel in a 24-bit BMP image.
 */
t_pixel bmp24_convolution(t_bmp24 * img, t_pixel ** tempData, int x, int y, float ** kernel, int kernelSize);
/**
 * Applies a convolution filter to a 24-bit BMP image using a given kernel.
 */
void bmp24_applyFilter(t_bmp24 * img, float ** kernel, int kernelSize);
/**
 * Applies a box blur filter to a 24-bit BMP image.
 */
void bmp24_boxBlur(t_bmp24 * img);
/**
 * Applies a Gaussian blur filter to a 24-bit BMP image.
 */
void bmp24_gaussianBlur(t_bmp24 * img);
/**
 * Applies an outline filter to a 24-bit BMP image.
 */
void bmp24_outline(t_bmp24 * img);
/**
 * Applies an emboss filter to a 24-bit BMP image.
 */
void bmp24_emboss(t_bmp24 * img);
/**
 * Applies a sharpen filter to a 24-bit BMP image.
 */
void bmp24_sharpen(t_bmp24 * img);

// Histogram Equalization (Color)
/**
 * Performs histogram equalization on a 24-bit BMP image (color version).
 */
void bmp24_equalize(t_bmp24 * img);

// Kernel Creation/Freeing Functions
/**
 * Creates a 3x3 box blur kernel for smoothing images.
 */
float ** createBoxBlurKernel();
/**
 * Creates a 3x3 Gaussian blur kernel for soft blurring.
 */
float ** createGaussianBlurKernel();
/**
 * Creates a 3x3 outline kernel for edge detection.
 */
float ** createOutlineKernel();
/**
 * Creates a 3x3 emboss kernel for embossing effect.
 */
float ** createEmbossKernel();
/**
 * Creates a 3x3 sharpen kernel for sharpening images.
 */
float ** createSharpenKernel();
/**
 * Frees a dynamically allocated convolution kernel.
 */
void freeKernel(float ** kernel, int size);

#endif // BMP24_H 