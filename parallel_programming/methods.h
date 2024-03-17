//
// Created by Andrey on 23/02/2024.
//
#include <windows.h>
#include <pthread.h>
#include <thread>

#include "bmp_reader.h"

#ifndef METHODS_H
#define METHODS_H

int max(int x, int y);
// brute
unsigned short int  color_predominance_in_pixel(unsigned int R, unsigned int G, unsigned int B);
void color_predominance_without_parallel(BMP_file* bmpf);
// CreateThread with CRITICAL_SECTION
DWORD __stdcall f_for_cthread(void *arg);
void color_predominance_with_cthread(BMP_file* bmpf, int NUM_OF_THREADS = 7);
// pthread_create
void* f_for_pthread(void *arg);
void color_predominance_with_pthread(BMP_file *bmpf, int NUM_OF_THREADS = 7);
//std::thread
void f_for_stdthread(int RGB_t[4] ,const unsigned char* data, int num, int data_size, int bytes_per_pixel, int num_of_thread);
void color_predominance_with_stdthread(BMP_file *bmpf, int NUM_OF_THREADS = 7);
//OMP
void color_predominance_with_OMP(BMP_file *bmpf, int NUM_OF_THREADS);
// CreateProcess with running processes from under another user
void color_predominance_with_cprocess(char* file_name, int NUM_OF_PROCESS);


#endif //METHODS_H
