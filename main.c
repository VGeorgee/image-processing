#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "macros.h"
#include "walsh.h"
#include "args.h"

#define BINARY_TRESHOLD 140
#define MAX_NUMBER_OF_SHAPES 1500
#define FEATURE_VECTOR_SAMPLING 16
#define NUMBER_OF_FEATURE_VECTORS FEATURE_VECTOR_SAMPLING * FEATURE_VECTOR_SAMPLING

#define MAX_NUMBER_OF_LEARNED_SHAPES 5000

//brightness transformation
    //grays-scale
        //tresholding
            //binarization -> one treshold value
    //brightness corrections


   // NEW_LIST(LEARNED_IMAGE_CONTEXT, learned_characters, )


// ocr -segment -in filename -out filename
// ocr -extract -db directory -in filename -out filename


int main(int argc, char **argv) {
    //printf("%p\n", read_feature_vector(argv[1]));
    //read_image(argv[1]);

/*
    NEW_LIST(IMAGE_CONTEXT, database, 1400);
    initialize_database(database, &database_count, argv[1]);
    //read_directory("input", "supervised/digits", '0', '9', database, &database_count);
    FOR(i, database_count){
        printf("element: %c %p\n", database[i].character, database[i].feature_vectors);
    }
  */
    extract(argv);
    return 0;
}

int main2(int argc, char **argv) {
    if(argc < 6) {
        fprintf(stderr, "format: %s [-segment|-extract] -in filename -out filename", crop_filename(argv[0]));
        return 1;
    }

    segment(argv);

    return 0;

    IMAGE_CONTEXT ctx;

    PIXEL_ARRAY img = stbi_load(argv[1], &WIDTH, &HEIGHT, &CHANNELS, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("\nLoaded image with a width of %dpx, a height of %dpx and %d channels\n", WIDTH, HEIGHT, CHANNELS);


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


    NEW_LIST(IMAGE_CONTEXT, collection, MAX_NUMBER_OF_SHAPES);
    collect_shapes(binarized, collection, &collection_count, ctx);
    save_collection(collection, collection_count, "");


    puts("Collection ran succesfully!");

    free(treshold_buffer);
    free(gray_img);
    free(histogram_equalized_img);
    free(binarized);

    return 0;
}

// read pic ---
// segment shapes ---
// calculate vectors ---
// save shapes and vectors to output ---
int segment(char **argv) {
    IMAGE_CONTEXT ctx = read_image(argv[get_index_of_param(argv, "-in")]);
    
    ALLOCATE_BUFFER(grayscale_image, SIZE);
    CALL_PROC(convert_to_grayscale, grayscale_image, ctx.image_start);
    SAVE_IMAGE("output/gray-scaled.jpg", grayscale_image);
    CHANNELS = 1;

    // MAKE("output/tresholded.jpg", treshold_buffer, grayscale_image, treshold_image);
    MAKE("output/histogram-equalized.jpg", histogram_equalized_img, grayscale_image, histogram_equalization);
    MAKE("output/median-filtered.jpg", median_fileter_image, grayscale_image, median_filter);
    MAKE("output/binarized.jpg", binarized, grayscale_image, binarize_image);
    
    ctx.image_start = binarized;

    NEW_LIST(IMAGE_CONTEXT, collection, MAX_NUMBER_OF_SHAPES);

    DEBUG("Collecting shapes");
    collect_shapes(binarized, collection, &collection_count, ctx);

    DEBUG("Calculating feature vectors");
    calculate_feature_vectors(collection, collection_count);

    DEBUG("Saving collection");
    save_collection(collection, collection_count, "");

    puts("Ran succesfully!");

    free(histogram_equalized_img);
    free(binarized);
    free(grayscale_image);

    return 0;
}

// initialize database ---
// read pic ---
// segment shapes
// calculate vectors
// search vectors
// save text to output
int extract(char **argv){

    int index_of_db = get_index_of_param(argv, "-db");
    if(index_of_db < 0){
        fprintf(stderr, "No input directory provided!\n");
        return 1;
    }


    int index_of_img = get_index_of_param(argv, "-in");
    if(index_of_img < 0){
        fprintf(stderr, "No input image provided!\n");
        return 1;
    }
    
    NEW_LIST(IMAGE_CONTEXT, database, MAX_NUMBER_OF_LEARNED_SHAPES);
    initialize_database(database, &database_count, argv[index_of_db]);

    IMAGE_CONTEXT ctx = read_and_binarize_img(argv[index_of_img]);
    NEW_LIST(IMAGE_CONTEXT, collection, MAX_NUMBER_OF_SHAPES);
    DEBUG("Collecting shapes");
    collect_shapes(ctx.image_start, collection, &collection_count, ctx);
    printf("found shapes = %d\n", collection_count);
    DEBUG("Calculating feature vectors");
    calculate_feature_vectors(collection, collection_count);



    /////
    for(int i = 0; i < 256; i++){
        printf("%f\n", collection[0].feature_vectors[i]);
    }
    save_collection(collection, collection_count, "");


    /////


    DEBUG("Matching feature vectors");

    int max_match_index = -1;
    int max_match = 0;
    FOR(collection_index, collection_count){
        FOR(database_index, database_count){
            IMAGE_CONTEXT new_shape = collection[collection_index];
            IMAGE_CONTEXT learned_shape = database[database_index];
            printf("matching begin for indexes: |%d| |%d|\n", collection_index, database_index);
            int match = 0;
            for(int vector_index = 0; vector_index < NUMBER_OF_FEATURE_VECTORS; vector_index++){
                //printf("%f == %f", new_shape.feature_vectors[vector_index], learned_shape.feature_vectors[vector_index]);
                if(new_shape.feature_vectors[vector_index] == learned_shape.feature_vectors[vector_index]){
                    match++;
                }
            }
            if(match > max_match){
                max_match = match;
                max_match_index = database_index;
                printf("dound max match: %c  with match: %d\n", database[database_index].character, match);
            }
        }
    }
    if(max_match_index != -1){
        printf("MATCHED::::::::::: %20c\n", database[max_match_index].character);
    } else {
        puts("NO MATCH FOUND!");
    }

}


IMAGE_CONTEXT read_and_binarize_img(char *file_name) {
    IMAGE_CONTEXT ctx = read_image(file_name);
    
    ALLOCATE_BUFFER(grayscale_image, SIZE);
    CALL_PROC(convert_to_grayscale, grayscale_image, ctx.image_start);
    SAVE_IMAGE("output/gray-scaled.jpg", grayscale_image);
    CHANNELS = 1;
    free(ctx.image_start);
    ctx.image_start = grayscale_image;
    return ctx;
}

void calculate_feature_vectors(IMAGE_CONTEXT *collection, int collection_count) {
    FOR(i, collection_count) {
        collection[i].feature_vectors = generate_feat_vectors(collection[i].image_start, FEATURE_VECTOR_SAMPLING);
    }
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
        APPEND_PIXEL(target, BINARIZE(PIXEL_OF_ITERATION(original), BINARY_TRESHOLD));
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

        
        FILE *fvout;
        if(collection[image].feature_vectors) {
            sprintf(file_name, "output/shapes/%d.fv", image);
            puts(file_name);
            fvout = fopen(file_name, "w+");

/*
            for(int i = 0; i < 256; i++){
                printf("%f\n", collection[image].feature_vectors[i]);
            }
  
  */
            fwrite(collection[image].feature_vectors, sizeof(double), NUMBER_OF_FEATURE_VECTORS, fvout);
            free(collection[image].feature_vectors);
        } else {
            puts("\n[!!! Feat vectors must be initialized!!!]\n");
        }
        fclose(fvout);
    }
}


void initialize_database(IMAGE_CONTEXT *database, int *database_count, const char *dir) {
    read_directory(dir, "digits", '0', '9', database, database_count);
    read_directory(dir, "lower", 'a', 'z', database, database_count);
    read_directory(dir, "upper", 'A', 'Z', database, database_count);
}

void read_directory(const char *dir_prefix, const char *dir, const char start, const char end, IMAGE_CONTEXT *database, int *database_count) {

    int count = *database_count;
    char dir_prefix_trimmed[200];
    strcpy(dir_prefix_trimmed, dir_prefix);
    trim_dir_backslash(dir_prefix_trimmed);

    for(char character = start; character <= end; character++) {
        if(count >= MAX_NUMBER_OF_LEARNED_SHAPES){
            fprintf(stderr, "MAX NUMBER OF DATABASE REACHED! [change MAX_NUMBER_OF_LEARNED_SHAPES macro to reach higher capacity]\n");
            return;
        }

        double *fv = (double *)0x1;
        int read_files = 0;

        char file_name[1000];
        while(fv) {
            sprintf(file_name, "%s\\%s\\%c\\%d.fv", dir_prefix_trimmed, dir, character, read_files);
            puts(file_name);
            fv = read_feature_vector(file_name);
            if(fv) {
                database[count].character = character;
                database[count++].feature_vectors = fv;
            }
            read_files++;
        }
        *database_count = count;
    }
}



double *read_feature_vector(const char *file_name) {
    FILE *vector_pointer = fopen(file_name, "r");
    double *fv = NULL;

    if(vector_pointer){
        fv = (double *)calloc(NUMBER_OF_FEATURE_VECTORS, sizeof(double));
        int readData = fread(fv, sizeof(double), NUMBER_OF_FEATURE_VECTORS, vector_pointer);

        for(int i = 0; i < NUMBER_OF_FEATURE_VECTORS; i++){
            printf("%f\n", fv[i]);
        }
        fclose(vector_pointer);
    }
    
    return fv;
}


IMAGE_CONTEXT read_image(char *file_name) {
    IMAGE_CONTEXT ctx;

    ctx.image_start = stbi_load(file_name, &WIDTH, &HEIGHT, &CHANNELS, 0);
    if(ctx.image_start == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", WIDTH, HEIGHT, CHANNELS);

    return ctx;
}


/*
int compare_vectors(PIXEL_ARRAY a, PIXEL_ARRAY b, int size) {
    int diff = 0;
    FOR(index, size * size) {
        diff += ABS(VALUE(a++) - VALUE(b++));
    }
    return diff;
}
*/
