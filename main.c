#include <stdio.h>
#include "assembler.h"
#include <stdlib.h>
/*TODO:
 * document */

void alloc_check(void * x) {
    if(x == NULL) {
        fprintf(stderr,"memory allocation problems\n");
        exit(1);
    }
}



int main(int argc, char **argv) {
    int i;
    char *curr_file;
    if(num_files(argc) == 0) return 1;
    for(i = 1; i< argc; i++) {
        if ((curr_file = filename(argv[i])) != NULL) {
            if (pass_one(curr_file) == 0) {
                pass_two(curr_file);
            }
        }
    }
    return 0;
}