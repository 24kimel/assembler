#include <stdio.h>
#include "assembler.h"
#include <string.h>
#include <stdlib.h>
extern symbol_node *symbol_table;
extern ext_node *external_list;
extern command_image *code_img;
extern data_image *data_img;
extern long unsigned int ICF,DCF;
extern unsigned long code_img_length;
extern unsigned long data_img_length;
extern int data_exists,entries_exist;
/*the assembler makes 3 output files for each file opened: the first one is an object file
 * contains the binary image of the code/data of the input file.
 * the second one is an entry file, which contains all the symbols that were declared as an entry.
 * the third one is an external file, which contains all the symbols that were declared as external*/

/*this func checks if we have printed 4 bytes into a line of the .ob file.
 * if we did so (if there are 4 spaces), the func will open a new line, and print the current address*/
void new_line_check(int *space_count,unsigned long *address, FILE *ob_file) {
    if(*space_count == 4) {
        /*new line*/
        *space_count = 0;
        fprintf(ob_file,"\n%04lu",*address);
        *address+=4;
    }
}

/*this func creates the output files. will always create .ob file, and will create .ent and .ext files if required*/
void output(char *filename) {
    FILE *ob_file;
    FILE *ent_file;
    FILE *ext_file;
    char *ob_fname = (char*) malloc (MAX_FILE_NAME);
    char *ent_fname = (char*) malloc (MAX_FILE_NAME);
    char *ext_fname = (char*) malloc (MAX_FILE_NAME);
    int bytes_taken;
    int i;
    int space_count = 0;
    unsigned long curr_address;

    symbol_node *curr_1 = symbol_table;
    ext_node *curr_2 = external_list;
    alloc_check(ob_fname);
    alloc_check(ent_fname);
    alloc_check(ext_fname);
    filename=strtok(filename,".");
    strcpy(ob_fname,filename);
    strcpy(ent_fname,filename);
    strcpy(ext_fname,filename);
    strcat(ob_fname,".ob");
    strcat(ent_fname,".ent");
    strcat(ext_fname,".ext");

    /*do not open file if there is no code/data (object), entries (ent) or external symbols (ext)
    * we will create a func that'll check for these occasions*/
    ob_file = fopen(ob_fname,"w");

    /*checking for entries*/
    if(entries_exist == TRUE) {
        ent_file = fopen(ent_fname,"w");
        if(ent_file == NULL) {
            fprintf(stderr,"error: cannot make output file [%s]",ent_fname);
            return;
        }
        /*write to ent file*/
        while(curr_1!=NULL)  {
            if(curr_1->is_entry == TRUE) {
                fprintf(ent_file,"%s %04lu\n", curr_1->symbol, curr_1->address);
            }
            curr_1 = curr_1 ->next;
        }
    }
    /*checking if the external list is empty*/
    if(external_list != NULL) {
        ext_file = fopen(ext_fname,"w");
        /*write to ext file*/
        if(ext_file == NULL) {
            fprintf(stderr,"error: cannot make output file [%s]",ext_fname);
            return;
        }
        while(curr_2!=NULL) {
            fprintf(ext_file,"%s %04d\n",curr_2->label, curr_2->address);
            curr_2 = curr_2->next;
        }
    }

    if(ob_file == NULL) {
        fprintf(stderr,"error: cannot make output file [%s]",ob_fname);
        return;
    }
    /*write to OBJECT file*/
    /*the format of the object file is as follows:
     * at the beginning of the file, the number of bytes used for code and data (separately) is shown.
     * for code ICF-100, for data, DCF.
     * then for each line in the binary image, the binary image is printed in the little endian method in hex base.
     * to the left of the image, the address for that image is shown*/
    /*to code it this way we need to do a loop in the loop for data*/
    fprintf(ob_file,"     %lu %lu\n",ICF-100,DCF);
    for(i=0;i<code_img_length;i++) {
        fprintf(ob_file,"%04d %02X %02X %02X %02X\n", code_img[i].address, code_img[i].machine_code.w.b1, code_img[i].machine_code.w.b2, code_img[i].machine_code.w.b3, code_img[i].machine_code.w.b4);
    }
    if (data_exists) {
        curr_address = ICF;
        for (i = 0; i < data_img_length; i++) {
            if (i == 0) {
                fprintf(ob_file, "%04lu", curr_address);
                curr_address += 4;
            }
            bytes_taken = data_img[i].bytes_taken;
            switch (bytes_taken) { /*checks how many bytes was taken by each member of data_img to access the right member(s) for printing*/
                case 1:
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.b);
                    space_count++;
                    break;
                case 2:
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dh.h.b1);
                    space_count++;
                    new_line_check(&space_count, &curr_address, ob_file);
                    fprintf(ob_file, " %02X", data_img[i].machine_code.dh.h.b2);
                    space_count++;
                    break;
                case 4:
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
                    /*always 1,2, or 4*/
                default:
                    fprintf(stderr, "this should not happen (data printing for %s)\n", filename);
                    return;
            }
        }
    }
    free(code_img);
    free(data_img);
    deallocate_symbol_table();
    deallocate_external_list();
    free(ob_fname);
    free(ent_fname);
    free(ext_fname);
}


char* filename(char* name){
    if(strcmp(name+strlen(name)-3,".as")!=0) {
        fprintf(stderr,"error: non-compatible file format. all input files should be assembly files (file: %s)",name);
        return NULL;
    }
    name = strncpy(name,name,strlen(name)-3);
    return name;
}