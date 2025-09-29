#define ASCII_FULL

#include "pico/stdlib.h"
#include <stdio.h>
#include "ssd1306.h"
#include <string.h>
#include "display7segments.h"
#include "pico/multicore.h"
#include "json11.hpp"
#include "button.h"
#include "led.h"

const uint startLineLength = 8;
const char eof = 255;

static char * getLine(bool fullDuplex = true, char lineBreak = '\n') {
    char * pStart = (char*)malloc(startLineLength);
    char * pPos = pStart;
    size_t maxLen = startLineLength;
    size_t len = maxLen;
    int c;
    if(!pStart) {
        return NULL;
    }
    while(1) {
        c = getchar();
        if(c == eof || c == lineBreak) {
            break;
        }
        if (fullDuplex) {
            putchar(c);
        }
        if(--len == 0) {
            len = maxLen;
            char *pNew  = (char*)realloc(pStart, maxLen *= 2);
            if(!pNew) {
                free(pStart);
                return NULL;
            }
            pPos = pNew + (pPos - pStart);
            pStart = pNew;
        }
        if((*pPos++ = c) == lineBreak) {
            break;
        }
    }
    *pPos = '\0';
    return pStart;
}

char getChar(int char_index){
    if(char_index >= 0 && char_index < 25) {
        return (char)(65 + char_index);
    } else if(char_index >= 25 && char_index < 50) {
        return (char)(97 + char_index - 25);
    } else if(char_index >= 50 && char_index < 60) {
        return (char)(48 + char_index - 50);
    }
    return ' ';
}
