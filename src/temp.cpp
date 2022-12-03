#include "mpi.h"
#include "omp.h"

#include <deque>
#include <utility>
#include <mutex>

int main(int argc, char *argv[]) {
    int MAX_THREADS = 3;
    int MAX_WORK = 3;

    int provided;

    // MPI_Init(&argc, &argv);
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    omp_set_num_threads(MAX_THREADS);

    if (rank == 0) {
        // leader server
        while (true) {
            int value = 0;
            MPI_Status status;
            MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            switch (status.MPI_TAG) {
                case 0:
                    printf("Leader Server Received: %d from Rank %03d\n", value, status.MPI_SOURCE);
                    MPI_Send(&value, 1, MPI_INT, 2, status.MPI_TAG, MPI_COMM_WORLD);
                    break;
                default:
                    /* Unexpected message type */
                    MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    } else if (rank == 1) {
        // client
        int iter = 10;
        MPI_Request requests[iter];

        int count = 0;
        int value = 1; 
        while (count < iter) {
            printf("Client sent %d to leader server\n", value);
            int tag = 0; /* Action to perform */ 
            MPI_Isend(&value, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &requests[count]);
            value++;
            count++;
        }

        while (true) {
            int response = 0;
            MPI_Status status;
            MPI_Recv(&response, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("Client received response from Rank %03d\n", status.MPI_SOURCE);
        }
    } else if (rank > 1) {
        // worker server
        std::deque<std::pair<MPI_Status, int>> requests;
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                while (true) {
                    int value = 0;
                    MPI_Status status;
                    MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                    bool isFull = false;

                    #pragma omp critical
                    {
                        if (requests.size() >= MAX_WORK) {
                            isFull = true;
                            printf("Rank %03d (worker server) queue full\n", rank);
                        }
                    }

                    // if process request queue is full, send it to the next process
                    if (!isFull) {
                        std::pair<MPI_Status, int> pair = std::make_pair(status, value);
                        requests.push_back(pair);
                        printf("Rank %03d (worker server) Received: %d\n", rank, value);

                        #pragma omp task
                        {
                            #pragma omp critical
                            {
                                if (!requests.empty()) {
                                    std::pair<MPI_Status, int> req = requests.front();
                                    requests.pop_front();
                                    printf("Rank %03d Worker Thread %d: %d\n", rank, omp_get_thread_num(), req.second);
                                }
                            }
                        }
                    } else {
                        int next;
                        if (rank + 1 >= world_size) {
                            next = 0;
                            printf("Rank %03d send to leader server: %d\n", rank, status.MPI_SOURCE);
                        } else {
                            next = rank + 1;
                        }
                        MPI_Send(&value, 1, MPI_INT, next, status.MPI_TAG, MPI_COMM_WORLD);
                    }
                }
            }
        }
        
    }

    MPI_Finalize();
    return 0;
}