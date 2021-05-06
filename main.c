#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "macros.h"
#include "walsh.h"
#include "args.h"

#define BINARY_TRESHOLD 200
#define MAX_NUMBER_OF_SHAPES 3000
#define FEATURE_VECTOR_SAMPLING 16
#define NUMBER_OF_FEATURE_VECTORS FEATURE_VECTOR_SAMPLING * FEATURE_VECTOR_SAMPLING

#define MAX_NUMBER_OF_LEARNED_SHAPES 5000

#define PIXEL_COUNT_CUTOFF 20

//brightness transformation
    //grays-scale
        //tresholding
            //binarization -> one treshold value
    //brightness corrections


// ocr -segment -in filename -out filename
// ocr -extract -db directory -in filename -out filename

int binary_treshold = BINARY_TRESHOLD;

int debug = 0;

int main(int argc, char **argv) {

    init_walsh();

    if(has_arg(argv, argc, "-debug")){
        debug = 1;
    }

    if(has_arg(argv, argc, "-b")){
        binary_treshold = atoi(argv[get_index_of_param(argv, argc, "-b")]);
    }

    if(has_arg(argv, argc, "-segment")){
        return segment(argv, argc);
    } else if(has_arg(argv, argc, "-extract")) {
        return extract(argv, argc);
    } 
    
    return -1;
}


// read pic ---
// segment shapes ---
// calculate vectors ---
// save shapes and vectors to output ---
int segment(char **argv, int argc) {
    IMAGE_CONTEXT ctx = read_image(argv[get_index_of_param(argv, argc, "-in")]);
    
    ALLOCATE_BUFFER(grayscale_image, SIZE);
    CALL_PROC(convert_to_grayscale, grayscale_image, ctx.image_start);
    SAVE_IMAGE("output/gray-scaled.jpg", grayscale_image);
    CHANNELS = 1;
    
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
int extract(char **argv, int argc){

    int index_of_db = get_index_of_param(argv, argc, "-db");
    if(index_of_db < 0){
        fprintf(stderr, "No input directory provided!\n");
        return 1;
    }

    int index_of_img = get_index_of_param(argv, argc, "-in");
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
    printf("\tfound shapes = %d\n", collection_count);
    DEBUG("Calculating feature vectors");
    calculate_feature_vectors(collection, collection_count);

    char extracted_characters[10000] = {'\0'};
    int extracted_characters_length = 0;

    DEBUG("Matching feature vectors");
    int avg_width = 0;
    FOR(collection_index, collection_count){
        
        int max_match_index = -1;
        int max_match = INT_MAX;

        FOR(database_index, database_count){
            IMAGE_CONTEXT new_shape = collection[collection_index];
            IMAGE_CONTEXT learned_shape = database[database_index];
            
            int diff = compare_vectors_a(new_shape.feature_vectors, learned_shape.feature_vectors, NUMBER_OF_FEATURE_VECTORS);
           
            if(diff < max_match){
                max_match = diff;
                max_match_index = database_index;
            }
        }
        if(debug) printf("found max match: %d\n", max_match);
        avg_width += collection[collection_index - 1].original_width;
        if(max_match_index != -1){
            if(debug) printf("MATCHED: %3c\n", database[max_match_index].character);
            if(collection_index > 1){
                if( collection[collection_index - 1].start_y >= collection[collection_index].start_y){
                    CONCAT(extracted_characters, '\n');
                } else if( (collection[collection_index - 1].start_y +  collection[collection_index - 1].original_width + (avg_width / (double)collection_index) * 0.5) < collection[collection_index].start_y) {
                    CONCAT(extracted_characters, ' ');
                }
            }
            CONCAT(extracted_characters, database[max_match_index].character);
        }
    }
    DEBUG("Extracted text");
    puts("----------------------------------------------------------------------");
    printf("%s\n", extracted_characters);
    puts("----------------------------------------------------------------------\n");
    return 0;
}

double compare_vectors_a(double *a, double *b, int count) {
    double sum = 0;
    FOR(i, count) sum += ABS(a[i] - b[i]);
    return sum;
}

IMAGE_CONTEXT read_and_binarize_img(char *file_name) {
    IMAGE_CONTEXT ctx = read_image(file_name);
    
    ALLOCATE_BUFFER(grayscale_image, SIZE);
    CALL_PROC(convert_to_grayscale, grayscale_image, ctx.image_start);
    CHANNELS = 1;


    ALLOCATE_BUFFER(binary_image, SIZE);
    CALL_PROC(binarize_image, binary_image, grayscale_image);
    SAVE_IMAGE("output/binary_image.jpg", binary_image);
    
    free(ctx.image_start);
    free(grayscale_image);

    ctx.image_start = binary_image;
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
        APPEND_PIXEL(target, BINARIZE(PIXEL_OF_ITERATION(original), binary_treshold));
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

    ITERATE_IMAGE_INTERLEAVED(1) {
        if(is_shape(PIXEL_OF_ITERATION(original))) {

            NEW_RECURSION_CONTEXT();

            PAINT_SHAPE();

            if(shape_points_count > PIXEL_COUNT_CUTOFF) {
                ALLOCATE_BUFFER(target, RECURSION_SIZE);
                WHITEN(target, RECURSION_SIZE);

                NEW_IMAGE_CONTEXT(target);
                int offset = j;
                FOR_RANGE(index, 1, shape_points_count){
                    if(shape_points[index].y < offset){
                        offset = shape_points[index].y;
                    }
                    GET_CONTEXTED_PIXEL(target, target_ctx, (shape_points[index].x - shape_points[0].x), (shape_points[index].y - shape_points[0].y)) = 0;
                }
                PUSH_SHAPE(target_ctx);
                collected_shapes[collected_shapes_count - 1].start_y = offset ;
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
        scaled.original_width = shape.width;
        scaled.height = 64;
        scaled.width = 64;
        scaled.channels = 1;
        scaled.image_start = scaled_buffer;
        binarize_image(scaled_buffer, scaled_buffer, scaled);
        
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
            fvout = fopen(file_name, "wb+");

            fwrite(collection[image].feature_vectors, sizeof(double), NUMBER_OF_FEATURE_VECTORS, fvout);
            free(collection[image].feature_vectors);
        } else {
            puts("\n[!!! Feat vectors must be initialized!!!]\n");
        }
        fclose(fvout);
    }
}


void initialize_database(IMAGE_CONTEXT *database, int *database_count, const char *dir) {
    DEBUG("Initializing database");
    read_directory(dir, "digits", '0', '9', database, database_count);
    read_directory(dir, "lower", 'a', 'z', database, database_count);
    read_directory(dir, "upper", 'A', 'Z', database, database_count);
    printf("\tDatabase entries read: %5d\n", *database_count);
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
            fv = read_feature_vector(file_name);
            if(fv) {
                if(debug){
                    printf("\t%s\n", file_name);
                }
                database[count].character = character;
                database[count++].feature_vectors = fv;
            }
            read_files++;
        }
    }
    *database_count = count;
}


double *read_feature_vector(const char *file_name) {
    FILE *vector_pointer = fopen(file_name, "rb");
    double *fv = NULL;

    if(vector_pointer){
        fv = (double *)calloc(NUMBER_OF_FEATURE_VECTORS, sizeof(double));
        int readData = fread(fv, sizeof(double), NUMBER_OF_FEATURE_VECTORS, vector_pointer);
        fclose(vector_pointer);
    }
    
    return fv;
}


IMAGE_CONTEXT read_image(char *file_name) {
    IMAGE_CONTEXT ctx;
    DEBUG("Loading image");

    ctx.image_start = stbi_load(file_name, &WIDTH, &HEIGHT, &CHANNELS, 0);
    if(ctx.image_start == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }

    printf("\tLoaded image: [%d X %d X %d] - [%s]\n", WIDTH, HEIGHT, CHANNELS, file_name);

    return ctx;
}
