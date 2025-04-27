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

int main() {
    ImageType currentImageType = IMAGE_TYPE_NONE;
    t_bmp8 * currentImage8 = NULL;
    t_bmp24 * currentImage24 = NULL;

    char filename[256];
    int choice;
    int value;

    while (1) {
        printf("\n--- Image Processing Menu ---\n");
        printf("Current Image: ");
        if (currentImageType == IMAGE_TYPE_BMP8) {
            printf("8-bit Grayscale\n");
        } else if (currentImageType == IMAGE_TYPE_BMP24) {
            printf("24-bit Color\n");
        } else {
            printf("None loaded\n");
        }
        printf("-----------------------------\n");
        printf("1. Open an image\n");
        printf("2. Save current image\n");
        printf("3. Apply Operation/Filter\n");
        printf("4. Display image information\n");
        printf("5. Quit\n");
        printf(">>> Your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer(); // Consume the newline

        switch (choice) {
            case 1: // Open Image
                printf("Enter image file path: ");
                if (scanf("%255s", filename) != 1) {
                    printf("Invalid input for filename.\n");
                    clear_input_buffer();
                    continue;
                }
                clear_input_buffer(); // Consume the newline

                // Free previous image if any
                if (currentImage8) bmp8_free(currentImage8);
                if (currentImage24) bmp24_free(currentImage24);
                currentImage8 = NULL;
                currentImage24 = NULL;
                currentImageType = IMAGE_TYPE_NONE;

                ImageType type = check_bmp_type(filename);
                if (type == IMAGE_TYPE_BMP8) {
                    currentImage8 = bmp8_loadImage(filename);
                    if (currentImage8) {
                        currentImageType = IMAGE_TYPE_BMP8;
                        printf("8-bit grayscale image loaded successfully!\n");
                    }
                } else if (type == IMAGE_TYPE_BMP24) {
                    currentImage24 = bmp24_loadImage(filename);
                    if (currentImage24) {
                        currentImageType = IMAGE_TYPE_BMP24;
                        printf("24-bit color image loaded successfully!\n");
                    }
                } else {
                    printf("Failed to load image or unsupported format.\n");
                }
                break;

            case 2: // Save Image
                if (currentImageType == IMAGE_TYPE_NONE) {
                    printf("Error: No image loaded to save.\n");
                    break;
                }
                printf("Enter output file path: ");
                if (scanf("%255s", filename) != 1) {
                    printf("Invalid input for filename.\n");
                    clear_input_buffer();
                    continue;
                }
                clear_input_buffer();

                if (currentImageType == IMAGE_TYPE_BMP8) {
                    bmp8_saveImage(filename, currentImage8);
                    printf("8-bit image saved successfully!\n");
                } else { // BMP24
                    bmp24_saveImage(currentImage24, filename);
                    printf("24-bit image saved successfully!\n");
                }
                break;

            case 3: // Apply Operation/Filter
                if (currentImageType == IMAGE_TYPE_NONE) {
                    printf("Error: No image loaded.\n");
                    break;
                }

                printf("\n--- Apply Operation/Filter ---\n");
                printf(" 1. Negative\n");
                printf(" 2. Brightness\n");
                if (currentImageType == IMAGE_TYPE_BMP8) {
                    printf(" 3. Black and White (Threshold)\n");
                } else { // BMP24
                    printf(" 3. Grayscale Conversion\n");
                }
                printf(" 4. Box Blur\n");
                printf(" 5. Gaussian Blur\n");
                printf(" 6. Sharpen\n");
                printf(" 7. Outline\n");
                printf(" 8. Emboss\n");
                printf(" 9. Histogram Equalization\n");
                printf("10. Return to main menu\n");
                printf(">>> Your choice: ");

                if (scanf("%d", &choice) != 1) {
                    printf("Invalid input. Please enter a number.\n");
                    clear_input_buffer();
                    continue;
                }
                clear_input_buffer();

                int operation_applied = 1; // Flag to print success message

                switch (choice) {
                    case 1: // Negative
                        if (currentImageType == IMAGE_TYPE_BMP8) bmp8_negative(currentImage8);
                        else bmp24_negative(currentImage24);
                        break;
                    case 2: // Brightness
                        printf("Enter brightness value (-255 to 255): ");
                        if (scanf("%d", &value) != 1) { printf("Invalid value.\n"); clear_input_buffer(); operation_applied = 0; break; }
                        clear_input_buffer();
                        if (currentImageType == IMAGE_TYPE_BMP8) bmp8_brightness(currentImage8, value);
                        else bmp24_brightness(currentImage24, value);
                        break;
                    case 3: // Threshold (BMP8) or Grayscale (BMP24)
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            printf("Enter threshold value (0-255): ");
                            if (scanf("%d", &value) != 1) { printf("Invalid value.\n"); clear_input_buffer(); operation_applied = 0; break; }
                            clear_input_buffer();
                            bmp8_threshold(currentImage8, value);
                        } else {
                            bmp24_grayscale(currentImage24);
                        }
                        break;
                    case 4: // Box Blur
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            float ** kernel = createBoxBlurKernel(); // Use kernel from bmp24.c
                            bmp8_applyFilter(currentImage8, kernel, 3);
                            freeKernel(kernel, 3);
                        } else {
                            bmp24_boxBlur(currentImage24);
                        }
                        break;
                    case 5: // Gaussian Blur
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            float ** kernel = createGaussianBlurKernel();
                            bmp8_applyFilter(currentImage8, kernel, 3);
                            freeKernel(kernel, 3);
                        } else {
                            bmp24_gaussianBlur(currentImage24);
                        }
                        break;
                    case 6: // Sharpen
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            float ** kernel = createSharpenKernel();
                            bmp8_applyFilter(currentImage8, kernel, 3);
                            freeKernel(kernel, 3);
                        } else {
                            bmp24_sharpen(currentImage24);
                        }
                        break;
                    case 7: // Outline
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            float ** kernel = createOutlineKernel();
                            bmp8_applyFilter(currentImage8, kernel, 3);
                            freeKernel(kernel, 3);
                        } else {
                            bmp24_outline(currentImage24);
                        }
                        break;
                    case 8: // Emboss
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            float ** kernel = createEmbossKernel();
                            bmp8_applyFilter(currentImage8, kernel, 3);
                            freeKernel(kernel, 3);
                        } else {
                            bmp24_emboss(currentImage24);
                        }
                        break;
                    case 9: // Histogram Equalization
                        if (currentImageType == IMAGE_TYPE_BMP8) {
                            bmp8_equalize(currentImage8);
                        } else {
                            bmp24_equalize(currentImage24);
                        }
                        break;
                    case 10: // Return
                        operation_applied = 0;
                        break;
                    default:
                        printf("Invalid filter choice\n");
                        operation_applied = 0;
                        break;
                }
                if(operation_applied) {
                    printf("Operation applied successfully!\n");
                }
                break;

            case 4: // Display Info
                if (currentImageType == IMAGE_TYPE_NONE) {
                    printf("Error: No image loaded.\n");
                } else if (currentImageType == IMAGE_TYPE_BMP8) {
                    bmp8_printInfo(currentImage8);
                } else { // BMP24
                    bmp24_printInfo(currentImage24);
                }
                break;

            case 5: // Quit
                if (currentImage8) bmp8_free(currentImage8);
                if (currentImage24) bmp24_free(currentImage24);
                printf("Goodbye!\n");
                return 0;

            default:
                printf("Invalid choice. Please enter a number between 1 and 5.\n");
                break;
        }
    }

    return 0;
} 