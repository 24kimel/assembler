/*******************************************************************************
* Title                 :   Table Initialization, Build and Maintenance
* Filename              :   tables.c
* Author                :   Itai Kimelman
* Version               :   1.5.4
*******************************************************************************/
/** \file tables.c
 * \brief This module contains function that maintain all the tables necessary to the assembler
 * the tables are: the opcode table, the code image table, the data image table, te symbol table, and the external label list.
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "assembler.h"

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define NON_REAL_INDEX -1
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
/*tables*/
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

/*other global vars*/
unsigned long code_img_length = 0; /*length of code image table*/
unsigned long data_img_length = 0; /*length of data image table*/
int data_exists = FALSE; /*indicates if there is data*/
extern unsigned long DC; /*current data counter*/
int entries_exist = FALSE; /*indicates if there are labels that are entry points*/
/******************************************************************************
* Function Prototypes
*******************************************************************************/
void code_r_cmd(char *line, unsigned opcode, unsigned funct, cmd_in_binary *ptr_to_printable);
void code_i_cmd(char *line, unsigned opcode, cmd_in_binary *ptr_to_printable);
void code_j_cmd(char *line, unsigned opcode, cmd_in_binary *ptr_to_printable);

int complete_missing_info_i(unsigned long label_address, unsigned long IC);
int complete_missing_info_j(char *label, unsigned long label_address, unsigned long IC);

void code_db(char *line, int num_args, int pos);
void code_dh(char *line, int num_args, int pos);
void code_asciz(char *line, int pos);
void code_dw(char *line, int num_args, int pos);

/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : initialize_tables();
*//**
* \section Description: this function initializes all the tables (besides the opcode table) to NULL,
*                       and all of the flags to FALSE accordingly
*******************************************************************************/
void initialize_tables(){
    code_img = NULL;
    data_img = NULL;
    symbol_table = NULL;
    external_list = NULL;
    entries_exist = FALSE;
    data_exists = FALSE;
}
/******************************************************************************
* Functions For Order Lines
*******************************************************************************/
/******************************************************************************
* Function : get_opcode(char *line);
*//**
* \section Description: gets the opcode for the current order line.
*                       used only when the assembler knows this line is an order line
* \param  		line - the current line
*
* \return 		the opcode for the current order line
*
*******************************************************************************/
unsigned get_opcode(char *line) {
    if(order_index(line) == -1) {
        return NON_REAL_OPCODE;
    }
    return opcode_table[order_index(line)].opcode;
}

/******************************************************************************
* Function : get_opcode(char *line);
*//**
* \section Description: gets the opcode for the current order line.
*                       used only when the assembler knows this line is an order line
* \param        opcode - opcode of current order
* \return 		the expected number of operands for this line
*******************************************************************************/
int num_ops_expected(unsigned opcode) {
    /*according to project guidelines*/
    if(opcode == 0 || (opcode >=10 && opcode <=18)) {
        return 3;
    }

    if(opcode == 1 || (opcode >=19 && opcode <=24)) {
        return 2;
    }
    if(opcode >=30 && opcode <=32) {
        return 1;
    }
    return 0;
}
/******************************************************************************
* Function : order_index(char *line);
*//**
* \section Description: gets the order info from opcode table by obtaining the correct index for it (see opcode table).
*                       if the order associated with this line is not in the opcode table,
*                       the assembler will report an error.
*                       used only when the assembler knows this line is non other than an order line.
* \param  		line - the current line
*
* \return 		the index for the current order line in the opcode table. -1 if order does not exist
*
*******************************************************************************/
int order_index(char *line) {
    int i;
    char word[MAX_LINE+1];
    scan_op(line, word);
    /*looking for it in the opcode table, which is sorted alphabetically*/
    for(i = 0; i < NUM_ORDERS; i++) {
        if(strcmp(word,opcode_table[i].name)<0) {
            fprintf(stderr, "error: order (%s) does not exist ", word);
            return NON_REAL_INDEX;
        }
        if(strcmp(word,opcode_table[i].name)==0) {
            return i;
        }
    }
    /*order is not in the table*/
    fprintf(stderr, "error: order (%s) does not exist ", word);
    return NON_REAL_INDEX;
}

/******************************************************************************
* Function : get_funct(char *line);
*//**
* \section Description: gets the funct for the current order line.
*                       used only when the assembler knows this line is an order line
* \param  		line - the current line
*
* \return 		the funct for the current order line
*
*******************************************************************************/
unsigned get_funct(char *line) {
    return opcode_table[order_index(line)].funct;
}

/******************************************************************************
* Function : cmd_to_info(char *line, unsigned IC);
*//**
* \section Description: this function is called only if there are no errors in this order line.
*                       it code the order line to machine code according to the bit fields
*                       given in the project instructions. the result will be held in the code image table
* \param  		line - the current line(points after optional label)
* \param        IC - current instruction counter (see project instructions)
*******************************************************************************/
void cmd_to_info(char *line, unsigned IC) {
    unsigned type;
    unsigned opcode = get_opcode(line);
    unsigned funct = get_funct(line);
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
            code_r_cmd(line, opcode, funct, &printable);
            break;
        case I_CMD:
            code_i_cmd(line, opcode, &printable);
            break;
        case J_CMD:
            code_j_cmd(line, opcode, &printable);
            break;
    }
    img.address = IC;
    img.machine_code = printable;
    code_img = (command_image*) realloc (code_img,(code_img_length)* sizeof(command_image));
    alloc_check(code_img);
    code_img[code_img_length-1] = img;
}

/******************************************************************************
* Function : code_r_cmd(char *line, unsigned opcode, unsigned funct, cmd_in_binary *ptr_to_printable);
*//**
* \section Description: this function codes R commands into machine code
* \param  		line - the current line(points after order)
* \param        opcode - opcode of this R order
* \param        funct - funct of this R order
* \param        ptr_to_printable - pointer to a union with a divided word, which allows the assembler to divide orders into bytes later on
*******************************************************************************/
void code_r_cmd(char *line, unsigned opcode, unsigned funct, cmd_in_binary *ptr_to_printable) {
    unsigned regs[] = {0,0,0};
    int i;
    r_command r_cmd;
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
    ptr_to_printable->r_cmd = r_cmd;
}

/******************************************************************************
* Function : code_i_cmd(char *line, unsigned opcode, cmd_in_binary *ptr_to_printable);
*//**
* \section Description: this function codes I commands into machine code
* \param  		line - the current line(points after order)
* \param        opcode - opcode of this I order
* \param        ptr_to_printable - pointer to a union with a divided word, which allows the assembler to divide orders into bytes later on
*******************************************************************************/
void code_i_cmd(char *line, unsigned opcode, cmd_in_binary *ptr_to_printable) {
    int can_code_immed;
    unsigned regs[] = {0,0,0};
    int i;
    i_command i_cmd;
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
    ptr_to_printable->i_cmd = i_cmd;
}

/******************************************************************************
* Function : code_j_cmd(char *line, unsigned opcode, cmd_in_binary *ptr_to_printable);
*//**
* \section Description: this function codes I commands into machine code
* \param  		line - the current line(points after order)
* \param        opcode - opcode of this I order
* \param        ptr_to_printable - pointer to a union with a divided word, which allows the assembler to divide orders into bytes later on
*******************************************************************************/
void code_j_cmd(char *line, unsigned opcode, cmd_in_binary *ptr_to_printable) {
    j_command j_cmd;
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
    ptr_to_printable->j_cmd = j_cmd;
}

/******************************************************************************
* Function : complete_missing_info(char *label,char order_type, unsigned IC);
*//**
* \section Description: this function the missing info about conditional branch and J orders, where labels can show as operands
*                       and the assembler does not know their address when passing on the file for the 1st time
*                       given in the project instructions. the result will be held in the code image table.
 *
* \errors:      1. external label is used in conditional branch orders.
*               2. label requested as an operand does not exist.
*               3. address difference in conditional branch orders is out of 16 bit range
*               4. algorithm flaw in assembler
 *
* \param  		label - the label with unknown address
* \param        order_type - 'I' or 'J' according to the order type
* \param        IC - the current instruction counter
*
* \return       TRUE if the info was completed successfully. FALSE if error was found
*******************************************************************************/
int complete_missing_info(char *label, char order_type, unsigned long IC) {
    unsigned long label_address;
    int i;
    symbol_node *curr = symbol_table;
    if(order_type == 'J') {
        for(i = 0; i < code_img_length; i++) {
            /*no info need to be completed. a register has already been coded into the binary image:*/
            if(IC == code_img[i].address && code_img[i].machine_code.j_cmd.reg == TRUE)
                return TRUE;
        }
    }
    /*pass on the symbol table.*/
    while(curr!=NULL) {
        if(strcmp(label,curr->symbol) == 0) {
            label_address = curr->address;
            break;
        }
        curr = curr->next;
    }
    if(curr == NULL) {
        fprintf(stderr,"error: label used as operand does not exist ");
        return FALSE;
    }
    if(order_type == 'I') {
        return complete_missing_info_i(label_address, IC);
    }
    if(order_type == 'J') {
        return complete_missing_info_j(label, label_address, IC);
    }
    fprintf(stderr,"error: this should not happen (algorithm flaw in assembler) ");
    return FALSE;
}

/******************************************************************************
* Function : complete_missing_info_i(unsigned long label_address, unsigned long IC);
*//**
* \section Description: this function completes the missing info about conditional branch orders.
*                       it has the address of the label that shows up as an operand, and it has the instruction counter
*                       of this order, so it just searches the array for the right order and completes it with the information it has
* \param        label - the label that shows up as an operand in this J order
* \param  		IC - address of the conditional branch order
* \param        label_address - address of the label that shows up as an operand in he order
* \return       TRUE if no error was found (see \errors in complete_missing_info)
*******************************************************************************/
int complete_missing_info_i(unsigned long label_address, unsigned long IC) {
    int i;
    if(!in_lim((long int)(label_address-IC),16)) {
        fprintf(stderr,"error: immed value should be in 16 bit limits ");
        return FALSE;
    }
    if(label_address == 0) {
        fprintf(stderr,"error: external symbol cannot be used in conditional branch orders ");
        return FALSE;
    }
    for(i = 0; i < code_img_length; i++) {
        if(code_img[i].address == IC) {
            code_img[i].machine_code.i_cmd.immed = label_address - IC;
            return TRUE;
        }
    }
    fprintf(stderr,"error: this should not happen (algorithm flaw in assembler) ");
    return FALSE;
}

/******************************************************************************
* Function : complete_missing_info_j(unsigned long label_address);
*//**
* \section Description: this function completes the missing info about J orders that are not "stop".
*                       it searches the array for this J order and puts the parameter "label_address"
*                       into the "address" field. if the label is external, it will be added to the external label list
* \param        label_address - address of the label that shows up as an operand in he order
* \param        IC - address of this J order
* \return       TRUE if no error was found (see \errors in complete_missing_info)
*******************************************************************************/
int complete_missing_info_j(char *label, unsigned long label_address, unsigned long IC) {
    int i;
    if(label_address == 0) { /*external label*/
        add_to_ext_list(IC,label);
    }
    for(i = 0; i < code_img_length; i++) {
        if(code_img[i].address == IC) {
            code_img[i].machine_code.j_cmd.address = label_address;
            return TRUE;
        }
    }
    fprintf(stderr,"error: this should not happen (algorithm flaw in assembler) ");
    return FALSE;
}
/******************************************************************************
* Functions For Data Storage Directive Lines
*******************************************************************************/
/******************************************************************************
* Function : data_to_info(char *line);
*//**
* \section Description: this function is called only if there are no errors in this data directive line.
*                       it code the data directive line to machine code according to the project instructions.
*                       the result will be held in the data image table
* \param  		line - the current line(points after optional label)
*******************************************************************************/
void data_to_info(char *line) {
    int d = is_data(line);
    int len;
    int num_args;
    int pos = data_img_length;    /* we start to update in this position */
    if(data_exists == FALSE) {
        data_exists = TRUE;
    }

    line+= next_op(line,FALSE);
    if(d == ASCIZ) {
        len = asciz_len(line);          /* num characters */
        data_img_length += (len+1);       /* placeholder for null term */
    } else {
        num_args = get_num_args(line);  /* num numbers */
        data_img_length += num_args;
    }
    data_img = (data_image*) realloc(data_img,data_img_length *(sizeof(data_image)));
    alloc_check(data_img);

    switch(d) {
        case DB:
            code_db(line,num_args,pos);
            break;
        case DH:
            code_dh(line,num_args,pos);
            break;
        case ASCIZ:
            code_asciz(line,pos);
            break;
        case DW:
            code_dw(line,num_args,pos);
            break;
    }
}

/******************************************************************************
* Function : code_db(char *line, int num_args, int pos);
*//**
* \section Description: this function puts the data into the data image table for .db directives
* \param  		line - pointer to the current line after the directive
* \param        num_args - the number of arguments in this line
* \param        pos - index of the 1st cell in the data image table that I didnt code the data into
*******************************************************************************/
void code_db(char *line, int num_args, int pos) {
    /*each argument takes 1 byte*/
    int i;
    data_img[pos].machine_code.b = atoi(line);
    data_img[pos].address=DC;
    data_img[pos].bytes_taken = ONE_BYTE;
    DC+=ONE_BYTE;
    for(i=1;i< num_args;i++) {
        line+=next_op(line,TRUE);
        data_img[pos+i].machine_code.b = atoi(line);
        data_img[pos+i].address=DC;
        data_img[pos+i].bytes_taken = ONE_BYTE;
        DC+=ONE_BYTE;
    }
}

/******************************************************************************
* Function : code_dh(char *line, int num_args, int pos);
*//**
* \section Description: this function puts the data into the data image table for .dh directives
* \param  		line - pointer to the current line after the directive
* \param        num_args - the number of arguments in this line
* \param        pos - index of the the 1st cell in the data image table that I didnt code the data into
*******************************************************************************/
void code_dh(char *line, int num_args, int pos) {
    /*each argument takes 2 bytes*/
    int i;
    data_img[pos].machine_code.dh.img = atoi(line);
    data_img[pos].address=DC;
    data_img[pos].bytes_taken = HALF_WORD;
    DC+=HALF_WORD;
    for(i=1;i< num_args;i++) {
        line+=next_op(line,TRUE);
        data_img[pos+i].machine_code.dh.img = atoi(line);
        data_img[pos+i].address=DC;
        data_img[pos+i].bytes_taken = HALF_WORD;
        DC+=HALF_WORD;
    }
}

/******************************************************************************
* Function : code_asciz(char *line, int num_args, int pos);
*//**
* \section Description: this function puts the data into the data image table for .asciz directives
* \param  		line - pointer to the current line after the directive
* \param        pos - index of the 1st cell in the data image table that I didnt code the data into
*******************************************************************************/
void code_asciz(char *line, int pos) {
    /*each character takes 1 byte*/
    int i;
    line++; /*skipping the opening '\"'*/
    /*encoding all the chars of the directive to all the cells left but the last one*/
    /*last cell in array saved for '\0'*/
    for(i=pos;i<(data_img_length-1);i++) {
        data_img[i].machine_code.b = (unsigned)(line[i]);
        data_img[i].address = DC;
        data_img[i].bytes_taken = ONE_BYTE;
        DC+=ONE_BYTE;
    }
    /*adding the null character*/
    data_img[i].machine_code.b = 0;
    data_img[i].address = DC;
    data_img[i].bytes_taken = ONE_BYTE;
    DC+=ONE_BYTE;
}

/******************************************************************************
* Function : code_dw(char *line, int num_args, int pos);
*//**
* \section Description: this function puts the data into the data image table for .dw directives
* \param  		line - pointer to the current line after the directive
* \param        num_args - the number of arguments in this line
* \param        pos - index of the 1st cell in the data image table that I didnt code the data into
*******************************************************************************/
void code_dw(char *line, int num_args, int pos) {
    /*each argument takes 4 bytes*/
    int i;
    data_img[pos].machine_code.dw.img = atol(line);
    data_img[pos].address=DC;
    data_img[pos].bytes_taken = WORD;
    DC+=WORD;
    for(i=1;i< num_args;i++) {
        line+=next_op(line,TRUE);
        data_img[pos+i].machine_code.dw.img = atol(line);
        data_img[pos+i].address=DC;
        data_img[pos+i].bytes_taken = WORD;
        DC+=WORD;
    }
}

/******************************************************************************
* Function : update_data_img(unsigned ICF);
*//**
* \section Description: at the end of the 1st pass, to maintain continuity in addresses,
*                       ICF (see pass_one.c) is added to each address in the data table
* \param  		ICF - the final value of IC (see pass_one.c)
*******************************************************************************/
void update_data_img(unsigned ICF) {
    int i;
    for(i=0;i<data_img_length;i++)
        data_img[i].address+=ICF;
}

/******************************************************************************
* Function : create_symbol(symbol_node *dest,unsigned address, char *symbol, int attribute, int is_entry);
*//**
* \section Description: this function creates a symbol_node (ready to add to the symbol table)
*                       with the attributes given. for explanation about each attribute ot the symbol, see assembler.h
* \param  		dest - pointer to the result. must be allocated memory to it before using this function
*******************************************************************************/
void create_symbol(symbol_node *dest, unsigned address, char *symbol, int attribute, int is_entry) {
    dest->next = NULL;
    dest->address = address;
    dest->attribute = attribute;
    strcpy(dest->symbol, symbol);
    dest->is_entry = is_entry;
}

/******************************************************************************
* Function : add_symbol(unsigned address, char *symbol, int attribute, int is_entry);
*//**
* \section Description: this function adds the symbol represented by the parameters given to the symbol table.
*                       for explanation about each attribute ot the symbol, see assembler.h.
*                       an error will e detected if the assembler will try to add a symbol with an identical name to another symbol in the table
* \return  FALSE if error occurs, TRUE if the symbol was added successfully
*******************************************************************************/
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
        if(strcmp((node->symbol),(curr->symbol))==0) { /*checking if symbol already exists*/
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

/******************************************************************************
* Function : add_ent(char *symbol);
*//**
* \section Description: this function is called when an entry point has been detected,
*                       it sees if the symbol does ont exist, or if it is external. if it is one of thw two,
*                       the assembler will report an error. otherwise, it will turn on the "is_entry" flag in the correct symbol_node
*                       in the symbol table.
*
* \param  		symbol - the name of the symbol
* \return               TRUE is the symbol can be an entry, FALSE if an error was found
*******************************************************************************/
int add_ent(char *symbol) {
    symbol_node *curr = symbol_table;
    if(entries_exist == FALSE)
        entries_exist = TRUE;
    while(curr!=NULL) {
        if(strcmp(curr->symbol,symbol) == 0) {
            /*don't need a loop. there is only one attribute*/
            if(curr->attribute == EXTERNAL) {
                fprintf(stderr,"error: the symbol (%s) cannot be an entry and external at the same time ",symbol);
                return FALSE;
            }
            curr->is_entry = TRUE;
            return TRUE;
        }
        curr = curr->next;
    }
    /*checking if the symbol does not exist, which is not valid*/
    fprintf(stderr,"error: the symbol requested as an entry point does not exist ");
    return FALSE;
}

/******************************************************************************
* Function : update_symbol_table(unsigned ICF);
*//**
* \section Description: at the end of the 1st pass, to maintain continuity in addresses,
*                       ICF (see pass_one.c) is added to each address of a symbol that has the attribute "data"
*                       in the symbol table.
* \param  		ICF - the final value of IC (see pass_one.c)
*******************************************************************************/
void update_symbol_table(unsigned long ICF) {
    symbol_node *curr = symbol_table;
    while(curr != NULL) {
        if(curr->attribute == DATA) {
            curr->address+=ICF;
        }
        curr = curr->next;
    }
}

/******************************************************************************
* Function : create_ext_node(ext_node *dest,unsigned address, char *label);
*//**
* \section Description: this function creates an ext_node (ready to add to the external label list)
*                       with the attributes given. for explanation about each attribute ot the external label, see assembler.h
* \param  		dest - pointer to the result. must be allocated memory to it before using this function
*******************************************************************************/
void create_ext_node(ext_node *dest,unsigned address, char *label) {
    dest->address = address;
    strcpy(dest->label,label);
    dest->next = NULL;
}

/******************************************************************************
* Function : add_to_ext_list(unsigned address, char *label);
*//**
* \section Description: this function adds the external label represented by the parameters given to the external label list.
*                       for explanation about each attribute ot the external label, see assembler.h
*******************************************************************************/
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

/*************** END OF FUNCTIONS ***************************************************************************/
