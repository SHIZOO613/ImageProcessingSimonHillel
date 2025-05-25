# Image Processing Project

## Description
This project is a C program for basic image processing on BMP files. It supports both 8-bit grayscale and 24-bit color BMP images. Users can load, save, and apply various filters (negative, brightness, grayscale, blur, sharpen, outline, emboss, etc.) via a menu-driven interface.

## Compilation

To compile the project, use a C compiler such as `gcc`.

Example command (adjust file list as needed):

```sh
gcc -o image_processor main.c bmp24.c bmp8.c -lm
```

- The `-lm` flag links the math library (required for some filters).

## Execution

Run the program from the terminal:

```sh
./image_processor
```

Follow the on-screen menu to open images, apply filters, and save results.

## Test Images

The following BMP images are included for testing:
- `flowers_color.bmp`
- `image1.bmp`
- `image2.bmp`
- `image3.bmp`

(Replace with your actual image filenames if different.)

## Implemented Features

- Load and save 8-bit grayscale and 24-bit color BMP images
- Display image information
- Apply filters:
  - Negative
  - Brightness adjustment
  - Grayscale (for color images)
  - Box blur
  - Gaussian blur
  - Sharpen
  - Outline
  - Emboss
  - Threshold (for grayscale images)
- Histogram equalization (color and grayscale)

## Known Bugs / Limitations

- Only uncompressed BMP files are supported.
- Only 8-bit and 24-bit BMP images are supported.
- Some advanced filters for grayscale images are placeholders (not implemented).
- The program does not support other image formats (JPEG, PNG, etc.).
- Error handling for corrupted or non-standard BMP files is basic.

---

## Submission Checklist

- [x] All `.c` and `.h` source files included
- [x] All test images included
- [x] README.md with all required sections 