//
//  Syntactic.c
//  Complier_Homework
//
//  Created by 叶梅北宁 on 2016/11/12.
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
    Sharp = 128, Begin, If, Then, While, Do, End,
    Var, Num,
    Assign, Eq, Ne, Lt, Gt, Le, Ge, Add, Sub, Mul, Div, Inc, Dec,
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

            token = Num;
            return;
        } else if (token == ':') {
            //parse := :

            if (*src == '=') {
                src++;
                token = Assign;
            } else {
                token = Eof;
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

        } else if (token == '~' || token == ';' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            //parse other token
            return;
        } else {
            switch (token)
            {
                case '+': {token = Add; return; }
                case '-': {token = Sub; return; }
                case '*': {token = Mul; return; }
                case '/': {token = Div; return; }
                case '=': {token = Eq; return; }
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

void expression(int level) {
    if (token == Sharp) {
        return;
    }

    if (token == Eof) {
        printf("%d: unexpected token EOF of expression\n", line);
        exit(-1);
    }

    if (token == Num) {
        match(Num);

        //do something
    } else if (token == Var) {
        match(Var);

        //do something
    } else if (token == '(') {

        expression(Assign);
        match(')');

    } else if (token == Add) {
        match(Add);

        expression(Inc);

    } else if (token == Sub) {
        match(Sub);

        expression(Dec);

    }

    while (token >= level) {

        if (token == Assign) {

            match(Assign);

            expression(Assign);
        } else if (token == Add) {

            match(Add);

            expression(Mul);
        } else if (token == Sub) {

            match(Sub);

            expression(Mul);
        } else if (token == Mul) {

            match(Mul);

            expression(Inc);
        } else if (token == Div) {

            match(Div);

            expression(Inc);
        } else {
            printf("%d: compiler error, token = %d\n", line, token);
            exit(-1);
        }
    }
}


void statement() {

    //begin ... end

    if (token == Begin) {

        match(Begin);
        while (token != End) {
            statement();
        }
        match(End);
    } else if (token == ';') {
        match(';');
    } else {
        expression(Assign);
        match(';');
    }
}

void program()
{
    next();
    while (token != Sharp) {
        statement();
    }

    printf("success!\n");
}


int main(int argc, char **argv)
{
    int i;
    FILE *fp;

    argc--;
    argv++;

    if (argc < 1) {
        printf("usage: Syntactic file ...\n");
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
    i = Begin;
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
    program();
    return 0;
}