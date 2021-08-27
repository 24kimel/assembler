#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"

/*this func returns TRUE if the length of this line does not go over 80 characters. It will report an error if not*/
int length_check(char *line) {
    /*subtracting the newline character from the character count, then checking if there are characters in the line than the maximum allowed*/
    if(strlen(line)-1 > MAX_LINE) {
        fprintf(stderr,"error: line length above maximum (80 characters) ");
        return FALSE;
    }
    /*there are 80 or fewer characters in this line -> no error*/
    return TRUE;
}
/*return TRUE on success FALSE if last line
 * fp - input file, line - pre allocated string with size of MAX_LINE+1 chars*/
int read_line(FILE *fp, char* line) {
    if(fgets(line, MAX_LINE, fp) == NULL)
        return FALSE;
    return TRUE;
}


/*returns non-zero if the character c is a white space but is not the end of the line/file*/
int spaceln(char c) {
    if(isspace(c) && c!='\n' && c!=EOF) return TRUE;
    return FALSE;
}

/*returns non-zero if the line is empty*/
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

int intlen(char *line) {
    char *ptr;
    int i;
    ptr = line;
    i = 0;
    if(*ptr == '+' || *ptr == '-') {
        ptr++;
        i++;
    }
    while(!isspace(*ptr) && *ptr != ',') {
        if(!isdigit(*ptr)) return i;
        ptr++;
        i++;
    }
    return i;
}

/*this method will return non-zero if the assembler will skip this line(if this line is empty or a comment line)*/
int meaningless(char *line) {
    char *ptr = line;
    if(*ptr == '\n' || *ptr == ';' || *ptr == EOF)
        return TRUE;
    return FALSE;
}

int start_label(char *line) {
    int i = 0;
    char *word;
    word = (char*) malloc (sizeof(char) * MAX_LINE+1);
    alloc_check(word);
    while(!isspace(line[i])) {
        word[i] = line[i];
        i++;
    }
    word[i] = '\0';
    if(word[i-1] != ':') {
        free(word);
        return FALSE;
    }
    strtok(word,":");
    if(!is_label(word,0)) {
        free(word);
        return FALSE;
    }
    free(word);
    return TRUE;
}

/*returns non-zero if the next word in line can be a label. According to
 * page 28 in manual, a string can be a label if it only contains characters that are
 * alphanumeric, starts with a letter, and contains up to 31 characters.*/
int is_label(char *line, int err) {
    char *ptr = line;
    int i = 0;
    if(!isalpha(ptr[0])) {
        if(err == TRUE)
            fprintf(stderr, "error: a label should start with a letter ");
        return FALSE;
    }

    while(!isspace(ptr[i]) && ptr[i] != '\0' && !endline(ptr[i])) {
        if(!isalnum(ptr[i]) && !endline(ptr[i])) {
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

/**/
void scan_label (char *line, char*label) {
    int i;
    i=0;
    while(!isspace(line[i]) && line[i]!=':') {
        label[i] = line[i];
        i++;
    }
    label[i] = '\0';
}

int is_data(char *line) {
    int i = 0;
    int retval = 0;
    char *word = (char*) malloc(sizeof(char) * (MAX_LINE+1));
    alloc_check(word);
    while(!isspace(line[i])) {
        word[i] = line[i];
        i++;
    }
    word[i] = '\0';
    if(strcmp(word,".db")==0)
        retval =  1;
    if(strcmp(word,".dh")==0)
        retval = 2;
    if(strcmp(word,".asciz")==0)
        retval = 3;
    if(strcmp(word,".dw")==0)
        retval = 4;
    free(word);
    return retval;
}

int ent_ext(char *line) {
    int i = 0;
    char *word = (char*) malloc(sizeof(char) * (MAX_LINE+1));
    alloc_check(word);
    while(!isspace(line[i])) {
        word[i] = line[i];
        i++;
    }
    word[i] = '\0';
    if(strcmp(word,".entry")==0)
        return 1;
    if(strcmp(word,".extern") ==0)
        return 2;
    return 0;
}

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

int order_structure(char *line) {
    char *ptr = line;
    unsigned oc;
    int i;
    while(spaceln(*ptr))
        ptr++;
    oc = get_opcode(line);
    if(next_op(ptr,FALSE) == -1)
        return FALSE;
    ptr+= next_op(ptr,FALSE);
    if(oc==0) { /*3 registers needed*/
        if(register_num(ptr,TRUE)==-1)
            return FALSE;
        for(i = 0; i < 2; i++) {
            if(next_op(ptr,1) == -1)
                return FALSE;
            ptr+= next_op(ptr,1);
            if(register_num(ptr,TRUE)==-1)
                return FALSE;
        }
        ptr++;
        ptr+= intlen(ptr);
        if(empty(ptr))
            return TRUE;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }

    if(oc==1) { /*2 registers needed*/
        if(register_num(ptr,TRUE)==-1)
            return FALSE;
        if(next_op(ptr,TRUE) == -1)
            return FALSE;
        ptr+= next_op(ptr,1);
        if(register_num(ptr,TRUE)==-1)
            return FALSE;
        ptr++;
        ptr+= intlen(ptr);
        if(empty(ptr))
            return TRUE;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }

    if(oc<=14  || (oc>=19 && oc<=24)) { /*2 registers needed, immed between them*/
        if(register_num(ptr,TRUE)==-1)
            return FALSE;
        if(next_op(ptr,TRUE) == -1)
            return FALSE;
        ptr+=next_op(ptr,TRUE);
        if(check_immed(ptr) == FALSE)
            return FALSE;
        if(next_op(ptr,TRUE) == -1)
            return FALSE;
        ptr+=next_op(ptr,TRUE);
        if(register_num(ptr,TRUE)==-1)
            return FALSE;
        ptr++;
        ptr+= intlen(ptr);
        if(empty(ptr))
            return TRUE;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }

    if(oc<=18) {
        for(i = 0; i < 2; i++) {
            if(register_num(ptr,TRUE)==-1)
                return FALSE;
            if(next_op(ptr,TRUE) == -1)
                return FALSE;
            ptr+= next_op(ptr,TRUE);
        }
        if(!is_label(ptr,TRUE))
            return FALSE;
        while(isalpha(*ptr) || isdigit(*ptr)) ptr++;
        if(empty(ptr))
            return TRUE;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }

    if(oc==30) {
        int result;
        if (is_label(ptr, FALSE) == 0 && register_num(ptr, FALSE) == -1) {
            fprintf(stderr,"error: this operand is not a label or a register ");
            return 0;
        }
        if (is_label(ptr, FALSE))
            result = 1;
        else result = 2;

        if(result == 1) {
            while(isalpha(*ptr) || isdigit(*ptr)) ptr++;
        }
        else {
            ptr++;
            ptr+= intlen(ptr);
        }
        if(empty(ptr)) return result;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }

    if(oc <= 32) {
        if(is_label(ptr,1) == FALSE)
            return FALSE;
        while(isalpha(*ptr) || isdigit(*ptr))
            ptr++;
        if(empty(ptr))
            return TRUE;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }

    if(oc == 63) {
        if(empty(ptr))
            return TRUE;
        else {
            fprintf(stderr,"error: too much operands ");
            return FALSE;
        }
    }
    /*every opcode in the table should answer TRUE to one and only one of the if statements mentioned above*/
    fprintf(stderr,"this should not happen (opcode table error) ");
    return FALSE;
}

int register_num(char *line, int err) {
    char *ptr = line;
    int reg;

    if(*ptr != '$') {
        if(err == TRUE)
            fprintf(stderr, "error: a register should be here (a register starts with a $, followed by an integer between 0 and 31)");
        return -1;
    }
    if(!isdigit(*(++ptr))) {
        if(err == TRUE)
            fprintf(stderr, "error: a register should be here (a register starts with a $, followed by an integer between 0 and 31)");
        return -1;
    }
    reg = atoi(ptr);
    if(!(reg>=0 && reg<=31)) {
        if(err == TRUE)
            fprintf(stderr, "error: register number %d does not exist ",reg);
        return -1;
    }
    ptr+=intlen(ptr);
    if(*ptr!=(char)0 && !spaceln(*ptr) && *ptr != ',' && !endline(*ptr)) {
        fprintf(stderr,"error: invalid register. after the register number, there can only be a comma or a space character ");
        return -1;
    }
    return reg;
}

char* scan_op(char *line, char *op) {
    char *ptr = line;
    int i=0;
    while (spaceln(*ptr))
        ptr++;
    while(!spaceln(ptr[i]) && ptr[i] != ',' && !endline(ptr[i])) {
        op[i] = ptr[i];
        i++;
    }
    op[i] = '\0';
    return op;
}

/*TODO: check for errors in next_op every time I traverse*/
/*points directly at operand. works without errors like the reg*/
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
        return -1;
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

/*points after optional label*/
int compatible_args(char *line) {
    char *directive_name;
    char *ptr = line;
    int i,d;
    int num_args;
    d = is_data(ptr);
    ptr+= next_op(ptr,0);
    if(d == ASCIZ) { /*.asciz*/
        while(spaceln(*ptr)) ptr++;
        if(*ptr!='\"') {
            fprintf(stderr,".asciz directive should contain a string in double quotation marks ");
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
    }
    if(intlen(ptr) == 0) {
        fprintf(stderr,"%s only works with integers ",directive_name);
        return FALSE;
    }
    for(i=1; i < num_args; i++) {
            if (next_op(ptr, TRUE) == -1)
                return FALSE;
            ptr += next_op(ptr, TRUE);
            if(intlen(ptr) == 0) {
                fprintf(stderr,"%s only works with integers ",directive_name);
                return FALSE;
            }
            if(intlen(ptr) == 0)
                return FALSE;
    }
    return TRUE;
}
/*this func gets a pointer to the line after the ".asciz" directive, and counts how many characters are requested to save (not including the NULL char at the end)
 * this func will only be used if the directive is valid*/
int asciz_len(char *line) {
    char *ptr;
    int length = 0;
    ptr = (char*) malloc (MAX_LINE+1);
    alloc_check(ptr);
    strcpy(ptr,line);
    ptr++; /*skipping '\"'*/
    while(!empty(ptr)) {
        ptr++;
        length++;
    }
    length--; /*subtracting the closing '\"'*/
    return length;
}
/*points after optional label and directive*/
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
            fprintf(stderr,"invalid argument");
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