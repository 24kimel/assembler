/*******************************************************************************
* Title                 :   Output files management
* Filename              :   files.c
* Author                :   Itai Kimelman
* Version               :   1.5.0
*******************************************************************************/
/** \file files.c
 * \brief If there are no errors in the 1st and 2nd pass on
 * this current file, files.c will create all the output files needed
 * 1. a .ob file - for the code and data image
 * 2. an .ent file - for all the labels that are entry points (if there are any)
 * 3. an .ext file - for all the external labels used as operands (if there are any)
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include "assembler.h"
#include <string.h>
#include <stdlib.h>

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
extern symbol_node *symbol_table;
extern ext_node *external_list;
extern command_image *code_img;
extern data_image *data_img;
extern long unsigned int ICF,DCF;
extern unsigned long code_img_length;
extern unsigned long data_img_length;
extern int data_exists,entries_exist;

/******************************************************************************
* Function Prototypes
*******************************************************************************/
void write_to_ob_file(FILE *ob_file, char *ob_fname);
void write_to_ent_file(FILE *ent_file);
void write_to_ext_file(FILE *ext_file);
/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : filename(char*)
*//**
* \section Description: This function checks if the file given is
*                       an assembly file. if not - it prints an error
*
*  This function is used to check dor the file format before opening it
*
* \param  		name - the name of the file
*
* \return 		the name of the file without .as (if it is a .as file). NULL if error occurs
*******************************************************************************/
char* filename(char* name){
    if(strcmp(name+strlen(name)-3,".as")!=0) {
        fprintf(stderr,"error: non-compatible file format. all input files should be assembly files (file: %s)",name);
        return NULL;
    }
    return name;
}

/******************************************************************************
* Function : num_files(int argc)
*//**
* \section Description: This function checks for the number of arguments and makes sure
*                       it is not below 2 or over 4. otherwise, ut prints an error
*
*  The assembler works with 1-3 files of input - hence argc should be between 2 and 4.
*  This function is used to make sure that there is at least 1 input file, and at most 3
*
* \param  		argc - the number of arguments in the command line
*
* \return 		TRUE if there is at least 1 input file, and at most 3. FALSE if not
*******************************************************************************/
int num_files (int argc) {
    if(argc<MIN_ARGUMENTS) {
        fprintf(stderr,"error: no input files");
        return FALSE;
    }
    if(argc>MAX_ARGUMENTS) {
        fprintf(stderr,"error: too many input files(more than 3)");
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************
* Function : new_line_check(int *space_count,unsigned long *address, FILE *ob_file);
*//**
* \section Description: this function checks if we have printed 4 bytes into the object file.
*                       if we did, it will create a newline and print the current address of the data.
*                       this function is only used when printing data because we may print a number of bytes
*                       that is not divisible by 4.
*
* \param  		space_count - the number of spaces in the line(if there is 4, a newline is needed)
* \param        address - the current address of the data.
* \param        ob_file - pointer to output object file (see \brief)
*
*******************************************************************************/
void new_line_check(int *space_count,unsigned long *address, FILE *ob_file) {
    if(*space_count == WORD) {
        /*new line*/
        *space_count = 0;
        fprintf(ob_file,"\n%04lu",*address);
        *address+=WORD;
    }
}

/******************************************************************************
* Function : output(char *file_name)
*//**
* \section Description: creates output files (see \brief)
*
* \param  		file_name - the name of the source file
* \return       0 if no error was found. otherwise:  1
*
*******************************************************************************/
int output(char *file_name) {
    FILE *ob_file;
    FILE *ent_file;
    FILE *ext_file;
    char *ob_fname = (char*) malloc (MAX_FILE_NAME);
    char *ent_fname = (char*) malloc (MAX_FILE_NAME);
    char *ext_fname = (char*) malloc (MAX_FILE_NAME);

    /*making all the needed file names*/
    alloc_check(ob_fname);
    alloc_check(ent_fname);
    alloc_check(ext_fname);
    file_name=strtok(file_name,".");
    strcpy(ob_fname,file_name);
    strcpy(ent_fname,file_name);
    strcpy(ext_fname,file_name);
    strcat(ob_fname,".ob");
    strcat(ent_fname,".ent");
    strcat(ext_fname,".ext");

    /*do not open file if there are no entry points (ent) or external symbols (ext)*/
    /*checking for entries*/
    if(entries_exist == TRUE) {
        ent_file = fopen(ent_fname,"w");
        if(ent_file == NULL) {
            fprintf(stderr,"error: cannot make output file [%s]",ent_fname);
			free(ob_fname);
			free(ent_fname);
			free(ext_fname);
			return 1;
        }
        write_to_ent_file(ent_file);
    }

    /*checking if the external labels*/
    if(external_list != NULL) {
        ext_file = fopen(ext_fname,"w");
        /*write to ext file*/
        if(ext_file == NULL) {
            fprintf(stderr,"error: cannot make output file [%s]",ext_fname);
			free(ob_fname);
			free(ent_fname);
			free(ext_fname);
			return 1;
        }
        write_to_ext_file(ext_file);
    }

    /*writing to object file*/
    ob_file = fopen(ob_fname,"w");
    if(ob_file == NULL) {
        fprintf(stderr,"error: cannot make output file [%s]",ob_fname);
    	free(ent_fname);
		free(ob_fname);
    	free(ext_fname);
        return 1;
    }
    write_to_ob_file(ob_file,ob_fname);
	free(ob_fname);
    free(ent_fname);
    free(ext_fname);
	return 0;
}

/******************************************************************************
* Function : write_to_ob_file(FILE *ob_file, char *ob_fname);
*//**
* \section Description: writes to object file
*
* \param  		ob_file - pointer to the object file
* \param        ob_fname - the name of the object file
*
* \note         the format of the object file is as follows:
*               at the beginning of the file, the number of bytes used for code and data (separately) is shown.
*               for code ICF-100, for data, DCF.
*               then for each line in the binary image, the binary image is printed in the little endian method in hex base.
*               to the left of the image, the address for that image is shown
*               to code it this way we need to do a loop in the loop for data
*               using the partition to bytes made by the unions and structs
*******************************************************************************/
void write_to_ob_file(FILE *ob_file, char *ob_fname) {
    int i;
    unsigned long curr_address;
    int bytes_taken;
    int space_count = 0;
    fprintf(ob_file,"     %lu %lu\n",ICF-100,DCF);
    for(i=0;i<code_img_length;i++) {
        fprintf(ob_file,"%04d %02X %02X %02X %02X\n", code_img[i].address, code_img[i].machine_code.w.b1, code_img[i].machine_code.w.b2, code_img[i].machine_code.w.b3, code_img[i].machine_code.w.b4);
    }
    if (data_exists) {
        curr_address = ICF;
        for (i = 0; i < data_img_length; i++) {
            if (i == 0) {
                fprintf(ob_file, "%04lu", curr_address);
                curr_address += WORD;
            }
            bytes_taken = data_img[i].bytes_taken;
            switch (bytes_taken) { /*checks how many bytes was taken by each member of data_img to access the right member(s) for printing*/
                case ONE_BYTE:
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.b);
                    space_count++;
                    break;
                case HALF_WORD:
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dh.h.b1);
                    space_count++;
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dh.h.b2);
                    space_count++;
                    break;
                case WORD:
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dw.w.b1);
                    space_count++;
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dw.w.b2);
                    space_count++;
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dw.w.b3);
                    space_count++;
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dw.w.b4);
                    space_count++;
                    break;
                    /*always should be 1,2, or 4*/
                default:
                    fprintf(stderr, "this should not happen (data printing for %s)\n", ob_fname);
                    return;
            }
        }
    }
}

/******************************************************************************
* Function : write_to_ent_file(FILE *ent_file);
*//**
* \section Description: writes to entry file
*
* \param  		ent_file - pointer to the entry file
*
* \note         the format of the entry file is as follows:
*               for each label (that opens a line) that is an entry point
*               (\example - "K: .dw 31,-12" and also ".entry K" in the same file)
*               the entry file will contain the label and its address
*******************************************************************************/
void write_to_ent_file(FILE *ent_file) {
    symbol_node *curr_1 = symbol_table;
    /*write to ent file*/
    while(curr_1!=NULL)  {
        if(curr_1->is_entry == TRUE) {
            fprintf(ent_file,"%s %04lu\n", curr_1->symbol, curr_1->address);
        }
        curr_1 = curr_1 ->next;
    }
}

/******************************************************************************
* Function : write_to_ext_file(FILE *ext_file);
*//**
* \section Description: writes to external file
*
* \param  		ext_file - pointer to the external file
*
* \note         the format of the external file is as follows:
*               for each external label that is used as an operand in J orders,
*               the file will contain the label and the address of the order
*******************************************************************************/
void write_to_ext_file(FILE *ext_file) {
    ext_node *curr_2 = external_list;
    while(curr_2!=NULL) {
        fprintf(ext_file,"%s %04d\n",curr_2->label, curr_2->address);
        curr_2 = curr_2->next;
    }
}
/*************** END OF FUNCTIONS ***************************************************************************/