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
#define ITERATE_IMAGE_WITH_BOUNDS(n) FOR_RANGE(i, n, (HEIGHT - n)) FOR_RANGE(j, n, (WIDTH - n))
#define VALUE(x) *(x)
#define POST_INCREMENT(x) (x++)
#define INCREMENT(x) (++x)
#define SET(a, b) (a = b)
#define TWO_DIM_VALUE(arr, i, j) arr[(i * WIDTH) + j]
#define is_in_boundary(i, j) (i >= 0 && j >= 0 && i < HEIGHT && j < WIDTH)
#define ALLOCATE_BUFFER(name, size) unsigned char *name = calloc(size, 1)
#define SAVE_IMAGE(target, image) stbi_write_jpg(target, WIDTH, HEIGHT, 1, image, 100);
#define MAKE(name, target, original, proc) ALLOCATE_BUFFER(target, SIZE);CALL_PROC(proc, target, original);SAVE_IMAGE(name, target);

#define PROCEDURE(name) void name(PIXEL_ARRAY target, PIXEL_ARRAY original, IMAGE_CONTEXT ctx)
#define CALL_PROC(procedure, target, original) procedure(target, original, ctx)
#define INT_ARRAY(name, ...) int name[] = {__VA_ARGS__}
#define ADD(a, b) a += b
#define DISTINCT_BYTE_VALUES 256
#define ZEROED_ARRAY(name, length) int name[length] = { 0 }
#define ROLL_SUM_ARRAY(arr, a) (arr[a] += arr[a - 1])

void convert_to_grayscale(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx);
void treshold_image(unsigned char *target_buffer, unsigned char *original_image, IMAGE_CONTEXT ctx);
void histogram_equalization(unsigned char *target_buffer, unsigned char *grayscale_image, IMAGE_CONTEXT ctx);
int get_treshold_mapping(int *treshold_values, int *treshold_map, int value_to_classify);
unsigned char calculate_grayscale_value(unsigned char *pixel);
void convert_image_to_2_dimension(PIXEL_TYPE *target, PIXEL_TYPE *image, IMAGE_CONTEXT ctx);
void print_boundaries(PIXEL_ARRAY image, IMAGE_CONTEXT ctx);


int cmp(int *a, int *b){
    return *a - *b;
}


#define NEW_LIST(name, size) int name[size], name##_length = size, name##_count = 0;
#define SORT(array) qsort(array, array##_length, sizeof(int), cmp) 
#define MEDIAN(array) array[array##_length / 2]
#define PUSH(array, value) array[array##_count++] = value


int map_treshold(int *treshold_values, int *treshold_map, int value_to_classify){
    if(treshold_values[0] > value_to_classify){
        return treshold_map[0];
    }
    int i;
    for(i = 1;treshold_values[i] != -1 && (treshold_values[i] < value_to_classify); i++);

    return treshold_map[i];
}


PIXEL_TYPE calculate_equalization(int value, int size){
    return ((((double)DISTINCT_BYTE_VALUES - 1) / size) * value);
}

int X_OFFSET[] = {-1, -1, -1,  0, 0, 0,  1, 1, 1};
int Y_OFFSET[] = {-1,  0,  1, -1, 0, 1, -1, 0, 1};

int X_OFFSET_WINDOW_3X3_COLUMN[] = {-1,  0,  1,-1, 0, 1};
int Y_OFFSET_WINDOW_3X3_COLUMN[] = {-1, -1, -1, 1, 1, 1};

int X_OFFSET_WINDOW_3X3_ROW[] = {-1, -1, -1, 1, 1, 1};
int Y_OFFSET_WINDOW_3X3_ROW[] = {-1,  0,  1,-1, 0, 1};