#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_QUAD 2048
#define MAX_VAR  128
#define MAX_STACK 512

void error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Compile Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

typedef struct {
    char op[16], arg1[32], arg2[32], result[32];
} Quad;

Quad quads[MAX_QUAD];
int quad_p = 0;

void emit(const char* op, const char* a1, const char* a2, const char* res) {
    if (quad_p >= MAX_QUAD) error("Too many quadruples!");
    strncpy(quads[quad_p].op, op, 15);
    strncpy(quads[quad_p].arg1, a1, 31);
    strncpy(quads[quad_p].arg2, a2, 31);
    strncpy(quads[quad_p].result, res, 31);
    printf("[%03d] %-10s %-10s %-10s %-10s\n", quad_p, op, a1, a2, res);
    quad_p++;
}

typedef enum {
    TK_FUNC, TK_RETURN, TK_IF, TK_WHILE, TK_ID, TK_NUM, 
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE, TK_COMMA, TK_SEMICOLON,
    TK_ASSIGN, TK_PLUS, TK_MINUS, TK_MUL, TK_DIV, TK_EQ, TK_LT, TK_GT, TK_EOF
} TokenType;

typedef struct { TokenType type; char text[32]; } Token;
Token cur_tk;
const char *src_ptr;

void next_tk() {
    while (isspace(*src_ptr)) src_ptr++;
    
    // 處理跳過註解
    if (*src_ptr == '/' && *(src_ptr+1) == '/') {
        while (*src_ptr && *src_ptr != '\n') src_ptr++;
        next_tk(); return;
    }

    if (!*src_ptr) { cur_tk.type = TK_EOF; return; }

    if (isdigit(*src_ptr)) {
        int i = 0;
        while (isdigit(*src_ptr)) cur_tk.text[i++] = *src_ptr++;
        cur_tk.text[i] = '\0';
        cur_tk.type = TK_NUM;
    } else if (isalpha(*src_ptr) || *src_ptr == '_') {
        int i = 0;
        while (isalnum(*src_ptr) || *src_ptr == '_') cur_tk.text[i++] = *src_ptr++;
        cur_tk.text[i] = '\0';
        if (!strcmp(cur_tk.text, "func")) cur_tk.type = TK_FUNC;
        else if (!strcmp(cur_tk.text, "return")) cur_tk.type = TK_RETURN;
        else if (!strcmp(cur_tk.text, "if")) cur_tk.type = TK_IF;
        else if (!strcmp(cur_tk.text, "while")) cur_tk.type = TK_WHILE;
        else cur_tk.type = TK_ID;
    } else {
        char c = *src_ptr++;
        cur_tk.text[0] = c; cur_tk.text[1] = '\0';
        switch (c) {
            case '(': cur_tk.type = TK_LPAREN; break;
            case ')': cur_tk.type = TK_RPAREN; break;
            case '{': cur_tk.type = TK_LBRACE; break;
            case '}': cur_tk.type = TK_RBRACE; break;
            case '+': cur_tk.type = TK_PLUS; break;
            case '-': cur_tk.type = TK_MINUS; break;
            case '*': cur_tk.type = TK_MUL; break;
            case '/': cur_tk.type = TK_DIV; break;
            case ',': cur_tk.type = TK_COMMA; break;
            case ';': cur_tk.type = TK_SEMICOLON; break;
            case '<': cur_tk.type = TK_LT; break;
            case '>': cur_tk.type = TK_GT; break;
            case '=':
                if (*src_ptr == '=') { src_ptr++; cur_tk.type = TK_EQ; strcpy(cur_tk.text, "=="); }
                else cur_tk.type = TK_ASSIGN;
                break;
            default: error("Unknown character: %c", c);
        }
    }
}

int temp_count = 0;
void gen_temp(char *s) { sprintf(s, "t%d", ++temp_count); }

void expect(TokenType type) {
    if (cur_tk.type != type) error("Unexpected token: %s", cur_tk.text);
    next_tk();
}

void expr(char *res);

void factor(char *res) {
    if (cur_tk.type == TK_NUM) {
        gen_temp(res); emit("IMM", cur_tk.text, "-", res); next_tk();
    } else if (cur_tk.type == TK_ID) {
        char name[32]; strcpy(name, cur_tk.text); next_tk();
        if (cur_tk.type == TK_LPAREN) { // 函數調用
            next_tk(); int n = 0;
            while (cur_tk.type != TK_RPAREN) {
                char a[32]; expr(a); emit("PARAM", a, "-", "-"); n++;
                if (cur_tk.type == TK_COMMA) next_tk();
            }
            next_tk(); gen_temp(res); 
            char buf[10]; sprintf(buf, "%d", n); emit("CALL", name, buf, res);
        } else strcpy(res, name);
    } else if (cur_tk.type == TK_LPAREN) {
        next_tk(); expr(res); expect(TK_RPAREN);
    }
}

void term(char *res) {
    char l[32], r[32], t[32]; factor(l);
    while (cur_tk.type == TK_MUL || cur_tk.type == TK_DIV) {
        char op[10]; strcpy(op, cur_tk.type == TK_MUL ? "MUL" : "DIV");
        next_tk(); factor(r); gen_temp(t); emit(op, l, r, t); strcpy(l, t);
    }
    strcpy(res, l);
}

void arith(char *res) {
    char l[32], r[32], t[32]; term(l);
    while (cur_tk.type == TK_PLUS || cur_tk.type == TK_MINUS) {
        char op[10]; strcpy(op, cur_tk.type == TK_PLUS ? "ADD" : "SUB");
        next_tk(); term(r); gen_temp(t); emit(op, l, r, t); strcpy(l, t);
    }
    strcpy(res, l);
}

void expr(char *res) {
    char l[32], r[32], t[32]; arith(l);
    if (cur_tk.type == TK_EQ || cur_tk.type == TK_LT || cur_tk.type == TK_GT) {
        char op[10];
        if (cur_tk.type == TK_EQ) strcpy(op, "CMP_EQ");
        else if (cur_tk.type == TK_LT) strcpy(op, "CMP_LT");
        else strcpy(op, "CMP_GT");
        next_tk(); arith(r); gen_temp(t); emit(op, l, r, t); strcpy(res, t);
    } else strcpy(res, l);
}

void stmt() {
    if (cur_tk.type == TK_IF) {
        next_tk(); expect(TK_LPAREN); char c[32]; expr(c); expect(TK_RPAREN);
        expect(TK_LBRACE);
        int jf = quad_p; emit("JMP_F", c, "-", "?");
        while (cur_tk.type != TK_RBRACE) stmt();
        expect(TK_RBRACE);
        sprintf(quads[jf].result, "%d", quad_p);
    } else if (cur_tk.type == TK_WHILE) {
        int start = quad_p;
        next_tk(); expect(TK_LPAREN); char c[32]; expr(c); expect(TK_RPAREN);
        expect(TK_LBRACE);
        int jf = quad_p; emit("JMP_F", c, "-", "?");
        while (cur_tk.type != TK_RBRACE) stmt();
        expect(TK_RBRACE);
        char buf[10]; sprintf(buf, "%d", start); emit("JMP", "-", "-", buf);
        sprintf(quads[jf].result, "%d", quad_p);
    } else if (cur_tk.type == TK_ID) {
        char name[32]; strcpy(name, cur_tk.text); next_tk();
        expect(TK_ASSIGN); char r[32]; expr(r); emit("STORE", r, "-", name);
        expect(TK_SEMICOLON);
    } else if (cur_tk.type == TK_RETURN) {
        next_tk(); char r[32]; expr(r); emit("RET_VAL", r, "-", "-");
        expect(TK_SEMICOLON);
    }
}


int main(int argc, char **argv) {
    if (argc < 2) { printf("Usage: %s <file>\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1], "r");
    if (!f) return 1;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1); fread(buf, 1, sz, f); buf[sz] = 0; fclose(f);

    src_ptr = buf;
    next_tk();
    while (cur_tk.type != TK_EOF) {
        if (cur_tk.type == TK_FUNC) {
            next_tk(); char name[32]; strcpy(name, cur_tk.text);
            emit("FUNC_BEG", name, "-", "-");
            next_tk(); expect(TK_LPAREN);
            while (cur_tk.type == TK_ID) {
                emit("FORMAL", cur_tk.text, "-", "-"); next_tk();
                if (cur_tk.type == TK_COMMA) next_tk();
            }
            expect(TK_RPAREN); expect(TK_LBRACE);
            while (cur_tk.type != TK_RBRACE) stmt();
            expect(TK_RBRACE); emit("FUNC_END", name, "-", "-");
        } else stmt();
    }
    return 0;
}
