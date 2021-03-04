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
    convert_to_grayscale(gray_img, img, ctx);
    SAVE_IMAGE("output/gray-scaled.jpg", gray_img, gray_channels);

    CHANNELS = 1;
    ALLOCATE_BUFFER(treshold_buffer, gray_img_size);
    treshold_image(treshold_buffer, gray_img, ctx);
    SAVE_IMAGE("output/tresholded.jpg", treshold_buffer, gray_channels);

    ALLOCATE_BUFFER(histogram_equalized_img, gray_img_size);
    histogram_equalization(histogram_equalized_img, gray_img, ctx);
    SAVE_IMAGE("output/histogram-equalized.jpg", histogram_equalized_img, gray_channels);

    ALLOCATE_BUFFER(median_fileter_image, gray_img_size);
    printf("asd\n");
    //PIXEL_TYPE image[HEIGHT][WIDTH];
    //convert_image_to_2_dimension(image, gray_img, ctx);
    printf("asd\n");
    median_filter(median_fileter_image, gray_img, ctx);
    SAVE_IMAGE("output/median-filtered.jpg", median_fileter_image, gray_channels);

    free(treshold_buffer);
    free(gray_img);
    free(histogram_equalized_img);

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
        SET(VALUE(POST_INCREMENT(target)), map_treshold(treshold, map, PIXEL_OF_ITERATION(original)));
    }
}


PROCEDURE(histogram_equalization){
    ZEROED_ARRAY(histogram, DISTINCT_BYTE_VALUES);

    ITERATE_IMAGE {
        INCREMENT(histogram[PIXEL_OF_ITERATION(original)]);
    }

    ZEROED_ARRAY(target_color, DISTINCT_BYTE_VALUES);

    FOR_RANGE(i, 1, DISTINCT_BYTE_VALUES){
        SET(target_color[i], calculate_equalization(histogram[i] += histogram[i - 1], SIZE));
    }

    ITERATE_IMAGE {
        SET(PIXEL_OF_ITERATION(target), target_color[PIXEL_OF_ITERATION(original)]);
    }
}


PROCEDURE(median_filter){
    ITERATE_IMAGE {
        NEW_LIST(medianer, 9);
        FOR(offset, 9) {
            int addressed_i = i + X_OFFSET[offset];
            int addressed_j = j + Y_OFFSET[offset];
            
            if(is_in_boundary(addressed_i, addressed_j)){
                PUSH(medianer,  GET_PIXEL(original, addressed_i, addressed_j));
            }
        }
        SORT(medianer);
        PIXEL_OF_ITERATION(target) = MEDIAN(medianer);
    }
}
