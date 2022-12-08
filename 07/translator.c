#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum command_type {
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL
};

struct file {
    char *fn;           // input filename
    char *bn;           // input file basename
    FILE *fp;           // input file pointer
    char *ofn;          // output file name
    FILE *ofp;          // output file pointer 
};

struct tok {
    char *buf;                  // full line read from file
    char *cur;                  // current token
    char *memseg;               // memory segment
    char *ind;                  // index (third argument) of push/pop
    int con;                    // set to 1 if memseg is a constant
    int line_ind;               // index of character being read
    enum command_type command;  // type of command
    struct file *file;          // input file
};

int free_file(struct file *, int);

int free_tok(struct tok *, int, int);

#define DO_NOT_FREE_FILE 0

#define FREE_FILE 1

#define MAX_LINE 256

#define IS_FILENAME_START_VALID(c) (\
            (c >= 'A' && c <= 'Z'))

#define IS_LINE_COMMENT_OR_EMPTY(c) (\
            (c == '/' || c == '\n' || c == '\r'))

#define IS_CHARACTER_ALPHANUMERIC(c) (\
            (c >= '!' && c <= '~'))

int fname_check(char *fname) {
    if (!IS_FILENAME_START_VALID(fname[0])) return -1;
    if (strcmp(".vm", fname+strlen(fname)-3) != 0) return -1; 
    return 0;
}

void determine_memseg_index(struct tok *tok) {
    size_t buf_size = strlen(tok->buf)+1;
    char *word = calloc(buf_size + 1, sizeof(char));
    int i = 0;

    for (;;) {
        if (!IS_CHARACTER_ALPHANUMERIC(tok->buf[tok->line_ind])) {
            ++tok->line_ind;
            continue;
        }
        break;
    }
    for(;;) {
        if (!IS_CHARACTER_ALPHANUMERIC(tok->buf[tok->line_ind])) {
            break;
        }
        word[i++] = tok->buf[tok->line_ind++];
    }
    free(tok->cur);
    tok->cur = word;
    tok->ind = word;
}

void determine_memseg(struct tok *tok) {
    size_t buf_size = strlen(tok->buf)+1;
    char *word = calloc(buf_size + 1, sizeof(char));
    int i = 0;

    for (;;) {
        if (!IS_CHARACTER_ALPHANUMERIC(tok->buf[tok->line_ind])) {
            ++tok->line_ind;
            continue;
        }
        break;
    }
    for(;;) {
        if (!IS_CHARACTER_ALPHANUMERIC(tok->buf[tok->line_ind])) {
            break;
        }
        word[i++] = tok->buf[tok->line_ind++];
    }
    free(tok->cur);
    tok->cur = word;

    if (strcmp("constant", tok->cur) == 0) {
        determine_memseg_index(tok);
        char *memseg = calloc(5, sizeof(char));
        strcpy(memseg, "@");
        strcat(memseg, tok->ind);
        tok->con = 1;
        tok->memseg = memseg;
    }
    else if (strcmp("local", tok->cur) == 0) {
        char *memseg = calloc(5, sizeof(char));
        strcpy(memseg, "@LCL");
        tok->memseg = memseg;
        determine_memseg_index(tok);

    }
    else if (strcmp("argument", tok->cur) == 0) {
        char *memseg = calloc(5, sizeof(char));
        strcpy(memseg, "@ARG");
        tok->memseg = memseg;
        determine_memseg_index(tok);
    }
    else if (strcmp("this", tok->cur) == 0) {
        char *memseg = calloc(6, sizeof(char));
        strcpy(memseg, "@THIS");
        tok->memseg = memseg;
        determine_memseg_index(tok);
    }
    else if (strcmp("that", tok->cur) == 0) {
        char *memseg = calloc(6, sizeof(char));
        strcpy(memseg, "@THAT");
        tok->memseg = memseg;
        determine_memseg_index(tok);
    }
    else if (strcmp("static", tok->cur) == 0) {
        char *memseg = calloc(strlen(tok->file->bn)+2, sizeof(char));
        strcpy(memseg, strcat("@", tok->file->bn));
        tok->memseg = memseg;
        determine_memseg_index(tok);
    }
    else {
        fprintf(stderr, "%s is not a valid memory segment\n", tok->cur);
        free_tok(tok, FREE_FILE, 1);
    }
}

int determine_command_type(struct tok *tok) {
    size_t buf_size = strlen(tok->buf)+1;
    char *word = calloc(buf_size + 1, sizeof(char));
    int i;
    for (i = 0; i <= (int) buf_size +1; i++) {
        if (!IS_CHARACTER_ALPHANUMERIC(tok->buf[i])) {
            tok->line_ind = i;
            break;
        }
        tok->line_ind = i;
        word[i] = tok->buf[i];
    }
    word[++i] = '\0';
    tok->cur = word;

    enum command_type t;
    if (strcmp(tok->cur, "push") == 0) {
        t = C_PUSH;
        determine_memseg(tok);
    } 
    else if (strcmp(tok->cur, "pop") == 0) {
        t = C_POP;
        determine_memseg(tok);
    }
    else if (strcmp(tok->cur, "add") == 0) {
        t = C_ARITHMETIC;
    }
    else if (strcmp(tok->cur, "sub") == 0) {
        t = C_ARITHMETIC;
    }
    else if (strcmp(tok->cur, "neg") == 0) {
        t = C_ARITHMETIC;
    }
    else {
        fprintf(stderr, "token \"%s\" not recognized\n", tok->cur);
        return -1;
    }
    tok->command = t;
    return 0;
}

#define CONST_ACCESS(file, seg) (\
            fprintf(file, "%s\nD=A\n", seg))

#define SEG_ACCESS(file, seg, ind) (\
            fprintf(file, "%s\nA=M+%s\nD=M\n", seg, ind))

#define INCR_STACK "@SP\nM=M+1\n"

#define STACK_MEM "@SP\nA=M\nM=D"

#define WRITE_ASM_ARITHMETIC(file, sign) (\
            fprintf(file, "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=M%cD\n%s", sign, INCR_STACK))

#define WRITE_ASM_PUSH(file) (\
            fprintf(file, "@SP\nA=M\nM=D\n%s", INCR_STACK)) 

#define WRITE_ASM_POP(file, seg, ind) (\
            fprintf(file,"@SP\nM=M-1\nA=M\nD=M\n%s\nA=M+%s\nM=D\n", seg, ind))

int code_writer(struct tok *tok) {
    FILE *ofp = tok->file->ofp;
    switch(tok->command) {
        case C_ARITHMETIC:
            char sign = !strcmp(tok->cur, "add") ? '+': '-';
            WRITE_ASM_ARITHMETIC(ofp, sign);
            break;

        case C_PUSH:
            tok->con == 1
                ? CONST_ACCESS(ofp, tok->memseg)
                : SEG_ACCESS(ofp, tok->memseg, tok->ind);
            WRITE_ASM_PUSH(ofp);
            break;

        case C_POP:
            WRITE_ASM_POP(ofp, tok->memseg, tok->ind);
            break;

        default:
            fprintf(stderr, "%s\n", "Failed to write ASM to file");
            return -1;
    }
    return 0;
}

int read_lines(struct file *file, struct tok *tok) {
    char line[MAX_LINE];
    tok->file = file;
    while (fgets(line, MAX_LINE, tok->file->fp) != NULL) {
        if (IS_LINE_COMMENT_OR_EMPTY(line[0]))
            continue;
        tok->buf = (char *) &line;

        if (determine_command_type(tok) == -1) {
            return -1;
        }
        if (code_writer(tok) == -1) {
            return -1;
        };
    }
    return 0;
}

int open_file(struct file *file, char *fname) {
    file->fn = strdup(fname);
    file->bn = strndup(fname, strlen(fname)-3);
    file->ofn = strdup(file->bn);
    strcat(file->ofn, ".asm");

    if (file == NULL || file->fn == NULL || file->bn == NULL || file->bn == NULL) {
        fprintf(stderr, "error allocating space for file name");
        return -1;
    }
    if ((file->fp = fopen(file->fn, "r")) == NULL) {
        fprintf(stderr, "%s does not exist\n", file->fn);
        return -1;
    }
    if ((file->ofp = fopen(file->ofn, "w")) == NULL) {
        fprintf(stderr, "could not create %s\n", file->ofn);
        return -1;
    };
    return 0;
}


int main(int argc, char *argv[]) {
    // check that filename matches guidelines
    if (argc != 2) {
        fprintf(stderr, "No file provided\n");
        exit(1);
    }
    if (fname_check(argv[1]) == -1) {
        fprintf(stderr, "input file should start with an upper case character and end in '.vm'\n");
        exit(1);
    }

    struct tok *tok = malloc(sizeof *tok);
    struct file *file = malloc(sizeof *file);

    // build file struct to be passed around
    if (open_file(file, argv[1]) == -1) {
        free_file(file, -1);
        free_tok(tok, DO_NOT_FREE_FILE, 1);
    }
    if (read_lines(file, tok) == -1) {
        free_tok(tok, FREE_FILE, 1);
    }
    free_tok(tok, FREE_FILE, 0);
    return 0;
}

int free_tok(struct tok *tok, int ff, int status) {
    if (ff == FREE_FILE)
        free_file(tok->file, -1);
    if (tok->cur != NULL)
        free(tok->cur);
    if (tok->memseg != NULL)
        free(tok->memseg);
    if (tok->ind != NULL)
        free(tok->ind);
    free(tok);
    exit(status);
}

int free_file(struct file *file, int status) {
    if (file->fn != NULL)
       free(file->fn);
    if (file->bn != NULL)
       free(file->bn);
    if (file->ofn != NULL)
       free(file->ofn);
    if (file->fp != NULL)
        fclose(file->fp);
    if (file->ofp != NULL)
        fclose(file->ofp);
    free(file);
    if (status == -1)
        return 0;
    exit(status);
}