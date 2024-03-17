#include <mpi.h>
#include <cstdio>
#include <bitset>
#include "bmp_reader.h"

int max(int x, int y)
{
    return x > y ? x : y;
};

int  color_predominance_in_pixel(unsigned char R, unsigned char G, unsigned char B){
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

    int index[num_of_process];
    for (int i=0; i<num_of_process; i++)
        index[i] = i;


    int edges[(num_of_process-1)*2];
    for (int i = 0; i < (num_of_process-1) * 2; i++)
        edges[i] = 0;

    for (int i=1; i<num_of_process; i++)
        edges[i] += i;


    MPI_Comm StarComm;
    MPI_Graph_create(MPI_COMM_WORLD, num_of_process, index, edges, 0, &StarComm);



    if (world_rank== 0) {

        char fn[] =  "C:\\parallel_programming\\test2.bmp"; // path to img
        BMP_file* bmpf = load_BMP_file(fn);

        int data_size = bmpf->dibhdr.wight * bmpf->dibhdr.height * bmpf->dibhdr.bits_per_pixel / 8;
        int bytes_per_pixel = bmpf->dibhdr.bits_per_pixel / 8;
        unsigned char *data = bmpf->data;

        for (int i = 1; i < num_of_process; ++i) {
            MPI_Send(&data_size, 1, MPI_INT, i, 0, StarComm);
            MPI_Send(&bytes_per_pixel, 1, MPI_INT, i, 0, StarComm);
            MPI_Send(data, data_size, MPI_UNSIGNED_CHAR , i, 0, StarComm);
        }

        int RGB[4];
        for (int & i : RGB)
            i = 0;



        for (int i = 1; i < num_of_process; ++i) {
            int RGB_t[4];
            MPI_Recv(RGB_t, 4, MPI_INT, i, MPI_ANY_TAG, StarComm, MPI_STATUS_IGNORE);
            for (int j = 0; j < 4; j++)
                RGB[j] += RGB_t[j];
        }


        printf("pixels with a predominant red color=%d\n"
               "pixels with a predominant green color=%d\n"
               "pixels with a predominant blue color=%d\n"
               "pixels without a predominant color=%d\n",
               RGB[0], RGB[1], RGB[2], RGB[3]);

        free_BMP_file(bmpf);
    }

    if (world_rank != 0) {

        int RGB_t[4];
        for (int & i : RGB_t)
            i = 0;


        int data_size;
        int bytes_per_pixel;
        MPI_Recv(&data_size, 1, MPI_INT, 0, MPI_ANY_TAG, StarComm, MPI_STATUS_IGNORE);
        MPI_Recv(&bytes_per_pixel, 1, MPI_INT, 0, MPI_ANY_TAG, StarComm, MPI_STATUS_IGNORE);
        unsigned char data[data_size];
        MPI_Recv(data, data_size, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, StarComm, MPI_STATUS_IGNORE);

        int start = (world_rank - 1) * (data_size / bytes_per_pixel / (num_of_process - 1) * bytes_per_pixel);
        int end = world_rank * (data_size / bytes_per_pixel / (num_of_process - 1) * bytes_per_pixel);
        if ((world_rank) == (num_of_process - 1)) end = data_size;

        for (int j = start; j < end; j += bytes_per_pixel)
            RGB_t[color_predominance_in_pixel(data[j], data[j + 1], data[j + 2])]++;


        MPI_Send(RGB_t, 4, MPI_INT, 0, 0, StarComm);

    }

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}

