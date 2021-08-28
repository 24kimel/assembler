/*******************************************************************************
* Title                 :   The Second Assembler Pass
* Filename              :   pass_two.c
* Author                :   Itai Kimelman
* Version               :   1.5.3
*******************************************************************************/
/** \file pass_two.c
 * \brief This module performs the 2nd assembler pass
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
int err2;

/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : pass_two_error(char *file_name, unsigned long num_ln)
*//**
* \section Description Description: this function is used when an error in input occurs during the 2nd pass.
*                       it turns on the err2 flag, and prints out the name of the file and line number,
*                       so the user will know where the error was found.
*
* \param  		file_name - the name of the current file
* \param        num_ln - the number of the current line
*
*******************************************************************************/
void pass_two_error(char* file_name,unsigned long num_ln) {
    err2 = STATUS_ERR;
    fprintf(stderr,"[%s | %lu]\n",file_name,num_ln);
}

/******************************************************************************
* Function : pass_two(char *file_name)
*//**
* \section Description: this function performs the 2nd assembler pass on the current file.
*                       it follows the algorithm mentioned below.
* \param  		file_name - the name of the current file
* \return       STATUS_OK if no error was found. otherwise:  STATUS_ERR
*\note
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
 * 10.  MAKE THE OUTPUT FILES
*******************************************************************************/
int pass_two(char *file_name) {
    FILE *curr_file;
    unsigned long num_ln =0;
    char *line = NULL;
    char *pos = NULL;
    char *label = NULL;
    unsigned long IC = 100;
    unsigned opcode;
    int i;
    char order_type;
    err2 = STATUS_OK;
   	if((curr_file=fopen(file_name,"r"))==NULL) {
        fprintf(stderr,"error while opening file");
     	err2 = STATUS_ERR;
        return err2;
    }
    if((fseek(curr_file,0,SEEK_SET)) != 0) {
        fprintf(stderr,"error trying to pass on the file %s\n", file_name);
        err2 = STATUS_ERR;
        return err2;
    }
    line = (char*) malloc (sizeof(char) * MAX_LINE+1);
    alloc_check(line);
    label = (char*) malloc (sizeof(char) * MAX_LABEL+1);
    alloc_check(label);
    while(TRUE) {
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
        if(ent_ext(pos) == EXTERN || is_data(pos))
            continue;

        if(ent_ext(pos) == ENTRY) { /*entry directive(step 5)*/
            /*skipping to the operand of the directive*/
            /*step 6:*/
            if(check_ent_ext(pos) == FALSE) {
                pass_two_error(file_name,num_ln);
            } else {
                pos+= next_op(pos,FALSE);
                scan_label(pos,label);
                if (add_ent(label) == FALSE) {
                    pass_two_error(file_name, num_ln);
                }
            }
        } else {
            /*steps 7 and 8*/
            opcode = get_opcode(pos);
            /*conditional branch orders*/
            if(opcode>=15 && opcode<=18) {
                /*find the label that shows up here. skipping to the 3rd operand*/
                pos += next_op(pos,FALSE);
                for(i = 0; i < 2; i++)
                    pos += next_op(pos,TRUE);
                scan_label(pos, label);
                order_type = 'I';
            } else {
                if(opcode>=30 && opcode <=32) {
                    /*find the label that shows up here skipping to the 1st operand*/
                    pos += next_op(pos,FALSE);
                    scan_label(pos, label);
                    order_type = 'J';
                }
            }

            if ((opcode>=15 && opcode<=18) || (opcode>=30 && opcode <=32)) {
                if (complete_missing_info(label, order_type, IC) == FALSE)
                    pass_two_error(file_name,num_ln);
            }
            IC+=WORD;
        }
    }
    /*step 9*/
    free(line);
    free(label);
    return err2;
}

/*************** END OF FUNCTIONS ***************************************************************************/