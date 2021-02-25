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


void treshold_image(PIXEL_ARRAY target_buffer, PIXEL_ARRAY original_image, IMAGE_CONTEXT ctx){
    int treshold[] = {60, 80, 100, -1};
    int map[] = {0, 80, 160, 255, -1};

    ITERATE_IMAGE {
        SET(VALUE(POST_INCREMENT(target_buffer)), get_treshold_mapping(treshold, map, PIXEL_OF_ITERATION(original_image)));
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




PIXEL_TYPE calculate_equalization(int value, int size){
    return ((((double)DISTINCT_BYTE_VALUES - 1) / size) * value);
}
void histogram_equalization(PIXEL_ARRAY target_buffer, PIXEL_ARRAY grayscale_image, IMAGE_CONTEXT ctx){
    ZEROED_ARRAY(int, histogram, DISTINCT_BYTE_VALUES);

    ITERATE_IMAGE {
        INCREMENT(histogram[PIXEL_OF_ITERATION(grayscale_image)]);
    }

    ZEROED_ARRAY(int, target_color, DISTINCT_BYTE_VALUES);

    FOR_RANGE(i, 1, DISTINCT_BYTE_VALUES){
        SET(target_color[i], calculate_equalization(ROLL_SUM_ARRAY(histogram, i), SIZE));
    }

    ITERATE_IMAGE {
        SET(PIXEL_OF_ITERATION(target_buffer), target_color[PIXEL_OF_ITERATION(grayscale_image)]);
    }
}

int X_OFFSET[] = {-1, -1, -1,  0, 0, 0,  1, 1, 1};
int Y_OFFSET[] = {-1,  0,  1, -1, 0, 1, -1, 0, 1};

int X_OFFSET_WINDOW_3X3_COLUMN[] = {-1,  0,  1,-1, 0, 1};
int Y_OFFSET_WINDOW_3X3_COLUMN[] = {-1, -1, -1, 1, 1, 1};

int X_OFFSET_WINDOW_3X3_ROW[] = {-1, -1, -1, 1, 1, 1};
int Y_OFFSET_WINDOW_3X3_ROW[] = {-1,  0,  1,-1, 0, 1};
int get_median_3x3(PIXEL_ARRAY median_target, IMAGE_CONTEXT ctx){

}
void median_filter(PIXEL_ARRAY target_buffer, PIXEL_ARRAY grayscale_image, IMAGE_CONTEXT ctx){

}


