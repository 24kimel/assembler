#include <stdio.h>
#include "assembler.h"
#include <stdlib.h>
/*TODO:
 * debug
 * document*/

void alloc_check(void * x) {
    if(x == NULL) {
        fprintf(stderr,"memory allocation problems\n");
        exit(1);
    }
}

int num_files (int argc) {
    if(argc<MIN_ARGUMENTS) {
        fprintf(stderr,"error: no input files");
        return FALSE;
    }
    if(argc>MAX_ARGUMENTS) {
        fprintf(stderr,"error: too much input files(more than 3)");
        return FALSE;
    }
    return TRUE;
}

int main(int argc, char **argv) {
    int i;
    char *curr_file;
    if(num_files(argc) == 0) return 1;
    for(i = 1; i< argc; i++) {
        if ((curr_file = filename(argv[i])) != NULL) {
            if (pass_one(curr_file) == 0) {
                if(pass_two(curr_file) == 0)
                    continue;
            }
            return 1;
        }
    }
    return 0;
}