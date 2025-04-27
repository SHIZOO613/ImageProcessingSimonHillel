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

// Part 2: Allocation and Deallocation
t_pixel ** bmp24_allocateDataPixels(int width, int height);
void bmp24_freeDataPixels(t_pixel ** pixels, int height);
t_bmp24 * bmp24_allocate(int width, int height);
void bmp24_free(t_bmp24 * img);

// Part 2: File I/O Helpers (Provided in description)
void file_rawRead(uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file);
void file_rawWrite(uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file);

// Part 2: Pixel Data Read/Write (To be implemented)
void bmp24_readPixelData(t_bmp24 * image, FILE * file);
void bmp24_writePixelData(t_bmp24 * image, FILE * file);
// Helper for single pixel read/write (Optional, can be inline in Data functions)
// void bmp24_readPixelValue(t_bmp24 * image, int x, int y, FILE * file);
// void bmp24_writePixelValue(t_bmp24 * image, int x, int y, FILE * file);

// Part 2: Loading and Saving
t_bmp24 * bmp24_loadImage(const char * filename);
void bmp24_saveImage(t_bmp24 * img, const char * filename);
void bmp24_printInfo(t_bmp24 * img); // Added for consistency

// Part 2: Image Processing
void bmp24_negative(t_bmp24 * img);
void bmp24_grayscale(t_bmp24 * img);
void bmp24_brightness(t_bmp24 * img, int value);

// Part 2: Convolution Filters
t_pixel bmp24_convolution(t_bmp24 * img, t_pixel ** tempData, int x, int y, float ** kernel, int kernelSize);
void bmp24_applyFilter(t_bmp24 * img, float ** kernel, int kernelSize); // General filter application
// Specific filter functions (can call bmp24_applyFilter with predefined kernels)
void bmp24_boxBlur(t_bmp24 * img);
void bmp24_gaussianBlur(t_bmp24 * img);
void bmp24_outline(t_bmp24 * img);
void bmp24_emboss(t_bmp24 * img);
void bmp24_sharpen(t_bmp24 * img);

// Part 3: Histogram Equalization (Color)
void bmp24_equalize(t_bmp24 * img);

// --- Kernel Creation/Freeing Functions (Defined in bmp24.c) --- //
float ** createBoxBlurKernel();
float ** createGaussianBlurKernel();
float ** createOutlineKernel();
float ** createEmbossKernel();
float ** createSharpenKernel();
void freeKernel(float ** kernel, int size);

#endif // BMP24_H 