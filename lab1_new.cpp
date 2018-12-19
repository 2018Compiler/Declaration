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

  // gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' ' || ch == '\t')
		getch();

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

  ////////////////////////////////////////////////////////////
int output_flag = 1;
void declaration_err() {
	if (output_flag) {
		output_flag = 0;
		symset set1;
		set1 = createset(SYM_SEMICOLON, SYM_COMMA, SYM_END, SYM_IDENTIFIER, SYM_NULL);
		test(set1, phi, 19);
		destroyset(set1);
		getsym();
	}
}

// IdenInfo:
// direct_father, ��һ��identifier��һ��Ƕ�׵ı�����iden_info�ı��; right_parentheses, ����������;
// specifier, �����������ͱ�ʶ��, ��int, void
// type, ������Ϣ; head, ������Ϣǰ��˵������
typedef struct {
	int direct_father, right_parentheses, specifier;
	char type[300];
	char head[100];
}IdenInfo;

IdenInfo iden_info[10];
// father, ��ǰ�������identifier��һ��Ƕ�׵ı�����iden_info�ı��
int iden_info_storage = 10, iden_info_processing, father = 0, stack_top = 0;
// fun_para_flag, ����0�Ļ���ǰ����ı����Ǻ����Ĳ����б��ڵı���, ������
int right_parentheses, specifier, fun_para_flag;

//��ʼ������
void init_iden_info(int k) {
	for (int i = k; i < iden_info_storage; i++) {
		iden_info[i].head[0] = '\0';
		iden_info[i].type[0] = '\0';
	}
}

// ��ջ����, ���浱ǰ�����identifier��Ӧ��iden_info�ĸ�����Ϣ
void push_iden_info() {
	iden_info[iden_info_processing].direct_father = father;
	iden_info[iden_info_processing].right_parentheses = right_parentheses;
	iden_info[iden_info_processing].specifier = specifier;
	iden_info_processing++;
}


int HEAD = 1, TYPE = 2;
// ���ɱ�����б��ڵ������ַ���ƴ�ӵ�ָ��λ��(head��type), numָʾ�ɱ�����б�Ĵ�С, flagָʾƴ�ӵ�head����type
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

// ȡ���ַ����ڵĵ�һ�����ż������
void abstract_parentheses(char *source, char * dest) {
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

// ���������Ͳ��Ǻ���ʱ�ļ���������
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
	char ValueType[300];
	int first_char = 0;
	abstract_parentheses(type, ValueType);
	for (int i = 0; i < strlen(ValueType); i++) {
		if (ValueType[i] >= 'a' && ValueType[i] <= 'z') {
			first_char = i;
			break;
		}
	}
	if (type[0] == 'a' && ValueType[first_char] == 'f') {
		printf("\nError: Array of functions is not allowed.\nerror!\n");
		return;
	}
	printf("OK! type size : %d\n\n", size);
	printf("ValueType is : %s\n\n", &ValueType[first_char]);
}

// �����������Ǻ���ʱ�ļ���������
void function(char * type) {
	char ValueType[300];
	int para_i_right = 0, return_i_left = 0, paren_level = 0;
	abstract_parentheses(type, ValueType);
	for (int i = 0; i < strlen(ValueType); i++) {
		if (ValueType[i] == '(')
			paren_level++;
		else if (ValueType[i] == ')')
			paren_level--;
		else if (paren_level == 0 && ValueType[i] == '=' && ValueType[i + 1] == '>') {
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

// �ܵ��������
void output() {
	if (output_flag) {
		printf(iden_info[iden_info_processing].head);
		printf(iden_info[iden_info_processing].type);
		if (!fun_para_flag) {
			printf("\nType checking...");
			if (iden_info[iden_info_processing].type[0] == 'f')
				function(iden_info[iden_info_processing].type);
			else
				non_function(iden_info[iden_info_processing].type);
		}
		printf("\n");
	}
	else
		output_flag = 1;
}

void translation_unit() {
	if (sym == SYM_INT || sym == SYM_VOID) {
		init_iden_info(0);
		iden_info_processing = 0;
		right_parentheses = 0;
		specifier = 0;
		stack_top = 0;
		father = 0;
		fun_para_flag = 0;
		output_flag = 1;
		declaration();
		translation_unit();
	}
}

void declaration() {
	declaration_specifiers();
	init_declarator_list();
	if (sym == SYM_SEMICOLON)
		getsym();
}

void declaration_specifiers() {
	type_specifier();
}

void init_declarator_list() {
	init_declarator();
	_init_declarator_list();
}

void _init_declarator_list() {
	if (specifier == SYM_INT)
		add_output(TYPE, 1, "int");
	else if (specifier == SYM_VOID)
		add_output(TYPE, 1, "void");
	char str[100];
	int i;
	for (i = 0; i < right_parentheses; i++)
		str[i] = ')';
	str[i] = '\0';
	add_output(TYPE, 1, str);

	output();
	if (fun_para_flag == 0) {
		stack_top = 0;
		iden_info_processing = 0;
		right_parentheses = 0;
		init_iden_info(iden_info_processing);
	}
	if (sym == SYM_COMMA) {
		getsym();
		init_declarator();
		_init_declarator_list();
	}
}

void init_declarator() {
	declarator();
}

void type_specifier() {
	if (sym == SYM_INT) {
		specifier = SYM_INT;
		getsym();
	}
	else if (sym == SYM_VOID) {
		specifier = SYM_VOID;
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
		// * �ŵ����ȼ���� [] �� () �ϵ�, ���������[]��()�ķ�����, �������ǵ�������Ϣ��, �ټ���ָ���������Ϣ
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
		stack_top++;
		if (fun_para_flag)
			add_output(HEAD, 3, "parameter ", id, " is type of: ");
		else
			add_output(HEAD, 2, id, " is type of: ");
		getsym();
	}

	// Χ����identifier���ҵ�����, �뺯���޹�, �������⴦��
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
			// ����array�����ͺʹ�С, ����������һ
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

			push_iden_info();
			right_parentheses = 0;
			father = iden_info_processing - 1;
			// ����һ��identifier���������, ��ζ�ź����identifier����ǰ���identifier(father)�����Ĳ����б�, �ȴ�������
			// identifier, �õ�������Ϣ, Ȼ��������identifier, �����������Ҹ���fatherΪiden_info_processing,
			// push���浱ǰ�����iden_info����Ϣ,���һ�ʹiden_info_processing����1

			getsym();
			fun_para_flag += 1;
			if (sym == SYM_INT || sym == SYM_VOID)
				parameter_type_list();

			if (sym == SYM_RPAREN) {
				getsym();
				fun_para_flag -= 1;

				// �����б����, Ҫ����������Ϣ����������Ӧ��iden_info,
				// ����iden_info_processing����Ϊ��ǰ�����iden_info��direct_father
				iden_info_processing = father;
				father = iden_info[iden_info_processing].direct_father;
				right_parentheses = iden_info[iden_info_processing].right_parentheses;
				add_output(TYPE, 1, "function(");
				right_parentheses++;

				// ������iden_info��������Ϣ����iden_info
				for (int i = iden_info_processing + 1; i < stack_top - 1; i++) {
					add_output(TYPE, 2, iden_info[i].type, " X ");
				}
				if (stack_top - 1 > iden_info_processing)
					add_output(TYPE, 1, iden_info[stack_top - 1].type);

				//��ʼ���˵�ǰ���ڴ���ĸ�iden_info�Ժ��������iden_info, �������ڵ�ջ�������ڴ����iden_info���±�+1
				init_iden_info(iden_info_processing + 1);
				add_output(TYPE, 1, " => ");
				specifier = iden_info[iden_info_processing].specifier;
				stack_top = iden_info_processing + 1;
			}
			else {
				declaration_err();
				output_flag = 0;
				printf("\nUnpaired parentheses.\n");
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
	parameter_declaration();
	_parameter_list();
}

void _parameter_list() {
	if (specifier == SYM_INT)
		add_output(TYPE, 1, "int");
	else if (specifier == SYM_VOID)
		add_output(TYPE, 1, "void");
	char str[100];
	int i;
	for (i = 0; i < right_parentheses; i++)
		str[i] = ')';
	str[i] = '\0';
	add_output(TYPE, 1, str);
	output();
	if (sym == SYM_COMMA) {
		push_iden_info();
		// ֻpush, ������father, ʹ�ý�Ҫ�������һ��identifier��fatherҲ�ǵ�ǰ�����identifier��father
		right_parentheses = 0;
		getsym();
		specifier = sym;
		parameter_declaration();
		_parameter_list();
	}
}

void parameter_declaration() {
	declaration_specifiers();
	declarator();
}

void main()
{
	char s[80];
	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
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
} // main