#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "macros.h"
#include "walsh.h"
#include "args.h"

//brightness transformation
    //grays-scale
        //tresholding
            //binarization -> one treshold value
    //brightness corrections


   // NEW_LIST(LEARNED_IMAGE_CONTEXT, learned_characters, )


// ocr -segment -in filename -out
// ocr -extract -in filename -out


int main(int argc, char **argv) {
    if(argc == 1) {
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

    NEW_LIST(IMAGE_CONTEXT, collection, 1500);
    collect_shapes(binarized, collection, &collection_count, ctx);
    save_collection(collection, collection_count, "");


    puts("Collection ran succesfully!");

    free(treshold_buffer);
    free(gray_img);
    free(histogram_equalized_img);
    free(binarized);

    return 0;
}


PROCEDURE(convert_to_grayscale) {
    ITERATE_IMAGE {
        SET(VALUE(POST_INCREMENT(target)), calculate_grayscale_value(PIXEL_ADDRESS(original)));
        if(CHANNELS == 4) {
            SET(VALUE(target + 1), VALUE(PIXEL_ADDRESS(original) + 1));
        }
    }
}
PIXEL_TYPE calculate_grayscale_value(PIXEL_ARRAY pixel) {
    return ((VALUE(pixel) + VALUE(pixel + 1) + VALUE(pixel + 2))/3.0);
}


PROCEDURE(treshold_image) {
    INT_ARRAY(treshold, 60, 80, 100, -1);
    INT_ARRAY(map, 0, 80, 160, 255, -1);

    ITERATE_IMAGE {
        APPEND_PIXEL(target, map_treshold(treshold, map, PIXEL_OF_ITERATION(original)));
    }
}

PROCEDURE(binarize_image) {
    ITERATE_IMAGE {
        APPEND_PIXEL(target, BINARIZE(PIXEL_OF_ITERATION(original), 140));
    }
}


PROCEDURE(histogram_equalization) {
    ZEROED_ARRAY(histogram, DISTINCT_BYTE_VALUES);

    ITERATE_IMAGE {
        INCREMENT(histogram[PIXEL_OF_ITERATION(original)]);
    }

    ZEROED_ARRAY(target_color, DISTINCT_BYTE_VALUES);

    FOR_RANGE(i, 1, DISTINCT_BYTE_VALUES) {
        target_color[i] = calculate_equalization(histogram[i] += histogram[i - 1], SIZE);
    }

    ITERATE_IMAGE {
        PIXEL_OF_ITERATION(target) = target_color[PIXEL_OF_ITERATION(original)];
    }
}


PROCEDURE(median_filter) {
    ITERATE_IMAGE_WITH_BOUNDS(1) {
        NEW_LIST(int, medianer, 9);
        FOR(offset, 9) {
            PUSH(medianer,  GET_PIXEL(original, (i + X_OFFSET[offset]), (j + Y_OFFSET[offset])));
        }
        SORT(medianer);
        PIXEL_OF_ITERATION(target) = MEDIAN(medianer);
    }
}


void collect_shapes(PIXEL_ARRAY original, IMAGE_CONTEXT *array, int *array_counter, IMAGE_CONTEXT ctx) {

    int number_of_shapes = 0;
    char file_name[200] = {0};
    
    ITERATE_IMAGE_INTERLEAVED(3) {
        if(is_shape(PIXEL_OF_ITERATION(original))) {

            NEW_RECURSION_CONTEXT();

            PAINT_SHAPE();

            if(shape_points_count > 100) {
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



    int array_count = 0;
    FOR(index, collected_shapes_count) {

        sprintf(file_name, "output/shapes/%d.png", number_of_shapes++);
        puts(file_name);
        IMAGE_CONTEXT shape = GET_SHAPE(index);

        ALLOCATE_BUFFER(scaled_buffer, 64 * 64);
        WHITEN(scaled_buffer, 64 * 64);
        
        stbir_resize_uint8(shape.image_start, shape.width, shape.height, 0, scaled_buffer, 64, 64, 0, 1);
        IMAGE_CONTEXT scaled = shape;
        scaled.height = 64;
        scaled.width = 64;
        scaled.channels = 1;
        scaled.image_start = scaled_buffer;

        
        PUSH(array, scaled);
    }
    VALUE(array_counter) = array_count;
}


void save_collection(IMAGE_CONTEXT *collection, int size, const char *dir) {
    char file_name[500]; 
    FOR(image, size) {
        sprintf(file_name, "output/shapes/%d.png", image);
        puts(file_name);
        stbi_write_jpg(file_name, 64, 64, 1, collection[image].image_start, 100);
    }
}


int compare_vectors(PIXEL_ARRAY a, PIXEL_ARRAY b, int size) {
    int diff = 0;
    FOR(index, size * size) {
        diff += ABS(VALUE(a++) - VALUE(b++));
    }
    return diff;
}
