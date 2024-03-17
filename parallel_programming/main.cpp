
#include <stdio.h>
#include <time.h>

#include "bmp_reader.h"
#include "methods.h"

// X = 3 * 205 + 1 = 616
// A = X % 4 = 0
// B = 7 + X % 7 = 7

int B = 7;

int main() {
    char fn[] =  "C:\\parallel_programming\\test2.bmp"; // path to img
    BMP_file* bmpf = load_BMP_file(fn);
    //print_BMP_Headers(bmpf);
    printf("--------------------\n");
    clock_t begin = clock();

    //color_predominance_without_parallel(bmpf); // brute
    //color_predominance_with_cthread(bmpf, B); // CreateThread with CRITICAL_SECTION
    //color_predominance_with_pthread(bmpf, B); //pthread_create
    //color_predominance_with_stdthread(bmpf, B); //std::thread
    color_predominance_with_OMP(bmpf, B); // OMP
    //color_predominance_with_cprocess(fn, B); // CreateProcess with running processes from under another user

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("--------------------\n");
    printf("time: %f sec\n", time_spent);
    free_BMP_file(bmpf);
    return 0;
}

