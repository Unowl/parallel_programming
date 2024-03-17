//
// Created by Andrey on 23/02/2024.
//

#include <stdio.h>
#include <windows.h>
#include <pthread.h>
#include <thread>
#include <vector>
#include <omp.h>

#include <unistd.h>


#include "bmp_reader.h"
#include "methods.h"



int RGB[4];

int max(int x, int y)
{
    return x > y ? x : y;
};

unsigned short int  color_predominance_in_pixel(unsigned int R, unsigned int G, unsigned int B){
    if (R > max(G,B)){
        return 0;
    } else if (G > max(R,B)){
        return 1;
    } else if (B > max(G,R)) {
        return 2;
    } else{
        return 3;
    };
};

//brute

void color_predominance_without_parallel(BMP_file* bmpf){
    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
    for (int & i : RGB)
        i = 0;
    for(int i=0; i<data_size; i+=bytes_per_pixel){
        RGB[color_predominance_in_pixel(bmpf->data[i], bmpf->data[i+1], bmpf->data[i+2])]++;
    };
    printf("pixels with a predominant red color=%d\n"
           "pixels with a predominant green color=%d\n"
           "pixels with a predominant blue color=%d\n"
           "pixels without a predominant color=%d\n",
           RGB[0], RGB[1], RGB[2], RGB[3]);
};

// CreateThread with CRITICAL_SECTION

struct ARGS{
    int num;
    unsigned char* data;
    int data_size;
    int bytes_per_pixel;
    int num_of_thread;
};

CRITICAL_SECTION cs;

DWORD __stdcall f_for_cthread(void *arg){
    ARGS* t = (ARGS*)arg;

    int start = t->num * (t->data_size / t->bytes_per_pixel / t->num_of_thread * t->bytes_per_pixel);
    int end = (t->num + 1) * (t->data_size / t->bytes_per_pixel / t->num_of_thread * t->bytes_per_pixel);
    if (t->num == t->num_of_thread - 1) end = t->data_size;

    int RGB_t[4];
    for (int & i : RGB_t)
        i = 0;

    for (int i = start; i < end; i += t->bytes_per_pixel)
        RGB_t[color_predominance_in_pixel(t->data[i], t->data[i + 1], t->data[i + 2])]++;;

    EnterCriticalSection(&cs);
    for (int i = 0; i < 4; i++)
        RGB[i] += RGB_t[i];

    LeaveCriticalSection(&cs);

    //printf("thread %d\n", t->num);
    return 0;
}

void color_predominance_with_cthread(BMP_file *bmpf, int NUM_OF_THREADS) {
    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
    for (int & i : RGB)
        i = 0;

    ARGS param[NUM_OF_THREADS];
    DWORD th_id[NUM_OF_THREADS];
    HANDLE th_h[NUM_OF_THREADS];

    InitializeCriticalSection(&cs);
    for (int i=0; i<NUM_OF_THREADS; i++){
        param[i].num = i;
        param[i].data = bmpf->data;
        param[i].bytes_per_pixel = bytes_per_pixel;
        param[i].data_size = data_size;
        param[i].num_of_thread = NUM_OF_THREADS;
        th_h[i] = CreateThread(NULL, 0, f_for_cthread, param+i, 0, th_id+1);
        if (!th_h[i]) printf("error %d\n", i);
    }

    WaitForMultipleObjects(NUM_OF_THREADS, th_h, TRUE, INFINITE);
    DeleteCriticalSection(&cs);

    printf("pixels with a predominant red color=%d\n"
           "pixels with a predominant green color=%d\n"
           "pixels with a predominant blue color=%d\n"
           "pixels without a predominant color=%d\n",
           RGB[0], RGB[1], RGB[2], RGB[3]);
}

// pthread_create

struct ARGS_pthread{
    int num;
    unsigned char* data;
    int data_size;
    int bytes_per_pixel;
    int num_of_thread;
    int RGB[4];
};

void* f_for_pthread(void *arg) {
    ARGS_pthread* t = (ARGS_pthread*)arg;

    int start = t->num * (t->data_size / t->bytes_per_pixel / t->num_of_thread * t->bytes_per_pixel);
    int end = (t->num + 1) * (t->data_size / t->bytes_per_pixel / t->num_of_thread * t->bytes_per_pixel);
    if (t->num == t->num_of_thread - 1) end = t->data_size;

    for (int & i : t->RGB)
        i = 0;

    for (int i = start; i < end; i += t->bytes_per_pixel)
        t->RGB[color_predominance_in_pixel(t->data[i], t->data[i + 1], t->data[i + 2])]++;;

    //printf("thread %d\n", t->num);
    return 0;

};

void color_predominance_with_pthread(BMP_file *bmpf, int NUM_OF_THREADS){
    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
    for (int & i : RGB)
        i = 0;

    ARGS_pthread param[NUM_OF_THREADS];
    int status;
    pthread_t threads[NUM_OF_THREADS];
    int status_addr;

    for (int i=0; i<NUM_OF_THREADS; i++){
        param[i].num = i;
        param[i].data = bmpf->data;
        param[i].bytes_per_pixel = bytes_per_pixel;
        param[i].data_size = data_size;
        param[i].num_of_thread = NUM_OF_THREADS;
        status = pthread_create(&threads[i], NULL, f_for_pthread, (void*) &param[i]);
        if (status != 0) {
            printf("error: can't create thread, status = %d\n", status);
        }
    };


    for (int i = 0; i < NUM_OF_THREADS; i++) {
        status = pthread_join(threads[i], (void**)&status_addr);
        if (status != 0)
            printf("error: can't join thread, status = %d\n", status);

    };

    for (int j = 0; j < NUM_OF_THREADS; j++){
        for (int i = 0; i < 4; i++)
            RGB[i] += param[j].RGB[i];
    }
    

    printf("pixels with a predominant red color=%d\n"
           "pixels with a predominant green color=%d\n"
           "pixels with a predominant blue color=%d\n"
           "pixels without a predominant color=%d\n",
           RGB[0], RGB[1], RGB[2], RGB[3]);
}

//std::thread

void f_for_stdthread(int RGB_t[4] ,const unsigned char* data, int num, int data_size, int bytes_per_pixel, int num_of_thread) {

    int start = num * (data_size / bytes_per_pixel / num_of_thread * bytes_per_pixel);
    int end = (num + 1) * (data_size / bytes_per_pixel / num_of_thread * bytes_per_pixel);
    if (num == num_of_thread - 1) end = data_size;

    for (int i=0; i<4; i++)
        RGB_t[i] = 0;


    for (int i = start; i < end; i += bytes_per_pixel)
        RGB_t[color_predominance_in_pixel(data[i], data[i + 1], data[i + 2])]++;

};


void color_predominance_with_stdthread(BMP_file *bmpf, int NUM_OF_THREADS){

    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
    for (int & i : RGB)
        i = 0;
    int RGB_t[NUM_OF_THREADS][4];
    std::vector<std::thread> threads;
    const unsigned char *data = bmpf->data;

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        threads.emplace_back(f_for_stdthread,  RGB_t[i],  data, i, data_size, bytes_per_pixel, NUM_OF_THREADS);
    }


    for(auto&& i : threads) {
        i.join();
    }

    for (int j = 0; j < NUM_OF_THREADS; j++){
        for (int i = 0; i < 4; i++)
            RGB[i] += RGB_t[j][i];
    }

    printf("pixels with a predominant red color=%d\n"
           "pixels with a predominant green color=%d\n"
           "pixels with a predominant blue color=%d\n"
           "pixels without a predominant color=%d\n",
           RGB[0], RGB[1], RGB[2], RGB[3]);
}

// OMP

void color_predominance_with_OMP(BMP_file *bmpf, int NUM_OF_THREADS){

    int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
    int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
    for (int & i : RGB)
        i = 0;

    omp_set_num_threads(NUM_OF_THREADS);
    #pragma omp parallel
    {
        int num = omp_get_thread_num();
        int start = num * (data_size / bytes_per_pixel / NUM_OF_THREADS * bytes_per_pixel);
        int end = (num + 1) * (data_size / bytes_per_pixel / NUM_OF_THREADS * bytes_per_pixel);
        if (num == NUM_OF_THREADS - 1) end = data_size;

        int RGB_t[4];
        for (int & i : RGB_t)
            i = 0;

        for (int i = start; i < end; i += bytes_per_pixel)
            RGB_t[color_predominance_in_pixel(bmpf->data[i], bmpf->data[i + 1], bmpf->data[i + 2])]++;
        #pragma omp critical
        {
            for (int i = 0; i < 4; i++)
                RGB[i] += RGB_t[i];
        }
    }


    printf("pixels with a predominant red color=%d\n"
           "pixels with a predominant green color=%d\n"
           "pixels with a predominant blue color=%d\n"
           "pixels without a predominant color=%d\n",
           RGB[0], RGB[1], RGB[2], RGB[3]);
}

// CreateProcess with running processes from under another user
void color_predominance_with_cprocess(char* file_name, int NUM_OF_PROCESS){

    STARTUPINFOA si[NUM_OF_PROCESS];
    PROCESS_INFORMATION pi[NUM_OF_PROCESS];

    HANDLE hToken;
    LogonUser("LocalService", "NT AUTHORITY", NULL, LOGON32_LOGON_SERVICE, LOGON32_PROVIDER_DEFAULT, &hToken);

    for (int i = 0; i < NUM_OF_PROCESS; ++i) {
        char cmd_args[256];

        sprintf(cmd_args, "%d %d %s", i, NUM_OF_PROCESS, file_name);

        ZeroMemory(&si[i], sizeof(si[i]));
        si[i].cb = sizeof(si[i]);
        ZeroMemory(&pi[i], sizeof(pi[i]));
        //bool bProcess = CreateProcessA("C:\\parallel_programming\\for_process.exe", cmd_args, NULL, NULL, FALSE, 0, NULL, NULL, &si[i], &pi[i]);
        bool bProcess = CreateProcessAsUserA(hToken,"C:\\parallel_programming\\cmake-build-debug\\for_process.exe", cmd_args, NULL, NULL, FALSE, 0, NULL, NULL, &si[i], &pi[i]);
        if (bProcess == false) {
            printf("CreateProcess failed: %ld\n" , GetLastError());
        }
    }

    // wait for all processes to end
    for (int i = 0; i < NUM_OF_PROCESS; ++i) {
        WaitForSingleObject(pi[i].hProcess, INFINITE);
        char path[256];
        sprintf(path, "C:\\parallel_programming\\res_%d.txt", i);
        FILE* f = fopen(path, "r");
        int* c;
        for(int j=0; j<4; j++) {
            fscanf(f, "%d", &c[0]);
            RGB[j] += c[0];
        }
        fclose(f);
    }
    // close all handles
    for (int i = 0; i < NUM_OF_PROCESS; ++i) {
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
    }


    printf("pixels with a predominant red color=%d\n"
           "pixels with a predominant green color=%d\n"
           "pixels with a predominant blue color=%d\n"
           "pixels without a predominant color=%d\n",
           RGB[0], RGB[1], RGB[2], RGB[3]);

}

