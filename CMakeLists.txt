cmake_minimum_required(VERSION 3.10)
project(ImageProcessing)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(image_processing
        main.c
        bmp8.c
        bmp24.c
)

target_include_directories(image_processing PRIVATE .)

# Link against the math library for functions like round()
target_link_libraries(image_processing PRIVATE m)
