#include <stdio.h>
#include <stdlib.h>
#include "macros.h"

//brightness transformation
    //grays-scale
        //tresholding
            //binarization -> one treshold value
    //brightness corrections


int main(int argc, char **argv)
{
    if(argc == 1){
        puts("No arguments given!");
        return 1;
    }
      
    IMAGE_CONTEXT ctx;

    PIXEL_ARRAY img = stbi_load(argv[1], &WIDTH, &HEIGHT, &CHANNELS, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", WIDTH, HEIGHT, CHANNELS);


    int gray_channels = CHANNELS == 4 ? 2 : 1;
    size_t gray_img_size = SIZE * gray_channels;



    ALLOCATE_BUFFER(gray_img, gray_img_size);
    CALL_PROC(convert_to_grayscale, gray_img, img);
    SAVE_IMAGE("output/gray-scaled.jpg", gray_img);

    CHANNELS = 1;

    MAKE("output/tresholded.jpg", treshold_buffer, gray_img, treshold_image);
    MAKE("output/histogram-equalized.jpg", histogram_equalized_img, gray_img, histogram_equalization);
    MAKE("output/median-filtered.jpg", median_fileter_image, gray_img, median_filter);
    MAKE("output/binarized.jpg", binarized, gray_img, binarize_image);
    
    ctx.image_start = binarized;
    collect_shapes(binarized, ctx);
    puts("Collection ran succesfully!");

    free(treshold_buffer);
    free(gray_img);
    free(histogram_equalized_img);
    free(binarized);

    return 0;
}


PROCEDURE(convert_to_grayscale){
    ITERATE_IMAGE {
        SET(VALUE(POST_INCREMENT(target)), calculate_grayscale_value(PIXEL_ADDRESS(original)));
        if(CHANNELS == 4) {
            SET(VALUE(target + 1), VALUE(PIXEL_ADDRESS(original) + 1));
        }
    }
}
PIXEL_TYPE calculate_grayscale_value(PIXEL_ARRAY pixel){
    return ((VALUE(pixel) + VALUE(pixel + 1) + VALUE(pixel + 2))/3.0);
}


PROCEDURE(treshold_image){
    INT_ARRAY(treshold, 60, 80, 100, -1);
    INT_ARRAY(map, 0, 80, 160, 255, -1);

    ITERATE_IMAGE {
        APPEND_PIXEL(target, map_treshold(treshold, map, PIXEL_OF_ITERATION(original)));
    }
}

PROCEDURE(binarize_image){
    ITERATE_IMAGE{
        APPEND_PIXEL(target, BINARIZE(PIXEL_OF_ITERATION(original), 140));
    }
}


PROCEDURE(histogram_equalization){
    ZEROED_ARRAY(histogram, DISTINCT_BYTE_VALUES);

    ITERATE_IMAGE {
        INCREMENT(histogram[PIXEL_OF_ITERATION(original)]);
    }

    ZEROED_ARRAY(target_color, DISTINCT_BYTE_VALUES);

    FOR_RANGE(i, 1, DISTINCT_BYTE_VALUES){
        target_color[i] = calculate_equalization(histogram[i] += histogram[i - 1], SIZE);
    }

    ITERATE_IMAGE {
        PIXEL_OF_ITERATION(target) = target_color[PIXEL_OF_ITERATION(original)];
    }
}


PROCEDURE(median_filter){
    ITERATE_IMAGE_WITH_BOUNDS(1) {
        NEW_LIST(int, medianer, 9);
        FOR(offset, 9) {
            PUSH(medianer,  GET_PIXEL(original, (i + X_OFFSET[offset]), (j + Y_OFFSET[offset])));
        }
        SORT(medianer);
        PIXEL_OF_ITERATION(target) = MEDIAN(medianer);
    }
}


void collect_shapes(PIXEL_ARRAY original, IMAGE_CONTEXT ctx){

    int number_of_shapes = 0;
    char file_name[200] = {0};
    
    ITERATE_IMAGE_INTERLEAVED(3) {
        if(is_shape(PIXEL_OF_ITERATION(original))){

            NEW_RECURSION_CONTEXT();

            PAINT_SHAPE();

            if(shape_points_count > 100){
                ALLOCATE_BUFFER(target, RECURSION_SIZE);
                WHITEN(target, RECURSION_SIZE);

                NEW_IMAGE_CONTEXT(target);
                FOR_RANGE(index, 1, shape_points_count){
                    GET_CONTEXTED_PIXEL(target, target_ctx, (shape_points[index].x - shape_points[0].x), (shape_points[index].y - shape_points[0].y)) = 0;
                }
                PUSH_SHAPE(target_ctx);
            }
        }
    }
    
    SORT_SHAPES();
    FOR(index, collected_shapes_count){
        sprintf(file_name, "output/shapes/%d.png", number_of_shapes++);
        puts(file_name);
        IMAGE_CONTEXT shape = GET_SHAPE(index);
        SAVE_CTX(file_name, shape.image_start, shape);
    }
}

void scale_image(PIXEL_ARRAY target, PIXEL_ARRAY original, IMAGE_CONTEXT ctx, int target_size){
    int width_increment = WIDTH / target_size;
    int height_increment = HEIGHT / target_size;
    
    FOR_INCREMENT(i, HEIGHT, width_increment) {
        FOR_INCREMENT(j, WIDTH, height_increment) {

            NEW_LIST(int, pixels_to_median, width_increment * height_increment);

            FOR(target_x, width_increment){
                FOR(target_y, height_increment){
                    PUSH(pixels_to_median, GET_PIXEL(original, target_x, target_y));
                }
            }
            
            SORT(pixels_to_median);

            APPEND_PIXEL(target, MEDIAN(pixels_to_median));
        }
    }
}