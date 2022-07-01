#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** void print_file(FILE *, FILE *); */

typedef struct {
  char *name;
  char *value;
} SYMBOLS;

/** {{{ jmp struct */
size_t jmp_size = 8;
SYMBOLS jmp_mcodes[] = {"null", "000", "JGT", "001", "JEQ", "010",
                        "JGE",  "011", "JLT", "100", "JNE", "101",
                        "JLE",  "110", "JMP", "111"};
/** }}} */

/** {{{ comp struct */
size_t comp_size = 28;
SYMBOLS comp_mcodes[] = {
    "0",   "101010", "1",   "111111", "-1",  "001100", "D",   "001100",
    "A",   "110000", "M",   "110000", "!D",  "001101", "!A",  "110001",
    "!M",  "110001", "-D",  "001111", "-A",  "110011", "-M",  "110011",
    "D+1", "011111", "A+1", "110111", "M+1", "110111", "D-1", "001110",
    "A-1", "110010", "M-1", "110010", "D+A", "000010", "D+M", "000010",
    "D-A", "010011", "D-M", "010011", "A-D", "000111", "M-D", "000111",
    "D&A", "000000", "D&M", "000000", "D|A", "010101", "D|M", "010101",
};
/** }}} */

/** {{{ dest struct */
size_t dest_size = 8;
SYMBOLS dest_mcodes[] = {
    "null", "000", "M",  "001", "D",  "010", "DM",  "011",
    "A",    "100", "AM", "101", "AD", "110", "ADM", "111",
};
/* }}} */

/** {{{ builtins struct */
size_t builtinS_size = 22;
SYMBOLS builtinS[] = {
    "R0",   "0",  "R1",     "1",     "R2",  "2",    "R3",  "3",  "R4",   "4",
    "R5",   "5",  "R6",     "6",     "R7",  "7",    "R8",  "8",  "R9",   "9",
    "R10",  "10", "R11",    "11",    "R12", "12",   "R13", "13", "R14",  "14",
    "R15",  "15", "SP",     "0",     "LCL", "1",    "ARG", "2",  "THIS", "3",
    "THAT", "4",  "SCREEN", "16384", "KBD", "24576"};
/** }}} */

/** TODO incorporate symbol table and account for label destinations*/
/** This is where a first pass comes in */

#define MAX_LINE 256
#define BIT_RANGE 16
#define COMP_RANGE 7
#define DEST_RANGE 3
#define JMP_RANGE 3

#define MAX_ADDR (int)pow(2, BIT_RANGE - 1)

/** {{{ comp, dest, and jmp func */
void comp(char *nstr, char *symbol) {
  for (int i = 0; i < comp_size; i++) {
    if (strcmp((comp_mcodes + i)->name, symbol) == 0) {
      strcat(nstr, (strchr(symbol, 'M') != NULL) ? "1" : "0");
      strcat(nstr, (comp_mcodes + i)->value);
      break;
    }
  }
}

void dest(char *nstr, char *symbol) {
  for (int i = 0; i < dest_size; i++) {
    if (strcmp((dest_mcodes + i)->name, symbol) == 0) {
      strcat(nstr, (dest_mcodes + i)->value);
      break;
    }
  }
}

void jmp(char *nstr, char *symbol) {
  for (int i = 0; i < jmp_size; i++) {
    if (strcmp((jmp_mcodes + i)->name, symbol) == 0) {
      strcat(nstr, (jmp_mcodes + i)->value);
    }
  }
}
/** }}} */

/** {{{ c_comp_dest  */
char *c_comp_dest(char *nstr, char *line, char *delim_ptr) {
  strcpy(nstr, "111");

  int comp_start = labs(line - delim_ptr);

  /** finding comp code */
  char *comp_symbol = line + comp_start + 1;
  comp(nstr, comp_symbol);

  /** finding dest code */
  *(line + comp_start) = '\0';
  dest(nstr, line);

  /** add no jump */
  strcat(nstr, "000\n\0");

  return nstr;
}
// }}}

/** {{{ c_comp_jump  */
char *c_comp_jump(char *nstr, char *line, char *delim_ptr) {
  int jmp_start = labs(line - delim_ptr);

  strcpy(nstr, "111");

  /** finding comp code */
  *(line + jmp_start) = '\0';
  comp(nstr, line);
  /** printf("%s\n", nstr); */

  /** not storing destination yet */
  strcat(nstr, "000");

  /** finding jmp code */
  char *jmp_symbol = line + jmp_start + 1;
  jmp(nstr, jmp_symbol);

  strcat(nstr, "\n");

  return nstr;
}
/** }}} */

/** {{{ c_instruction */
int c_instruction(char *line, FILE *out) {
  char *c;
  char *nstr = (char *)malloc(BIT_RANGE);

  if ((c = strchr(line, '=')) != NULL) {
    c_comp_dest(nstr, line, c);

  } else {
    c = strchr(line, ';');
    c_comp_jump(nstr, line, c);
  }
  if (fputs(nstr, out) == EOF) {
    printf("Error writing symbols to out file");
    return -1;
  }
  free(nstr);
  return 0;
}
/** }}} */

/** {{{ symbol instruction fun */
int symbol(char *line, FILE *out) {
  char *tmp = line, nstr[BIT_RANGE];

  if (isdigit(*++line)) {
    int num;
    num = atoi(line);

    int i = 0;
    while (i < BIT_RANGE) {
      /** adding '0' + int for type cast to char  */
      nstr[i] = (!!((num << i) & MAX_ADDR)) + '0';
      i++;
    }
    nstr[i++] = '\n';
    nstr[i] = '\0';
  }

  if (fputs(nstr, out) == EOF) {
    printf("Error writing symbols to out file");
    return -1;
  }

  return 0;
}
/** }}} */

typedef enum { A_NSTR, C_NSTR, L_NSTR } INST_TYPE;

#define NSTR_TYPE(c) \
  ((c == '@') || (c == '(') ? (c == '@') ? (A_NSTR) : (L_NSTR) : (C_NSTR))

int normalize(char *line) {
  int c;
  if ((c = *line) == '/' || c == '\0' || c == '\n' || c == '\r') {
    return -1;
  }
  char *cr;
  if ((cr = strchr(line, '\r')) != NULL) {
    *(line + labs(line - cr)) = '\0';
  }
  return c;
}

struct Counter {
  int linenum;
  int symbnum;
};

/** {{{ build_symbol_table */
int build_symbol_table(char *line, SYMBOLS *table, struct Counter *pos) {
  ++pos->linenum;
  if (*line != '(') {
    return 1;
  } else {
    ++pos->symbnum;
    char *column = ++line;
    while (*column != ')') {
      ++column;
    }
    *column = '\0';
  }
  char *name = (char *)malloc(strlen(line));
  char *value = (char *)malloc(sizeof(char));

  name = line;
  sprintf(value, "%d", pos->linenum);

  (table + pos->symbnum)->name = name;
  (table + pos->symbnum)->value = value;
  printf("%s\n", (table + pos->symbnum)->name);

  return 0;
}
/** }}} */

#define TABLE_SIZE 100

/** {{{ parser */
int parser(char *infile, char *outfile) {
  FILE *in, *in1, *out;
  in = fopen(infile, "r");
  in1 = fopen(infile, "r");
  out = fopen(outfile, "w");

  int c;
  char line[MAX_LINE];
  /** cannot initialize a struct* with values like char * */
  struct Counter pos = {0, 0};
  int size_mult = 1;
  SYMBOLS *table = (SYMBOLS *)malloc(TABLE_SIZE);

  while (fgets(line, MAX_LINE, in) != NULL) {
    if ((c = normalize(line)) == -1) {
      continue;
    }
    build_symbol_table(line, table, &pos);

    if (pos.symbnum == TABLE_SIZE * size_mult) {
      if ((realloc(table, TABLE_SIZE * ++size_mult) == NULL)) {
        printf("error reallocating space to symbol table");
        return -1;
      }
    }
  }

  /** for (int i = 0; i < 100; i++) { */
  /**   printf("%s\n", (table+i)->name); */
  /**      */
  /** } */

  while (fgets(line, MAX_LINE, in1) != NULL) {
    if ((c = normalize(line)) == -1) {
      continue;
    }

    INST_TYPE _type = NSTR_TYPE(c);

    continue;

    if (_type == A_NSTR || _type == L_NSTR) {
      symbol(line, out);
    } else if (_type == C_NSTR) {
      c_instruction(line, out);
    } else {
      continue;
    }
  }
  fclose(in);
  fclose(in1);
  fclose(out);
  return 0;
}
/** }}} */

/** {{{ main */
int main(int argc, char *argv[]) {
  char *infile, *outfile;

  if (argc == 2) {
    infile = argv[1];
    outfile = "Prog.hack";
  } else if (argc == 3) {
    infile = argv[1];
    outfile = argv[2];
  } else {
    printf("wrong number of arguments supplied.");
    return -1;
  }

  if (parser(argv[1], outfile) == -1) {
    printf("%s", "Error parsing");
    return -1;
  }

  return 0;
}
/** }}} */
