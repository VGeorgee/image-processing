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
    int x,y;
    double sum = 0;
    for(x = 0; x < 64; x++) {
        for(y = 0; y < 64; y++) {
            sum += target[x] * kernel_function(x, y, u, v);
        }
    }
    return sum;
}
