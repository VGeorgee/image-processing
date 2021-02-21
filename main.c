#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

//brightness transformation
    //grays-scale
        //tresholding
            //binarization -> one treshold value
    //brightness corrections


typedef struct image_context {
    int width;
    int height;
    int channels;
} IMAGE_CONTEXT;


#define WIDTH ctx.width
#define HEIGHT ctx.height
#define CHANNELS ctx.channels
#define GET_PIXEL(matrix, i, j) (*(matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS)))
#define GET_PIXEL_ADDRESS(matrix, i, j) (matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS))
#define PIXEL(matrix) (*(matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS)))
#define FOR(x, y) for(int x = 0; x < y; x++)
#define ITERATE_IMAGE FOR(i, HEIGHT) FOR(j, WIDTH) 
#define VALUE(x) *(x)
#define is_in_boundary(i, j) (i >= 0 && j >= 0 && i < height && j < width)
#define ALLOCATE_BUFFER(name, size) unsigned char *name = malloc(size)
void convert_to_grayscale(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx);
void treshold_image(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx);
void histogram_equalization(unsigned char *target_buffer, unsigned char *grayscale_image, int gray_levels, IMAGE_CONTEXT ctx);


int main(int argc, char **argv)
{
    if(argc == 1){
        puts("No arguments given!");
        return 1;
    }
      
    IMAGE_CONTEXT ctx;

    unsigned char *img = stbi_load(argv[1], &WIDTH, &HEIGHT, &CHANNELS, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", WIDTH, HEIGHT, CHANNELS);

    size_t img_size = WIDTH * HEIGHT * CHANNELS;

    int gray_channels = CHANNELS == 4 ? 2 : 1;
    size_t gray_img_size = WIDTH * HEIGHT * gray_channels;

    ALLOCATE_BUFFER(gray_img, gray_img_size);
    convert_to_grayscale(gray_img, img, ctx);
    stbi_write_jpg("output/gray-scaled.jpg", WIDTH, HEIGHT, gray_channels, gray_img, 100);

    CHANNELS = 1;
    ALLOCATE_BUFFER(treshold_buffer, gray_img_size);
    treshold_image(treshold_buffer, gray_img, ctx);
    stbi_write_jpg("output/tresholded.jpg", WIDTH, HEIGHT, gray_channels, treshold_buffer, 100);

    ALLOCATE_BUFFER(histogram_equalized_img, gray_img_size);
    histogram_equalization(histogram_equalized_img, gray_img, 4, ctx);
    stbi_write_jpg("output/histogram-equalized.jpg", WIDTH, HEIGHT, gray_channels, histogram_equalized_img, 100);


    free(treshold_buffer);
    free(gray_img);
    free(histogram_equalized_img);

    return 0;
}


unsigned char calculate_grayscale_value(unsigned char *pixel){
    return ((VALUE(pixel) + VALUE(pixel + 1) + VALUE(pixel + 2))/3.0);
}
void convert_to_grayscale(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx){
    ITERATE_IMAGE {
        VALUE(target_buffer++) = calculate_grayscale_value(GET_PIXEL_ADDRESS(original_image, i, j));
        if(CHANNELS == 4) {
            VALUE(target_buffer + 1) = VALUE(GET_PIXEL_ADDRESS(original_image, i, j) + 1);
        }
    }
}



int get_treshold_mapping(int *treshold_values, int *treshold_map, int value_to_classify);
void treshold_image(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx){
    int treshold[] = {60, 80, 100, -1};
    int map[] = {0, 80, 160, 255, -1};

    ITERATE_IMAGE {
        VALUE(target_buffer++) = get_treshold_mapping(treshold, map, PIXEL(original_image));
    }
}

int get_treshold_mapping(int *treshold_values, int *treshold_map, int value_to_classify){
    if(treshold_values[0] > value_to_classify){
        return treshold_map[0];
    }
    int i;
    for(i = 1;treshold_values[i] != -1 && (treshold_values[i] < value_to_classify); i++);

    return treshold_map[i];
}




void histogram_equalization(unsigned char *target_buffer, unsigned char *grayscale_image, int gray_levels, IMAGE_CONTEXT ctx){
    int histogram[257] = {0};
    int target_color[257] = {0};

    ITERATE_IMAGE {
        histogram[PIXEL(grayscale_image)]++;
    }

    for(int i = 1; i < 256; i++){
        target_color[i] = (unsigned char)((255.0/(HEIGHT * WIDTH)) * (histogram[i] += histogram[i - 1]));
    }

    ITERATE_IMAGE {
        PIXEL(target_buffer) = target_color[PIXEL(grayscale_image)];
    }
}
