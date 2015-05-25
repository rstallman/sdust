#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tables.h"
#include "translate_utils.h"
#include "translate.h"
#include "utils.h"



static int translate_ulong(unsigned long int* output, const char* str, unsigned long int lower_bound,
		unsigned long int upper_bound) {

	if (!str || !output) {
		return -1;
	}


	char * pEnd;
	unsigned long int res;
	if(strstr(str, "0x") != NULL || strstr(str, "0X") != NULL) {
		res = strtoul(str, &pEnd,  16);
	}
	else {
		res = strtoul(str, &pEnd, 10);
	}

	if (*pEnd != '\0')    return -1;

	*output = res;


	if(res >= lower_bound && res <=  upper_bound)
		return 0;
	else
		return -1;

}
/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and blt pseudoinstructions. Your pseudoinstruction 
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are 
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number 
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into 
        a lui-ori pair.

   And for blt:
    - your expansion should use the fewest number of instructions possible.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that 
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {

     if (strcmp(name, "li") == 0) {
        /* YOUR CODE HERE */
	  
	  if(num_args != 2)  return 0;
	   
	  long int imm;
	  long int lowerbound = -2147483647L;  // see limits.h 
	  long int upperbound =  2147483647L;  // see limits.h 
	  int err = translate_num(&imm, args[1], lowerbound, upperbound);

 
	 if(err == -1)  return 0;

	 char  tmp[255];
	 
	 if(imm >= -32769 && imm <= 32768) { // imm fit into 16 bit
	                                     // signed number
	      sprintf(tmp, " %ld", imm);

	      char* addiuargs[3] = {args[0], "$zero", tmp};
	      write_inst_string(output,"addiu", addiuargs, 3);
	      return 1;
	 }
	 else {

		  unsigned long uimm;
		  err = translate_ulong(&uimm, args[1], 0, 0xFFFFFFFF);

	      int upper16 =  (uimm >> 16) & 0xFFFF;
	      int lower16 =  uimm & 0xFFFF;
	    
	      sprintf(tmp, " 0x%x", upper16);
	      char* luiargs[2] = {"$at", tmp};
	      write_inst_string(output,"lui", luiargs, 2);

	      sprintf(tmp, " 0x%x", lower16);
	      char* oriargs[3] = {args[0], "$at", tmp};
	      write_inst_string(output, "ori",  oriargs, 3);

	      return 2;
	 }
	 
	 
    } else if (strcmp(name, "blt") == 0) {

	  /* YOUR CODE HERE */

	  if(num_args != 3)  return 0;
	  
	  char* sltargs[3] = {"$at", args[0], args[1]};
	  write_inst_string(output, "slt", sltargs, 3);
	  
	  char* bneargs[3] = {"$at", "$zero", args[2]};
	  write_inst_string(output, "bne", bneargs, 3);
	  
	  return 2;
	  
    } else {
        write_inst_string(output, name, args, num_args);
        return 1;
    }
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write 
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

   Note the use of helper functions. Consider writing your own! If the function
   definition comes afterwards, you must declare it first (see translate.h).

   Returns 0 on success and -1 on error. 
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args, uint32_t addr,
    SymbolTable* symtbl, SymbolTable* reltbl) {
    if (strcmp(name, "addu") == 0)       return write_rtype(0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0)    return write_rtype(0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0)   return write_rtype(0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0)  return write_rtype(0x2b, output, args, num_args);
    else if (strcmp(name, "sll") == 0)   return write_shift(0x00, output, args, num_args);
    else if (strcmp(name, "jr") == 0)    return write_jr(0x08, output, args, num_args);
    else if (strcmp(name, "addiu") == 0) return write_addiu(0x09, output, args, num_args);
    else if (strcmp(name, "ori") == 0)   return write_ori(0x0d, output, args, num_args);
    else if (strcmp(name , "lui") == 0)  return write_lui(0x0f, output, args, num_args);
    else if (strcmp(name, "lb") == 0)    return write_mem(0x20, output, args, num_args);
    else if (strcmp(name ,"lbu") == 0)   return write_mem(0x24, output, args, num_args);
    else if (strcmp(name, "lw") == 0)    return write_mem(0x23, output, args, num_args);
    else if (strcmp(name, "sb") == 0)    return write_mem(0x28, output, args, num_args);
    else if (strcmp(name, "sw") == 0)    return write_mem(0x2B, output, args, num_args);
    else if (strcmp(name, "beq") == 0)   return write_branch(0x04, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "bne") == 0)   return write_branch(0x05, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "j") == 0)     return write_jump(0x02, output, args, num_args, addr, reltbl);    // label always  need relocation, set addr to be 0
    else if (strcmp(name, "jal") == 0)   return write_jump(0x03, output, args, num_args, addr, reltbl);    //label always need relocation, set addr to be 0
    
    /* YOUR CODE HERE */
    else                                 return -1;
}





int write_jump(uint8_t opcode, FILE* output, char** args, size_t num_args, 
	       uint32_t addr, SymbolTable* reltbl) {
     
     if(num_args != 1 || addr > 0xFFFFFFF || (addr % 4) != 0 )  return -1;
     
     int64_t labelAddress =  get_addr_for_symbol(reltbl, args[0]);

     if(labelAddress == -1)   add_to_table(reltbl, args[0], addr);
     
     
     uint32_t instruction = 0;
    

     instruction |= (opcode << 26);             // opcode
     
     write_inst_hex(output, instruction);

     return 0;
  
}



int write_branch(uint8_t opcode, FILE* output, char** args, size_t num_args, 
		 uint32_t addr, SymbolTable* symtbl) {

     if(num_args != 3) return -1;
     
     int rs = translate_reg(args[0]);
     int rt = translate_reg(args[1]);
     int64_t labelAddress =  get_addr_for_symbol(symtbl, args[2]);
     
     if(rs == -1 || rt == -1 || labelAddress == -1 ||
	(labelAddress % 4 != 0) || labelAddress >  32768 ||
	labelAddress < -32769) return -1;

     
     int imm = (labelAddress - addr - 4) / 4; // addr = pc addr

     
     uint32_t instruction = 0;
     
     instruction |= (imm & 0xFFFF);   // funct, lower six bit
     instruction |= (rt << 16);       // rt
     instruction |= (rs << 21);       // rs
     instruction |= (opcode << 26);   // opcode
    
     write_inst_hex(output, instruction);

     return 0;
     
}


/*
 * split arg string like "-100($t1)" to two strings
 * "-100" and "$t1" , store them in pointer array and
 * return pointer to pointer array
 */

static int splitArgs(char* arguments, char** arg0, char** arg1) {

     char args[256];
     strcpy(args, arguments);
     
     char  tmp[256];
     
     char* leftp = strchr(args, '(');
     char* rightp = strchr(args, ')');
     
     if(leftp == NULL || rightp == NULL || (rightp - args) != strlen(args) - 1 )
	  return -1;

     *rightp = '\0';
     strcpy(tmp, leftp + 1);
     *arg1 = strdup(tmp);

     *leftp = '\0';
     strcpy(tmp, args);
     *arg0 = strdup(tmp);
     
     return 0;
     
}


int write_mem(uint8_t opcode, FILE* output, char** args, size_t num_args) {

     if(num_args != 2) return -1;
     
     int rt = translate_reg(args[0]);

     char* arg0;
     char* arg1;
     
     int res =  splitArgs(args[1], &arg0, &arg1);

     // printf("slit res: %d\n", res);

     if (res == -1) return -1;
     
     int rs = translate_reg(arg1);
     long int imm;
     
     int err = translate_num(&imm, arg0, -32769, 32768);

     free(arg0);
     free(arg1);

     // printf("rs: %d, rt: %d, err: %d\n", rs, rt, err);
     
     if( rs == -1 || rt == -1  || err == -1)  return -1;  // invalid reg
    
     uint32_t instruction = 0;
     
     instruction |= (imm & 0xFFFF);   // funct, lower six bit
     instruction |= (rt << 16);       // rt
     instruction |= (rs << 21);       // rs
     instruction |= (opcode << 26);   // opcode
    
     write_inst_hex(output, instruction);

     return 0;
     
     
}




int write_lui(uint8_t opcode, FILE* output, char** args, size_t num_args) {

     if(num_args != 2) return -1;
     
     int rt = translate_reg(args[0]);
    
     long int imm;
     
     int err = translate_num(&imm, args[1], 0, 65535);
     
     if( rt == -1  || err == -1)  return -1;  // invalid reg
    
     uint32_t instruction = 0;
     
     instruction |= (imm & 0xFFFF);   // funct, lower six bit
     instruction |= (rt << 16);       // rt
   
     instruction |= (opcode << 26);   // opcode
    
     write_inst_hex(output, instruction);

     return 0;

}

int write_ori(uint8_t opcode, FILE* output, char** args, size_t num_args) {

     if(num_args != 3) return -1;
     
     int rt = translate_reg(args[0]);
     int rs = translate_reg(args[1]);
     long int imm;
     
     int err = translate_num(&imm, args[2], 0, 65535);
     
     if( rs == -1 || rt == -1  || err == -1)  return -1;  // invalid reg
    
     uint32_t instruction = 0;
     
     instruction |= (imm & 0xFFFF);   // funct, lower six bit
     instruction |= (rt << 16);       // rt
     instruction |= (rs << 21);       // rs
     instruction |= (opcode << 26);   // opcode
    
     write_inst_hex(output, instruction);

     return 0;
     
}



int write_addiu(uint8_t opcode, FILE* output, char** args, size_t num_args) {

     if(num_args != 3) return -1;
     
     int rt = translate_reg(args[0]);
     int rs = translate_reg(args[1]);
     long int imm;
     
     int err = translate_num(&imm, args[2], -32769, 32768);
     
     if( rs == -1 || rt == -1  || err == -1)  return -1;  // invalid reg
    
     uint32_t instruction = 0;
     
     instruction |= (imm & 0xFFFF);   // funct, lower six bit
     instruction |= (rt << 16);       // rt
     instruction |= (rs << 21);       // rs
     instruction |= (opcode << 26);   // opcode
    
     write_inst_hex(output, instruction);

     return 0;

}

/*
 * helper function for jr $reg instruction
 */

int write_jr(uint8_t funct, FILE* output, char** args, size_t num_args) {

     if(num_args != 1) return -1;
     
     int rs = translate_reg(args[0]);

     if( rs == -1 )  return -1;  // invalid reg
    
     uint32_t instruction = 0;
     
     instruction |= (funct & 0x3F);   // funct, lower six bit
     
     instruction |= (rs << 21);       // rs
    
     write_inst_hex(output, instruction);

     return 0;

}


/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {

     if(num_args != 3) return -1;
     
     int rd = translate_reg(args[0]);
     int rs = translate_reg(args[1]);
     int rt = translate_reg(args[2]);
     
     if(rd == -1 || rs == -1 || rt == -1)  return -1;  // invalid reg
    
     uint32_t instruction = 0;
     
     instruction |= (funct & 0x3F);   // funct, lower six bit
     instruction |= (rd << 11);       // rd
     instruction |= (rt << 16);       // rt
     instruction |= (rs << 21);       // rs
    
     write_inst_hex(output, instruction);

     return 0;
}

/* A helper function for writing shift instructions. You should use 
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_shift(uint8_t funct, FILE* output, char** args, size_t num_args) {

     if(num_args != 3) return -1;
     
    long int shamt;
    int rd = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int err = translate_num(&shamt, args[2], 0, 31);

    
    if(err == -1 || rd == -1 || rt == -1)  return -1;

    
    uint32_t instruction = 0;

    
    instruction |= (funct & 0x3F);   // funct, lower six bit
    instruction |= (shamt << 6);     // shamt
    instruction |= (rd << 11);       // rd
    instruction |= (rt << 16);       // rt
   
    write_inst_hex(output, instruction);
     
    return 0;
}


#if 0
// test code

int main()
{

     char* args[3] = {"$a0", "$t1", "$t3" };

     
     //  write_rtype(0x21, stdout, args, 3);
     //  write_rtype(0x25, stdout, args, 3);
     //  write_rtype(0x2a, stdout, args, 3);

     char* nargs[3] = {"$v0", "$s0", "10" };
     write_shift(0x00, stdout, nargs, 3);

     char* jargs[1] = {"$t0"};
     write_jr(0x08, stdout, jargs, 1);

     
     char* iargs[3] = {"$t1", "$t2", "-100"};
     write_addiu(0x09, stdout, iargs, 3);

     char* oargs[2] = {"$t1",  "0xEEFF"};
     write_lui(0x0f,  stdout, oargs, 2);

     char* lbargs[2] = {"$t1", "-100($s3)"};
     write_mem(0x2B, stdout, lbargs, 2);


     // test beq

     SymbolTable* tb1 = create_table(SYMTBL_UNIQUE_NAME);
     SymbolTable* tb2 = create_table(SYMTBL_UNIQUE_NAME);
     
     add_to_table(tb1, "Loop1", 0x400000);
     add_to_table(tb1, "Loop2", 0xF00000);
     add_to_table(tb1, "Loop3", 0xFF0000);
     add_to_table(tb1, "Done",  0x00400010);
     
     char* beqargs[3] = {"$t1", "$t2", "Done"};

     write_branch(0x04, stdout, beqargs, 3, 0x00400030, tb1);

     char* jumpargs[1] = {"Done"};

     /*
     int64_t labelAddress =  get_addr_for_symbol(tb1, jumpargs[0]);
	 
     if(labelAddress != -1) {
	      // if label in symtbl, find its address and go!
	  return write_jump(0x03, stdout, jargs, 1, labelAddress, NULL);
     }
     else {
	      // else label need relocation, set addr to be 0 and go
	  return write_jump(0x03, stdout, jargs, 1, 0, tb2);
     }

     */
     
     char* liargs[2] = {"$a1", "-299999"};
     write_pass_one(stdout, "li",  liargs, 2);



     char* bltargs[3] = {"$a0", "$a1", "Loop1"};
     write_pass_one(stdout, "blt", bltargs, 3);
     
     free_table(tb1);
     free_table(tb2);
     
     
     return 0;
     
}

#endif
