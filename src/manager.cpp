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
    int iter = 100;

    std::vector<MPI_Comm> comms;
    comms.push_back(intercomm);
    comms.push_back(second_intercomm);

    int count = 0;
    int value = 1;
    int commIndex = 0;
    int childIndex = 0;

    while (count < iter) {
        int tag = 0; /* Action to perform */
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

    while (true) {
        int value = 0;
        MPI_Status status;
        MPI_Status second_status;
        MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, intercomm, &status);
        MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, second_intercomm, &second_status);

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