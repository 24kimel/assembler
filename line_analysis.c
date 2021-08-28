/*******************************************************************************
* Title                 :   Line Analysis
* Filename              :   line_analysis.c
* Author                :   Itai Kimelman
* Version               :   1.5.1
*******************************************************************************/
/** \file line_analysis.c
 * \brief This file contains function that help analyzing each line in the source file
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"
/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : length_check(char *line)
*//**
* \section Description: This function checks if the length of the line is over 80 characters
*
* This function is used to check if the current line is too long. report error if so
*
* \param  		line - the current line
*
* \return 		TRUE if the length of the current line is fine.
*
*******************************************************************************/
int length_check(char *line) {
    /*subtracting the newline character from the character count, then checking if there are characters in the line than the maximum allowed*/
    if(strlen(line)-1 > MAX_LINE) {
        fprintf(stderr,"error: line length above maximum (80 characters) ");
        return FALSE;
    }
    /*there are 80 or fewer characters in this line -> no error*/
    return TRUE;
}

/******************************************************************************
* Function : read_line(FILE *fp, char* line)
*//**
* \section Description: reads the next input line from the current file.
*
* \param  		line - the current line
* \param        fp - the current file
* \return 		TRUE if EOF did not occur
*
*******************************************************************************/
int read_line(FILE *fp, char* line) {
    if(fgets(line, MAX_LINE, fp) == NULL)
        return FALSE;
    return TRUE;
}

/******************************************************************************
* Function : spaceln(char)
*//**
* \section Description: checks if the current character is a white space (and not newline or EOF)
*
* \param  		c - the current character
* \return 		TRUE if the current character is a white space (and not newline or EOF)
*
*******************************************************************************/
int spaceln(char c) {
    if(isspace(c) && c!='\n' && c!=EOF) return TRUE;
    return FALSE;
}

/******************************************************************************
* Function : empty(char *line)
*//**
* \section Description: checks if the current line is empty (only white spaces)
*
* \param  		line - the current line
* \return 		TRUE if the current line is empty of non-white characters
*
*******************************************************************************/
int empty(char *line) {
    char *ptr;
    if(strcmp(line,"") == 0)
        return TRUE;
    ptr = (char*) malloc (MAX_LINE);
    alloc_check(ptr);
    strcpy(ptr,line);
    while(spaceln(*ptr))
        ptr++;
    if(endline(*ptr))
        return TRUE;
    return FALSE;
}

/******************************************************************************
* Function : intlen(char *line)
*//**
* \section Description: checks the length of the string that represents
*          an integer starting at the address of the current line
*          (example: intlen("-445") = 4)
*
* \param  		line - the current line
* \return 		0 if there is no integer starting at the position of line.
*               otherwise, the length of the string that represents
*               an integer starting at the address of the current line
*
*******************************************************************************/
int intlen(char *line) {
    char *ptr;
    int i;
    ptr = line;
    i = 0;
    if(*ptr == '+' || *ptr == '-') {
        ptr++;
        i++;
    }
    while(!isspace((int)*ptr) && *ptr != ',') {
        if(!isdigit((int)*ptr)) return i;
        ptr++;
        i++;
    }
    return i;
}

/******************************************************************************
* Function : meaningless(char *line)
*//**
* \section Description: checks if the current line is meaningless to the assembler
*                       (an empty line or a comment line)
*                       (a comment line is a line that its 1st non-white character is ';')
*
* \param  		line - the current line
* \return 		TRUE if the current line is meaningless to the assembler
*
*******************************************************************************/
int meaningless(char *line) {
    char *ptr = line;
    if(*ptr == '\n' || *ptr == ';' || *ptr == EOF)
        return TRUE;
    return FALSE;
}

/******************************************************************************
* Function : start_label(char *line)
*//**
* \section Description: checks if the 1st field in the line is a label
*
* \param  		line - the current line
* \return 		TRUE if the 1st field in the line is a label
*
*******************************************************************************/
int start_label(char *line) {
    int i = 0;
    char *word;
    word = (char*) malloc (sizeof(char) * MAX_LINE+1);
    alloc_check(word);
    /*scanning the 1st field into word*/
    while(!isspace((int)line[i])) {
        word[i] = line[i];
        i++;
    }
    word[i] = '\0';
    if(word[i-1] != ':') {
        free(word);
        return FALSE;
    }
    strtok(word,":");
    /*checking if the string before ':' is a valid label*/
    if(!is_label(word,FALSE)) {
        free(word);
        return FALSE;
    }
    free(word);
    return TRUE;
}

/******************************************************************************
* Function : is_label(char *line, int err)
*//**
* \section Description: checks if the 1st field in the string pointed by label is a label
*
* \param  		line - the current line (or part of it)
* \param        err - flag for error reporting.
*
* \note:        for example: the line ".add $3,$5,$9" is perfectly fine,
*               but it dos not have a label in it. in pass_one.c we check if the 1st field in this line is a label,
*               and we use this function. we should not report an error if the line does not start with a label for it is optional.
*               hence the need for the flag "err"
*
* \note:        a valid label is a string with max length of 31 that starts with a letter, and contains only alphanumeric characters
* \return 		TRUE if the 1st field in the line is a label
*******************************************************************************/
int is_label(char *line, int err) {
    char *ptr = line;
    int i = 0;
    if(!isalpha((int)ptr[0])) {
        if(err == TRUE)
            fprintf(stderr, "error: a label should start with a letter ");
        return FALSE;
    }

    while(!isspace((int)ptr[i]) && ptr[i] != '\0' && !endline(ptr[i])) {
        if(!isalnum((int)ptr[i]) && !endline(ptr[i])) {
            if(err == TRUE)
                fprintf(stderr,"error: label contains illegal characters. a proper label should contain only alphanumeric characters ");
            return FALSE;
        }
        i++;
    }
    if(i>MAX_LABEL) {
        fprintf(stderr,"error: label length above 31 characters ");
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************
* Function : scan_label(char *line, char *label)
*//**
* \section Description: scans the label from the current line to label.
* This function is only used when we know we have a valid label
*
* \param  		line - the current line(or part of it)
* \param        label - the label we write into. has to have allocated memory to it before
*
*******************************************************************************/
void scan_label (char *line, char *label) {
    int i;
    i=0;
    while(!isspace((int)line[i]) && line[i]!=':') {
        label[i] = line[i];
        i++;
    }
    label[i] = '\0';
}

/******************************************************************************
* Function : is_data(char *line)
*//**
* \section Description: this function checks if this line is a data storage directive
*
* \param  		line - the current line(or part of it)
* \return       0 if not data directive. otherwise, the number associated with the directive detected
*******************************************************************************/
int is_data(char *line) {
    int i = 0;
    int ret_val = FALSE;
    char *word = (char*) malloc(sizeof(char) * (MAX_LINE+1));
    alloc_check(word);
    while(!isspace((int)line[i])) {
        word[i] = line[i];
        i++;
    }
    word[i] = '\0';
    if(strcmp(word,".db")==0)
        ret_val =  DB;
    if(strcmp(word,".dh")==0)
        ret_val = DH;
    if(strcmp(word,".asciz")==0)
        ret_val = ASCIZ;
    if(strcmp(word,".dw")==0)
        ret_val = DW;
    free(word);
    return ret_val;
}

/******************************************************************************
* Function : ent_ext(char *line)
*//**
* \section Description: this function checks if this line is an .entry or .extern directive
*
* \param  		line - the current line(or part of it)
* \return       1 if .entry, 2 if .extern. 0 otherwise
*******************************************************************************/
int ent_ext(char *line) {
    int i = 0;
    char *word = (char*) malloc(sizeof(char) * (MAX_LINE+1));
    alloc_check(word);
    while(!isspace((int)line[i])) {
        word[i] = line[i];
        i++;
    }
    word[i] = '\0';
    if(strcmp(word,".entry")==0)
        return ENTRY;
    if(strcmp(word,".extern") ==0)
        return EXTERN;
    return FALSE;
}

/******************************************************************************
* Function : check_immed(char *line)
*//**
* \section Description: this function checks the line points to an operand that is a whole number. if not, it reports an error
*
* \param  		line - the current line(or part of it)
* \return       TRUE if the operand is a valid immed value (integer within 16 bit limits)
*******************************************************************************/
int check_immed(char *line) {
    long value;
    char *ptr = line;

    if(intlen(ptr)==0) {
        fprintf(stderr,"error: a number should be here ");
        return FALSE;
    }
    value = atol(ptr);
    if(!in_lim(value,16)) {
        fprintf(stderr,"error: immed value should be in 16 bit limits ");
        return FALSE;
    }
    ptr+=intlen(ptr);
    if(!spaceln(*ptr) && *ptr != ',') {
        fprintf(stderr,"error: invalid operand (should be a number) ");
        return FALSE;
    }
    while(spaceln(*ptr)) ptr++;
    if(*ptr != ',') {
        fprintf(stderr,"error: invalid operand (should be a number) ");
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************
* Function : num_commas(char *line)
*//**
* \section Description: this function checks for te number of commas in this line.
*                       good for checking for the number of operands
*
* \param  		line - the current line(or part of it)
* \return       TRUE if the structure of the order line is ok
*******************************************************************************/
int num_commas(char *line) {
    int commas = 0;
    int i;
    for(i = 0; i< strlen(line); i++) {
        if(line[i] == ',')
            commas++;
    }
    return commas;
}

/******************************************************************************
* Function : order_structure(char *line)
*//**
* \section Description: this function checks if the structure of the order line is ok
*          there are a lot of different structures for different orders, so this function
*          is used to take any order line and check its operands
*
* \param  		line - the current line(or part of it)
* \return       TRUE if the structure of the order line is ok
*******************************************************************************/
int order_structure(char *line) {
    char *ptr = line;
    unsigned oc;
    int i;
    while(spaceln(*ptr))
        ptr++;
    oc = get_opcode(line);
    if(oc == NON_REAL_OPCODE)
        return FALSE;
    /*a comma separates every two operands, so for an order with x operands, there are supposed to be x-1 commas*/
    /*checking if there are not enough operands(checking the other way later*/
    if(num_commas(line) < (num_ops_expected(oc)-1)) {
        fprintf(stderr,"error: not enough operands for this order ");
        return FALSE;
    }
    if(next_op(ptr,FALSE) == NON_VALID_OPERAND)
        return FALSE;
    ptr+= next_op(ptr,FALSE);

    if(oc==0) { /*3 registers needed*/
        if(register_num(ptr,TRUE) == NOT_REG)
            return FALSE;
        for(i = 0; i < 2; i++) {
            if(next_op(ptr,TRUE) == NON_VALID_OPERAND)
                return FALSE;
            ptr+= next_op(ptr,1);
            if(register_num(ptr,TRUE) == NOT_REG)
                return FALSE;
        }
        ptr++;
        ptr+= intlen(ptr);
    }

    if(oc==1) { /*2 registers needed*/
        if(register_num(ptr,TRUE) == NOT_REG)
            return FALSE;
        if(next_op(ptr,TRUE) == NON_VALID_OPERAND)
            return FALSE;
        ptr+= next_op(ptr,TRUE);
        if(register_num(ptr,TRUE) == NOT_REG)
            return FALSE;
        ptr++;
        ptr+= intlen(ptr);
    }

    if((oc>=9 && oc<=14)  || (oc>=19 && oc<=24)) { /*2 registers needed, immed between them*/
        if(register_num(ptr,TRUE)==NOT_REG)
            return FALSE;
        if(next_op(ptr,TRUE) == NON_VALID_OPERAND)
            return FALSE;
        ptr+=next_op(ptr,TRUE);
        if(check_immed(ptr) == FALSE)
            return FALSE;
        if(next_op(ptr,TRUE) == NON_VALID_OPERAND)
            return FALSE;
        ptr+=next_op(ptr,TRUE);
        if(register_num(ptr,TRUE) == NOT_REG)
            return FALSE;
        ptr++;
        ptr+= intlen(ptr);
    }

    if(oc>=15 && oc<=18) { /*2 register, then a label needed*/
        for(i = 0; i < 2; i++) {
            if(register_num(ptr,TRUE) == NOT_REG)
                return FALSE;
            if(next_op(ptr,TRUE) == NON_VALID_OPERAND)
                return FALSE;
            ptr+= next_op(ptr,TRUE);
        }
        if(!is_label(ptr,TRUE))
            return FALSE;
        while(isalpha((int)*ptr) || isdigit((int)*ptr)) ptr++;
    }
    if(oc==30) { /*jmp order. a register OR a label needed*/
        int result;
        if (is_label(ptr, FALSE) == FALSE && register_num(ptr, FALSE) == NOT_REG) {
            fprintf(stderr,"error: this operand is not a label or a register ");
            return FALSE;
        }
        if (is_label(ptr, FALSE))
            result = 1;
        else result = 2;

        if(result == 1) {
            while(isalpha((int)*ptr) || isdigit((int)*ptr)) ptr++;
        }
        else {
            ptr++;
            ptr+= intlen(ptr);
        }
    }

    if(oc == 31 || oc == 32) { /*a label needed*/
        if(is_label(ptr,TRUE) == FALSE)
            return FALSE;
        while(isalpha((int)*ptr) || isdigit((int)*ptr))
            ptr++;
    }

    /*we have gone over all the operands*/
    if(empty(ptr))
        return TRUE;
    else {
        fprintf(stderr,"error: too much operands ");
        return FALSE;
    }
}

/******************************************************************************
* Function : register_num(char *line)
*//**
* \section Description: this function checks if the current operand is a valid register
*                       (a '$' followed by an integer between 0 and 31). the assembler
*                       calls this function only when it identifies that there should
*                       register in the current position of line, so if the result of
*                       this function is negative, the assembler will report an error
*
* \param  		line - the current line(or part of it)
* \return       TRUE if the current operand is a valid register
*******************************************************************************/
int register_num(char *line, int err) {
    char *ptr = line;
    int reg;

    if(*ptr != '$') {
        if(err == TRUE)
            fprintf(stderr, "error: a register should be here (a register starts with a $, followed by an integer between 0 and 31) ");
        return NOT_REG;
    }
    if(!isdigit((int)*(++ptr))) {
        if(err == TRUE)
            fprintf(stderr, "error: a register should be here (a register starts with a $, followed by an integer between 0 and 31) ");
        return NOT_REG;
    }
    reg = atoi(ptr);
    if(!(reg>=REG_MIN && reg<=REG_MAX)) {
        if(err == TRUE)
            fprintf(stderr, "error: register number %d does not exist ",reg);
        return NOT_REG;
    }
    ptr+=intlen(ptr);
    if(*ptr!=(char)0 && !spaceln(*ptr) && *ptr != ',' && !endline(*ptr)) {
        fprintf(stderr,"error: invalid register. after the register number, there can only be a comma or a space character ");
        return NOT_REG;
    }
    return reg;
}

/******************************************************************************
* Function : scan_op(char *line)
*//**
* \section Description: this function scans the next field in the current line into op
*
* \param  		line - the current line(or part of it)
* \param        op - the operand we write into. has to have allocated memory to it before
*******************************************************************************/
void scan_op(char *line, char *op) {
    char *ptr = line;
    int i=0;
    while (spaceln(*ptr))
        ptr++;
    while(!spaceln(ptr[i]) && ptr[i] != ',' && !endline(ptr[i])) {
        op[i] = ptr[i];
        i++;
    }
    op[i] = '\0';
}

/******************************************************************************
* Function : next_op(char *line)
*//**
* \section Description: this function checks the distance between the current line position
*                       and the next field. if a comma should separate fields and there is no comma,
*                       the assembler will report an error.
*
* \param  		line - the current line(or part of it)
* \param        comma - flag that indicates if a comma should separate this field from the next one
* \return       -1 if error occurs. otherwise, the distance between the current line position
*               and the next field
********************************************************************************/
int next_op(char *line, int comma) {
    char *ptr = line;
    int distance = 0;
    char *op = (char *)malloc(MAX_LINE+1);
    alloc_check(op);
    scan_op(ptr, op);
    ptr+=strlen(op);
    distance+=strlen(op);
    while(spaceln(*ptr)) {
        ptr++;
        distance++;
    }
    if(!comma) {
        free(op);
        return distance;
    }
    if(*ptr!=',') {
        fprintf(stderr,"error: a comma should separate operands ");
        free(op);
        return NON_VALID_OPERAND;
    }
    ptr++;
    distance++;
    while(spaceln(*ptr)) {
        ptr++;
        distance++;
    }
    free(op);
    return distance;
}

/******************************************************************************
* Function : compatible_args(char *line)
*//**
* \section Description: this function checks if the structure of the data directive line given is ok.
*                       called only if the current line is a data directive. if not, it will
*                       report an error according to the error in the line.
*
* \param  		line - the current line(or part of it)
* \return       TRUE if the structure of the data directive line given is ok
*******************************************************************************/
int compatible_args(char *line) {
    char *directive_name;
    char *ptr = line;
    int i,d;
    int num_args;
    d = is_data(ptr);
    ptr+= next_op(ptr,FALSE);
    if (empty(ptr)) {
        fprintf(stderr,"error: no arguments in this directive line ");
    }
    if(d == ASCIZ) { /*.asciz*/
        while(spaceln(*ptr)) ptr++;
        if(*ptr!='\"') {
            fprintf(stderr,"error: .asciz directive should contain a string in double quotation marks ");
            return FALSE;
        }
        ptr++; /*skipping the opening '\"'*/
        while(!empty(ptr+1)) {
            ptr++;
        }
        if(*ptr == '\"')
            return TRUE;
        fprintf(stderr,"error: no closing \" in .asciz directive ");
        return FALSE;
    }

    num_args = get_num_args(ptr);
    if(num_args == 0)
        return FALSE;
    switch(d) {
        case DB:
            directive_name = ".db";
            break;
            case DH:
                directive_name = ".dh";
                break;
                case DW:
                    directive_name = ".dw";
                    break;
                    default:
                        fprintf(stderr,"this should not happen [compatible_args switch]");
                        return FALSE;
    }
    if(intlen(ptr) == 0) {
        fprintf(stderr,"error: %s only works with integers ",directive_name);
        return FALSE;
    }
    for(i=1; i < num_args; i++) {
        if (next_op(ptr, TRUE) == NON_VALID_OPERAND)
            return FALSE;
        ptr += next_op(ptr, TRUE);
        if(intlen(ptr) == 0) {
            fprintf(stderr,"error: %s only works with integers ",directive_name);
            return FALSE;
        }
        if(intlen(ptr) == 0)
            return FALSE;
    }
    return TRUE;
}

/******************************************************************************
* Function : asciz_len(char *line)
*//**
* \section Description: this function checks the length of the string in the asciz directive.
*                       called only when the assembler identifies an asciz directive without any errors.
*
* \param  		line - the current line(after ".asciz")
* \return       the length of the string in the asciz directive
*******************************************************************************/
int asciz_len(char *line) {
    char *ptr;
    int length = 0;
    ptr = (char*) malloc (MAX_LINE+1);
    alloc_check(ptr);
    strcpy(ptr,line);
    ptr++; /*skipping opening '\"'*/
    while(!empty(ptr)) {
        ptr++;
        length++;
    }
    length--; /*subtracting the closing '\"'*/
    return length;
}

/******************************************************************************
* Function : get_num_args(char *line)
*//**
* \section Description: this checks how many arguments are in this .db,.dh or .dw directive line
*
* \param  		line - the current line(after the directive identifier)
* \return       the number of arguments that are in this .db,.dh or .dw directive line
*******************************************************************************/
int get_num_args(char *line) {
    char *ptr = line;
    int num_args = 1;
    while(spaceln(*ptr)) ptr++;
    if(intlen(ptr) == 0)
        return 0;
    ptr+=intlen(ptr);
    while (empty(ptr) == FALSE) {
        while (spaceln(*ptr))
            ptr++;
        if(*ptr != ',') {
            fprintf(stderr,"invalid argument ");
            return 0;
        }
        ptr++;
        while(spaceln(*ptr)) ptr++;
        if(intlen(ptr) == 0)
            return 0;
        ptr+=intlen(ptr);
        num_args++;
    }
    return num_args;
}

/*************** END OF FUNCTIONS ***************************************************************************/