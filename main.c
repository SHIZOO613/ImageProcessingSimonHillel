#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bmp8.h"
#include "bmp24.h"

// Enum to track the type of the currently loaded image
typedef enum {
    IMAGE_TYPE_NONE,
    IMAGE_TYPE_BMP8,
    IMAGE_TYPE_BMP24
} ImageType;

// Function to check BMP type without loading fully
ImageType check_bmp_type(const char * filename) {
    FILE * file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s to check type.\n", filename);
        return IMAGE_TYPE_NONE;
    }

    uint16_t type_magic;
    uint16_t depth;

    // Check magic number
    if (fread(&type_magic, sizeof(uint16_t), 1, file) != 1 || type_magic != BMP_TYPE) {
        printf("Error: Invalid BMP magic number in %s\n", filename);
        fclose(file);
        return IMAGE_TYPE_NONE;
    }

    // Check color depth (offset 28)
    if (fseek(file, BITMAP_DEPTH_OFFSET, SEEK_SET) != 0 || fread(&depth, sizeof(uint16_t), 1, file) != 1) {
        printf("Error: Could not read color depth from %s\n", filename);
        fclose(file);
        return IMAGE_TYPE_NONE;
    }

    fclose(file);

    if (depth == 8) {
        return IMAGE_TYPE_BMP8;
    } else if (depth == 24) {
        return IMAGE_TYPE_BMP24;
    } else {
        printf("Error: Unsupported BMP color depth (%u) in %s\n", depth, filename);
        return IMAGE_TYPE_NONE;
    }
}

// Helper to clear input buffer after scanf
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void displayMenu() {
    printf("\nPlease choose an option:\n");
    printf("1. Open an image\n");
    printf("2. Save an image\n");
    printf("3. Apply a filter\n");
    printf("4. Display image information\n");
    printf("5. Quit\n");
    printf(">>> Your choice: ");
}

void displayFilterMenu() {
    printf("\nPlease choose a filter:\n");
    printf("1. Negative\n");
    printf("2. Brightness\n");
    printf("3. Black and white\n");
    printf("4. Box Blur\n");
    printf("5. Gaussian blur\n");
    printf("6. Sharpness\n");
    printf("7. Outline\n");
    printf("8. Emboss\n");
    printf("9. Return to the previous menu\n");
    printf(">>> Your choice: ");
}

int main() {
    ImageType currentImageType = IMAGE_TYPE_NONE;
    t_bmp8 * currentImage8 = NULL;
    t_bmp24 * currentImage24 = NULL;

    char filename[256];
    int choice, filterChoice, brightness;
    int isColor = 0;  // 0 for grayscale, 1 for color

    while (1) {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:  // Open image
                // Free previously loaded images to prevent memory leaks
                if (currentImage24) { bmp24_free(currentImage24); currentImage24 = NULL; }
                if (currentImage8) { bmp8_free(currentImage8); currentImage8 = NULL; }
                printf("File path: ");
                scanf("%255s", filename);
                clear_input_buffer();
                // Try to load as color image first
                currentImage24 = bmp24_loadImage(filename);
                if (currentImage24) {
                    isColor = 1;
                    currentImageType = IMAGE_TYPE_BMP24;
                    printf("Color image loaded successfully!\n");
                } else {
                    // If color loading fails, try grayscale
                    currentImage8 = bmp8_loadImage(filename);
                    if (currentImage8) {
                        isColor = 0;
                        currentImageType = IMAGE_TYPE_BMP8;
                        printf("Grayscale image loaded successfully!\n");
                    } else {
                        printf("Error: Could not load image. Please check the file path and format.\n");
                    }
                }
                break;

            case 2:  // Save image
                printf("File path: ");
                scanf("%255s", filename);
                clear_input_buffer();
                if (isColor && currentImage24) {
                    bmp24_saveImage(currentImage24, filename);
                    printf("Color image saved successfully!\n");
                } else if (!isColor && currentImage8) {
                    bmp8_saveImage(filename, currentImage8);
                    printf("Grayscale image saved successfully!\n");
                } else {
                    printf("Error: No image loaded. Please open an image first.\n");
                }
                break;

            case 3:  // Apply filter
                if ((!isColor && !currentImage8) || (isColor && !currentImage24)) {
                    printf("Error: No image loaded. Please open an image first.\n");
                    break;
                }

                displayFilterMenu();
                if (scanf("%d", &filterChoice) != 1) {
                    printf("Invalid input. Please enter a number.\n");
                    clear_input_buffer();
                    break;
                }
                clear_input_buffer();

                if (isColor) {
                    switch (filterChoice) {
                        case 1:
                            bmp24_negative(currentImage24);
                            break;
                        case 2:
                            printf("Enter brightness value (-255 to 255): ");
                            if (scanf("%d", &brightness) != 1) {
                                printf("Invalid input.\n");
                                clear_input_buffer();
                                break;
                            }
                            clear_input_buffer();
                            bmp24_brightness(currentImage24, brightness);
                            break;
                        case 3:
                            bmp24_grayscale(currentImage24);
                            break;
                        case 4:
                            bmp24_boxBlur(currentImage24);
                            break;
                        case 5:
                            bmp24_gaussianBlur(currentImage24);
                            break;
                        case 6:
                            bmp24_sharpen(currentImage24);
                            break;
                        case 7:
                            bmp24_outline(currentImage24);
                            break;
                        case 8:
                            bmp24_emboss(currentImage24);
                            break;
                        case 9:
                            continue;
                        default:
                            printf("Invalid filter choice.\n");
                            continue;
                    }
                } else {
                    switch (filterChoice) {
                        case 1:
                            bmp8_negative(currentImage8);
                            break;
                        case 2:
                            printf("Enter brightness value (-255 to 255): ");
                            if (scanf("%d", &brightness) != 1) {
                                printf("Invalid input.\n");
                                clear_input_buffer();
                                break;
                            }
                            clear_input_buffer();
                            bmp8_brightness(currentImage8, brightness);
                            break;
                        case 3:
                            printf("Enter threshold value (0 to 255): ");
                            if (scanf("%d", &brightness) != 1) {
                                printf("Invalid input.\n");
                                clear_input_buffer();
                                break;
                            }
                            clear_input_buffer();
                            bmp8_threshold(currentImage8, brightness);
                            break;
                        case 4:
                            // Box blur kernel
                            {
                                float kernel[3][3] = {
                                    {1.0f/9, 1.0f/9, 1.0f/9},
                                    {1.0f/9, 1.0f/9, 1.0f/9},
                                    {1.0f/9, 1.0f/9, 1.0f/9}
                                };
                                float **kernelPtr = (float **)malloc(3 * sizeof(float *));
                                for (int i = 0; i < 3; i++) {
                                    kernelPtr[i] = kernel[i];
                                }
                                bmp8_applyFilter(currentImage8, kernelPtr, 3);
                                free(kernelPtr);
                            }
                            break;
                        case 5:
                            // Gaussian blur kernel
                            {
                                float kernel[3][3] = {
                                    {1.0f/16, 2.0f/16, 1.0f/16},
                                    {2.0f/16, 4.0f/16, 2.0f/16},
                                    {1.0f/16, 2.0f/16, 1.0f/16}
                                };
                                float **kernelPtr = (float **)malloc(3 * sizeof(float *));
                                for (int i = 0; i < 3; i++) {
                                    kernelPtr[i] = kernel[i];
                                }
                                bmp8_applyFilter(currentImage8, kernelPtr, 3);
                                free(kernelPtr);
                            }
                            break;
                        case 6:
                            // Sharpen kernel
                            {
                                float kernel[3][3] = {
                                    {0, -1, 0},
                                    {-1, 5, -1},
                                    {0, -1, 0}
                                };
                                float **kernelPtr = (float **)malloc(3 * sizeof(float *));
                                for (int i = 0; i < 3; i++) {
                                    kernelPtr[i] = kernel[i];
                                }
                                bmp8_applyFilter(currentImage8, kernelPtr, 3);
                                free(kernelPtr);
                            }
                            break;
                        case 7:
                            // Outline kernel
                            {
                                float kernel[3][3] = {
                                    {-1, -1, -1},
                                    {-1, 8, -1},
                                    {-1, -1, -1}
                                };
                                float **kernelPtr = (float **)malloc(3 * sizeof(float *));
                                for (int i = 0; i < 3; i++) {
                                    kernelPtr[i] = kernel[i];
                                }
                                bmp8_applyFilter(currentImage8, kernelPtr, 3);
                                free(kernelPtr);
                            }
                            break;
                        case 8:
                            // Emboss kernel
                            {
                                float kernel[3][3] = {
                                    {-2, -1, 0},
                                    {-1, 1, 1},
                                    {0, 1, 2}
                                };
                                float **kernelPtr = (float **)malloc(3 * sizeof(float *));
                                for (int i = 0; i < 3; i++) {
                                    kernelPtr[i] = kernel[i];
                                }
                                bmp8_applyFilter(currentImage8, kernelPtr, 3);
                                free(kernelPtr);
                            }
                            break;
                        case 9:
                            continue;
                        default:
                            printf("Invalid filter choice.\n");
                            continue;
                    }
                }
                printf("Filter applied successfully!\n");
                break;

            case 4:  // Display image information
                if (isColor && currentImage24) {
                    printf("Color Image Info:\n");
                    printf("Width: %d\n", currentImage24->width);
                    printf("Height: %d\n", currentImage24->height);
                    printf("Color Depth: %d\n", currentImage24->colorDepth);
                } else if (!isColor && currentImage8) {
                    bmp8_printInfo(currentImage8);
                } else {
                    printf("Error: No image loaded\n");
                }
                break;

            case 5:  // Quit
                if (currentImage8) bmp8_free(currentImage8);
                if (currentImage24) bmp24_free(currentImage24);
                printf("Goodbye!\n");
                return 0;

            default:
                printf("Invalid choice\n");
                break;
        }
    }

    return 0;
} 