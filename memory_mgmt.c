/*******************************************************************************
* Title                 :   Memory management
* Filename              :   memory_mgmt.c
* Author                :   Itai Kimelman
* Version               :   1.5.0
*******************************************************************************/

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
extern symbol_node *symbol_table;
extern ext_node *external_list;
extern command_image *code_img;
extern data_image *data_img;
extern unsigned long code_img_length, data_img_length;

/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : alloc_check(void *);
*//**
* \section Description Description:
* this function checks if the pointer given points to NULL, and terminates the program if it is
* This function is used to check if memory allocation failed after using malloc, calloc or realloc
*
* \param  		x the pointer given (and allocated before)
*
*******************************************************************************/
void alloc_check(void * x) {
    if(x == NULL) {
        fprintf(stderr,"memory allocation problems\n");
        mem_deallocate();
        exit(1);
    }
}

/******************************************************************************
* Function : mem_allocate();
*//**
* \section Description Description:
* this functions allocates memory for the code image and data image table
* This function is used to allocate memory for the code image and data image table
* before we read the source file.
*
*******************************************************************************/
void mem_allocate() {
    /*allocating for the code image table*/
    code_img_length = 0;
    code_img = (command_image*) malloc(sizeof(command_image));
    alloc_check(code_img);
    /*allocating for the data image table*/
    data_img_length = 0;
    data_img = (data_image*) malloc(sizeof(data_image));
    alloc_check(data_img);

}

/******************************************************************************
* Function : mem_deallocate();
*//**
* \section Description Description:
* this functions deallocates memory for all of the tables
* This function is used to deallocate memory for all of the tables before we
* terminate the program
*
*******************************************************************************/
void mem_deallocate() {
    void deallocate_external_list();
    void deallocate_symbol_table();
    free(code_img);
    free(data_img);
    deallocate_external_list();
    deallocate_symbol_table();
}

/******************************************************************************
* Function : deallocate_external_list();
*//**
* \section Description Description:
* this functions deallocates memory for the external label list
* This function is used to deallocate memory for the external label list
* before we terminate the program
*
*******************************************************************************/
void deallocate_external_list(){
    /*freeing each node*/
    ext_node *curr;
    while(external_list!=NULL) {
        curr = external_list;
        external_list = external_list->next;
        free(curr->label);
        free(curr);
    }
}

/******************************************************************************
* Function : deallocate_symbol_table();
*//**
* \section Description Description:
* this functions deallocates memory for the symbol table
* This function is used to deallocate memory for the symbol table
* before we terminate the program
*
*******************************************************************************/
void deallocate_symbol_table(){
    /*freeing each node*/
    symbol_node *curr;
    while(symbol_table!=NULL) {
        curr = symbol_table;
        symbol_table = symbol_table->next;
        free(curr->symbol);
        free(curr);
    }

}

/*************** END OF FUNCTIONS ***************************************************************************/