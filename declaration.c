#include <stdio.h>
#include "pl0.h"

typedef struct{
    int TYPE;
    int SIZE;
}type_info;

void translation_unit(){
    declaration();
    translation_unit();
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

void init_declaration_list(){
    init_declarator();
    _init_declarator_list();
}

void init_declarator(){
    declarator();
}

void type_specifier(){
    if(sym == SYM_INT){
        getsym();
    }
    else if(sym == SYM_VOID){
        getsym();
    }
    else{
        /*Some error information*/
    }
}

void declarator(){
    if(sym == SYM_TIMES){
        pointer();
        direct_declarator();
    }
    else{
        direct_declarator();
    }
}

void direct_declarator(){
    if(sym == SYM_IDENTIFIER){
        getsym();
        _direct_declarator();
    }
    else if(sym == SYM_LPAREN){
        getsym();
        declarator();
        if(sym == SYM_RPAREN)
            getsym();
        else{
            /*Some error information.*/
        }
    }
}

void _direct_declarator(){
    if(sym == SYM_LSQBRACKET || sym == SYM_LPAREN){
        if(sym == SYM_LSQBRACKET){
            getsym();
            if(sym == SYM_NUMBER){
                getsym();
            }
            else{
                /*Some error*/
            }
         if(sym == SYM_RSQBRACKET){
                getsym();
            }
            else{
                /*Some error*/
            }
        }
        else if(sym == SYM_LPAREN){
            getsym();
            if(sym == SYM_INT || sym == SYM_VOID)
                parameter_type_list();
            if(sym == SYM_RPAREN)
                getsym();
            else{
                /*Some error*/
            }
        }
        _direct_declarator();
    }
}

void pointer(){
    if(sym == SYM_TIMES){
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
    if(sym == SYM_COMMA){
        getsym();
        parameter_declaration();
        _parameter_list();
    }
}

void parameter_declaration(){
    declaration_specifiers();
    declarator();
}

