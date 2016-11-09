//
//  lexical.c
//  Complier_Homework
//
//  Created by 叶梅北宁 on 2016/10/12.
//  Copyright © 2016年 叶梅北宁. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

char *src, *last_pos;
int pool_size;
int line;
int token, token_val;
int *current_id, *symbols;

enum {
    Eof = -1,
    Sharp = 0, Begin, If, Then, While, Do, End,
    Var = 10, Num, Add, Sub, Mul, Div, Colon, Assign,
    Lt = 20, Ne, Le, Gt, Ge, Eq, Sem, Op, Cp,
};

enum {
    Token, Hash, Name, IdSize
};

void next()
{
    int hash;

    while (token = *src) {
        //parse token

        ++src;
        last_pos = src - 1;
        if (token == '\n') {
            ++line;
        } else if ((token >= 'a' && token <= 'z') || (token >='A' && token <= 'Z') || (token == '_')) {
            //parse identifier
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }

            //search existing identifier
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            //store new id

            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Var;

            return;
        } else if (token >= '0' && token <= '9') {
            //parse number

            token_val = token - '0';

            while (*src >= '0' && *src <= '9') {
                token_val = token_val * 10 + (*src - '0');
                src++;
            }


            if (!(*src == '=' || *src == ')' || *src == '+' || *src == '-' || *src == '*' || *src == '/' || *src == ';')) {
                printf("Unexpected token %d, in line %d", token, line);
                exit(-1);
            }

            token = Num;
            return;
        } else if (token == ':') {
            //parse := :

            if (*src == '=') {
                src++;
                token = Assign;
            } else {
                token = Colon;
            }

            return;
        } else if (token == '<') {
            //parse <, <=, <>

            if (*src == '=') {
                src++;
                token = Le;
            } else if (*src == '>') {
                src++;
                token = Ne;
            } else {
                token = Lt;
            }

            return;
        } else if (token == '>') {
            //parse >, >=

            if (*src == '=') {
                src++;
                token = Ge;
            } else {
                token = Gt;
            }

            return;
        } else {
            switch (token)
            {
                case '+': {token = Add; return; }
                case '-': {token = Sub; return; }
                case '*': {token = Mul; return; }
                case '/': {token = Div; return; }
                case '=': {token = Eq; return; }
                case ';': {token = Sem; return; }
                case '(': {token = Op; return; }
                case ')': {token = Cp; return; }
                case '#': {token = Sharp; return; }
                default: ;
            }
        }
    }
    token = Eof;
    return;
}

void match(int tk)
{
    if (token == tk) {
        next();
    } else {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
}

void scanner()
{
    if (token != Num) {
        //print key words , variable , operator
        printf("(%2d, %.*s)\n", token, src - last_pos, last_pos);
    } else if (token == Num) {
        printf("(%2d, %d)\n", token, token_val);
    }
    next();
}

void lexical()
{
    next();
    while (token != Eof) {
        scanner();
    }
}


int main(int argc, char **argv)
{
    int i;
    FILE *fp;

    argc--;
    argv++;

    if (argc < 1) {
        printf("usage: lexical file ...\n");
        return -1;
    }

    pool_size = 256 * 1024;
    line = 1;

    if (!(symbols = malloc(pool_size))) {
        printf("could not malloc(%d) for symbol table\n", pool_size);
        return -1;
    }
    memset(symbols, 0, pool_size);

    src = "begin if then while do end";
    i = 1;
    while (i <= End) {
        next();
        current_id[Token] = i++;
    }

    if ((fp = fopen(*argv, "r")) == NULL) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }
    if (!(src = malloc(pool_size))) {
        printf("could not malloc(%d) for source area\n", pool_size);
        return -1;
    }

    if ((i = fread(src, sizeof(char), pool_size, fp)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0;
    fclose(fp);
    lexical();
    return 0;
}