#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

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
		while ((!feof(infile)) // added & modified by alex 01-02-09
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

void getsym(void)// gets a symbol from input stream.
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();
	if (ch == -1)
		return;
	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
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
		} while (isdigit(ch));
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
	}
} // getsym

void test(symset s1, symset s2, int n)
{
	symset s;
	if (!inset(sym, s1))
	{
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

int output_flag = 1;
void declaration_err() {
	if (output_flag) {
		output_flag = 0;
		symset set1;
		set1 = createset(SYM_SEMICOLON, SYM_COMMA, SYM_END, SYM_NULL);
		test(set1, phi, 19);
		destroyset(set1);
		getsym();
	}
}
typedef struct {
	int direct_father;
	char type[300];
	char head[100];
}IdenInfo;
IdenInfo iden_info[10];
int iden_info_storage = 10, iden_info_processing, father = 0;
int right_parentheses, nearest_type, fun_para_flag, is_function_flag;
int HEAD = 1, TYPE = 2;

void init_iden_info(int k) {
	for (int i = k; i < iden_info_storage; i++) {
		iden_info[i].head[0] = '\0';
		iden_info[i].type[0] = '\0';
	}
}

void add_output(int flag, int num, ...) {
	va_list p;
	char* arg;
	va_start(p, num);
	for (; num > 0; num--) {
		arg = va_arg(p, char*);
		if (flag == HEAD)
			strcat(iden_info[iden_info_processing].head, arg);
		else if (flag == TYPE)
			strcat(iden_info[iden_info_processing].type, arg);
	}
	va_end(p);
}

void  abstract_parentheses(char *source, char * dest) {
	int length = strlen(source);
	int l = 0, r = 0, paren_level = 0;
	for (int i = 0; i < length; i++) {
		if (source[i] == '(') {
			paren_level++;
			if (paren_level == 1)
				l = i;
		}
		else if (source[i] == ')') {
			if (paren_level == 1)
				r = i;
			paren_level--;
		}
	}
	if (l > 0 && r > 0) {
		memcpy(dest, &source[l + 1], r - l - 1);
		dest[r - l - 1] = '\0';
	}
	else
		strcpy(dest, source);
}

void non_function(char * type) {
	int length = strlen(type), size = 1;
	for (int i = 0; i < length; i++) {
		if (type[i] == '(') {
			int j = i + 1, num = 0;
			for (; j < length; j++) {
				if (type[j] >= '0' && type[j] <= '9') {
					num *= 10;
					num += type[j] - '0';
				}
				else break;
			}
			if (!num)
				break;
			size *= num;
			i = j;
		}
	}
	printf("OK! type size : %d\n\n", size);
	char ValueType[300];
	int first_char = 0;
	abstract_parentheses(type, ValueType);
	for (int i = 0; i < strlen(ValueType); i++) {
		if (ValueType[i] >= 'a' && ValueType[i] <= 'z') {
			first_char = i;
			break;
		}
	}
	printf("ValueType is : %s\n\n", &ValueType[first_char]);
}

void function(char * type) {
	if (type[0] == 'a') {
		printf("\nError: Array of functions is not allowed.\nerror!\n");
		return;
	}
	else {
		char ValueType[300];
		int para_i_right = 0, return_i_left = 0;
		abstract_parentheses(type, ValueType);
		for (int i = 0; i < strlen(ValueType); i++) {
			if (ValueType[i] == '=' && ValueType[i + 1] == '>') {
				para_i_right = i;
				return_i_left = i + 3;
				break;
			}
		}
		if (ValueType[return_i_left] == 'a') {
			printf("\nError: Array or Function can not be returned from functions.\nerror!\n");
			return;
		}
		ValueType[para_i_right] = '\0';
		printf("OK!\nPara-Type is : %s, Return-Type is : %s\n", ValueType, &ValueType[return_i_left]);
	}

}

void output() {
	if (output_flag) {
		printf(iden_info[iden_info_processing].head);
		printf(iden_info[iden_info_processing].type);
		if (!fun_para_flag) {
			printf("\nType checking...");
			if (is_function_flag)
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

void translation_unit() {
	if (sym == SYM_INT || sym == SYM_VOID) {
		init_iden_info(0);
		iden_info_processing = -1;
		right_parentheses = 0;
		is_function_flag = 0;
		nearest_type = 0;
		fun_para_flag = 0;
		output_flag = 1;
		declaration();
		translation_unit();
	}
}

void declaration() {
	int temp = nearest_type;
	nearest_type = sym;
	declaration_specifiers();
	init_declarator_list();
	if (sym == SYM_SEMICOLON)
		getsym();
	nearest_type = temp;
}

void declaration_specifiers() {
	type_specifier();
}

void init_declarator_list() {
	init_declarator();
	_init_declarator_list();
}

void _init_declarator_list() {
	int temp = right_parentheses;
	if (nearest_type == SYM_INT)
		add_output(TYPE, 1, "int");
	else if (nearest_type == SYM_VOID)
		add_output(TYPE, 1, "void");
	char str[100];
	int i;
	for (i = 0; i < right_parentheses; i++)
		str[i] = ')';
	str[i] = '\0';
	add_output(TYPE, 1, str);
	output();
	iden_info_processing = -1;
	right_parentheses = 0;
	init_iden_info(iden_info_processing);
	if (sym == SYM_COMMA) {
		right_parentheses = 0;
		getsym();
		init_declarator();
		_init_declarator_list();
		right_parentheses = temp;
	}
}

void init_declarator() {
	declarator();
}

void type_specifier() {
	if (sym == SYM_INT) {
		getsym();
	}
	else if (sym == SYM_VOID) {
		getsym();
	}
	else {
		declaration_err();
		output_flag = 0;
		printf("\nUndefined type of variable: %s\n", id);
	}
}

int pointer_level = 0;
void declarator() {
	if (sym == SYM_TIMES) {
		int temp = pointer_level;
		pointer_level = 0;
		pointer();
		direct_declarator();
		for (int i = pointer_level; i > 0; i--)
			add_output(TYPE, 1, "pointer(");
		right_parentheses += pointer_level;
		pointer_level = temp;
	}
	else {
		direct_declarator();
	}
}

void direct_declarator() {
	if (sym == SYM_IDENTIFIER) {
		iden_info[iden_info_processing + 1].direct_father = father;
		iden_info_processing++;
		if (fun_para_flag)
			add_output(HEAD, 3, "parameter ", id, " is type of: ");
		else
			add_output(HEAD, 2, id, " is type of: ");
		getsym();
	}
	else if (sym == SYM_LPAREN) {
		getsym();
		declarator();
		if (sym == SYM_RPAREN)
			getsym();
		else {
			declaration_err();
			output_flag = 0;
			printf("\nUnpaired parentheses.\n");
		}
	}
	_direct_declarator();
}

void _direct_declarator() {
	if (sym == SYM_LSQBRACKET || sym == SYM_LPAREN) {
		if (sym == SYM_LSQBRACKET) {
			getsym();
			if (sym == SYM_NUMBER) {
				char num_str[100];
				itoa(num, num_str, 10);
				add_output(TYPE, 3, "array(", num_str, ", ");
				right_parentheses += 1;
				getsym();
			}
			else {
				declaration_err();
				output_flag = 0;
				printf("\nSize of the array required.\n");
			}
			if (sym == SYM_RSQBRACKET) {
				getsym();
			}
			else {
				declaration_err();
				output_flag = 0;
				printf("\nUnpaired square bracket.\n");
			}
		}
		else if (sym == SYM_LPAREN) {
			getsym();
			is_function_flag = 1;
			fun_para_flag += 1;
			father = iden_info_processing;
			if (sym == SYM_INT || sym == SYM_VOID)
				parameter_type_list();

			if (sym == SYM_RPAREN) {
				getsym();
				fun_para_flag -= 1;
				int temp = iden_info_processing;
				char str[300];
				str[0] = '\0';
				iden_info_processing = iden_info[iden_info_processing].direct_father;
				father = iden_info[iden_info_processing].direct_father;
				add_output(TYPE, 1, "function(");
				right_parentheses++;
				for (int i = iden_info_processing + 1; i <= temp; i++) {
					add_output(TYPE, 2, iden_info[i].type, " X ");
				}
				init_iden_info(iden_info_processing + 1);
				iden_info[iden_info_processing].type[strlen(iden_info[iden_info_processing].type) - 3] = '\0';
				add_output(TYPE, 1, " => ");

			}
			else {
				declaration_err();
				output_flag = 0;
				printf("\nUnpaired parentheses.");
			}
		}
		_direct_declarator();
	}
}

void pointer() {
	if (sym == SYM_TIMES) {
		pointer_level++;
		getsym();
		pointer();
	}
}

void parameter_type_list() {
	parameter_list();
}

void parameter_list() {
	int temp = nearest_type, temp2 = right_parentheses;
	nearest_type = sym;
	right_parentheses = 0;
	parameter_declaration();
	_parameter_list();
	nearest_type = temp;
	right_parentheses = temp2;
}

void _parameter_list() {
	int temp = right_parentheses;
	if (nearest_type == SYM_INT)
		add_output(TYPE, 1, "int");
	else if (nearest_type == SYM_VOID)
		add_output(TYPE, 1, "void");
	char str[100];
	int i;
	for (i = 0; i < right_parentheses; i++)
		str[i] = ')';
	str[i] = '\0';
	add_output(TYPE, 1, str);

	output();

	if (sym == SYM_COMMA) {
		right_parentheses = 0;
		getsym();
		int temp2 = nearest_type;
		nearest_type = sym;
		parameter_declaration();
		_parameter_list();
		nearest_type = temp2;
		right_parentheses = temp;
	}
}

void parameter_declaration() {
	declaration_specifiers();
	declarator();
}

//////////////////////////////////////////////////////////////////////
int main() {
	char *filename = "example.txt";
	char s[80];
	int i;
	//printf("Please input source file name: "); // get file name to be compiled
	//scanf("%s", s);
	printf("\n\nStarting parsing variable declaration...\n\n\n");
	if ((infile = fopen(filename, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}
	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;
	getsym();
	translation_unit();
	system("pause");
}