#include "../inc/assembler.h"
#include "../inc/util.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

// global variabels to determine whether I need to print two lines (in the "li" pseudo instuction)
int LiSecondLine = 0;
int SecondFlag = 0;
bool Error3 = false; // false denotes that there are no error and only for the instuction in the form of lb rd, offset(rs1) /sb rs2, offset(rs1)

// Instruction Types containing all types of instructions
typedef enum
{
  INSTR_R_TYPE,
  INSTR_I_TYPE,
  INSTR_S_TYPE,
  INSTR_SB_TYPE,
  INSTR_U_TYPE,
  INSTR_UJ_TYPE,
  INSTR_PSEUDO,
  INSTR_INVALID = -1
} InstructionType;

// Instruction that describe how a instruction is constructed
typedef struct
{
  uint32_t funct7;
  uint32_t rs2;
  uint32_t rs1;
  uint32_t funct3;
  uint32_t rd;
  uint32_t opcode;
} Instruction;

// Instruction Format
typedef struct
{
  const char *name;
  uint32_t opcode;
  uint32_t funct3;
  uint32_t funct7;
  InstructionType type;
} InstructionFormat;

// function declaration
char *strdup(const char *s);
int get_register_number(const char *reg_name);
int parse_immediate_and_register(const char *str, int *reg_num);
void trim_newline(char *str);
bool isNumeric(const char *str);
const InstructionFormat *get_instruction_format(const char *mark);
uint32_t get_machine_code(InstructionFormat ins, int rd, int rs1, int rs2);
int get_register_number(const char *reg_name);
uint32_t assemble_instruction(const char *instruction);

// string copy
char *strdup(const char *s)
{
  char *d = malloc(strlen(s) + 1);
  if (d == NULL)
    return NULL;
  strcpy(d, s);
  return d;
}

// parse_immediate_and_register
int parse_immediate_and_register(const char *str, int *reg_num)
{
  int imm;
  char reg[10];
  // sscanf(str, "%d(%[^)])", &imm, reg);
  int parsed_items = sscanf(str, "%d(%[^)])", &imm, reg);
  if (parsed_items != 2)
  {
    Error3 = true;
    return -1;
  }
  *reg_num = get_register_number(reg);
  return imm;
}

// remove the "\n" of each line
void trim_newline(char *str)
{
  if (str == NULL)
    return;
  int length = strlen(str);
  if (length > 0 && str[length - 1] == '\n')
  {
    str[length - 1] = '\0';
  }
}

// check " The imm or offset in instructions may not be a number, " (Error Handling 3)
bool isNumeric(const char *str)
{
  if (str == NULL || *str == '\0')
  {
    return false;
  }
  if (*str == '+' || *str == '-' || isdigit((unsigned char)*str))
  {
    if (*(str + 1) == '\0' && (*str == '+' || *str == '-'))
    {
      return false;
    }

    str++;
  }
  else
  {
    return false;
  }
  while (*str)
  {
    if (!isdigit((unsigned char)*str))
    {
      return false;
    }
    str++;
  }
  return true;
}

// Instruction Set Table where 0xFFFFFFFF denotes not exist
const InstructionFormat instructionFormats[] = {
    // R-Type instructions
    {"add", 0x33, 0x00, 0x00, INSTR_R_TYPE},
    {"mul", 0x33, 0x00, 0x01, INSTR_R_TYPE},
    {"sub", 0x33, 0x00, 0x20, INSTR_R_TYPE},
    {"sll", 0x33, 0x01, 0x00, INSTR_R_TYPE},
    {"mulh", 0x33, 0x01, 0x01, INSTR_R_TYPE},
    {"slt", 0x33, 0x02, 0x00, INSTR_R_TYPE},
    {"sltu", 0x33, 0x03, 0x00, INSTR_R_TYPE},
    {"xor", 0x33, 0x04, 0x00, INSTR_R_TYPE},
    {"div", 0x33, 0x04, 0x01, INSTR_R_TYPE},
    {"srl", 0x33, 0x05, 0x00, INSTR_R_TYPE},
    {"sra", 0x33, 0x05, 0x20, INSTR_R_TYPE},
    {"or", 0x33, 0x06, 0x00, INSTR_R_TYPE},
    {"rem", 0x33, 0x06, 0x01, INSTR_R_TYPE},
    {"and", 0x33, 0x07, 0x00, INSTR_R_TYPE},

    // I-Type instructions
    {"lb", 0x03, 0x00, 0xFFFFFFFF, INSTR_I_TYPE},
    {"lh", 0x03, 0x01, 0xFFFFFFFF, INSTR_I_TYPE},
    {"lw", 0x03, 0x02, 0xFFFFFFFF, INSTR_I_TYPE},
    {"lbu", 0x03, 0x04, 0xFFFFFFFF, INSTR_I_TYPE},
    {"lhu", 0x03, 0x05, 0xFFFFFFFF, INSTR_I_TYPE},
    {"addi", 0x13, 0x00, 0xFFFFFFFF, INSTR_I_TYPE},
    {"slli", 0x13, 0x01, 0x00, INSTR_I_TYPE},
    {"slti", 0x13, 0x02, 0xFFFFFFFF, INSTR_I_TYPE},
    {"sltiu", 0x13, 0x03, 0xFFFFFFFF, INSTR_I_TYPE},
    {"xori", 0x13, 0x04, 0xFFFFFFFF, INSTR_I_TYPE},
    {"srli", 0x13, 0x05, 0x00, INSTR_I_TYPE},
    {"srai", 0x13, 0x05, 0x20, INSTR_I_TYPE},
    {"ori", 0x13, 0x06, 0xFFFFFFFF, INSTR_I_TYPE},
    {"andi", 0x13, 0x07, 0xFFFFFFFF, INSTR_I_TYPE},
    {"jalr", 0x67, 0x00, 0xFFFFFFFF, INSTR_I_TYPE},
    {"ecall", 0x73, 0x00, 0x000, INSTR_I_TYPE},

    // S-Type instructions
    {"sb", 0x23, 0x00, 0xFFFFFFFF, INSTR_S_TYPE},
    {"sh", 0x23, 0x01, 0xFFFFFFFF, INSTR_S_TYPE},
    {"sw", 0x23, 0x02, 0xFFFFFFFF, INSTR_S_TYPE},

    // SB-Type instructions
    {"beq", 0x63, 0x00, 0xFFFFFFFF, INSTR_SB_TYPE},
    {"bne", 0x63, 0x01, 0xFFFFFFFF, INSTR_SB_TYPE},
    {"blt", 0x63, 0x04, 0xFFFFFFFF, INSTR_SB_TYPE},
    {"bge", 0x63, 0x05, 0xFFFFFFFF, INSTR_SB_TYPE},
    {"bltu", 0x63, 0x06, 0xFFFFFFFF, INSTR_SB_TYPE},
    {"bgeu", 0x63, 0x07, 0xFFFFFFFF, INSTR_SB_TYPE},

    // U-Type instructions
    {"lui", 0x37, 0xFFFFFFFF, 0xFFFFFFFF, INSTR_U_TYPE},
    {"auipc", 0x17, 0xFFFFFFFF, 0xFFFFFFFF, INSTR_U_TYPE},

    // UJ-Type instructions
    {"jal", 0x6F, 0xFFFFFFFF, 0xFFFFFFFF, INSTR_UJ_TYPE},

    // Pseudo instructions
    {"beqz", 0x00, 0x00, 0x00, INSTR_PSEUDO},
    {"bnez", 0x00, 0x00, 0x00, INSTR_PSEUDO},
    {"j", 0x00, 0x00, 0x00, INSTR_PSEUDO},
    {"jr", 0x00, 0x00, 0x00, INSTR_PSEUDO},
    {"li", 0x00, 0x00, 0x00, INSTR_PSEUDO},
    {"mv", 0x00, 0x00, 0x00, INSTR_PSEUDO},

    // End
    {NULL, 0, 0, 0, INSTR_INVALID}};

// Function to get instruction format based on instruction mark
const InstructionFormat *get_instruction_format(const char *mark)
{
  for (int i = 0; instructionFormats[i].name != NULL; ++i)
  {
    if (strcmp(mark, instructionFormats[i].name) == 0)
    {
      return &instructionFormats[i];
    }
  }
  return NULL;
}

// uint32_t get_machine_code(InstructionFormat ins, uint32_t rd, uint32_t rs1, uint32_t rs2)
uint32_t get_machine_code(InstructionFormat ins, int rd, int rs1, int rs2)
{
  Instruction i;
  for (int j = 0; instructionFormats[j].name != NULL; ++j)
  {
    // printf("%s\n", ins.name);
    // printf("%s\n", instructionFormats[j].name);
    if (strcmp(ins.name, instructionFormats[j].name) == 0)
    {
      // printf("%s\n", ins.name);
      i.opcode = instructionFormats[j].opcode;
      i.rd = rd;
      i.funct3 = instructionFormats[j].funct3;
      i.rs1 = rs1;
      i.rs2 = rs2;
      i.funct7 = instructionFormats[j].funct7;
      //----------------------------------------------------------------------------------
      // It should be noted that the function arguments may not fit well with the actual bits 😨
      //-----------------------------------------------------------------------------------
      // in R type, the argument of the function is the same as the
      // R-TYPE	funct7	rs2	rs1	funct3	rd	opcode
      // Bits	  7	      5	  5	  3	      5	  7
      if (ins.type == INSTR_R_TYPE)
      {
        return (i.funct7 << 25) | (i.rs2 << 20) | (i.rs1 << 15) | (i.funct3 << 12) | (i.rd << 7) | (i.opcode);
      }
      // in I type, rs2 here denotes imm 💡, while, set funct7 to 0xFFFFFFFF to denote not exist
      // I-TYPE	imm[11:0]	rs1	funct3	rd	opcode
      // Bits	  12	      5  	3	      5  	7
      else if (ins.type == INSTR_I_TYPE)
      {
        if (i.funct7 == 0xFFFFFFFF)
        {
          return (i.rs2 << 20) | (i.rs1 << 15) | (i.funct3 << 12) | (i.rd << 7) | (i.opcode);
        }
        else // for slli,srli,sral
        {
          return (i.funct7 << 25) | (i.rs2 << 20) | (i.rs1 << 15) | (i.funct3 << 12) | (i.rd << 7) | (i.opcode);
        }
      }
      // in S type,
      // S-TYPE	imm[11:5]	rs2	rs1	funct3	imm[4:0]	opcode
      // Bits	  7	        5	  5	  3     	5	        7
      // here   funct7
      // the argument of the function "rd" here denotes imm, and funct7 will always be 0xFFFFFFFF
      else if (ins.type == INSTR_S_TYPE)
      {
        uint32_t imm_11_5 = ((i.rd & 0x00000FE0) >> 5) << 25; // 0000 0000 0000 0000 0000 1111 1110 0000
        uint32_t imm_4_0 = (i.rd & 0x0000001F) << 7;          // 0000 0000 0000 0000 0000 0000 0001 1111
        i.rs2 = i.rs2 << 20;
        i.rs1 = i.rs1 << 15;
        i.funct3 = i.funct3 << 12;
        return imm_11_5 | (i.rs2) | (i.rs1) | (i.funct3) | imm_4_0 | (i.opcode);
      }
      // SB-TYPE	imm[12]	imm[10:5]	rs2	rs1	funct3	imm[4:1]	imm[11]	opcode
      // Bits	1	6	5	5	3	4	1	7
      // the argument of the function "rd" here denotes imm
      else if (ins.type == INSTR_SB_TYPE)
      {
        uint32_t imm_12 = ((i.rd & 0x00001000) >> 12) << 31;  // 0000 0000 0000 0000 0001 0000 0000 0000
        uint32_t imm_10_5 = ((i.rd & 0x000007E0) >> 5) << 25; // 0000 0000 0000 0000 0000 0111 1110 0000
        uint32_t imm_4_1 = ((i.rd & 0x0000001E) >> 1) << 8;   // 0000 0000 0000 0000 0000 0000 0001 1110
        uint32_t imm_11 = ((i.rd & 0x00000800) >> 11) << 7;   // 0000 0000 0000 0000 0000 1000 0000 0000
        i.rs2 = i.rs2 << 20;
        i.rs1 = i.rs1 << 15;
        i.funct3 = i.funct3 << 12;
        return imm_12 | imm_10_5 | (i.rs2) | (i.rs1) | (i.funct3) | imm_4_1 | imm_11 | (i.opcode);
      }
      // let rs2 to be the immediate
      else if (ins.type == INSTR_U_TYPE)
      {
        uint32_t imm_31_12 = (i.rs2 & 0x000FFFFF) << 12; // 0000 0000 0001 1111 1111 1111 1111 1110
        // uint32_t imm_31_12 = ((i.rs2 >> 12) & 0xFFFFF) << 12; //
        i.rd = (i.rd) << 7;
        return imm_31_12 | (i.rd) | (i.opcode);
      }
      else if (ins.type == INSTR_UJ_TYPE)
      {
        uint32_t imm_20 = ((i.rs2 & 0x00100000) >> 20) << 31;    // 0000 0000 0001 0000 0000 0000 0000 0000
        uint32_t imm_10_1 = ((i.rs2 & 0x000007FE) >> 1) << 21;   // 0000 0000 0000 0000 0000 0111 1111 1110
        uint32_t imm11 = ((i.rs2 & 0x00000800) >> 11) << 20;     // 0000 0000 0000 0000 0000 1000 0000 0000
        uint32_t imm_19_12 = ((i.rs2 & 0x000FF000) >> 12) << 12; // 0000 0000 0000 1111 1111 0000 0000 0000
        i.rd = (i.rd) << 7;
        return imm_20 | imm_10_1 | imm11 | imm_19_12 | (i.rd) | (i.opcode);
      }
      else
      {
        return 0;
      }
    }
  }
  return 0;
}

// int get_register_number(const char *reg_name)
// {
//   // printf("Received register name: '%s'\n", reg_name);
//   if (strcmp(reg_name, "zero") == 0)
//   {
//     // printf("hey");
//     return 0;
//   }
//   if (strcmp(reg_name, "ra") == 0)
//     return 1;
//   if (strcmp(reg_name, "sp") == 0)
//     return 2;
//   if (strcmp(reg_name, "gp") == 0)
//     return 3;
//   if (strcmp(reg_name, "tp") == 0)
//     return 4;
//   if (strcmp(reg_name, "fp") == 0 || strcmp(reg_name, "s0") == 0)
//     return 8;

//   if (reg_name[0] == 'x')
//   {
//     int num = atoi(reg_name + 1);
//     if (num >= 0 && num < 32)
//     {
//       return num;
//     }
//     else
//     {
//       return -1; // Error Handling 2
//     }
//   }

//   if (reg_name[0] == 't')
//   {
//     int num = atoi(reg_name + 1);
//     if (num >= 0 && num <= 6)
//     {
//       if (num < 3)
//         return num + 5; // t0-t2 对应 x5-x7
//       else
//         return num + 25; // t3-t6 对应 x28-x31
//     }
//     else
//     {
//       return -1; // Error Handling 2
//     }
//   }

//   if (reg_name[0] == 's')
//   {
//     int num = atoi(reg_name + 1);
//     if (num == 1)
//     {
//       return 9;
//     }
//     else if (num >= 2 && num <= 11)
//     {
//       return num + 16;
//     }
//     else
//     {
//       return -1; // Error Handling 2
//     }
//   }

//   if (reg_name[0] == 'a')
//   {
//     int num = atoi(reg_name + 1);
//     if (num >= 0 && num <= 7)
//     {
//       return num + 10; // a0-a7 对应 x10-x17
//     }
//     else
//     {
//       return -1; // Error Handling 2
//     }
//   }

//   return -1;
// }

int get_register_number(const char *reg_name)
{
  if (strcmp(reg_name, "zero") == 0)
  {
    return 0;
  }
  if (strcmp(reg_name, "ra") == 0)
    return 1;
  if (strcmp(reg_name, "sp") == 0)
    return 2;
  if (strcmp(reg_name, "gp") == 0)
    return 3;
  if (strcmp(reg_name, "tp") == 0)
    return 4;
  if (strcmp(reg_name, "fp") == 0 || strcmp(reg_name, "s0") == 0)
    return 8;

  if (reg_name[0] == 'x' && isNumeric(reg_name + 1))
  {
    int num = atoi(reg_name + 1);
    if (num >= 0 && num < 32)
    {
      return num;
    }
  }

  if (reg_name[0] == 't' && isNumeric(reg_name + 1))
  {
    int num = atoi(reg_name + 1);
    if (num >= 0 && num <= 6)
    {
      if (num < 3)
        return num + 5; // t0-t2 对应 x5-x7
      else
        return num + 25; // t3-t6 对应 x28-x31
    }
  }

  if (reg_name[0] == 's' && isNumeric(reg_name + 1))
  {
    int num = atoi(reg_name + 1);
    if (num == 1)
    {
      return 9;
    }
    else if (num >= 2 && num <= 11)
    {
      return num + 16;
    }
  }

  if (reg_name[0] == 'a' && isNumeric(reg_name + 1))
  {
    int num = atoi(reg_name + 1);
    if (num >= 0 && num <= 7)
    {
      return num + 10; // a0-a7 对应 x10-x17
    }
  }

  return -1;
}

uint32_t assemble_instruction(const char *instruction)
{
  char *tokens[4] = {NULL, NULL, NULL, NULL};
  char *instr_copy = strdup(instruction);
  if (instr_copy == NULL)
  {
    return 0;
  }
  char *token = strtok(instr_copy, " ");
  int num_args = 0;
  while (token != NULL && num_args < 4)
  {
    tokens[num_args] = token;
    trim_newline(tokens[num_args]);
    num_args++;
    token = strtok(NULL, " ");
  }

  const char *mark = tokens[0];
  const InstructionFormat *fmt = get_instruction_format(mark); // a InstructionFormat 类型的struct，这个结构体有自己的名字，类型等成员属性（变量）
  uint32_t machine_code = 0;
  int imm = 0, rs1_num = 0, rs2_num = 0, rd_num = 0;

  // Error Handling 1: Non-existent instruction👿
  if (fmt == NULL)
  { // Invalid mark
    free(instr_copy);
    return 0;
  }

  // Some helper debugger(?
  // printf("Token 1: %s\n", tokens[1]);
  // printf("Token 2: %s\n", tokens[2]);
  // printf("rd_num:%d\n", rd_num);
  // printf("rs1_num:%d\n", rs1_num);
  // printf("rs2_num:%d\n", rs2_num);
  // printf("imm_num:%d\n", imm);

  // Determine the instruction type and process accordingly
  switch (fmt->type)
  {
  case INSTR_R_TYPE:
    rd_num = get_register_number(tokens[1]);
    rs1_num = get_register_number(tokens[2]);
    rs2_num = get_register_number(tokens[3]);
    if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
    {
      free(instr_copy);
      return 0;
    }
    rd_num = (uint32_t)rd_num;
    rs1_num = (uint32_t)rs1_num;
    rs2_num = (uint32_t)rs2_num;
    machine_code = get_machine_code(*fmt, rd_num, rs1_num, rs2_num);
    break;

  // I-TYPE	imm[11:0]	rs1	funct3	rd	opcode
  // Bits	  12	      5  	3     	5	  7
  // here
  case INSTR_I_TYPE:
    if (num_args == 4) // addi rd, rs1, imm
    {
      rd_num = get_register_number(tokens[1]);
      rs1_num = get_register_number(tokens[2]);
      // The immediate is the last part for I-Type instructions
      // check if Immediate out of range 👿
      if (isNumeric(tokens[3])) // Error Handling 3
      {
        imm = atoi(tokens[3]);
      }
      else
      {
        free(instr_copy);
        return 0;
      }
      if ((imm < -2048) || (imm > 2047)) // EH4
      {
        free(instr_copy);
        return 0;
      }
      if (fmt->funct7 != 0xFFFFFFFF) // if it is shift logic, it should be ranged [0,31]
      {
        if ((imm < 0) || (imm > 31))
        {
          free(instr_copy);
          return 0;
        }
      }
    }
    else if (num_args == 1)
    {
      free(instr_copy);
      return 0x00000073;
    }
    else // lb rd, offset(rs1)
    {
      // The immediate and rs1 are combined for load-type instructions
      rd_num = get_register_number(tokens[1]);
      imm = parse_immediate_and_register(tokens[2], &rs1_num);
      if (Error3 == true)
      {
        Error3 = false; // reset
        free(instr_copy);
        return 0;
      }
      if ((imm < -2048) || (imm > 2047)) // EH4
      {
        free(instr_copy);
        return 0;
      }
    }
    if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
    {
      free(instr_copy);
      return 0;
    }
    rd_num = (uint32_t)rd_num;
    rs1_num = (uint32_t)rs1_num;
    machine_code = get_machine_code(*fmt, rd_num, rs1_num, imm);
    break;
  // S-TYPE	imm[11:5]	rs2	rs1	funct3	imm[4:0]	opcode
  // Bits	  7	        5	  5	  3	      5       	7
  // here                             rd_num
  case INSTR_S_TYPE: // sb rs2, offset(rs1)
    // here the rd_num denotes imm[4:0]
    // check if Immediate out of range 👿
    rs2_num = get_register_number(tokens[1]);
    imm = parse_immediate_and_register(tokens[2], &rs1_num);
    if (Error3 == true)
    {
      Error3 = false; // reset
      free(instr_copy);
      return 0;
    }
    if ((imm < -2048) || (imm > 2047)) // EH4
    {
      free(instr_copy);
      return 0;
    }
    if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
    {
      free(instr_copy);
      return 0;
    }
    if (rs1_num == -1 || rs2_num == -1) // check if it is a bad register👿
    {
      free(instr_copy);
      return 0;
    }
    if ((imm < -4096) || (imm > 4095)) // check if it is a bad imm👿
    {
      free(instr_copy);
      return 0;
    }
    rd_num = imm;
    rd_num = (uint32_t)rd_num;
    rs1_num = (uint32_t)rs1_num;
    rs2_num = (uint32_t)rs2_num;
    machine_code = get_machine_code(*fmt, rd_num, rs1_num, rs2_num);
    break;
  case INSTR_SB_TYPE:
    rs1_num = get_register_number(tokens[1]);
    rs2_num = get_register_number(tokens[2]);
    if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
    {
      free(instr_copy);
      return 0;
    }
    // imm = atoi(tokens[3]); Erroe3
    if (isNumeric(tokens[3])) // Error Handling 3
    {
      imm = atoi(tokens[3]);
    }
    else
    {
      free(instr_copy);
      return 0;
    }
    if (rs1_num == -1 || rs2_num == -1) // check if it is a bad register👿
    {
      free(instr_copy);
      return 0;
    }
    if ((imm < -4096) || (imm > 4095)) // check if it is a bad imm👿
    {
      free(instr_copy);
      return 0;
    }
    rd_num = (uint32_t)imm;
    rs1_num = (uint32_t)rs1_num;
    rs2_num = (uint32_t)rs2_num;
    machine_code = get_machine_code(*fmt, rd_num, rs1_num, rs2_num);
    break;
  case INSTR_U_TYPE:
    rd_num = get_register_number(tokens[1]);
    if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
    {
      free(instr_copy);
      return 0;
    }
    // imm = atoi(tokens[2]); Error 3
    if (isNumeric(tokens[2])) // Error Handling 3
    {
      imm = atoi(tokens[2]);
    }
    else
    {
      free(instr_copy);
      return 0;
    }
    if (rd_num == -1) // check if it is a bad register👿
    {
      free(instr_copy);
      return 0;
    }
    if ((imm < 0) || (imm > 1048575)) // check if it is a bad imm👿
    {
      free(instr_copy);
      return 0;
    }
    rd_num = (uint32_t)rd_num;
    rs1_num = (uint32_t)rs1_num;
    rs2_num = (uint32_t)imm;
    machine_code = get_machine_code(*fmt, rd_num, rs1_num, rs2_num);
    break;
  case INSTR_UJ_TYPE:
    rd_num = get_register_number(tokens[1]);
    if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
    {
      free(instr_copy);
      return 0;
    }
    // imm = atoi(tokens[2]); Error 3
    if (isNumeric(tokens[2])) // Error Handling 3
    {
      imm = atoi(tokens[2]);
    }
    else
    {
      free(instr_copy);
      return 0;
    }
    if (rd_num == -1) // check if it is a bad register👿
    {
      free(instr_copy);
      return 0;
    }
    if ((imm < -1048576) || (imm > 1048575)) // check if it is a bad imm👿
    {
      free(instr_copy);
      return 0;
    }
    rd_num = (uint32_t)rd_num;
    rs1_num = (uint32_t)rs1_num;
    rs2_num = (uint32_t)rs2_num;
    rs2_num = (uint32_t)imm;
    machine_code = get_machine_code(*fmt, rd_num, rs1_num, rs2_num);
    break;
  case INSTR_PSEUDO:
    // Process pseudo-instructions here, potentially reusing logic from the primary types
    if (strcmp(mark, "beqz") == 0)
    {
      // beqz rs1, label => beq rs1, x0, label (where x0 is rs2)
      rs1_num = get_register_number(tokens[1]);
      if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
      {
        free(instr_copy);
        return 0;
      }
      // imm = atoi(tokens[2]); Error 3
      if (isNumeric(tokens[2])) // Error Handling 3
      {
        imm = atoi(tokens[2]);
      }
      else
      {
        free(instr_copy);
        return 0;
      }
      if ((imm < -4096) || (imm > 4095)) // check if it is a bad imm👿
      {
        free(instr_copy);
        return 0;
      }
      uint32_t imm_12 = ((imm & 0x00001000) >> 12) << 31;  // 0000 0000 0000 0000 0001 0000 0000 0000
      uint32_t imm_10_5 = ((imm & 0x000007E0) >> 5) << 25; // 0000 0000 0000 0000 0000 0111 1110 0000
      uint32_t imm_4_1 = ((imm & 0x0000001E) >> 1) << 8;   // 0000 0000 0000 0000 0000 0000 0001 1110
      uint32_t imm_11 = ((imm & 0x00000800) >> 11) << 7;   // 0000 0000 0000 0000 0000 1000 0000 0000
      rs2_num = 0x00 << 20;
      rs1_num = rs1_num << 15;
      uint32_t funct3 = 0x00 << 12;
      machine_code = imm_12 | imm_10_5 | (rs2_num) | (rs1_num) | (funct3) | imm_4_1 | imm_11 | (0x63);
      break;
    }
    else if (strcmp(mark, "bnez") == 0)
    {
      // bnez rs1, label => bne rs1, zero, label
      rs1_num = get_register_number(tokens[1]);
      if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
      {
        free(instr_copy);
        return 0;
      }
      // imm = atoi(tokens[2]); Error 3
      if (isNumeric(tokens[2])) // Error Handling 3
      {
        imm = atoi(tokens[2]);
      }
      else
      {
        free(instr_copy);
        return 0;
      }
      if ((imm < -4096) || (imm > 4095)) // check if it is a bad imm👿
      {
        free(instr_copy);
        return 0;
      }
      uint32_t imm_12 = ((imm & 0x00001000) >> 12) << 31;  // 0000 0000 0000 0000 0001 0000 0000 0000
      uint32_t imm_10_5 = ((imm & 0x000007E0) >> 5) << 25; // 0000 0000 0000 0000 0000 0111 1110 0000
      uint32_t imm_4_1 = ((imm & 0x0000001E) >> 1) << 8;   // 0000 0000 0000 0000 0000 0000 0001 1110
      uint32_t imm_11 = ((imm & 0x00000800) >> 11) << 7;   // 0000 0000 0000 0000 0000 1000 0000 0000
      rs2_num = 0x00 << 20;
      rs1_num = rs1_num << 15;
      uint32_t funct3 = 0x01 << 12;
      machine_code = imm_12 | imm_10_5 | (rs2_num) | (rs1_num) | (funct3) | imm_4_1 | imm_11 | (0x63);
      break;
    }
    else if (strcmp(mark, "j") == 0)
    {
      // j label => jal zero, label
      // imm = atoi(tokens[1]); Error 3
      if (isNumeric(tokens[1])) // Error Handling 3
      {
        imm = atoi(tokens[1]);
      }
      else
      {
        free(instr_copy);
        return 0;
      }
      if ((imm < -1048576) || (imm > 1048575)) // check if it is a bad imm👿
      {
        free(instr_copy);
        return 0;
      }
      uint32_t imm_20 = ((imm & 0x00100000) >> 20) << 31;    // 0000 0000 0001 0000 0000 0000 0000 0000
      uint32_t imm_10_1 = ((imm & 0x000007FE) >> 1) << 21;   // 0000 0000 0000 0000 0000 0111 1111 1110
      uint32_t imm11 = ((imm & 0x00000800) >> 11) << 20;     // 0000 0000 0000 0000 0000 1000 0000 0000
      uint32_t imm_19_12 = ((imm & 0x000FF000) >> 12) << 12; // 0000 0000 0000 1111 1111 0000 0000 0000
      machine_code = imm_20 | imm_10_1 | imm11 | imm_19_12 | (0x00) | (0x6f);
      break;
    }
    else if (strcmp(mark, "jr") == 0)
    {
      // jr rs1 => jalr x0, rs1, 0 (I type)
      rs1_num = get_register_number(tokens[1]);
      if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
      {
        free(instr_copy);
        return 0;
      }
      if ((imm < -2048) || (imm > 2047)) // EH4
      {
        free(instr_copy);
        return 0;
      }
      machine_code = (0x00 << 20) | (rs1_num << 15) | (0x00 << 12) | (0x00 << 7) | (0x67);
      break;
    }
    else if (strcmp(mark, "li") == 0)
    {
      rd_num = get_register_number(tokens[1]);
      if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
      {
        free(instr_copy);
        return 0;
      }
      // imm = atoi(tokens[2]); Error 3
      if (isNumeric(tokens[2])) // Error Handling 3
      {
        imm = atoi(tokens[2]);
      }
      else
      {
        free(instr_copy);
        return 0;
      }
      if ((imm >= -2048) && (imm < 2048))
      {
        // li rd, immediate => addi rd, zero, immediate
        rd_num = (uint32_t)rd_num;
        rs1_num = (uint32_t)rs1_num;
        machine_code = (imm << 20) | (rs1_num << 15) | (0x00 << 12) | (rd_num << 7) | (0x13);
        break;
      }
      else // wether to determin?
      {
        // li rd, immediate => lui rd, imm[31:12]
        //                     addi rd, rd, imm[11:0]
        // uint32_t imm_31_12 = (imm & 0x000FFFFF) << 12; // 0000 0000 0001 1111 1111 1111 1111 1110
        // rd_num = (rd_num) << 7;
        uint32_t imm_31_12 = ((imm >> 12) & 0xFFFFF) << 12;
        LiSecondLine = (imm << 20) | (rd_num << 15) | (0x00 << 12) | (rd_num << 7) | (0x13);
        SecondFlag = 1;
        if (!((imm >> 11) & 1))
        {
          machine_code = imm_31_12 | (rd_num << 7) | (0x37);
        }
        else
        {
          machine_code = (imm_31_12 + 0x00001000) | (rd_num << 7) | (0x37);
        }
        break;
      }
    }
    else if (strcmp(mark, "mv") == 0)
    {
      // mv rd, rs1 => addi rd, rs1, 0
      rd_num = get_register_number(tokens[1]);
      rs1_num = get_register_number(tokens[2]);
      if (rd_num == -1 || rs1_num == -1 || rs2_num == -1) // Error Handling 2: check if it is a bad register 👿
      {
        free(instr_copy);
        return 0;
      }
      if ((imm < -2048) || (imm > 2047)) // EH4
      {
        free(instr_copy);
        return 0;
      }
      machine_code = (0x00 << 20) | (rs1_num << 15) | (0x00 << 12) | (rd_num << 7) | (0x13);
      break;
    }
    else
    {
      machine_code = 0;
    }
    break;
  default:
    break;
  }
  free(instr_copy);
  return machine_code;
}

int assembler(FILE *input_file, FILE *output_file)
{
  char line[256];
  while (fgets(line, sizeof(line), input_file))
  {
    uint32_t machine_code;
    if (strstr(line, "ecall") != NULL)
    {
      // If "ecall" is found, return 0x00000073
      machine_code = 0x00000073;
      goto label;
    }
    machine_code = assemble_instruction(line);
    if ((machine_code != 0) && (SecondFlag == 0))
    {
    label:
      dump_code(output_file, machine_code);
    }
    else if (machine_code != 0 && (SecondFlag != 0))
    {
      dump_code(output_file, machine_code);
      dump_code(output_file, LiSecondLine);
      SecondFlag = 0;
    }
    else
    {
      dump_error_information(output_file);
    }
  }
  return 0;
}
