//
//  main.c
//  Test
//
//  Created by 叶梅北宁 on 2016/10/01.
//  Copyright © 2016年 叶梅北宁. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int debug;
int assembly;
int cycle;

int token; //token
char *src, *old_src; //source code
int pool_size; //default size of stack/code/text
int line; //line

//virtual machine
int *text, *old_text; //text segment
int *stack; //stack segment
char *data; //data segment;
int *pc, *bp, *sp, ax; //registers of virtual machine
int index_of_bp; //index of bp pointer on stack

// pc 指令寄存器
// sp 指针寄存器 -> 栈顶
// bp 基址寄存器
// ax 通用寄存器 指令计算结果

//x86 instructions
enum {
    LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
    OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};


//lex

enum {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};


enum {
    Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize
};

int token_val; // current token value
int *current_id, *symbols; // symbol table

//variable type
enum {
    CHAR, INT, PTR
};
int *id_main;

//expression type, declaration type
int base_type;
int expr_type;


void next() //词法分析
{
    char *last_pos;
    int hash;

    while (token = *src) {
        //parse token
        ++src;
        if (token == '\n') {
            if (assembly) {
                // print compile info
                printf("%d: %.*s", line, src-old_src, old_src);
                old_src = src;

                while (old_text < text) {
                    printf("%8.4s", & "LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                                      "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                                      "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[*++old_text * 5]);

                    if (*old_text <= ADJ)
                        printf(" %d\n", *++old_text);
                    else
                        printf("\n");
                }
            }
            ++line;
        } else if (token == '#') {
            //parse macro

            //skip
            while (*src != 0 && *src != '\n') {
                src++;
            }
        } else if ((token >= 'a' && token <= 'z') || (token >='A' && token <= 'Z') || (token == '_')) {
            //parse identifier
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||(*src == '_')) {
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
            token = current_id[Token] = Id;
            return;
        } else if (token >= '0' && token <= '9') {
            //parse number
            token_val = token - '0';
            if (token_val > 0) {
                //parse dec
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val * 10 + (*src - '0');
                    src++;
                }
            } else if (*src == 'x' || *src == 'X') {
                //parse hex
                token = *++src;
                while ((token >= '0' && token <= '9') || (token >= 'a' && token <='f') || (token >= 'A' && token <= 'F')) {
                    token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                    token = *++src;
                }
            } else {
                //parse oct
                while (*src >= '0' && *src <= '7') {
                    token_val = token_val * 8 + *src - '0';
                    src++;
                }
            }

            token = Num;
            return;
        } else if (token == '"' || token == '\'')
        {
            //parse string and char

            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    //only support /n
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }

                if (token == '"') {
                    *data++ = token_val;
                }
            }

            src++;
            if (token == '"') {
                token_val = (int) last_pos;
            } else {
                token = Num;
            }

            return;
        } else if (token == '/') {
            //parse comments and div
            if (*src == '/') {
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // div ()/()
                token = Div;
                return;
            }
        } else if (token == '=') {
            //parse = and ==

            if (*src == '=') {
                src++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        } else if (token == '+') {
            //parse + and ++

            if (*src == '+') {
                src++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        } else if (token == '-') {
            //parse - and --
            if (*src == '-') {
                src++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        } else if (token == '!') {
            //parse !=
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        } else if (token == '<') {
            //parse <=, <<, <
            if (*src == '=') {
                src++;
                token = Le;
            } else if (*src == '<') {
                src++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        } else if (token == '>') {
            //parse >=, >>, >
            if (*src == '=') {
                src++;
                token = Ge;
            } else if (*src == '<') {
                src++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        } else if (token == '|') {
            //parse | and ||
            if (*src == '|') {
                src++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        } else if (token == '&') {
            //parse & and &&
            if (*src == '&') {
                src++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        } else if (token == '^') {
            token = Xor;
            return;
        } else if (token == '%') {
            token = Mod;
            return;
        } else if (token == '*') {
            token = Mul;
            return;
        } else if (token == '[') {
            token = Brak;
            return;
        } else if (token == '?') {
            token = Cond;
            return;
        } else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            //parse other token
            return;
        }
    }
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

void expression(int level) //表达式解析
{
    int *id;
    int tmp;
    int *addr;

    if (!token) {
        printf("%d: unexpected token EOF of expression\n", line);
        exit(-1);
    }

    if (token == Num) {
        match(Num);

        //emit code
        *++text = IMM;
        *++text = token_val;
        expr_type = INT;
    } else if (token == '"') {

        *++text = IMM;
        *++text = token_val;

        match('"');

        // char *a;
        // a = "xxxx"
        //     "xxxx";
        while (token == '"') {
            match('"');
        }

        data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
        expr_type = PTR;
    } else if (token == Sizeof) {
        //support sizeof(int), sizeof(char), sizeof(*...)

        match(Sizeof);
        match('(');
        expr_type = INT;

        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            match(Char);
            expr_type = CHAR;
        }

        while (token == Mul) {
            match(Mul);
            expr_type = expr_type + PTR;
        }

        match(')');

        //emit code
        *++text = IMM;
        *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);

        expr_type = INT;
    } else if (token == Id) {
        //variable

        match(Id);

        id = current_id;
        if (token == '(') {
            // func call

            match('(');

            tmp = 0; //number of argument
            while (token != ')') {
                expression(Assign);
                *++text = PUSH;
                tmp++;

                if (token == ',') {
                    match(',');
                }
            }

            match(')');

            //emit code
            if (id[Class] == Sys) {
                //system func
                *++text = id[Value];
            } else if (id[Class] == Fun) {
                //func call
                *++text = CALL;
                *++text = id[Value];
            } else {
                printf("%d: bad function call\n", line);
                exit(-1);
            }

            if (tmp > 0) {
                //remove arguments from stack;
                *++text = ADJ;
                *++text = tmp;
            }

            expr_type = id[Type];
        } else if (id[Class] == Num) {
            //enum variable

            *++text = IMM;
            *++text = id[Value];
            expr_type = INT;
        } else {
            //variable

            if (id[Class] == Loc) {
                *++text = LEA;
                *++text = index_of_bp - id[Value];
            } else if (id[Class] == Glo) {
                *++text = IMM;
                *++text = id[Value];
            } else {
                printf("%d: undefined variable\n", line);
                exit(-1);
            }

            //emit code

            expr_type = id[Type];
            *++text = (expr_type == CHAR) ? LC : LI;
        }
    } else if (token == '(') {
        //强制转换

        match('(');

        if (token == Int || token == Char) {
            tmp = (token == Char) ? CHAR : INT; // cast type
            match(token);
            while (token == Mul) {
                match(Mul);
                tmp = tmp + PTR;
            }
            match(')');

            expression(Inc); // (++)

            expr_type = tmp;
        } else {
            expression(Assign);
            match(')');
        }
    } else if (token == Mul) {
        // *a pointer

        match(Mul);
        expression(Inc); //Inc(++)
        if (expr_type >= PTR) {
            expr_type = expr_type - PTR;
        } else {
            printf("%d: bad dereference\n", line);
            exit(-1);
        }
        *++text = (expr_type == CHAR) ? LC : LI;
    } else if (token == And) {
        //& get addr

        match(And);
        expression(Inc); //get address

        if (*text == LC || *text == LI) {
            text--;
        } else {
            printf("%d: bad address of\n", line);
            exit(-1);
        }

        expr_type = expr_type + PTR;
    } else if (token == '!') {
        //not

        match('!');
        expression(Inc);

        //emit code
        *++text = PUSH;
        *++text = IMM;
        *++text = 0;
        *++text = EQ;

        expr_type = INT;
    } else if (token == '~') {
        //bitwise not

        match('~');
        expression(Inc);

        //emit code expr XOR -1
        *++text = PUSH;
        *++text = IMM;
        *++text = -1;
        *++text = XOR;

        expr_type = INT;
    } else if (token == Add) {
        // +
        match(Add);

        expression(Inc);

        expr_type = INT;
    } else if (token == Sub) {
        // -
        match(Sub);

        if (token == Num) {
            *++text = IMM;
            *++text = -token_val;
            match(Num);
        } else {
            *++text = IMM;
            *++text = -1;
            *++text = PUSH;
            expression(Inc);
            *++text = MUL;
        }

        expr_type = INT;
    } else if (token == Inc || token == Dec) {
        // ++a --a
        tmp = token;
        match(token);
        expression(Inc);

        if (*text == LC) {
            *text = PUSH;
            *++text = LC;
        } else if (*text == LI) {
            *text = PUSH;
            *++text = LI;
        } else {
            printf("%d: bad lvalue of pre-increment\n", line);
            exit(-1);
        }

        *++text = PUSH;
        *++text = IMM;

        *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
        *++text = (tmp == Inc) ? ADD : SUB;
        *++text = (expr_type == CHAR) ? SC : SI;
    }

    //binary operator

    while (token >= level) {

        tmp = expr_type;

        if (token == Assign) {
            //var = expr

            match(Assign);
            if (*text == LC || *text == LI) {
                *text = PUSH; // save the lvalue's pointer
            } else {
                printf("%d: bad lvalue in assignment\n", line);
                exit(-1);
            }

            expression(Assign);
            expr_type = tmp;

            *++text = (expr_type == CHAR) ? SC : SI;
        } else if (token == Cond) {
            // var = expr ? a : b

            match(Cond);

            *++text = JZ;
            addr = ++text;
            expression(Assign);

            if (token == ':') {
                match(':');
            } else {
                printf("%d: missing colon in conditional\n", line);
                exit(-1);
            }

            *addr = (int)(text + 3);
            *++text = JMP;
            addr = ++text;
            expression(Cond);

            *addr = (int)(text + 1);
        } else if (token == Lor) {
            // ||
            match(Lor);

            *++text = JNZ;
            addr = ++text;

            expression(Lan);
            *addr = (int)(text + 1);
            expr_type = INT;
        } else if (token == Lan) {
            // &&
            match(Lan);

            *++text = JZ;
            addr = ++text;

            expression(Or);
            *addr = (int)(text + 1);
            expr_type = INT;
        } else if (token == Or) {
            // bitwise |
            match(Or);

            *++text = PUSH;
            expression(Xor);

            *++text = OR;
            expr_type = INT;
        } else if (token == Xor) {
            // bitwise ^
            match(Xor);

            *++text = PUSH;
            expression(And);

            *++text = XOR;
            expr_type = INT;
        } else if (token == And) {
            // bitwise &
            match(And);

            *++text = PUSH;
            expression(Eq);

            *++text = AND;
            expr_type = INT;
        } else if (token == Eq) {
            // equal ==
            match(Eq);
            *++text = PUSH;
            expression(Ne);
            *++text = EQ;
            expr_type = INT;
        } else if (token == Ne) {
            // not equal !=
            match(Ne);
            *++text = PUSH;
            expression(Lt);
            *++text = NE;
            expr_type = INT;
        } else if (token == Lt) {
            // <
            match(Lt);
            *++text = PUSH;
            expression(Shl);
            *++text = LT;
            expr_type = INT;
        } else if (token == Gt) {
            // >
            match(Gt);
            *++text = PUSH;
            expression(Shl);
            *++text = GT;
            expr_type = INT;
        } else if (token == Le) {
            // less than or equal to
            match(Le);
            *++text = PUSH;
            expression(Shl);
            *++text = LE;
            expr_type = INT;
        } else if (token == Ge) {
            // greater than or equal to
            match(Ge);
            *++text = PUSH;
            expression(Shl);
            *++text = GE;
            expr_type = INT;
        } else if (token == Shl) {
            // shift left
            match(Shl);
            *++text = PUSH;
            expression(Add);
            *++text = SHL;
            expr_type = INT;
        } else if (token == Shr) {
            // shift right
            match(Shr);
            *++text = PUSH;
            expression(Add);
            *++text = SHR;
            expr_type = INT;
        } else if (token == Add) {
            //add
            match(Add);
            *++text = PUSH;
            expression(Mul);

            expr_type = tmp;
            if (expr_type > PTR) {
                // pointer, not `char *`
                *++text = PUSH;
                *++text = IMM;
                *++text = sizeof(int);
                *++text = MUL;
            }
            *++text = ADD;
        } else if (token == Sub) {
            // sub
            match(Sub);
            *++text = PUSH;
            expression(Mul);


            if (tmp > PTR && tmp == expr_type) {
                // pointer subtraction
                *++text = SUB;
                *++text = PUSH;
                *++text = IMM;
                *++text = sizeof(int);
                *++text = DIV;
                expr_type = INT;
            } else if (tmp > PTR) {
                // pointer movement
                *++text = PUSH;
                *++text = IMM;
                *++text = sizeof(int);
                *++text = MUL;
                *++text = SUB;
                expr_type = tmp;
            } else {
                // numeral subtraction
                *++text = SUB;
                expr_type = tmp;
            }

        } else if (token == Mul) {
            // multiply
            match(Mul);

            *++text = PUSH;
            expression(Inc);
            *++text = MUL;
            expr_type = tmp;
        } else if (token == Div) {
            // divide
            match(Div);

            *++text = PUSH;
            expression(Inc);
            *++text = DIV;
            expr_type = tmp;
        } else if (token == Mod) {
            // Modulo
            match(Mod);

            *++text = PUSH;
            expression(Inc);
            *++text = MOD;
            expr_type = tmp;
        } else if (token == Inc || token == Dec) {
            // x++, x--

            if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            } else if (*text == LC) {
                *text = PUSH;
                *++text = LC;
            } else {
                printf("%d: bad value in increment\n", line);
                exit(-1);
            }

            // postfix
            *++text = PUSH;
            *++text = IMM;
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
            *++text = (token == Inc) ? ADD : SUB;
            *++text = (expr_type == CHAR) ? SC : SI;
            *++text = PUSH;                                             //
            *++text = IMM;                                              // 执行相反的增/减操作
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);   //
            *++text = (token == Inc) ? SUB : ADD;                       //

            match(token);
        } else if (token == Brak) {
            //array a[x]

            match(Brak);
            *++text = PUSH;
            expression(Assign);
            match(']');

            if (tmp > PTR) {
                // pointer, `not char *`
                *++text = PUSH;
                *++text = IMM;
                *++text = sizeof(int);
                *++text = MUL;
            } else if (tmp < PTR) {
                printf("%d: pointer type expected\n", line);
                exit(-1);
            }

            expr_type = tmp - PTR;
            *++text = ADD;
            *++text = (expr_type == CHAR) ? LC : LI;
        } else {
            printf("%d: compiler error, token = %d\n", line, token);
            exit(-1);
        }
    }
}

void statement()
{
    //support these kinds statements
    //if (...) {...}
    //while (...) {...}
    //{...}
    //return xxx;
    //expression

    int *a, *b;

    if (token == If) {
        //if () {} else {}
        match(If);
        match('(');
        expression(Assign);
        match(')');

        //emit if code
        *++text = JZ;
        b = ++text;

        statement(); //parse statement
        if (token == Else) {
            match(Else);

            //emit code JMP b
            *b = (int)(text + 3);
            *++text = JMP;
            b = ++text;

            statement();
        }

        *b = (int)(text + 1);
    } else if (token == While) {
        //while

        match(While);
        a = text + 1;
        match('(');
        expression(Assign);
        match(')');

        *++text = JZ;
        b = ++text;
        statement();

        *++text = JMP;
        *++text = (int)a;
        *b = (int)(text + 1);
    } else if (token == Return) {
        //return

        match(Return);

        if (token != ';') {
            expression(Assign);
        }

        match(';');

        *++text = LEV;
    } else if (token == '{') {
        //statement

        match('{');

        while (token != '}') {
            statement();
        }

        match('}');
    } else if (token == ';') {
        match(';');
    } else {
        expression(Assign);
        match(';');
    }
}

//function parse

void function_parameter()
{
    int type;
    int params;
    params = 0;
    while (token != ')') {
        //parse int xxx, int xx, ...

        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        //parse pointer int *xxx
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        //parse variable name
        if (token != Id) {
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }

        match(Id);

        //store local variable
        current_id[BClass] = current_id[Class];
        current_id[Class] = Loc;
        current_id[BType] = current_id[Type];
        current_id[Type] = type;
        current_id[BValue] = current_id[Value];
        current_id[Value] = params++;

        if (token == ',') {
            match(',');
        }
    }

    index_of_bp = params + 1;
}

void function_body()
{
    //... { ... }

    int pos_local; //position of local variable on the stack
    int type;
    pos_local = index_of_bp;

    //local variable declaration
    while (token == Int || token == Char) {
        base_type = (token == Int) ? INT : CHAR;
        match(token);

        while (token != ';') {
            type = base_type;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                // invalid declaration
                printf("%d: bad local declaration\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                // identifier exists
                printf("%d: duplicate local declaration\n", line);
                exit(-1);
            }
            match(Id);

            //store variable
            current_id[BClass] = current_id[Class];
            current_id[Class] = Loc;
            current_id[BType] = current_id[Type];
            current_id[Type] = type;
            current_id[BValue] = current_id[Value];
            current_id[Value] = ++pos_local;

            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }

    //save stack size of local variable
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    //statements
    while (token != '}') {
        statement();
    }

    //leavel func
    *++text = LEV;
}

void function_declaration()
{
    //parse func_name (...) {...}

    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();

    //恢复全局信息
    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type] = current_id[BType];
            current_id[Value] = current_id[BValue];
        }

        current_id = current_id + IdSize;
    }
}

void enum_declaration()
{
    //parse enum
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            printf("%d: bad enum identifier %d\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            // {a = 1}
            next();
            if (token != Num) {
                printf("%d: bad enum initializer\n", line);
                exit(-1);
            }
            i = token_val;
            next();
        }

        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}

void global_declaration() //全局的定义语句
{
    int type; //type for variable
    int i; //temp

    base_type = INT;

    if (token == Enum) {
        //parse enum enum[id] { a = 10, b, c }
        match(Enum);
        if (token != '{') {
            match(Id);
            //skip id
        }
        if (token == '{') {
            match('{');
            //parse assign
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }

    if (token == Int) {
        match(Int);
    } else if (token == Char) {
        match(Char);
        base_type = CHAR;
    }

    while (token != ';' && token != '}') {
        //parse variable
        type = base_type;

        while (token == Mul) {
            //parse int *******a;
            match(Mul);
            type = type + PTR;
        }

        if (token != Id) {
            //invalid declaration
            printf("%d: bad global declaration\n", line);
            exit(-1);
        }

        if (current_id[Class]) {
            //identifier exists
            printf("%d: duplicate global declaration\n", line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            //value is the memory address of func
            current_id[Value] = (int)(text + 1);
            function_declaration();
        } else {
            //variable declaration
            current_id[Class] = Glo;
            current_id[Value] = (int)data;
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }

    }
    next();
}

void program() //语法分析
{
    next();
    while (token > 0) {
        global_declaration();
    }
}


int eval() //虚拟机入口
{
    int op, *tmp;
    int cycle;
    cycle = 0;
    while (1) {
        cycle ++;
        op = *pc++;

        if (debug) {
            printf("%d> %.4s", cycle,
                   & "LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                   "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                   "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[op * 5]);
            if (op <= ADJ)
                printf(" %d\n", *pc);
            else
                printf("\n");

        }
        
        if (op == IMM) { ax = *pc++; } //load immediate value to ax
        else if (op == LC) { ax = *(char *)ax; } //LC LI 指令 ax中存放地址
        else if (op == LI) { ax = *(int *)ax; }
        else if (op == SC) { ax = *(char *)*sp++ = ax; } //SC SI 指令 ax中存放数据，栈顶存放地址
        else if (op == SI) { ax = *(int *)*sp++ = ax; } //MOV指令拆成以上5条
        else if (op == PUSH) { *(--sp) = ax; } //PUSH ax入栈
        else if (op == JMP) { pc = (int *)*pc; } //JMP pc寄存器中存放下一条指令
        else if (op == JZ) { pc = ax ? pc + 1 : (int *)*pc; } // if ax == 0 then JMP
        else if (op == JNZ) { pc = ax ? (int *)*pc : pc + 1; } // if ax != 0 then JMP
        else if (op == CALL) { *--sp = (int)(pc + 1); pc = (int *)*pc; } // call func 参数顺序入栈
        else if (op == ENT) { *--sp = (int)bp; bp = sp; sp = sp - *pc++; } //保存当前栈指针
        else if (op == ADJ) { sp = sp + *pc++; } //add esp, <size> 调用子函数时压入栈中的数据清除
        else if (op == LEV) { sp = bp; bp = (int *)*sp++; pc = (int *)*sp++; } //restore call frame and PC
        else if (op == LEA) { ax = (int)(bp + *pc++); } //load address for arguments. 获得子函数传入参数的地址
        
        //计算指令
        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;
        
        //系统指令
        else if (op == EXIT) { printf("exit(%d)", *sp); return *sp; }
        else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
        else if (op == CLOS) { ax = close(*sp);}
        else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) { ax = (int)malloc(*sp); }
        else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp); }
        else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp); }
         
        else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
        
    }
    
    return 0;
}


int main(int argc, char** argv) {
    int i, fd;
    int *tmp;

    //for lldb debug
    argc = 2;
    *(argv + 1) = "/Users/yemeibeining/Desktop/Complier/miniC/test/test.c";

    argc--;
    argv++;

    if (argc > 0 && **argv == '-' && (*argv)[1] == 's') {
        assembly = 1;
        --argc;
        ++argv;
    }
    if (argc > 0 && **argv == '-' && (*argv)[1] == 'd') {
        debug = 1;
        --argc;
        ++argv;
    }
    if (argc < 1) {
        printf("usage: miniC [-s] [-d] file ...\n");
        return -1;
    }

    pool_size = 256 * 1024; //default size
    line = 1;

    // virtual machine

    //alloc memory for virtual machine
    if (!(text = old_text = malloc(pool_size))) {
        printf("could not malloc(%d) for text area\n", pool_size);
        return -1;
    }
    
    if (!(data = malloc(pool_size))) {
        printf("could not malloc(%d) for data area\n", pool_size);
        return -1;
    }
    
    if (!(stack = malloc(pool_size))) {
        printf("could not malloc(%d) for stack area\n", pool_size);
        return -1;
    }

    if (!(symbols = malloc(pool_size))) {
        printf("could not malloc(%d) for symbol table\n", pool_size);
        return -1;
    }
    
    memset(text, 0, pool_size);
    memset(data, 0, pool_size);
    memset(stack, 0, pool_size);
    memset(symbols, 0, pool_size);

    old_text = text;
    
    bp = sp = (int *)((int)stack + pool_size);
    ax = 0;

    // keywords parse and make a library

    src = "char else enum if int return sizeof while "
            "open read close printf malloc memset memcmp exit void main";

    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }
    //parse void main
    next();
    current_id[Token] = Char;
    next();
    id_main = current_id;

    //file operate

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }
    if (!(src = old_src = malloc(pool_size))) {
        printf("could not malloc(%d) for source area\n", pool_size);
        return -1;
    }
    // read the source file
    if ((i = read(fd, src, pool_size - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);
    
    program();


    if (!(pc = (int *)id_main[Value])) {
        printf("main() not defined\n");
        return -1;
    }

    // dump_text();
    if (assembly) {
        // only for compile
        return 0;
    }

    // setup stack
    sp = (int *)((int)stack + pool_size);
    *--sp = EXIT; // call exit if main returns
    *--sp = PUSH; 
    tmp = sp;
    *--sp = argc;
    *--sp = (int)argv;
    *--sp = (int)tmp;

    return eval();
}