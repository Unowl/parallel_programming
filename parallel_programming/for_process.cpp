//
// Created by Andrey on 11/03/2024.
//
#include <stdio.h>
#include <bitset>

#include "bmp_reader.h"
#include "methods.h"

void mwrite(int *m, int len, char *name){
    FILE* f = fopen(name, "wb");
    for(int i=0; i<len; i++)
        fprintf(f,"%d ",m[i]);
    fclose(f);
}


int main(int argc, char* argv[]) {

    if (argc != 3) printf("Error argc");

    unsigned int num =  atoi(argv[0]);  // number of process
    //unsigned int data_size = atoi(argv[1]);
    //unsigned int bytes_per_pixel = atoi(argv[2]);
    unsigned int num_of_process = atoi(argv[1]);
    char* fn = argv[2];

    BMP_file* bmpf = load_BMP_file(fn);

    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;

    int start = num * (data_size / bytes_per_pixel / num_of_process * bytes_per_pixel);
    int end = (num + 1) * (data_size / bytes_per_pixel / num_of_process * bytes_per_pixel);
    if (num == num_of_process - 1) end = data_size;

    int RGB[4];

    for (int & i : RGB)
        i = 0;

    for (int i = start; i < end; i += bytes_per_pixel)
        RGB[color_predominance_in_pixel(bmpf->data[i], bmpf->data[i + 1], bmpf->data[i + 2])]++;;


    char path[256];
    sprintf(path, "C:\\parallel_programming\\res_%d.txt", num);
    mwrite(RGB, 4, path);

}