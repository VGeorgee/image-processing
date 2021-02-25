#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PIXEL_TYPE unsigned char
#define PIXEL_ARRAY PIXEL_TYPE *

typedef struct image_context {
    PIXEL_ARRAY image_start;
    int width;
    int height;
    int channels;
} IMAGE_CONTEXT;

#define WIDTH ctx.width
#define HEIGHT ctx.height
#define CHANNELS ctx.channels
#define IMG_BUFFER_START ctx.image_start
#define SIZE (WIDTH * HEIGHT)
#define GET_PIXEL(matrix, i, j) (*(matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS)))
#define GET_PIXEL_ADDRESS(matrix, i, j) (matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS))
#define PIXEL_OF_ITERATION(matrix) (*(matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS)))
#define PIXEL_ADDRESS(matrix) (matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS))
#define FOR(x, y) for(int x = 0; x < y; x++)
#define FOR_RANGE(name, from, to) for(int name = from; name < to; name++)
#define ITERATE_IMAGE FOR(i, HEIGHT) FOR(j, WIDTH)
#define VALUE(x) *(x)
#define POST_INCREMENT(x) (x++)
#define INCREMENT(x) (++x)
#define SET(a, b) (a = b)
#define is_in_boundary(i, j) (i >= 0 && j >= 0 && i < HEIGHT && j < WIDTH)
#define ALLOCATE_BUFFER(name, size) unsigned char *name = malloc(size)
#define SAVE_IMAGE(target, image, channels) stbi_write_jpg(target, WIDTH, HEIGHT, channels, image, 100);

#define DISTINCT_BYTE_VALUES 256
#define ZEROED_ARRAY(type, name, length) type name[length] = { 0 }
#define ROLL_SUM_ARRAY(arr, a) (arr[a] += arr[a - 1])

void convert_to_grayscale(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx);
void treshold_image(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx);
void histogram_equalization(unsigned char *target_buffer, unsigned char *grayscale_image, IMAGE_CONTEXT ctx);
int get_treshold_mapping(int *treshold_values, int *treshold_map, int value_to_classify);
unsigned char calculate_grayscale_value(unsigned char *pixel);
