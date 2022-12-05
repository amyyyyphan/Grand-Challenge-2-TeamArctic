#include "mpi.h"
#include "omp.h"

#include <deque>
#include <utility>
#include <mutex>
#include <vector>
int main(int argc, char *argv[]) {
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

    MPI_Comm intercomm;
    MPI_Comm second_intercomm;
    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 2, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 2, MPI_INFO_NULL, 0, MPI_COMM_SELF, &second_intercomm, MPI_ERRCODES_IGNORE);
    //char* commands[1] = {"./worker"};
    //int max_procs[1] = {1};
    //MPI_Comm_spawn_multiple(1, commands, MPI_ARGVS_NULL, 2, max_procs, )
    int iter = 100;

    std::vector<MPI_Comm> comms;
    comms.push_back(intercomm);
    comms.push_back(second_intercomm);

    //int count = 0;
    //int value = 1;
    int commIndex = 0;
    int childIndex = 0;

    std::deque<int> works;

    for (int i = 0; i < iter; i++) {
        works.push_back(i);
    }

/*
    while (count < iter) {
        int tag = 0; //Action to perform
        MPI_Status status;
        //MPI_Send(&value, 1, MPI_INT, 0, tag, comms.at(0));
        //MPI_Send(&value, 1, MPI_INT, 1, tag, comms.at(0));

        printf("Manager sent %d to %d\n", value, childIndex);
        MPI_Send(&value, 1, MPI_INT, childIndex, tag, comms.at(commIndex));
        value++;
        count++;

        //Round robin under the assumption each intercomm has 2 processes
        if (comms.size() > 1) {
            if (commIndex == comms.size() - 1) commIndex = 0;
            else commIndex++;
        }
        if (childIndex == 1) childIndex = 0;
        else childIndex++;
    }
*/

    while (works.size() > 0) {
        int tag = 0; //Action to perform
        MPI_Status status;
        //MPI_Send(&value, 1, MPI_INT, 0, tag, comms.at(0));
        //MPI_Send(&value, 1, MPI_INT, 1, tag, comms.at(0));

        int value = works.front();
        works.pop_front();
        printf("Manager sent %d to %d\n", value, childIndex);
        MPI_Send(&value, 1, MPI_INT, childIndex, tag, comms.at(commIndex));
        //value++;
        //count++;

        //Round robin under the assumption each intercomm has 2 processes
        if (comms.size() > 1) {
            if (commIndex == comms.size() - 1) commIndex = 0;
            else commIndex++;
        }
        if (childIndex == 1) childIndex = 0;
        else childIndex++;
    }

    while (true) {
        int value[2];

        MPI_Request requests[comms.size()];
        for (int i = 0; i < comms.size(); i++) {
            MPI_Irecv(&value[i], 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comms.at(i), &requests[i]);
        }

        //Send work that was sent back to the manager since their queue was full
        while (works.size() > 0) {
            int tag = 0; //Action to perform
            MPI_Status status;
            //MPI_Send(&value, 1, MPI_INT, 0, tag, comms.at(0));
            //MPI_Send(&value, 1, MPI_INT, 1, tag, comms.at(0));

            int value = works.front();
            works.pop_front();
            printf("Manager sent %d to %d\n", value, childIndex);
            MPI_Send(&value, 1, MPI_INT, childIndex, tag, comms.at(commIndex));
            //value++;
            //count++;

            //Round robin under the assumption each intercomm has 2 processes
            if (comms.size() > 1) {
                if (commIndex == comms.size() - 1) commIndex = 0;
                else commIndex++;
            }
            if (childIndex == 1) childIndex = 0;
            else childIndex++;
        }
        
        MPI_Status statuses[comms.size()];
        MPI_Waitall(comms.size(), requests, statuses);
        int return_value;
        for (int i = 0; i < comms.size(); i++) {
            printf("status source %d and status tag %d\n", statuses[i].MPI_SOURCE, statuses[i].MPI_TAG);
            switch (statuses[i].MPI_TAG) {
            case 0:
                printf("Manager Received: %d from Children Rank %03d\n", value[i], statuses[i].MPI_SOURCE);
                break;
            case 1:
                printf("Manager received %d since capacity was full\n", value[i]);
                works.push_back(value[i]);
                break;
            default:
                // Unexpected message type 
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }

        /*
        for (int i = 0; i < comms.size(); i++) {
            printf("status source %d and status tag %d\n", statuses[i].MPI_SOURCE, statuses[i].MPI_TAG);
            switch (statuses[i].MPI_TAG) {
            case 0:
                printf("Manager Received: %d from Children Rank %03d\n", value, statuses[i].MPI_SOURCE);
                break;
            case 1:
                printf("Manager received %d since capacity was full\n", value);
                break;
            default:
                // Unexpected message type 
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
       */
    }

    MPI_Finalize();
    return 0;
}