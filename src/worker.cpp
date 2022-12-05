#include "mpi.h"
#include "omp.h"

#include <deque>
#include <utility>
#include <mutex>


// FOR SLEEP
#include <unistd.h>

int main(int argc, char *argv[]) {
    int MAX_THREADS = 3;
    int MAX_WORK = 4;

    int provided;

    // MPI_Init(&argc, &argv);
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    int size;
    MPI_Comm_remote_size(parent, &size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Children Rank %03d\n", rank);

    omp_set_num_threads(MAX_THREADS);

    std::deque<std::pair<MPI_Status, int>> requests;
    std::mutex mutex;

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        if (id == 0) {
            while (true) {
                int value = 0;
                MPI_Status status;
                MPI_Recv(&value, 1, MPI_INT, 0, MPI_ANY_TAG, parent, &status);
                std::pair<MPI_Status, int> pair = std::make_pair(status, value);
                requests.push_back(pair);
                printf("Children Rank %03d Received: %d\n", rank, value);

                // bool isFull = false;
                // mutex.lock();
                // if (requests.size() >= MAX_WORK) {
                //     isFull = true;
                //     printf("Children Rank %03d queue full\n", rank);
                // }
                // mutex.unlock();

                // // if process request queue is full, send it to the next process
                // if (!isFull) {
                //     std::pair<MPI_Status, int> pair = std::make_pair(status, value);
                //     requests.push_back(pair);
                //     printf("Rank %03d Received: %d\n", rank, value);
                // } else {
                //     int next;
                //     if (rank + 1 >= size) {
                //         next = 0;
                //         printf("Rank %03d send to leader server: %d\n", rank, status.MPI_SOURCE);
                //     } else {
                //         next = rank + 1;
                //     }
                //     MPI_Send(&value, 1, MPI_INT, next, status.MPI_TAG, MPI_COMM_WORLD);
                // }
            }
        } else {
            while (true) {
                mutex.lock();
                if (!requests.empty()) {
                    std::pair<MPI_Status, int> req = requests.front();
                    requests.pop_front();
                    printf("Children Rank %03d Thread %d: %d\n", rank, id, req.second);

                    int value = req.second;
                    MPI_Send(&value, 1, MPI_INT, 0, 0, parent);
                }
                mutex.unlock();
            }
        }
    }
    
    MPI_Finalize();
    return 0;
}