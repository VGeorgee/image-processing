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
    int offset;
} IMAGE_CONTEXT;

typedef struct point{
    int x;
    int y;
} POINT;

 
POINT shape_points[240000000];
int shape_points_count;

#define RESET_SHAPE_POINTS() shape_points[0].x = i;\
            shape_points[0].y = j;\
            int shape_points_count = 1; \

#define WIDTH ctx.width
#define HEIGHT ctx.height
#define CHANNELS ctx.channels
#define IMG_BUFFER_START ctx.image_start
#define SIZE (WIDTH * HEIGHT)
#define GET_PIXEL(matrix, i, j) (*(matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS)))
#define GET_CONTEXTED_PIXEL(matrix, ctx, i, j) (*(matrix + (i * ctx.width *  ctx.channels) + (j *  ctx.channels)))
#define GET_PIXEL_ADDRESS(matrix, i, j) (matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS))
#define PIXEL_OF_ITERATION(matrix) (*(matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS)))
#define PIXEL_ADDRESS(matrix) (matrix + (i * WIDTH * CHANNELS) + (j * CHANNELS))
#define APPEND_PIXEL(target, pixel) (*(target++)) = (pixel)
#define FOR(x, y) for(int x = 0; x < y; x++)
#define FOR_INCREMENT(x, y, increment) for(int x = 0; x < y; x+=increment)
#define FOR_RANGE(name, from, to) for(int name = from; name < to; name++)
#define ITERATE_IMAGE FOR(i, HEIGHT) FOR(j, WIDTH)
#define ITERATE_IMAGE_INTERLEAVED(n) FOR_INCREMENT(i, HEIGHT, n) FOR(j, WIDTH)
#define ITERATE_IMAGE_WITH_BOUNDS(n) FOR_RANGE(i, n, (HEIGHT - n)) FOR_RANGE(j, n, (WIDTH - n))
#define VALUE(x) *(x)
#define POST_INCREMENT(x) (x++)
#define INCREMENT(x) (++x)
#define SET(a, b) (a = b)
#define BINARIZE(a, b) (a > b ? 255 : 0)
#define TWO_DIM_VALUE(arr, i, j) arr[(i * WIDTH) + j]
#define is_in_boundary(i, j) (i >= 0 && j >= 0 && i < HEIGHT && j < WIDTH)
#define ALLOCATE_BUFFER(name, size) unsigned char *name = calloc(size, 1)
#define SAVE_IMAGE(target, image) stbi_write_jpg(target, WIDTH, HEIGHT, 1, image, 100);
#define SAVE_CTX(target, image, ctx) stbi_write_jpg(target, ctx.width, ctx.height, 1, image, 100);
#define MAKE(name, target, original, proc) ALLOCATE_BUFFER(target, SIZE);CALL_PROC(proc, target, original);SAVE_IMAGE(name, target);

#define WHITEN(buffer, size) for(int i = 0; i < size; buffer[i++] = 255);
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


#define NEW_LIST(type, name, size) type name[size]; int name##_length = size, name##_count = 0;
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


typedef struct {
    int moves[4];
    int starti;
    int startj;
} RECURSION_CONTEXT;
/*
    int up;
    int down;
    int left;
    int right;
*/
#define NEW_RECURSION_CONTEXT() RECURSION_CONTEXT rctx; \
    rctx.moves[0] = i; \
    rctx.moves[1] = i;\
    rctx.moves[2] = j;\
    rctx.moves[3] = j;\
    rctx.starti = i;\
    rctx.startj = j;\
    ctx.image_start = original;\
    shape_points[0].x = i;\
    shape_points[0].y = j;\
    int shape_points_count = 1; \

#define RECURSION_HEIGHT ((((rctx.moves[1] - rctx.moves[0])) + 1) )
#define RECURSION_WIDTH (((((rctx.moves[3]) - rctx.moves[2]) ) + 1) ) 
#define RECURSION_SIZE (RECURSION_HEIGHT * RECURSION_WIDTH)

#define NEW_IMAGE_CONTEXT(target) IMAGE_CONTEXT target_ctx;\
        target_ctx.image_start = target; \
        target_ctx.height = RECURSION_HEIGHT; \
        target_ctx.width = RECURSION_WIDTH;\
        target_ctx.channels = 1; \




#define PIXEL_TO_REPAINT 0
#define TOUCHED 1
#define is_shape(x) (x == PIXEL_TO_REPAINT)
#define RCTX(x) rctx->x
#define INCREMENT_BOUND(i) (RCTX(moves[i]++))
#define GET_BOUND(i) (RCTX(moves[i]))

int recurse_array_i[] = {1, 0, 0, -1};
int recurse_array_j[] = {0, 1, -1, 0};

/// TOUCHED PIXELS MUST BE REPAINTED TO ORIGINAL COLOR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void calculate_bounds_of_shape(RECURSION_CONTEXT *rctx, IMAGE_CONTEXT ctx, POINT *array, int *array_size, int i, int j){
    PIXEL_OF_ITERATION(ctx.image_start) = TOUCHED;

    if(array[0].x > i){
        array[0].x = i;
    }
    if(array[0].y > j){
        array[0].y = j;
    }
    array[VALUE(array_size)].x = i;
    array[VALUE(array_size)].y = j;
    INCREMENT(VALUE(array_size));

    FOR(recurse, 4){
        int r_i = recurse_array_i[recurse] + i;
        int r_j = recurse_array_j[recurse] + j;
        if(is_in_boundary(r_i, r_j) && (GET_PIXEL(ctx.image_start, r_i, r_j) == PIXEL_TO_REPAINT)){
            //printf("%d %d\n", r_i, r_j);
            if(r_i < GET_BOUND(0)){
                GET_BOUND(0) = r_i;
            }
            if(r_i > GET_BOUND(1)){
                GET_BOUND(1) = r_i;
            }
            if(r_j < GET_BOUND(2)){
                GET_BOUND(2) = r_j;
            }
            if(r_j > GET_BOUND(3)){
                GET_BOUND(3) = r_j;
            }
            //INCREMENT_BOUND(recurse);
            calculate_bounds_of_shape(rctx, ctx, array, array_size, r_i, r_j);
        }
    }
}                                                                                       
                                                                                                    //
#define MERGER(target, i, j)  (*(target.image_start + (((  ((i - RCTX(starti)) % target_ctx.height) ) ) * target_ctx.width * CHANNELS) + (( ( (j) % target_ctx.width )) * CHANNELS)))
                                                                                    //

                                                                                    

void paint_shape(RECURSION_CONTEXT *rctx, IMAGE_CONTEXT ctx, IMAGE_CONTEXT target_ctx, int i, int j, POINT *array, int *array_size){
    //printf("i=%d j=%d\n", i, j);
    //MERGER(target_ctx, i, j) = PIXEL_OF_ITERATION(ctx.image_start);
    if(array[0].x > i){
        array[0].x = i;
    }
    if(array[0].y > j){
        array[0].y = j;
    }
    array[VALUE(array_size)].x = i;
    array[VALUE(array_size)].y = j;
    INCREMENT(VALUE(array_size));

    PIXEL_OF_ITERATION(ctx.image_start) = TOUCHED + 1;

    FOR(recurse, 4){
        int r_i = recurse_array_i[recurse] + i;
        int r_j = recurse_array_j[recurse] + j;
        if(is_in_boundary(r_i, r_j) && (GET_PIXEL(ctx.image_start, r_i, r_j) == TOUCHED)){
           // INCREMENT_BOUND(recurse);
            paint_shape(rctx, ctx, target_ctx, r_i, r_j, array, array_size);
        }
    }
}

#define PAINT_SHAPE() calculate_bounds_of_shape(&rctx, ctx, shape_points, &shape_points_count, i, j)




