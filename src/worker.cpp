#include "mpi.h"
#include "omp.h"
#include "utility/ConcurrentDequeue.h"

#include <deque>
#include <utility>
#include <mutex>


// FOR SLEEP
#include <unistd.h>

int main(int argc, char *argv[]) {
    int MAX_THREADS = 3;
    int MAX_WORK = 6000;

    int provided;

    int mygivenID = -1;

    // MPI_Init(&argc, &argv);
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    int size;
    MPI_Comm_remote_size(parent, &size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //printf("Children Rank %03d\n", rank);

    omp_set_num_threads(MAX_THREADS);

    // considering we are just using sleep values maybe this can be an int
    //std::deque<int, int>> requests; // que of sleep values
    ConcurrentQueue<int> requests;

    // idea: three thread types
    // receive, do work, send status back
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        if (id == 0) {
            while (true) {
                int value = 0;
                MPI_Status status;
                MPI_Recv(&value, 1, MPI_INT, 0, MPI_ANY_TAG, parent, &status);
                //std::pair<MPI_Status, int> pair = std::make_pair(status, value);
                requests.Push(value);
                //printf("Children Rank %03d Received: %d\n", rank, value);

            }
        } else if (id == 1) {
            while (true) {
                if (requests.size() == 0) { // work being done adds time until next req processed
                    int reqValue;
                    requests.Get(reqValue);
                    //printf("Children Rank %03d Thread %d: %d\n", rank, id, req.second);
                    usleep(reqValue);
                }
            }
        } else {
            while (true) {
                int queSize = requests.size();
                MPI_Send(&queSize, 1, MPI_INT, 0, MPI_ANY_TAG, parent);
                usleep(30); //kind of like delay on heartbeat
            }
        }
    }
    
    MPI_Finalize();
    return 0;
}