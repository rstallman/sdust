
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    /* YOUR CODE HERE */
     
     SymbolTable* table = malloc(sizeof(SymbolTable));
     
     if(table == NULL)  allocation_failed();
        
     // inital capacity is 2
     Symbol* sym = malloc(sizeof(Symbol) * 2);
     if(sym == NULL) allocation_failed();

     table->tbl = sym;
     table->cap = 2;
     table->len = 0;
     table->mode = mode;
     
     return table;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
     /* YOUR CODE HERE */
     
     if(table == NULL) return;
     
     if(table->tbl != NULL) {
	  for(int i = 0; i < table->len; i++) {
	       if(table->tbl[i].name != NULL)
		    free(table->tbl[i].name);
	  }
	  
	  free(table->tbl);
     }
     
     free(table);
     
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
    /* YOUR CODE HERE */
     
     if((addr % 4) !=  0)  {
	  addr_alignment_incorrect();
	  return -1;
     }
     
     if(table->mode == SYMTBL_UNIQUE_NAME &&  get_addr_for_symbol(table, name) != -1) {
	       name_already_exists(name);
	       return -1;
     }
     
     
     if(table->len >= table->cap) {

	  int new_cap = table->cap * 2;
          
	  table->tbl = realloc(table->tbl,  new_cap * sizeof(Symbol));

	  if(table->tbl  == NULL)
	       allocation_failed();
	  
	  table->cap =  new_cap;
     }
     
     table->tbl[table->len].name = strdup(name);
     table->tbl[table->len].addr = addr;

     table->len++;
     
     return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
*/
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
     /* YOUR CODE HERE */
 
    // linear search

     if(table == NULL || table->tbl == NULL) return -1;
     
     for(int i = 0; i < table->len; i++) {
	  if(strcmp(table->tbl[i].name, name) == 0)
	       return table->tbl[i].addr;
     }
     
     return -1;   
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
*/
void write_table(SymbolTable* table, FILE* output) {
     /* YOUR CODE HERE */
    
     if(table == NULL || table->tbl == NULL) return;

     for(int i = 0; i < table->len; i++) {
	  write_symbol(output, table->tbl[i].addr, table->tbl[i].name);
     }
}



// test code
#if 0
int main()
{
     
     SymbolTable* tb1 = create_table(SYMTBL_UNIQUE_NAME);
     SymbolTable* tb2 = create_table(SYMTBL_UNIQUE_NAME);

     
     add_to_table(tb1, "Loop1", 0x400000);
     add_to_table(tb1, "Loop2", 0xF00000);
     add_to_table(tb1, "Loop3", 0xFF0000);
     add_to_table(tb1, "Loop4", 0xEF0000);
     add_to_table(tb1, "Done:", 0xEE0000);

     char str[255];
     strcpy(str, "Label0");
     
     for(int i = 0; i < 100; i++) {
	  str[5] = i;
	  add_to_table(tb1, str, 0xEFFF00);
	  add_to_table(tb2, str, 0xFFFFFF00);
     }
     
     write_table(tb1, stdout);
     
     int64_t res = get_addr_for_symbol(tb1, "Done:");

     if(res != -1) {
	  printf("Find symbol: addr: %lld\n", res);
     }
     else {
	  printf("No symbol in Table\n");	  
     }
     
     
     free_table(tb1);
     free_table(tb2);
     
     return 0;
}

#endif
