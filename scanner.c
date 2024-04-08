/*
 * File: scanner.c
 * Author: Jonathan Silvestri
 * Purpose: Generates a token upon request and returns the lexeme
 *          and a token for the parser
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

int line_number = 1;
char* lexeme = NULL;

int get_token() {
    char c;
    c =  getchar();

    if (c == '\n'){
        line_number++;
    }

    // skip whitespace and newlines
    while(c == ' ' || c == '\n' || c == '\t'){
        c = getchar();
        if (c == '\n'){
            line_number++;
        }
    }

    if (c == EOF){
        return EOF;
        line_number++;
    }

    // allocate a buffer for the characters
    char* buffer = (char*)malloc(128 * sizeof(char));
    int buffer_index = 0;

    // add current char to buffer and increment index
    buffer[buffer_index++] = c;

    if (c == '/'){
        c = getchar();
        // case for when following character is a digit or ID "/2", "/ foo"
        if (c != '*'){
            lexeme = strdup(buffer);
            ungetc(c, stdin);
            return opDIV;
        }
        while (1){
            c = getchar();
            if (c == '\n'){
                line_number++;
            }
            if (c == '*'){
                c = getchar();
                if (c == '/'){
                    return get_token();
                }
                ungetc(c, stdin);
            }
            if (c == EOF){
                exit(0);
            }
        }
    }
    // resets previous lexeme value
    free(lexeme);
    lexeme = strdup(buffer);

    if (isalpha(c)){
        // while c is alphanumeric or '_'
        while((c = getchar()) != EOF){
            if (!isalpha(c) && !isdigit(c) && (c != '_')){
                break;
            }
            buffer[buffer_index++] = c;
        }
        // once we find a non-alphanumeric char
        lexeme = strdup(buffer);
        ungetc(c, stdin);

        if(strcmp(lexeme,"int") == 0){
            return kwINT;
        }
        if(strcmp(lexeme,"if") == 0){
            return kwIF;
        }
        if(strcmp(lexeme,"else") == 0){
            return kwELSE;
        }
        if(strcmp(lexeme,"while") == 0){
            return kwWHILE;
        }
        if(strcmp(lexeme,"return") == 0){
            return kwRETURN;
        }
        return ID;
    }
    else if(isdigit(c)){
        while ((c = getchar()) != EOF){
            // set lexeme and return INTCON as soon as a non-integer character is found
            if (!(isdigit(c))){
                lexeme = strdup(buffer);
                ungetc(c, stdin);
                return INTCON;
            }
            // if c is a digit, add to buffer
            buffer[buffer_index++] = c;
        }
    }

    else if (c == '('){
        return LPAREN;
    }

    else if (c == ')'){
        return RPAREN;
    }

    else if (c == '{'){
        return LBRACE;
    }

    else if (c == '}'){
        return RBRACE;
    }

    else if (c == ','){
        return COMMA;
    }

    else if (c == ';'){
        return SEMI;
    }

    else if (c == '='){
        c = getchar();
        if (c == '='){
            buffer[buffer_index++] = c;
            lexeme = strdup(buffer);
            return opEQ;
        }
        lexeme = strdup(buffer);
        ungetc(c, stdin);
        return opASSG;
    }

    else if (c == '+'){
        return opADD;
    }

    else if (c == '-'){
        return opSUB;
    }

    else if (c == '*'){
        return opMUL;
    }

    else if (c == '!'){
        c = getchar();
        if (c == '='){
            buffer[buffer_index++] = c;
            lexeme = strdup(buffer);
            return opNE;
        }
        ungetc(c, stdin);
    }

    else if (c == '>'){
        c = getchar();
        if (c == '='){
            buffer[buffer_index++] = c;
            lexeme = strdup(buffer);
            return opGE;
        }
        lexeme = strdup(buffer);
        ungetc(c, stdin);
        return opGT;
    }

    else if (c == '<'){
        c = getchar();
        if (c == '='){
            buffer[buffer_index++] = c;
            lexeme = strdup(buffer);
            return opLE;
        }
        lexeme = strdup(buffer);
        ungetc(c, stdin);
        return opLT;
    }

    else if (c == '&'){
        c = getchar();
        
        if (c == '&'){
            buffer[buffer_index++] = c;
            lexeme = strdup(buffer);
            return opAND;
        }
        ungetc(c, stdin);
        return UNDEF;

    }
    else if (c == '|'){
        c = getchar();
        if (c == '|'){
            buffer[buffer_index++] = c;
            lexeme = strdup(buffer);
            return opOR;
        }
        ungetc(c, stdin);
        return UNDEF;
    }
    else {
        return UNDEF;
    }
}