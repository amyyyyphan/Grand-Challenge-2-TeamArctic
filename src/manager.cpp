#include "mpi.h"
#include "omp.h"
#include "utility/ConcurrentMap.h"


#include <deque>
#include <utility>
#include <mutex>
#include <vector>
#include <stdlib.h> //rand()
#include <unistd.h>
#include <cmath>
#include <utility>


int isLoadHeavy(ConcurrentMap<int,int> map) {
    int lazyProcesses = 0;
    std::vector vals = map.getValues();

    float sum = 0.0,mean = 0.0, variance = 0.0, stdDev = 0.0;
    for (int i = 0; i < vals.size(); i++) {
        sum += val[i];
        variance += pow(val[i] - mean, 2);
        if (val[i] == 0) 
            lazyProcesses++;
    }
    mean = sum/vals.size();
    variance = variance / vals.size();
    stdDev = sqrt(variance);

    int WORK_CAP = 8000; // MAX AVERAGE WORK ALLOWED
    int DEV_CAP = 900; // MAX DEVIATION ALLOWED
    if (mean >= WORK_CAP && stdDev <= DEV_CAP) {
        return 1;
    } else {
        return 0;
    }
    /*
    } else if (lazyProcesses > vals.size() / 2) {
        return -1;
    }
    */
    
}

int main(int argc, char *argv[]) {

    int MAX_THREADS = 7;
    int MAX_WORK = 6;
    int children_num = 4 // initial children
    int MAX_CHILDREN = 5;

    // thread safe if threads only doing read operations
   //std::vector<MPI_Comm> children;

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
       commMap.set(i,intercomm);
       statusMap.set(i,0);
    }
    

    // loop for reading input and then giving to comm child process
    // three groups of threads: sending, receiving, spawning/deleting children
    #pragma omp parallel 
    {
        #pragma omp single 
        {
            while(true) { // sending requests to lowest balance intercomm (process)
                std::pair <int,int> lowestValKeyPair = statusMap.getLowestValKeyPair();
                int value = sleepVals(rand() % 4);
                int tag = lowestValKeyPair.first; //sending manager assigned id as tag so they can send back

                MPI_Comm intercomm = commMap.get(lowestValKeyPair.first);
                MPI_Send(&value,1, MPI_INT,tag, intercomm);
                usleep(80); // small delay
            }
        }

        #pragma omp single
        {
            while(true) {
                std::vector<MPI_Comm> commVals = commMap.getValues();
                for (size_t i = 0; i < commVals; i++)
                {
                    #pragma omp task 
                    {
                        int value; // should be related to work balance
                        int tag; // should be manager assigned id
                        MPI_Recv(&value,1,MPI_INT,MPI_ANY_SOURCE,&tag,commVals[i],MPI_STATUS_IGNORE);

                        // do something with value
                        statusMap.set(tag,value);

                    }
                }
                usleep(50); // small delay between "heartbeat checks"
            }
        }

        #pragma omp single {
            while(true) {
                int systemStatus = isLoadHeavy(statusMap); //1 == load heavy, -1 == idle nodes, 0 == normal
                switch (systemStatus) { 
                case 1: // add process if not at max processes
                    if (commMap.size() != MAX_CHILDREN) {
                        MPI_Comm intercomm;
                        MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
                        int i = commMap.getMaxKey() + 1;
                        commMap.set(i,intercomm);
                        statusMap.set(i,0);
                    }
                    break;
                case 0: // keep going

                    break;
                case -1: // remove process (igonre till last)
                    // removing shouldnt really mess up other operations
                    break;
                default:
                    break;
                }
                usleep(1000); //delay between checks
            }
        }
    }

    MPI_Finalize();
    return 0;
}