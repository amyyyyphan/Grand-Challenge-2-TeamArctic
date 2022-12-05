#include "mpi.h"
#include "omp.h"

#include <deque>
#include <utility>
#include <mutex>

int main(int argc, char *argv[]) {
    int MAX_THREADS = 5;
    int MAX_WORK = 300;

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
    //omp_set_dynamic(MAX_THREADS);

    std::deque<std::pair<MPI_Status, int>> requests;
    
    #pragma omp parallel
    {
        #pragma omp master
        {
            while (true) {
                int value = 0;
                MPI_Status status;
                MPI_Recv(&value, 1, MPI_INT, 0, MPI_ANY_TAG, parent, &status);

                bool isFull = false;

                #pragma omp critical
                {
                    if (requests.size() >= MAX_WORK) {
                        isFull = true;
                        printf("Rank %03d (worker server) queue full\n", rank);
                    }
                }

                if (!isFull) {
                    std::pair<MPI_Status, int> pair = std::make_pair(status, value);
                    requests.push_back(pair);
                    printf("Children Rank %03d Received: %d\n", rank, value);

                    #pragma omp task
                    {
                        #pragma omp critical
                        {
                            if (!requests.empty()) {
                                std::pair<MPI_Status, int> req = requests.front();
                                requests.pop_front();
                                printf("Children Rank %03d Thread %d processing: %d\n", rank, omp_get_thread_num(), req.second);
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                                int value = req.second;
                                MPI_Send(&value, 1, MPI_INT, 0, 0, parent);
                            }
                        }
                    }
                }
                else {
                    printf("Work capacity full at rank %d thread %d, can't process %d\n", rank, omp_get_thread_num(), value);
                    MPI_Send(&value, 1, MPI_INT, 0, 1, parent);
                }
            }
        }
    }
    
    MPI_Finalize();
    return 0;
}