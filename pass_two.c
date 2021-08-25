#include <stdio.h>
#include "assembler.h"
#include <stdlib.h>
#include <string.h>
/*
 * the algorithm for the 2nd assembler pass is as follows:
 * 1. read the next line. if the file has ended : go to step 9.
 * 2. if this line is a comment line or an empty line, go to step 1.
 * 3. if the first field in this line is a label, skip it (already sorted out in pass one).
 * 4. is this line a directive that is not .entry? If it is, go to step 1 (already sorted out in pass one).
 * 5. is this an .entry directive? If not, go to step 7.
 * 6. add the attribute "entry" to the label's attributes in the symbol table.
 * if the label does not exist, report an error. go to step 1.(the label used as an operand). done up to here!
 * 7. this is an order line. if there is missing info about the order, code it now (if error occurs, report it).go to step 1.
 * 8. if in step 7 (only in type J order), there was an external label, add the label's name to the external label list for later use
 * 9. we have reached the end of the source file. If there were errors, do not build the output files.
 * 10. OUTPUT FILES LES GO*/
/*those are really easy to do*/

int err2;
void pass_two_error(char* filename,unsigned long num_ln) {
    err2 = TRUE;
    fprintf(stderr,"[%s | %lu]\n",filename,num_ln);
}
int pass_two(char *filename) {
    FILE *curr_file;
    unsigned long num_ln =0;
    char *line;
    char *pos = NULL;
    char *label;
    unsigned long IC = 100;
    unsigned opcode;
    int i;
    char order_type;
    err2 = FALSE;
    curr_file = fopen(filename,"r");
    alloc_check(curr_file);
    if((fseek(curr_file,0,SEEK_SET)) != 0) {
        fprintf(stderr,"error trying to pass on the file %s\n", filename);
        return 1;
    }
    line = (char*) malloc (sizeof(char) * MAX_LINE+1);
    alloc_check(line);
    label = (char*) malloc (sizeof(char) * MAX_LABEL+1);
    alloc_check(label);
    while(TRUE){
        num_ln++;
        /*step 1:*/
        if(read_line(curr_file,line) == FALSE)
            break;
        pos = line;
        while(spaceln(*pos))
            pos++;
        /*step 2:*/
        if(meaningless(pos))
            continue;
        /*step 3:*/
        if(start_label(pos)) {
            pos += next_op(pos,FALSE);
        }
        /*in the first pass we took care of all directives that are not .entry, so now we can skip them(step 4)*/
        if(ent_ext(pos) == 2 || is_data(pos))
            continue;

        if(ent_ext(pos) == 1) { /*entry directive(step 5)*/
            /*skipping to the operand of the directive*/
            /*step 6:*/
            pos += next_op(pos,FALSE);
            scan_label(pos, label);
            pos +=strlen(label);
            if(!empty(pos)) {
                fprintf(stderr,"too much operands for .entry ");
                pass_two_error(filename,num_ln);
            }
            if(add_ent(label) == FALSE) {
                pass_two_error(filename,num_ln);
            }
        } else {
            /*steps 7 and 8*/
            opcode = get_opcode(pos);
            if(opcode>=15 && opcode<=18) {
                /*find the label that shows up here*/
                pos += next_op(pos,0);
                for(i = 0; i < 2; i++)
                    pos += next_op(pos,1);
                scan_label(pos, label);
                order_type = 'I';
            } else {
                if(opcode>=30 && opcode <=32) {
                    /*find the label that shows up here*/
                    pos += next_op(pos,0);
                    scan_label(pos, label);
                    order_type = 'J';
                }
            }
            if ((opcode>=15 && opcode<=18) || (opcode>=30 && opcode <=32)) {
                if (complete_missing_info(label, order_type, IC) == 0)
                    pass_two_error(filename,num_ln);
            }
            IC+=4;
        }
    }
    /*step 9*/
    free((void *) (line));
    free((void *) (label));
    if(err2 == FALSE) {
        /*step 10*/
        output(filename);
        return 0;
    }
    return err2;
}