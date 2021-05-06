#include "macros.h"


#define BIT(number, nth) (!!(number & (1 << nth)))


int kernel[64][64] = {0};

void mini_kernel(int x, int u){
    kernel[x][u] = (BIT(x, 0) * BIT(u, 5)) % 2 == 1 ? -1 : 1;
    kernel[x][u] *= (BIT(x, 1) * BIT(u, 4)) % 2 == 1 ? -1 : 1;
    kernel[x][u] *= (BIT(x, 2) * BIT(u, 3)) % 2 == 1 ? -1 : 1;
    kernel[x][u] *= (BIT(x, 3) * BIT(u, 2)) % 2 == 1 ? -1 : 1;
    kernel[x][u] *= (BIT(x, 4) * BIT(u, 1)) % 2 == 1 ? -1 : 1;
    kernel[x][u] *= (BIT(x, 5) * BIT(u, 0)) % 2 == 1 ? -1 : 1;
}

void init_walsh(){
    for(int x = 0; x < 64; x++){
        for(int y = 0; y < 64; y++){
            mini_kernel(x, y);
        }
    }
}

double walsh_transform(unsigned char *target, int u, int v){
    IMAGE_CONTEXT ctx = { .width = 64, .height = 64, .channels = 1};
    int x,y;
    double sum = 0;
    for(x = 0; x < 64; x++) {
        for(y = 0; y < 64; y++) {
            sum += (GET_PIXEL(target, x, y)) * kernel[x][u] * kernel[y][v];
        }
    }
    return sum * ((double) 1 / 64.0);
}


double *generate_feat_vectors(unsigned char *target, int uv){
    int number_of_vectors = 0;
    double *feat_vectors = (double *)calloc(uv * uv, sizeof(double));
    for(int i = 1; i <= uv; i++) {
        for(int j = 1; j <= uv; j++) {
            feat_vectors[number_of_vectors++] = walsh_transform(target, i, j);
        }
    }
    return feat_vectors;
}