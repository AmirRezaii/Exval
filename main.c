#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum Op {
    PLUS,
    MINUS,
    SLASH,
    STAR
};

typedef struct {
    char text[256];
    size_t cursor;
} Lexer;

typedef struct {
    char *file_path;
    int row;
    int col;
} Loc;


typedef enum {
    OPEN_PAREN  = 1,
    CLOSE_PAREN = 1 << 1,
    NUMBER      = 1 << 2,
    OPERATOR    = 1 << 3,
    EOL         = 1 << 4,
    EOFF        = 1 << 5,
    SYMBOL      = 1 << 6,
    STRING      = 1 << 7,
} TokenType;

#define NUM_OF_TOKENS 8

const char *token_type_to_str(TokenType type) {
    assert(NUM_OF_TOKENS == 8  && "Non exhaustive use of token types");
    switch (type) {
        case OPEN_PAREN:
            return "(";
        case CLOSE_PAREN:
            return ")";
        case NUMBER:
            return "number";
        case OPERATOR:
            return "operator";
        case EOL:
            return "end of line";
        case EOFF:
            return "end of file";
        case SYMBOL:
            return "symbol";
        case STRING:
            return "string";
        default:
            assert(false && "Not a valid token type");
    }
}
char *token_types_to_str(TokenType types) {
    char *res = malloc(256);

    for (int i = 0; i < NUM_OF_TOKENS; i++) {
        const char *text = token_type_to_str(1 << i);
        if (i != 0) types >>= 1;
        if (types % 2 == 1) {
            strcat(res, text);
            if ((types >> 1) != 0) {
                strcat(res, " or ");
            }
        }
    }

    return res;
}

typedef struct {
    Loc loc;
    TokenType type;
    char text[256];
} Token;

typedef struct Expr Expr;

typedef union {
    Token tok;
    Expr *expr;
} Con;

struct Expr {
    Con left;
    Con right;
};

void strip_l(Lexer *lex) {
    while (isspace(lex->text[lex->cursor])) {
        lex->cursor++;
    }
}


enum Op char_to_operator(char c) {
    switch (c) {
        case '+':
            return PLUS;
        case '-':
            return MINUS;
        case '*':
            return STAR;
        case '/':
            return SLASH;
        default:
            assert(false && "Not a valid operator");
    }
}

Token next_symbol(Lexer *lex) {
    strip_l(lex);

    Token tok = {0};

    size_t start = lex->cursor;

    if (isdigit(lex->text[lex->cursor])) {
        while (isdigit(lex->text[lex->cursor])) {
            lex->cursor++;
        }

        strncpy(tok.text, lex->text + start, lex->cursor - start);
        tok.type = NUMBER;
        tok.loc = (Loc) {
            .row = lex->cursor
        };
    } else if (isalpha(lex->text[lex->cursor]) || lex->text[lex->cursor] == '_') {
        while (isalnum(lex->text[lex->cursor]) || lex->text[lex->cursor] == '_') {
            lex->cursor++;
        }

        strncpy(tok.text, lex->text + start, lex->cursor - start);
        tok.type = SYMBOL;
        tok.loc = (Loc) {
            .row = lex->cursor
        };
    }

    return tok;
}

Token next_token(Lexer *lex) {
    strip_l(lex);

    Token tok = {0};

    switch (lex->text[lex->cursor]) {
        case '"': {
            lex->cursor++;
            size_t start = lex->cursor;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            while (lex->text[lex->cursor] != '"') {
                if (lex->text[lex->cursor] == '\0') {
                    fprintf(stderr, "%d: ERROR: String not closed\n", tok.loc.row);
                    exit(1);
                }
                lex->cursor++;
            }
            lex->cursor++;
            tok.type = STRING;
            strncpy(tok.text, lex->text + start, lex->cursor - start - 1);
            break;
        }
        case '(': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = OPEN_PAREN;
            strcpy(tok.text, "(");
            break;
        }
        case ')': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = CLOSE_PAREN;
            strcpy(tok.text, ")");
            break;
        }
        case '+': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = OPERATOR;
            strcpy(tok.text, "+");
            break;
        }
        case '-': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = OPERATOR;
            strcpy(tok.text, "-");
            break;
        }
        case '/': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = OPERATOR;
            strcpy(tok.text, "/");
            break;
        }
        case '*': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = OPERATOR;
            strcpy(tok.text, "*");
            break;
        }
        case '\n': {
            lex->cursor++;
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = EOL;
            break;
        }
        case '\0': {
            tok.loc = (Loc) {
                .row = lex->cursor
            };
            tok.type = EOFF;
            break;
        }
        default:
            tok = next_symbol(lex);
    }

    return tok;
}

Token expect_token(Lexer *lex, TokenType type) {
    Token tok = next_token(lex);

    if ((tok.type & type) == 0) {
        fprintf(stderr, "%d: ERROR: Expected `%s` but got %s\n", tok.loc.row, token_types_to_str(type), token_type_to_str(tok.type));
        exit(1);
    }

    return tok;
}

Expr *parse_expression(Lexer *lex) {
    Token tok = next_token(lex);

    if (tok.type == CLOSE_PAREN) return NULL;

    Expr *expr = (Expr *) malloc(sizeof(Expr));

    if (tok.type == OPEN_PAREN) {
        expr->left.expr = parse_expression(lex);
        expr->right.expr = parse_expression(lex);

        return expr;
    }

    if (tok.type == NUMBER || tok.type == OPERATOR || tok.type == SYMBOL || tok.type == STRING) {
        expr->left.tok = tok;
        expr->right.expr = parse_expression(lex);

        return expr;
    }

    free(expr);
    return NULL;
}


long do_operation(long x, long y, enum Op op) {
    switch (op) {
        case PLUS:
            return x + y;
        case MINUS:
            return x - y;
        case STAR:
            return x * y;
        case SLASH:
            return x / y;
        default:
            assert(false && "Invalid operation");
    }

}

long eval_expression(Expr *expr);

long eval_operation(Expr *expr, enum Op op) {
    long result = atol(expr->left.tok.text);

    if (expr->left.expr != NULL) {
        result = do_operation(result, eval_expression(expr->left.expr), op);
    }

    expr = expr->right.expr;

    while (expr != NULL) {
        if (expr->left.expr != NULL) {
            result = do_operation(result, eval_expression(expr->left.expr), op);
        } else {
            result = do_operation(result, atol(expr->left.tok.text), op);
        }

        expr = expr->right.expr;
    }

    return result;
}

long eval_print(Expr *expr) {
    assert(expr->left.tok.type == STRING && "Not a string");
    if (expr->left.tok.type != STRING) return 0;
    printf("%s\n", expr->left.tok.text);
    return 1;
}

long eval_expression(Expr *expr) {
    if (expr->right.expr == NULL) {
        return eval_expression(expr->left.expr);
    } else if (expr->left.tok.type == OPERATOR) {
        return eval_operation(expr->right.expr, char_to_operator(expr->left.tok.text[0]));
    } else if (expr->left.tok.type == SYMBOL) {
        if (strcmp(expr->left.tok.text, "print") == 0) {
            return eval_print(expr->right.expr);
        } else if (strcmp(expr->left.tok.text, "exit") == 0) {
            exit(0);
        }
    }
}


void expr_del(Expr *expr) {
    if (expr == NULL) return;

    if (expr->left.expr == NULL && expr->right.expr == NULL) {
        free(expr);
    }
    expr_del(expr->left.expr);
    expr_del(expr->left.expr);
}

void repl() {
    Lexer lex = {0};

    while (fgets(lex.text, sizeof(lex.text), stdin) != NULL) {
        lex.cursor = 0;
        Expr *expr = parse_expression(&lex);
        long res = eval_expression(expr);
        expr_del(expr);
        printf("%ld\n", res);
    }
}

void compile_linux_x86_64() {
    assert(false && "Not Implemented");
}


int main(int argc, const char **argv) {
    if (argc < 2) {
        repl();
    } else {
        compile_linux_x86_64();
    }
    return 0;
}
