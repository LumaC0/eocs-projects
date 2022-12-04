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
    int ind;                    // current index being read
    enum command_type *command; // type of command
    struct file *file;          // input file
};

int close_file(struct file *);

int free_tok(struct tok *, int);

#define DO_NOT_CLOSE_FILE 0

#define CLOSE_FILE 1

#define MAX_LINE 256

#define is_filename_start_valid(c) (\
            (c >= 'A' && c <= 'Z'))

#define is_line_comment_or_empty(c) (\
            (c == '/' || c == '\n' || c == '\r'))

#define is_character_alphanumeric(c) (\
            (c >= '!' && c <= '~'))

int fname_check(char *fname) {
    if (!is_filename_start_valid(fname[0])) return -1;
    if (strcmp(".vm", fname+strlen(fname)-3) != 0) return -1; 
    return 0;
}

char *determine_memseg(struct tok *tok) {
    size_t buf_size = strlen(tok->buf)+1;
    char *word = calloc(buf_size + 1, sizeof(char));
    int i = 0;

    for (;;) {
        if (!is_character_alphanumeric(tok->buf[tok->ind])) {
            ++tok->ind;
            continue;
        }
        break;
    }
    for(;;) {
        if (!is_character_alphanumeric(tok->buf[tok->ind])) {
            break;
        }
        word[i++] = tok->buf[tok->ind++];
    }
    free(tok->cur);
    tok->cur = word;

    if (strcmp("constant", tok->cur)) {
        tok->memseg = NULL;
    }
    else if (strcmp("local", tok->cur)) {
        char *memseg = calloc(5, sizeof(char));
        strcpy(memseg, "@LCL");
        tok->memseg = memseg;
    }
    else if (strcmp("argument", tok->cur)) {
        char *memseg = calloc(5, sizeof(char));
        strcpy(memseg, "@ARG");
        tok->memseg = memseg;
    }
    else if (strcmp("this", tok->cur)) {
        char *memseg = calloc(6, sizeof(char));
        strcpy(memseg, "@THIS");
        tok->memseg = memseg;
    }
    else if (strcmp("that", tok->cur)) {
        char *memseg = calloc(6, sizeof(char));
        strcpy(memseg, "@THAT");
        tok->memseg = memseg;
    }
    else if (strcmp("static", tok->cur)) {
        char *memseg = calloc(strlen(tok->file->bn)+2, sizeof(char));
        strcpy(memseg, strcat("@", tok->file->bn));
        tok->memseg = memseg;
    }
    printf("%s\n", tok->memseg);
}

void determine_command_type(struct tok *tok) {
    size_t buf_size = strlen(tok->buf)+1;
    char *word = calloc(buf_size + 1, sizeof(char));
    int i;
    for (i = 0; i <= (int) buf_size +1; i++) {
        if (!is_character_alphanumeric(tok->buf[i])) {
            tok->ind = i;
            break;
        }
        tok->ind = i;
        word[i] = tok->buf[i];
    }
    word[++i] = '\0';
    tok->cur = word;

    enum command_type t;
    char *memseg;
    if (strcmp(tok->cur, "push") == 0) {
        t = C_PUSH;
        memseg = determine_memseg(tok);
    } 
    else if (strcmp(tok->cur, "pop") == 0) {
        t = C_POP;
        memseg = determine_memseg(tok);
    }
    else if (strcmp(tok->cur, "add") == 0) {
        t = C_ARITHMETIC;
        memseg = NULL;
    }
    else if (strcmp(tok->cur, "sub") == 0) {
        t = C_ARITHMETIC;
        memseg = NULL;
    }
    else {
        fprintf(stderr, "token \"%s\" not recognized\n", tok->cur);
        free_tok(tok, CLOSE_FILE);
    }
    tok->command = &t;
    tok->memseg = memseg;
}

void code_writer(struct tok *tok) {
    // if (tok->command == C_
    printf("inside code writer\n");

}

int read_lines(struct file *file) {
    char line[MAX_LINE];
    struct tok *new_tok = malloc(sizeof *new_tok);
    new_tok->file = file;
    while (fgets(line, MAX_LINE, new_tok->file->fp) != NULL) {
        if (is_line_comment_or_empty(line[0]))
            continue;
        new_tok->buf = (char *) &line;
        determine_command_type(new_tok);
        code_writer(new_tok);
    }

    return 0;
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
    strcpy(ofn, file->bn);
    strcat(ofn, ".ast");
    file->ofn = ofn;
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