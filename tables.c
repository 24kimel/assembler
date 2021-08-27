#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "assembler.h"

/*opcode table:*/

const cmd_info opcode_table[] = {{"add", 0, 1},{"addi", 10, 0},{"and", 0, 3},
                                 {"andi", 12, 0},{"beq", 16, 0},{"bgt", 18, 0},
                                 {"blt", 17, 0},{"bne", 15, 0},{"call", 32, 0},
                                 {"jmp", 30, 0},{"la", 31, 0},{"lb", 19, 0},
                                 {"lh", 23, 0},{"lw", 21, 0},{"move", 1, 1},
                                 {"mvhi", 1, 2},{"mvlo", 1, 3},{"nor", 0, 5},
                                 {"nori", 14, 0},{"or", 0, 4},{"ori", 13, 0},
                                 {"sb", 20, 0},{"sh", 24, 0},{"stop", 63, 0},
                                 {"sub", 0, 2},{"subi", 11, 0},{"sw", 22, 0}};

command_image *code_img;
data_image *data_img;
symbol_node *symbol_table;
ext_node *external_list;

unsigned get_opcode(char *line) {
    return opcode_table[order_index(line)].opcode;
}

int order_index(char *line) {
    int i;
    char *word = (char *)malloc(MAX_LINE+1);
    scan_op(line, word);
    for(i = 0; i < NUM_ORDERS; i++) {
        if(strcmp(word,opcode_table[i].name)<0) {
            fprintf(stderr,"line = %s\n", line);
            fprintf(stderr, "error: order %s does not exist ", word);
            free(word);
            return -1;
        }
        if(strcmp(word,opcode_table[i].name)==0) {
            free(word);
            return i;
        }
    }
    fprintf(stderr,"exited loop\nline = %s\n", line);
    fprintf(stderr, "error: order %s does not exist ", word);
    free(word);
    return -1;
}

unsigned get_funct(char *line) {
    return opcode_table[order_index(line)].funct;
}


unsigned long code_img_length = 0;
/*points after optional label*/
void cmd_to_info(char *line, unsigned IC) {
    unsigned type;
    unsigned opcode = get_opcode(line);
    unsigned funct = get_funct(line);
    unsigned regs[] = {0,0,0};
    int i;
    r_command r_cmd;
    i_command i_cmd;
    j_command j_cmd;
    int can_code_immed;
    cmd_in_binary printable;
    command_image img;
    line+= next_op(line,FALSE);
    code_img_length++;
    if(opcode<=1) {
        type = R_CMD;
    } else if(opcode<=24){
        type = I_CMD;
    } else type = J_CMD;

    switch(type) {
        case R_CMD:
            r_cmd.opcode = opcode;
            regs[0] = atoi(++line);
            if(opcode == 0) {
                for(i=1;i<3;i++) {
                    if(!empty(line))
                        line+=next_op(line,TRUE);
                    regs[i] = atoi(++line);
                }
            } else {
                if(!empty(line))
                    line+=next_op(line,TRUE);
                regs[2] = atoi(++line);
            }
            r_cmd.rs = regs[0];
            r_cmd.rt = regs[1];
            r_cmd.rd = regs[2];
            r_cmd.funct = funct;
            r_cmd.zeros = 0;
            printable.r_cmd = r_cmd;
            break;
        case I_CMD:
            i_cmd.opcode = opcode;
            if(opcode <= 14 || (opcode>=19 && opcode<=24)) {
                can_code_immed = TRUE;
            } else can_code_immed = FALSE;
            regs[0] = atoi(++line);
            for(i=1;i<3;i++) {
                line+= next_op(line,TRUE);
                if(can_code_immed && i==1) {
                    i_cmd.immed=atoi(line);
                } else regs[i] = atoi(++line);
            }
            /*happens if the command is an arithmetic or a load/save command:*/
            if(regs[2]!=0) regs[1] = regs[2];
            i_cmd.rs = regs[0];
            i_cmd.rt = regs[1];
            printable.i_cmd = i_cmd;
            break;
        case J_CMD:
            j_cmd.opcode = opcode;
            if(opcode <= 32) {
                if(*line != '$') {
                    j_cmd.reg = FALSE;
                } else {
                    j_cmd.reg = 1;
                    j_cmd.address = (unsigned)atoi(++line);
                }

            }
            else {
                j_cmd.reg = FALSE;
                j_cmd.address = 0;
            }
            printable.j_cmd = j_cmd;
            break;
    }

    img.address = IC;
    img.machine_code = printable;
    code_img = (command_image*) realloc (code_img,(code_img_length)* sizeof(command_image));
    alloc_check(code_img);
    code_img[code_img_length-1] = img;
}

void build_code_img() {
    code_img_length = 0;
    code_img = (command_image*) malloc(sizeof(command_image));
    alloc_check(code_img);
}
/*data image:*/

extern unsigned long DC;
unsigned long data_img_length = 0;
int data_exists =FALSE;

void data_to_info(char *line) {
    int d = is_data(line);
    int i;
    unsigned byte;
    int len;
    int num_args;
    int pos = data_img_length;    /* we start to update in this position */
    if(data_exists == FALSE) {
        data_exists = TRUE;
    }

    line+= next_op(line,FALSE);
    if(d==3) {
        len = asciz_len(line);          /* num characters */
        data_img_length += len+1;       /* placeholder for null term */
    } else {
        num_args = get_num_args(line);  /* num numbers */
        data_img_length += num_args;
    }
    data_img = (data_image*) realloc(data_img,data_img_length *(sizeof(data_image)));
    alloc_check(data_img);

    switch(d) {
        case DB:
            data_img[pos].machine_code.b = atoi(line);
            data_img[pos].address=DC;
            data_img[pos].bytes_taken = 1;
            DC+=1;
            for(i=1;i< num_args;i++) {
                line+=next_op(line,TRUE);
                data_img[pos+i].machine_code.b = atoi(line);
                data_img[pos+i].address=DC;
                data_img[pos+i].bytes_taken = 1;
                DC+=1;
            }
            break;
        case DH:
            data_img[pos].machine_code.dh.img = atoi(line);
            data_img[pos].address=DC;
            data_img[pos].bytes_taken = 2;
            DC+=2;
            for(i=1;i< num_args;i++) {
                line+=next_op(line,TRUE);
                data_img[pos+i].machine_code.dh.img = atoi(line);
                data_img[pos+i].address=DC;
                data_img[pos+i].bytes_taken = 2;
                DC+=2;
            }
            break;
        case ASCIZ:
            line++; /*skipping the opening '\"'*/
            for(i=0;i<len;i++) { /*encoding all the chars of the directive*/
                byte = (unsigned)(line[i]);
                data_img[pos+i].machine_code.b = byte;
                data_img[pos+i].address = DC++;
                data_img[pos+i].bytes_taken = 1;
            }/*adding the null character*/
            data_img[pos+i].machine_code.b = 0;
            data_img[pos+i].address = DC++;
            data_img[pos+i].bytes_taken = 1;
            break;
        case DW:
            data_img[pos].machine_code.dw.img = atol(line);
            data_img[pos].address=DC;
            data_img[pos].bytes_taken = 4;
            DC+=4;
            for(i=1;i< num_args;i++) {
                line+=next_op(line,TRUE);
                data_img[pos+i].machine_code.dw.img = atol(line);
                data_img[pos+i].address=DC;
                data_img[pos+i].bytes_taken = 4;
                DC+=4;
            }
            break;
    }
    return;
}


/*this func updates the data image table. it adds ICF to each address to attain memory continuity*/
void update_data_img(unsigned ICF) {
    int i;
    for(i=0;i<data_img_length;i++)
        data_img[i].address+=ICF;
}

/*this function build the data image table*/
int build_data_img() {
    data_img_length = 0;
    data_img = (data_image*) malloc(sizeof(data_image));
    alloc_check(data_img);
    return TRUE;
}

/*symbol table*/
/*refer to it as a pointer to check for NULL more easily*/

int entries_exist;

void create_symbol(symbol_node *dest,unsigned address, char *symbol, int attribute, int is_entry) {
    dest->next = NULL;
    dest->address = address;
    dest->attribute = attribute;
    strcpy(dest->symbol, symbol);
    dest->is_entry = is_entry;
}

/*this func adds the symbol represented by the parameters given to the symbol table. returns FALSE if error occurs, TRUE
 * if the symbol was added successfully*/
int add_symbol(unsigned address, char *symbol, int attribute, int is_entry) {
    symbol_node *node;
    symbol_node *curr;
    node = (symbol_node*)malloc(sizeof(symbol_node));
    node->symbol = (char*) malloc (MAX_LABEL+1);
    alloc_check(node);
    alloc_check(node->symbol);
    create_symbol(node, address ,symbol ,attribute ,is_entry);
    if (symbol_table == NULL) {
        symbol_table = node;
        return TRUE;
    }
    curr = symbol_table;
    while(curr!=NULL) {
        if(strcmp((node->symbol),(curr->symbol))==0) {
            fprintf(stderr, "symbol (%s) already exists, and cannot be used twice ", node->symbol);
            free(node->symbol);
            free(node);
            return FALSE;
        }
        curr = curr->next;
    }
    curr = symbol_table;
    while(curr->next!=NULL)
        curr = curr->next;
    curr->next = node;
    return TRUE;
}

int add_ent(char *symbol){
    symbol_node *curr = symbol_table;
    if(entries_exist == FALSE)
        entries_exist = TRUE;
    while(curr!=NULL) {
        if(strcmp(curr->symbol,symbol) == 0) {
            /*don't need a loop. there is only one attribute*/
            if(curr->attribute == EXTERNAL) {
                fprintf(stderr,"error: this symbol cannot be an entry and external at the same time ");
                return FALSE;
            }
            curr->is_entry = TRUE;
            return TRUE;
        }
        curr = curr->next;
    }
    fprintf(stderr,"error: the symbol requested as an entry point does not exist ");
    return FALSE;
}


void update_symbol_table(unsigned ICF) {
    symbol_node *curr = symbol_table;
    while(curr != NULL) {
        if(curr->attribute == DATA) {
            curr->address+=ICF;
        }
        curr = curr->next;
    }
}

/*list of all symbols that show up as operands, that were declared as external. we will create a global variable for that, and a func that resets the list for each file*/
void create_ext_node(ext_node *dest,unsigned address, char *label) {
    dest->address = address;
    strcpy(dest->label,label);
    dest->next = NULL;
}


void add_to_ext_list(unsigned address, char *label) {
    ext_node *node;
    ext_node *curr = external_list;
    node = (ext_node*) malloc(sizeof(ext_node));
    node->label = (char*) malloc(MAX_LABEL+1);
    alloc_check(node);
    alloc_check(node->label);
    create_ext_node(node,address,label);
    if(external_list == NULL) {
        external_list = node;
        return;
    }
    while(curr->next!=NULL)
        curr = curr->next;
    /*we are on the last node*/
    curr->next = node;
}

/*this func completes all the missing info about certain I and J orders, where labels can show as operands
 * and the assembler does not know their address when passing on the file for the 1st time*/
int complete_missing_info(char *label, char order_type, unsigned IC) {
    unsigned long address;
    int i;
    symbol_node *curr = symbol_table;
    if(order_type == 'J') {
        for(i = 0; i < code_img_length; i++) {
            /*no info need to be completed. a register has already been coded into the binary image:*/
            if(IC == code_img[i].address && code_img[i].machine_code.j_cmd.reg == TRUE)
                return TRUE;
        }
    }
    while(curr!=NULL) {
        if(strcmp(label,curr->symbol) == 0) {
            address = curr->address;
            break;
        }
        curr = curr->next;
    }
    if(curr == NULL) {
        fprintf(stderr,"error: label used as operand does not exist ");
        return FALSE;
    }
    /*pass on the symbol table.*/
    if(order_type == 'I') {
        if(!in_lim((long int)(address-IC),16)) {
            fprintf(stderr,"error: immed value should be in 16 bit limits ");
            return FALSE;
        }
        if(address == 0) {
            fprintf(stderr,"error: external symbol cannot be used in conditional branch orders ");
            return FALSE;
        }
        for(i = 0; i < code_img_length; i++) {
            if(code_img[i].address == IC) {
                code_img[i].machine_code.i_cmd.immed = address - IC;
                return TRUE;
            }
        }
        fprintf(stderr,"error: this should not happen (algorithm flaw in assembler) ");
        return FALSE;
    }
    if(order_type == 'J') {
        if(address == 0) { /*external label*/
            add_to_ext_list(IC,label);
            return TRUE;
        }
        for(i = 0; i < code_img_length; i++) {
            if(code_img[i].address == IC) {
                code_img[i].machine_code.j_cmd.address = address;
                return TRUE;
            }
        }
        fprintf(stderr,"error: this should not happen (algorithm flaw in assembler) ");
        return FALSE;
    }
    fprintf(stderr,"error: this should not happen (algorithm flaw in assembler) ");
    return FALSE;
}


void initialize_tables(){
    code_img = NULL;
    data_img = NULL;
    symbol_table = NULL;
    external_list = NULL;
    entries_exist = FALSE;
    data_exists = FALSE;
}