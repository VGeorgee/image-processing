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
    ctx.image_start = binarized;

    free(treshold_buffer);
    free(gray_img);
    free(histogram_equalized_img);
    free(binarized);

    return 0;
}


void convert_to_grayscale(PIXEL_ARRAY target_buffer, PIXEL_ARRAY original_image, IMAGE_CONTEXT ctx){
    ITERATE_IMAGE {
        SET(VALUE(POST_INCREMENT(target_buffer)), calculate_grayscale_value(PIXEL_ADDRESS(original_image)));
        if(CHANNELS == 4) {
            SET(VALUE(target_buffer + 1), VALUE(PIXEL_ADDRESS(original_image) + 1));
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
        APPEND_PIXEL(target, BINARIZE(PIXEL_OF_ITERATION(original)));
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
    
    ITERATE_IMAGE {
        if(is_shape(PIXEL_OF_ITERATION(original))){
        
            indexer[0].x = i;
            indexer[0].y = j;
            int indexer_count = 1;

            NEW_RECURSION_CONTEXT();
            calculate_bounds_of_shape(&rctx, ctx, indexer, &indexer_count, i, j);
            ALLOCATE_BUFFER(target, RECURSION_SIZE + 10);
            WHITEN(target, RECURSION_SIZE + 10);
            NEW_IMAGE_CONTEXT(target);

           
            for (int ij = 1; ij < indexer_count; ij++){
                GET_CONTEXTED_PIXEL(target, target_ctx, (indexer[ij].x - indexer[0].x), (indexer[ij].y - indexer[0].y)) = 0;
            }
            if(indexer_count > 100){
                sprintf(file_name, "output/recursions/%d.png", number_of_shapes++);
                puts(file_name);
                SAVE_CTX(file_name, target, target_ctx);
            }
        }
    }
}