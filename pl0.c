// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error


//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch


typedef struct{
    int storedSym;
    char storedID[MAXIDLEN + 1];
    int storedNum;
}storedSym;
storedSym* storedSym1, * storedSym2;
int restoreSymFlag = 0;

//////////////////////////////////////////////////////////////////////
// store read symbols
storedSym store_symbol(){
    storedSym temp;
    temp.storedSym = sym;
    strcpy(temp.storedID, id);
    temp.storedNum = num;
    return temp;
} // store_symbol

// load a read symbol
void load_symbol(storedSym *temp){
    sym = temp->storedSym;
    num = temp->storedNum;
    strcpy(id, temp->storedID);
} // load_symbol


//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
    if(restoreSymFlag){
        restoreSymFlag = 0;
        load_symbol(storedSym2);
        return;
    }
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym



//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

void assign(symset fsys){
	expression(fsys);
	int local_level, address, local_f;
	if(sym == SYM_BECOMES){
		getsym();
		cx--;
		if(code[cx].f == LOD){
			local_f = code[cx].f;
			local_level = code[cx].l;
			address = code[cx].a;
			assign(fsys);
			gen(STO, local_level, address);
			gen(local_f, local_level, address);
		}
		else {
			symset set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
			test(set1, phi, 12);
		}
	}
}

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;
	if (sym == SYM_IDENTIFIER){
		set = uniteset(fsys, createset(SYM_BECOMES));
		assign(set);
		cx--;
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		test(fsys, phi, 19);
        storedSym temp = store_symbol();
        storedSym1 = &temp;
		getsym();
		int cx3 = cx, cx4;
		if (sym == SYM_ELSE){
		    cx4 = cx;
		    gen(JMP, 0, 0);
		    getsym();
		    statement(fsys);
		    code[cx4].a = cx;
		    cx3++;
		}
		else{
            storedSym temp2 = store_symbol();
            storedSym2 = &temp2;
            load_symbol(storedSym1);
            restoreSymFlag = 1;
		}
		code[cx1].a = cx3;
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		while_count++;
		cx1 = cx;
		continue_cx[while_count] = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
		for (int i = 0; break_cx[while_count][i]; i++)
		{
			code[break_cx[while_count][i]].a = cx;

			break_cx[while_count][i] = 0;
		}
		break_num[while_count]=0;
		while_count--;
	}
	else if (sym == SYM_CONTINUE)
	{
		if (while_count > 0)
		{
			getsym();
			gen(JMP, 0,continue_cx[while_count]);
		}
	}
	else if (sym == SYM_BREAK)
	{
		if (while_count > 0)
		{
			getsym();
			break_cx[while_count][break_num[while_count]++] = cx;
			gen(JMP, 0, 0);
		}
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

        if (sym == SYM_INT || sym == SYM_VOID){
            translation_unit();
        } // if

        if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

////////////////////////////////////////////////////////////
int output_flag = 1;
void declaration_err(){
    if(output_flag){
        output_flag = 0;
        symset set1;
        set1 = createset(SYM_SEMICOLON, SYM_COMMA, SYM_END, SYM_IDENTIFIER, SYM_NULL);
        test(set1, phi, 19);
        destroyset(set1);
        getsym();
    }
}

// IdenInfo:
// direct_father, 这一个identifier上一级嵌套的变量的iden_info的编号; right_parentheses, 右括号数量;
// specifier, 左边最近的类型标识符, 如int, void
// type, 类型信息; head, 类型信息前的说明文字
typedef struct{
    int direct_father, right_parentheses, specifier;
    char type[300];
    char head[100];
}IdenInfo;

IdenInfo iden_info[10];
// father, 当前正处理的identifier上一级嵌套的变量的iden_info的编号
int iden_info_storage = 10, iden_info_processing, father = 0, stack_top = 0;
// fun_para_flag, 大于0的话当前处理的变量是函数的参数列表内的变量, 否则不是; is_function_flag, 是1表示当前处理的是函数的定义
int right_parentheses, specifier, fun_para_flag, is_function_flag;


void init_iden_info(int k){
    for(int i=k; i < iden_info_storage; i++){
        iden_info[i].head[0] = '\0';
        iden_info[i].type[0] = '\0';
    }
}

// 入栈操作, 保存当前处理的identifier对应的iden_info的各项信息
void push_iden_info(){
    iden_info[iden_info_processing].direct_father = father;
    iden_info[iden_info_processing].right_parentheses = right_parentheses;
    iden_info[iden_info_processing].specifier = specifier;
    iden_info_processing++;
}


int HEAD = 1, TYPE = 2;
// 将可变参数列表内的所有字符串拼接到指定位置(head或type), num指示可变参数列表的大小, flag指示拼接到head还是type
void add_output(int flag, int num, ...) {
    va_list p;
    char* arg;
    va_start(p, num);
    for(;num > 0; num--) {
        arg = va_arg(p, char*);
        if(flag == HEAD)
            strcat(iden_info[iden_info_processing].head, arg);
        else if(flag == TYPE)
            strcat(iden_info[iden_info_processing].type, arg);
    }
    va_end(p);
}

// 取出字符串内的第一对括号间的内容
char * abstract_parentheses(char *source, char * dest){
    int length = strlen(source);
    int l = 0, r = 0, paren_level = 0;
    for(int i = 0; i < length; i++){
        if(source[i] == '('){
            paren_level++;
            if(paren_level == 1)
                l = i;
        }
        else if(source[i] == ')'){
            if(paren_level == 1)
                r = i;
            paren_level--;
        }
    }
    if(l > 0 && r > 0){
        memcpy(dest, &source[l + 1], r - l - 1);
        dest[r - l - 1] = '\0';
    }
    else
        strcpy(dest, source);
}

// 变量的类型不是函数时的检查输出函数
void non_function(char * type){
    int length = strlen(type), size = 1;
    for(int i = 0; i < length; i++){
        if(type[i] == '('){
            int j = i + 1, num = 0;
            for(; j < length; j++){
                if(type[j] >= '0' && type[j] <= '9'){
                    num *= 10;
                    num += type[j] - '0';
                }
                else break;
            }
            if(!num)
                break;
            size *= num;
            i = j;
        }
    }
    printf("OK! type size : %d\n\n", size);

    char ValueType[300];
    int first_char = 0;
    abstract_parentheses(type, ValueType);
    for(int i = 0; i < strlen(ValueType); i++){
        if(ValueType[i] >= 'a' && ValueType[i] <= 'z'){
            first_char = i;
            break;
        }
    }
    printf("ValueType is : %s\n\n", &ValueType[first_char]);
}

// 变量的类型是函数时的检查输出函数
void function(char * type){
    if (type[0] == 'a'){
        printf("\nError: Array of function is not allowed.\nerror!\n");
        return;
    }
    else{
        char ValueType[300];
        int para_i_right = 0, return_i_left = 0;
        abstract_parentheses(type, ValueType);
        for(int i = 0; i < strlen(ValueType); i++){
            if(ValueType[i] == '=' && ValueType[i+1] == '>'){
                para_i_right = i;
                return_i_left = i + 3;
                break;
            }
        }
        if(ValueType[return_i_left] == 'a'){
            printf("\nError: Array or Function can not be returned from functions.\nerror!\n");
            return;
        }
        ValueType[para_i_right] = '\0';
        printf("OK!\nPara-Type is : %s, Return-Type is : %s\n", ValueType, &ValueType[return_i_left]);
    }

}

// 总的输出函数
void output(){
    if(output_flag){
        printf(iden_info[iden_info_processing].head);
        printf(iden_info[iden_info_processing].type);
        if(!fun_para_flag){
            printf("\nType checking...");
            if(is_function_flag)
                function(iden_info[iden_info_processing].type);
            else
                non_function(iden_info[iden_info_processing].type);
            is_function_flag = 0;
        }
        printf("\n");
    }
    else
        output_flag = 1;
}

void translation_unit(){
    if(sym == SYM_INT || sym == SYM_VOID){
        init_iden_info(0);
        iden_info_processing = 0;
        right_parentheses = 0;
        is_function_flag = 0;
        specifier = 0;
        stack_top = 0;
        father = 0;
        fun_para_flag = 0;
        output_flag = 1;
        declaration();
        translation_unit();
    }
}

void declaration(){
    declaration_specifiers();
    init_declarator_list();
    if(sym == SYM_SEMICOLON)
        getsym();
}

void declaration_specifiers(){
    type_specifier();
}

void init_declarator_list(){
    init_declarator();
    _init_declarator_list();
}

void _init_declarator_list(){
    if(specifier == SYM_INT)
        add_output(TYPE, 1, "int");
    else if(specifier == SYM_VOID)
        add_output(TYPE, 1, "void");
    char str[100];
    int i;
    for(i = 0; i < right_parentheses; i++)
        str[i] = ')';
    str[i] = '\0';
    add_output(TYPE, 1, str);

    output();

    stack_top = 0;
    iden_info_processing = 0;
    right_parentheses = 0;
    init_iden_info(iden_info_processing);
	if(sym == SYM_COMMA){
		getsym();
		init_declarator();
		_init_declarator_list();
	}
}

void init_declarator(){
    declarator();
}

void type_specifier(){
    if(sym == SYM_INT){
        specifier = SYM_INT;
        getsym();
    }
    else if(sym == SYM_VOID){
        specifier = SYM_VOID;
        getsym();
    }
    else{
        declaration_err();
        output_flag = 0;
        printf("\nUndefined type of variable: %s\n", id);
    }
}

int pointer_level = 0;
void declarator(){
    if(sym == SYM_TIMES){
        // * 号的优先级相对 [] 和 () 较低, 所以在完成[]和()的分析后, 填入它们的类型信息后, 再加上指针的类型信息
        int temp = pointer_level;
        pointer_level = 0;
        pointer();
        direct_declarator();
        for(int i = pointer_level; i > 0; i--)
            add_output(TYPE, 1, "pointer(");
        right_parentheses += pointer_level;
        pointer_level = temp;
    }
    else{
        direct_declarator();
    }
}

void direct_declarator(){
    if(sym == SYM_IDENTIFIER){
        stack_top++;
        if(fun_para_flag)
            add_output(HEAD, 3, "parameter ", id, " is type of: ");
        else
            add_output(HEAD, 2, id, " is type of: ");
        getsym();
    }

    // 围绕在identifier左右的括号, 与函数无关, 无须特殊处理
    else if(sym == SYM_LPAREN){
        getsym();
        declarator();
        if(sym == SYM_RPAREN)
            getsym();
        else{
            declaration_err();
            output_flag = 0;
            printf("\nUnpaired parentheses.\n");
        }
    }
    _direct_declarator();
}

void _direct_declarator(){
    if(sym == SYM_LSQBRACKET || sym == SYM_LPAREN){
        if(sym == SYM_LSQBRACKET){
            // 存入array的类型和大小, 右括号数加一
            getsym();
            if(sym == SYM_NUMBER){
                char num_str[100];
                itoa(num, num_str, 10);
                add_output(TYPE, 3, "array(", num_str, ", ");
                right_parentheses += 1;
                getsym();
            }
            else{
                declaration_err();
                output_flag = 0;
                printf("\nSize of the array required.\n");
            }
            if(sym == SYM_RSQBRACKET){
                getsym();
            }
            else{
                declaration_err();
                output_flag = 0;
                printf("\nUnpaired square bracket.\n");
            }
        }
        else if(sym == SYM_LPAREN){

            father = iden_info_processing;
            push_iden_info();
            // 遇到一个identifier后的左括号, 意味着后面的identifier都是前面的identifier(father)函数的参数列表, 先处理后面的
            // identifier, 得到类型信息, 然后回填给父identifier, 所以这里暂且更新father为iden_info_processing,
            // push保存当前处理的iden_info的信息,并且会使iden_info_processing增大1

            getsym();
            is_function_flag = 1;
            fun_para_flag += 1;
            if(sym == SYM_INT || sym == SYM_VOID)
                parameter_type_list();

            if(sym == SYM_RPAREN){
                getsym();
                fun_para_flag -= 1;

                // 参数列表结束, 要回填类型信息给函数名对应的iden_info,
                // 所以iden_info_processing更新为当前处理的iden_info的direct_father
                iden_info_processing = iden_info[iden_info_processing].direct_father;
                add_output(TYPE, 1, "function(");
                right_parentheses++;

                // 回填子iden_info的类型信息给父iden_info
                for(int i = iden_info_processing + 1; i < stack_top - 1; i ++){
                    add_output(TYPE, 2, iden_info[i].type, " X ");
                }
                add_output(TYPE, 1, iden_info[stack_top - 1].type);

                //初始化了当前正在处理的父iden_info以后的所有子iden_info, 所以现在的栈顶是正在处理的iden_info的下标+1
                init_iden_info(iden_info_processing + 1);
                add_output(TYPE, 1, " => ");
                specifier = iden_info[iden_info_processing].specifier;
                stack_top = iden_info_processing + 1;
            }
            else{
                declaration_err();
                output_flag = 0;
                printf("\nUnpaired parentheses.\n");
            }
        }
        _direct_declarator();
    }
}

void pointer(){
    if(sym == SYM_TIMES){
        pointer_level++;
        getsym();
        pointer();
    }
}

void parameter_type_list(){
    parameter_list();
}

void parameter_list(){
    parameter_declaration();
    _parameter_list();
}

void _parameter_list(){
    if(specifier == SYM_INT)
        add_output(TYPE, 1, "int");
    else if(specifier == SYM_VOID)
        add_output(TYPE, 1, "void");
    char str[100];
    int i;
    for(i = 0; i < right_parentheses; i++)
        str[i] = ')';
    str[i] = '\0';
    add_output(TYPE, 1, str);

    output();

    if(sym == SYM_COMMA){

        push_iden_info();
        // 只push, 不更新father, 使得将要处理的下一个identifier的father也是当前处理的identifier的father

        getsym();
        specifier = sym;
        parameter_declaration();
        _parameter_list();
    }
}

void parameter_declaration(){
    declaration_specifiers();
    declarator();
}

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);	//Relation Set
	
	// create begin symbol sets
	declbegsys = createset(SYM_VOID, SYM_INT, SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);					//Declaration Beginnging Symbol Set
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL );       		//Statement Beginning Symbol Set
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);	//Factor Beginning Symbol Set

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	printf("Compile Complete.");
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c


