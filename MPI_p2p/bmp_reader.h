//
// Created by Andrey on 22/02/2024.
//

#ifndef BMP_READER_H
#define BMP_READER_H

#pragma pack(1)
typedef struct BMP_Header {
    unsigned char ID[2];
    unsigned int file_size;
    unsigned char unused[4];
    unsigned int pixel_offset;
} BMP_Header;

typedef struct DIB_Header {
    unsigned int header_size;
    unsigned int wight;
    unsigned int height;
    unsigned short color_planes;
    unsigned short bits_per_pixel;
    unsigned int comp;
    unsigned int data_size; //что-то не так
    unsigned int pixel_width;
    unsigned int pixel_height;
    unsigned int color_count;
    unsigned int imp_colors_count;
} DIB_Header;

typedef struct BMP_file {
    BMP_Header bmphdr;
    DIB_Header dibhdr;
    unsigned char* data;
} BMP_file;
#pragma pop

void read_BMP_row_by_row(FILE* fp, BMP_file* bmpf);
BMP_file* load_BMP_file(char* file_name);
void print_BMP_Headers(BMP_file* bmpf);
void print_BMP_pixels(BMP_file* bmpf);
void free_BMP_file(BMP_file* bmpf);

#endif //BMP_READER_H
