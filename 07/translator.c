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
    int line_ind;               // index of character being read
    enum command_type *command; // type of command
    struct file *file;          // input file
};

int close_file(struct file *);

int free_tok(struct tok *, int);

#define DO_NOT_CLOSE_FILE 0

#define CLOSE_FILE 1

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

    int c;
    if (strcmp("constant", tok->cur) == 0) {
        determine_memseg_index(tok);
        char *memseg = calloc(5, sizeof(char));
        strcpy(memseg, "@");
        strcat(memseg, tok->ind);
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
        free_tok(tok, CLOSE_FILE);
    }
}

void determine_command_type(struct tok *tok) {
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
        free_tok(tok, CLOSE_FILE);
    }
    tok->command = &t;
}

#define WRITE_ASM_ARITHMETIC(file, sign) (\
            fprintf(file, "@SP\nM=M+1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=M%cD\n@SP\nM=M+1\n", sign))

#define WRITE_ASM_PUSH(file, seg, ind) (\
            fprintf(file, "%s\nA=M+%s\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", seg, ind)) 

#define WRITE_ASM_POP(seg, ind) (\
            fprintf(file,"@SP\nM=M-1\nA=M\nD=M\n%s\nA=M+%s\nM=D\n", seg, ind))

void code_writer(struct tok *tok) {
    FILE *ofp = tok->file->ofp;
    switch(*tok->command) {
        case C_ARITHMETIC:
            char sign = !strcmp(tok->cur, "add") ? '+': '-';
            WRITE_ASM_ARITHMETIC(ofp, sign);
            break;

        case C_PUSH:
            WRITE_ASM_PUSH(ofp, tok->memseg, tok->ind);
            break;

        case C_POP:
            WRITE_ASM_PUSH(ofp, tok->memseg, tok->ind);
            break;

        default:
            fprintf(stderr, "%s\n", "Failed to write ASM to file");
            free_tok(tok, CLOSE_FILE);
    }
}

int read_lines(struct file *file) {
    char line[MAX_LINE];
    struct tok *new_tok = malloc(sizeof *new_tok);
    new_tok->file = file;
    while (fgets(line, MAX_LINE, new_tok->file->fp) != NULL) {
        if (IS_LINE_COMMENT_OR_EMPTY(line[0]))
            continue;
        new_tok->buf = (char *) &line;
        determine_command_type(new_tok);
        code_writer(new_tok);
    }
}

struct file *open_file(char *fname) {
    char *fn, *bn, *ofn;
    FILE *fp, *ofp;
    struct file *file;

    file = malloc(sizeof *file);
    fn = calloc(strlen(fname)+1, sizeof(char));
    bn = calloc(strlen(fname)+1, sizeof(char));
    ofn = calloc(strlen(fname)+1, sizeof(char));

    strcpy(fn, fname);
    strcpy(bn, fname);
    bn[strlen(bn)-3] = '\0';
    file->fn = fn;
    file->bn = bn;

    if ((fp = fopen(file->fn, "r")) == NULL) {
        fprintf(stderr, "%s does not exist\n", file->fn);
        close_file(file);
    }
    file->fp = fp;
    strcpy(ofn, file->bn);
    strcat(ofn, ".asm");
    file->ofn = ofn;
    printf("%s\n", file->ofn);
    ofp = fopen(file->ofn, "w");
    file->ofp = ofp;
    return file;
}


int main(int argc, char *argv[]) {
    struct file *file;
    // check that filename matches guidelines
    if (argc != 2) exit(1);
    if (fname_check(argv[1]) == -1) exit(1);
    // build file struct to be passed around

    file = open_file(argv[1]);
    int result = read_lines(file);

    if (result == -1) {
        fprintf(stderr, "%s\n", "error in translation");
        close_file(file);
    }
    close_file(file);
    return 0;
}

int free_tok(struct tok *tok, int closef) {
    if (closef == CLOSE_FILE) {
        close_file(tok->file);
    }
    if (tok->cur != NULL)
        free(tok->cur);
    if (tok->buf != NULL)
        free(tok->buf);
    if (tok->command != NULL)
        free(tok->command);
    free(tok);
    exit(1);

}
int close_file(struct file *file) {
    if (file->fp != NULL)
       fclose(file->fp);
    free(file->fn);
    free(file);
    exit(0);
}