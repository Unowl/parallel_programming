//
// Created by Andrey on 22/02/2024.
//

#include <stdio.h>
#include <stdlib.h>

#include "bmp_reader.h"



void read_BMP_row_by_row(FILE* fp, BMP_file* bmpf) {
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel/8;
    int row_size = bytes_per_pixel * bmpf->dibhdr.wight;
    int row_padding = (4 - (row_size % 4)) % 4;
    int rows_written = 0;
    unsigned char* row = (unsigned char*)malloc(row_size+row_padding);
    unsigned char* p = &bmpf->data[(bmpf->dibhdr.height-1)*row_size];
    fseek(fp, bmpf->bmphdr.pixel_offset, SEEK_SET);
    while(rows_written < bmpf->dibhdr.height) {
        fread(row, row_size+row_padding, 1, fp);
        if(bytes_per_pixel == 3){
            for(int i =0; i<row_size; i+=bytes_per_pixel) {
              *p = row[i+2]; p++;
              *p = row[i+1]; p++;
              *p = row[i]; p++;
            };
        } else if(bytes_per_pixel == 4) {
            for(int i =0; i<row_size; i+=bytes_per_pixel) {
                *p = row[i+3]; p++;
                *p = row[i+2]; p++;
                *p = row[i+1]; p++;
                *p = row[i]; p++;
            };
        } else {
            printf("Error: don't working with bytes_per_pixel = %d\n", bytes_per_pixel);
        };
        rows_written++;
        p = p - 2*row_size;
    };
    free(row);
};

BMP_file* load_BMP_file(char* file_name){
    FILE* fp = fopen(file_name, "r");
    if(!fp) {
        printf("Can't open file \'%s\'\n", file_name);
        exit(0);
    };

    BMP_file* bmpf = (BMP_file*)malloc(sizeof(BMP_file));
    fread(&bmpf->bmphdr, sizeof(BMP_Header), 1, fp);
    fread(&bmpf->dibhdr, sizeof(DIB_Header), 1, fp);

    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;

    bmpf->data = (unsigned char*)malloc(data_size);
    read_BMP_row_by_row(fp, bmpf);
    fclose(fp);
    return bmpf;
};

void print_BMP_Headers(BMP_file* bmpf){
    printf("ID=%c%c\nfile_size=%d\npixel_offset=%d\n",
           bmpf->bmphdr.ID[0], bmpf->bmphdr.ID[1], bmpf->bmphdr.file_size, bmpf->bmphdr.pixel_offset);
    printf("header_size=%d\nwight=%d\n"
           "height=%d\ncolor_planes=%d\n"
           "bits_per_pixel=%d\ncompress=%d\n"
           "data_size=%d\npixel_width=%d\n"
           "pixel_height=%d\ncolor_count=%d\n"
           "imp_colors_count=%d\n",
           bmpf->dibhdr.header_size, bmpf->dibhdr.wight,
           bmpf->dibhdr.height, bmpf->dibhdr.color_planes,
           bmpf->dibhdr.bits_per_pixel, bmpf->dibhdr.comp,
           bmpf->dibhdr.data_size, bmpf->dibhdr.pixel_width,
           bmpf->dibhdr.pixel_height, bmpf->dibhdr.color_count,
           bmpf->dibhdr.imp_colors_count);
};

void print_BMP_pixels(BMP_file* bmpf){
    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    for(int i=0; i<data_size; i++){
        if(i % 16 == 0)
            printf("\n%04x: ", i);
        printf("%02x ", bmpf->data[i]);
    };
    printf("\n");
};

void free_BMP_file(BMP_file* bmpf) {
    if(bmpf) {
        free(bmpf);
    };
};


