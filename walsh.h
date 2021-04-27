#include "macros.h"


#define BIT(number, nth) (!!(number & (1 << nth)))


double kernel_function(int x, int y, int u, int v){
    double sum = 1;
    const int n = 6;
    for(int i = 0; i < n; i++ ){
        sum *= (((BIT(x, i) * BIT(u, n - i - 1)) + (BIT(y, i) * BIT(v, n - i - 1))) % 2) == 1 ? -1 : 1;
    }
    return sum * ((double) 1 / 64.0);
} 

double walsh_transform(unsigned char *target, int u, int v){
    IMAGE_CONTEXT ctx = { .width = 64, .height = 64, .channels = 1};
    int x,y;
    double sum = 0;
    for(x = 0; x < 64; x++) {
        for(y = 0; y < 64; y++) {
            sum += GET_PIXEL(target, x, y) * kernel_function(x, y, u, v);
        }
    }
    return sum;
}


double *generate_feat_vectors(unsigned char *target, int uv){
    int number_of_vectors = 0;
    double *feat_vectors = (double *)calloc(uv * uv, sizeof(double));
    for(int i = 0; i < uv; i++) {
        for(int j = 0; j < uv; j++) {
            feat_vectors[number_of_vectors++] = walsh_transform(target, i, j);
        }
    }
    return feat_vectors;
}