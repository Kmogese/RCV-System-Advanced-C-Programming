#include "rcv.h"
#include <stdlib.h> 

int main(int argc, char *argv[]) {
    // Check arguments, do 2 or 4
    if (argc != 2 && argc != 4) {
        printf("Usage: %s [-log N] <votes_file>\n", argv[0]);
        return 1;
    }

    // Check optional log 
    if (argc == 4) {
        if (strcmp(argv[1], "-log") == 0) {
            LOG_LEVEL = atoi(argv[2]); 
        } else {
            printf("Usage: %s [-log N] <votes_file>\n", argv[0]);
            return 1;
        }
    }

    // File at last argument
    char *filename = argv[argc - 1];

    // Load tally file
    tally_t *tally = tally_from_file(filename);
    if (tally == NULL) {
        printf("Could not load votes file. Exiting with error code 1\n");
        return 1;
    }

    // Run
    tally_election(tally);

    // Free tally memory
    tally_free(tally);

    return 0;
}
