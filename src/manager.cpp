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
    MPI_Comm third_intercomm;
    MPI_Comm fourth_intercomm;
    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercomm, MPI_ERRCODES_IGNORE);
    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &second_intercomm, MPI_ERRCODES_IGNORE);
    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &third_intercomm, MPI_ERRCODES_IGNORE);
    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &fourth_intercomm, MPI_ERRCODES_IGNORE);
    int iter = 10000;

    std::vector<MPI_Comm> comms;
    comms.push_back(intercomm);
    comms.push_back(second_intercomm);
    comms.push_back(third_intercomm);
    comms.push_back(fourth_intercomm);

    int commIndex = 0;

    std::deque<int> works;

    for (int i = 0; i < iter; i++) {
        works.push_back(i);
    }

    while (works.size() > 0) {
        int tag = 0; //Action to perform
        MPI_Status status;

        int value = works.front();
        works.pop_front();
        printf("Manager sent %d to %d\n", value, commIndex);
        MPI_Send(&value, 1, MPI_INT, 0, tag, comms.at(commIndex));

        if (commIndex == comms.size() - 1) commIndex = 0;
        else commIndex++;
    }

    int newSpawnCap = 5;
    int countNew = 0;
    while (true) {
        int value[comms.size()];

        MPI_Request requests[comms.size()];
        for (int i = 0; i < comms.size(); i++) {
            MPI_Irecv(&value[i], 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comms.at(i), &requests[i]);
        }

        //Send work that was sent back to the manager since their queue was full
        while (works.size() > 0) {
            int tag = 0; //Action to perform
            MPI_Status status;

            int value = works.front();
            works.pop_front();
            printf("Manager sent %d to %d\n", value, commIndex);
            MPI_Send(&value, 1, MPI_INT, 0, tag, comms.at(commIndex));

            if (commIndex == comms.size() - 1) commIndex = 0;
            else commIndex++;
        }
        
        MPI_Status statuses[comms.size()];
        int statIndex;
        MPI_Status status;
        MPI_Waitany(comms.size(), requests, &statIndex, &status);
        switch (status.MPI_TAG) {
            case 0:
                printf("Manager Received: %d from Children Rank %03d\n", value[statIndex], status.MPI_SOURCE);
                break;
            case 1:
                printf("Manager received %d since capacity was full\n", value[statIndex]);
                if (countNew < newSpawnCap) {
                    printf("creating new process\n");
                    countNew++;
                    MPI_Comm new_intercomm;
                    MPI_Comm_spawn("./worker", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &new_intercomm, MPI_ERRCODES_IGNORE);
                    comms.push_back(new_intercomm);
                }
                works.push_back(value[statIndex]);
                break;
            default:
                // Unexpected message type 
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Finalize();
    return 0;
}