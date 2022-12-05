#include "mpi.h"
#include "omp.h"
#include "utility/ConcurrentMap.h"


#include <deque>
#include <utility>
#include <mutex>
#include <vector>
#include <stdlib.h> //rand()

int main(int argc, char *argv[]) {

    int MAX_THREADS = 8;
    int MAX_WORK = 10;

    // thread safe if threads only doing read operations
    std::vector<MPI_Comm> children;

    // thread safe status map and comm map
    ConcurrentMap<int,int> statusMap;
    ConcurrentMap<int, MPI_Comm> commMap;

    // sleep values to be passed to workers
    int sleepVals[] = {1000,1200,800,1500};

    int provided;

    // MPI_Init(&argc, &argv);
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int universe_size, *universe_sizep, flag;
    MPI_Attr_get(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &universe_sizep, &flag);
    if (!flag) {
        printf("This MPI does not support UNIVERSE_SIZE");
    } else {
        universe_size = *universe_sizep;
    }

    if (universe_size == 1) {
        printf("No room to start workers");
    }

    omp_set_num_threads(MAX_THREADS);


    // create maps of children 
    for (int i = 0; i < children_num; i++) {
       MPI_Comm intercomm;
       MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
       commMap[i] = intercomm;
       statusMap[i] = 0;
       children.push_back(intercomm);
    }

    // input data
    

    // loop for reading input and then giving to comm child process
    // three groups of threads: sending, receiving, spawning/deleting children
    while(true) {
        //int inputData = sleepVals(rand() % 4);

        #pragma omp parallel {
            #pragma omp for {
                for (size_t i = 0; i < children.size; i++) {
                    
                    

                } 
            }
        }

    }
    

    int count = 0;
    int value = 1;
    int iter = 10;
    while (count < iter) {
        printf("Manager sent %d\n", value);
        int tag = 0; /* Action to perform */
        MPI_Status status;
        MPI_Send(&value, 1, MPI_INT, 0, tag, intercomm);
        MPI_Send(&value, 1, MPI_INT, 1, tag, intercomm);
        value++;
        count++;
    }


    while (true) {
        int value = 0;
        MPI_Status status;
        MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, intercomm, &status);

        switch (status.MPI_TAG) {
            case 0:
                printf("Manager Received: %d from Children Rank %03d\n", value, status.MPI_SOURCE);
                break;
            default:
                /* Unexpected message type */
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Finalize();
    return 0;
}