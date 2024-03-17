#include <mpi.h>
#include <cstdio>
#include <bitset>
#include <cstring>

#include "bmp_reader.h"

int max(int x, int y)
{
    return x > y ? x : y;
}

int  color_predominance_in_pixel(unsigned char R, unsigned char G, unsigned char B){
    if (R > max(G,B)){
        return 0;
    } else if (G > max(R,B)){
        return 1;
    } else if (B > max(G,R)) {
        return 2;
    } else{
        return 3;
    }
}

int main(int argc, char** argv) {

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    int num_of_process = atoi(argv[1]);
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    int data_size;
    int bytes_per_pixel;
    unsigned char *data;
    BMP_file *bmpf;
    int* RGB_t;

    if (world_rank== 0) {

        char fn[] = "C:\\parallel_programming\\test2.bmp"; // path to img
        bmpf = load_BMP_file(fn);
        data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
        bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
        data = new unsigned char [data_size];
        memcpy(data, bmpf->data, data_size);
    }

    MPI_Bcast(&data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&bytes_per_pixel, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(data, data_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    int RGB[4];
    for (int & i : RGB)
        i = 0;

    int start = (world_rank - 1) * (data_size / bytes_per_pixel / (num_of_process - 1) * bytes_per_pixel);
    int end = world_rank * (data_size / bytes_per_pixel / (num_of_process - 1) * bytes_per_pixel);
    if (world_rank == (num_of_process - 1)) end = data_size;
    if (world_rank == 0) start = 0;

    for (int j = start; j < end; j += bytes_per_pixel)
        RGB[color_predominance_in_pixel(data[j], data[j + 1], data[j + 2])]++;

    MPI_Gather(RGB, 4, MPI_INT, RGB_t, 4, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if(world_rank == 0){

        int RGB_[4];
        for (int & i : RGB_)
            i = 0;

        for (int i = 4; i < num_of_process*4; i+=4) {
            for (int j = 0; j < 4; j++)
                RGB_[j] += RGB_t[i+j];
        }


        printf("pixels with a predominant red color=%d\n"
               "pixels with a predominant green color=%d\n"
               "pixels with a predominant blue color=%d\n"
               "pixels without a predominant color=%d\n",
               RGB_[0], RGB_[1], RGB_[2], RGB_[3]);


        free_BMP_file(bmpf);

    }

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}

