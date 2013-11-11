/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2013 Surplus Users Ham Society
 *
 *   Real VMX is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Real VMX is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
 */
/* shell.slex - Parser tables and lexer for shell */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <a.out.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <ostool/private/shellLibP.h>
#include "shellParser.h"
/* Imports */
IMPORT SYMTAB_ID sysSymTable;
/* Locals */
LOCAL int         retractChar;
LOCAL int         lastChar;
LOCAL AUTO_STATE  autoState;
LOCAL char       *nextChar;
LOCAL char        tempStrings[MAX_SHELL_LINE];
LOCAL char       *nextTempString;
LOCAL void lexInit(
    void
    );
LOCAL void lexRetract(
    void
    );
LOCAL int lexScan(
    void
    );
LOCAL void lexError(
    char *str,
    char *msg
    );
LOCAL int stringToChar(
    char *str,
    char *pChar
    );
LOCAL char* addTempString(
    char *str
    );
LOCAL int getNum(
    char *str,
    char *fmt,
    VALUE *pValue
    );
LOCAL int getFloat(
    char *str,
    VALUE *pValue
    );
LOCAL int getString(
    char *str,
    int nChars,
    VALUE *pValue
    );
LOCAL int getChar(
    char *str,
    int nChars,
    VALUE *pValue
    );
LOCAL int getId(
    char *str,
    VALUE *pValue
    );
LOCAL int typeCast(
    char *str
    );
/* Macros */
#define RETRACT         lexRetract(); str[--nChars] = EOS

int lexActions(int state, char *str, int nChars, BOOL *pContinue)
{
  *pContinue = FALSE;
  switch(state) {
    case 1:
        { RETRACT; }
    break;
    case 2:
        { RETRACT; return str[0]; }
    break;
    case 3:
        {          return str[0]; }
    break;
    case 4:
        {          lexError(str, "invalid number"); return LEX_ERROR; }
    break;
    case 5:
        {          lexError(str, "invalid string"); return LEX_ERROR; }
    break;
    case 6:
        {          lexError(str, "invalid char"); return LEX_ERROR; }
    break;
    case 7:
        { RETRACT; return getNum(str, "%o", &yylval); }
    break;
    case 8:
        { RETRACT; return getNum(&str[2], "%x", &yylval); }
    break;
    case 9:
        { RETRACT; return getNum(&str[1], "%x", &yylval); }
    break;
    case 10:
        { RETRACT; return getNum(str, "%d", &yylval); }
    break;
    case 11:
        { RETRACT; return getFloat(str, &yylval); }
    break;
    case 12:
        { RETRACT; return getId(str, &yylval); }
    break;
    case 13:
        {          return getString(str, nChars, &yylval); }
    break;
    case 14:
        {          return getChar(str, nChars, &yylval); }
    break;
    case 15:
        {          return OR; }
    break;
    case 16:
        {          return AND; }
    break;
    case 17:
        {          return EQ; }
    break;
    case 18:
        {          return NE; }
    break;
    case 19:
        {          return GE; }
    break;
    case 20:
        {          return LE; }
    break;
    case 21:
        { RETRACT; return ROT_RIGHT; }
    break;
    case 22:
        { RETRACT; return ROT_LEFT; }
    break;
    case 23:
        {          return PTR; }
    break;
    case 24:
        {          return INCR; }
    break;
    case 25:
        {          return DECR; }
    break;
    case 26:
        {          return ADDA; }
    break;
    case 27:
        {          return SUBA; }
    break;
    case 28:
        {          return MULA; }
    break;
    case 29:
        {          return DIVA; }
    break;
    case 30:
        {          return MODA; }
    break;
    case 31:
        {          return SHLA; }
    break;
    case 32:
        {          return SHRA; }
    break;
    case 33:
        {          return ANDA; }
    break;
    case 34:
        {          return ORA; }
    break;
    case 35:
        {          return XORA; }
    break;
    case 36:
        {          return NL; }
    break;
    case 37:
        {          return ENDFILE; }
    break;
  }
  *pContinue = TRUE;
  return 0;
}

int lexNclasses = 27;

char lexClassTable[] = {
  26,
  25,  0,  0,  0, 26,  0,  0,  0,  0,  1, 25,  0,  0, 25,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   1, 16, 10,  0, 12, 23, 14, 11,  0,  0, 22, 20,  0, 19,  8, 21, 
   2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  0,  0, 18, 15, 17,  0, 
   0,  5,  5,  5,  5,  5,  5,  7,  7,  7,  7,  7,  7,  7,  7,  7, 
   7,  7,  7,  7,  7,  7,  7,  7,  6,  7,  7,  0,  9,  0, 24,  7, 
   0,  5,  5,  5,  5,  5,  5,  7,  7,  7,  7,  7,  7,  7,  7,  7, 
   7,  7,  7,  7,  7,  7,  7,  7,  6,  7,  7,  0, 13,  0,  0,  0, 
};

char lexStateTable[] = {
  -3, 1, 2, 8, 8,11,11,11, 9,-3,12,14, 6,19,20,21,22,23,24,25,26,16,27,28,29,-36,-37,
  -1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -10,-10, 3, 3,-4,-4, 4,-4,10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,
  -7,-7, 3, 3,-4,-4,-4,-4,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,
  -4,-4, 5, 5, 5, 5,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,
  -8,-8, 5, 5, 5, 5,-4,-4,-4,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,
  -4,-4, 7, 7, 7, 7,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,
  -9,-9, 7, 7, 7, 7,-4,-4,-4,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,
  -10,-10, 8, 8, 8,-4,-4,-4,10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,
  -11,-11,10,10,10,-4,-4,-4,-4,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,
  -11,-11,10,10,10,-4,-4,-4,-4,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,
  -12,-12,11,11,11,11,11,11,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,
  12,12,12,12,12,12,12,12,12,13,-13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,-5,-5,
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,-5,-5,
  14,14,14,14,14,14,14,14,14,15,14,-14,14,14,14,14,14,14,14,14,14,14,14,14,14,-6,-6,
  14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,-6,-6,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-29,-2,-2,-2,-2,-2,-2,17,-2,-2,-2,-2,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,18,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17, 0,17,17,17,17,17,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-15,-2,-34,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-16,-33,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-17,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-18,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-19,-2,30,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-20,-2,-2,31,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-27,-2,-23,-2,-25,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-26,-2,-2,-2,-2,-24,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-28,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-30,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-35,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
  -21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-32,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,
  -22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-31,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,
};
/******************************************************************************
 * lexInit - Initialize lex scanner
 *
 * RETURNS: N/A
 */
LOCAL void lexInit(
    void
    )
{
    retractChar = LEX_EMPTY;
}
/******************************************************************************
 * lexRetract - Retract last character
 *
 * RETURNS: N/A
 */
LOCAL void lexRetract(
    void
    )
{
    retractChar = lastChar;
}
/******************************************************************************
 * lexScan - Scanf input for lexme
 *
 * RETURNS: Next lexme
 */
LOCAL int lexScan(
    void
    )
{
    char str[MAX_SHELL_LINE + 1];
    int c, state, retAction, nChars;
    BOOL scanContinue;
    /* Do while scanContinue */
    do
    {
        /* If retract char */
        if (retractChar != LEX_EMPTY)
        {
            c = retractChar;
            retractChar = LEX_EMPTY;
        }
        else
        {
            c = *(nextChar++);
        }
        /* Reset state */
        state = 0;
        /* For all characters on one line */
        for (nChars = 0; nChars < MAX_SHELL_LINE; nChars++)
        {
            /* Store character */
            str[nChars] = c;
            /* Get state from state table */
            state = lexStateTable[state * lexNclasses + lexClassTable[c + 1]];
            /* If final state */
            if (state < 0)
            {
                /* Increase number of characters */
                nChars++;
                /* Break for loop */
                break;
            }
            /* Get next character */
            c = *(nextChar++);
        }
        /* Break out of loop when final state reached */
        state       = -state;
        str[nChars] =  EOS;
        lastChar    = c;
        retAction   = lexActions(state, str, nChars, &scanContinue);
    } while(scanContinue == TRUE);
    /* Return lex action */
    return retAction;
}
/******************************************************************************
 * lexError - Lex error function
 *
 * RETURNS: N/A
 */
LOCAL void lexError(
    char *str,
    char *msg
    )
{
    fprintf(stderr, "%s : %s\n", msg, str);
}
/******************************************************************************
 * lexNewLine - Initialize for new line
 *
 * RETURNS: N/A
 */
void lexNewLine(
    char *line
    )
{
    lexInit();
    nextChar       = line;
    nextTempString = tempStrings;
    autoState      = FIRST_LEXME;
}
/******************************************************************************
 * yylex - Parser for yacc
 *
 * RETURNS: Next lexme
 */
int yylex(
    void
    )
{
    static int keepCode;
    int code;
    /* Select auto state */
    switch (autoState) {
        /* First lex scan for line */
        case FIRST_LEXME:
            /* Scan for next lexme */
            code = lexScan();
            if (code == T_SYMBOL)
            {
                autoState = PR_OPEN;
            }
            else
            {
                autoState = NORMAL;
            }
            break;
        /* Parentesis not required */
        case NORMAL:
            /* Scan for next lexme */
            code = lexScan();
            if (code == ';')
            {
                autoState = FIRST_LEXME;
            }
            break;
        /* Searcing for parentesis open */
        case PR_OPEN:
            /* Scan for next lexme */
            code = lexScan();
            if (code == '(')
            {
                autoState = NORMAL;
            }
            else
            {
                keepCode  = code;
                code      = '(';
                autoState = PR_OPEN_DONE;
            }
            break;
            /* Foreced opening parentesis returned */
            case PR_OPEN_DONE:
            if ((keepCode == NL) || (keepCode == ';'))
            {
                code      = ')';
                autoState = PR_CLOSE_DONE;
            }
            else
            {
                code      = keepCode;
                autoState = PR_CLOSE;
            }
            break;
        /* Search for newline or semicolon */
        case PR_CLOSE:
            /* Scan for next lexme */
            code = lexScan();
            if ((code == NL) || (code == ';'))
            {
                keepCode  = code;
                code      = ')';
                autoState = PR_CLOSE_DONE;
            }
            break;
        /* Foreced closing parentesis returned */
        case PR_CLOSE_DONE:
            code      = keepCode;
            autoState = FIRST_LEXME;
            break;
        /* Default, unknown case */
        default:
            fprintf(stderr, "yylex: invalid state %#x\n", autoState);
            code = 0;
            break;
    }
    return code;
}
/******************************************************************************
 * stringToChar - Get character that might be escaped in string
 *
 * RETURNS: Number of characters processed
 */
LOCAL int stringToChar(
    char *str,
    char *pChar
    )
{
    int ret;
    int nr, nChars;
    char c;
    /* Reset number of characters processed */
    nChars = 1;
    /* If not an escaped character */
    if (*str != '\\')
    {
        *pChar = *str;
        ret = nChars;
    }
    else
    {
        /* Move on to next character in string */
        str++;
        /* If it is a 0-7 character */
        if ((*str >= '0') && (*str <= '7'))
        {
            sscanf(str, "%o", &nr);
            c = nr % 0400;
            while ((*str >= '0') && (*str <= '7'))
            {
                /* Advace number of characters processed */
                ++str;
                ++nChars;
            }
        }
        else
        {
            /* Increase number of characters processed */
            nChars++;
            /* Select control character */
            switch(*str)
            {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '\'':
                    c = '\'';
                    break;
                case '"':
                    c = '"';
                    break;
                case 'a':
                    c = 0x07;
                    break;
                case 'v':
                    c = 0x0b;
                    break;
                default:
                    c = *str;
                    break;
            }
        }
        /* Store character */
        *pChar = c;
        ret    = nChars;
    }
    return ret;
}
/******************************************************************************
 * addTempString - Add a temporary string
 *
 * RETURNS: Pointer to string just stored
 */
LOCAL char* addTempString(
    char *str
    )
{
    char *newString;
    /* Store pointer to current string storage area */
    newString = nextTempString;
    /* Copy string */
    while (*str != EOS)
    {
        str += stringToChar(str, nextTempString++);
    }
    *(nextTempString++) = EOS;
    return newString;
}
/******************************************************************************
 * getNum - Get number from scanned string
 *
 * RETURNS: NUMBER
 */
LOCAL int getNum(
    char *str,
    char *fmt,
    VALUE *pValue
    )
{
    /* Setup value identifier fields */
    pValue->side = RHS;
    pValue->type = T_INT;
    /* Get number */
    sscanf(str, fmt, &pValue->value.rv);
    return NUMBER;
}
/******************************************************************************
 * getFloat - Get floating point number from scanned string
 *
 * RETURNS: FLOAT
 */
LOCAL int getFloat(
    char *str,
    VALUE *pValue
    )
{
#ifdef INCLUDE_FLOAT
    /* Setup value identifier fields */
    pValue->side = RHS;
    pValue->type = T_DOUBLE;
    /* Get floating point number */
    sscanf(str, "%lf", &pValue->value.dp);
#endif /* INCLUDE_FLOAT */
    return FLOAT;
}
/******************************************************************************
 * getString - Get string quoted in scanned string
 *
 * RETURNS: STRING
 */
LOCAL int getString(
    char *str,
    int nChars,
    VALUE *pValue
    )
{
    /* Setup value identifier fields */
    pValue->side = RHS;
    pValue->type = T_INT;
    /* Terminate string before end quote */
    str[nChars - 1] = EOS;
    /* Store string after staring quote */
    pValue->value.rv = (int) addTempString(&str[1]);
    return STRING;
}
/******************************************************************************
 * getChar - Get character from scanned string
 *
 * RETURNS: CHAR or LEX_ERROR
 */
LOCAL int getChar(
    char *str,
    int nChars,
    VALUE *pValue
    )
{
    int ret;
    char c;
    int nr;
    /* Get number of processed characters in string after starting quote */
    nr = stringToChar(&str[1], &c);
    /* If invalid charater */
    if (nChars != (nr + 2))
    {
        lexError(str, "invalid char");
        ret = LEX_ERROR;
    }
    else
    {
        /* Setup value identifiers in struct */
        pValue->side = RHS;
        pValue->type = T_BYTE;
        /* Store character */
        pValue->value.byte = c;
        ret = CHAR;
    }
    return ret;
}
/******************************************************************************
 * getId - Match scanned string against identifier or keyword
 *
 * RETURNS: TYPECAST, T_SYMBOL, D_SYMBOL or U_SYMBOL
 */
LOCAL int getId(
    char *str,
    VALUE *pValue
    )
{
    int ret;
    char s[MAX_SHELL_LINE + 1];
    void *value;
    int type, taskId;
    SYM_TYPE sType;
    /* Get typecast */
    type = typeCast(str);
    if (type != ERROR)
    {
        pValue->type = (TYPE) type;
        ret = TYPECAST;
    }
    else
    {
        /* Copy symbol name to string with preceding underscore */
        s[0] = '_';
        strncpy(&s[1], str, MAX_SHELL_LINE);
        s[MAX_SHELL_LINE] = EOS;
        /* If symbol without preceding underscore found */
        if (symFindByName(sysSymTable, &s[1], &value, &sType) == OK)
        {
            /* Setup value struct */
            pValue->type     = T_INT;
            pValue->side     = LHS;
            pValue->value.lv = (int *) value;
            ret = D_SYMBOL;
        }
        else if (symFindByName(sysSymTable, &s[0], &value, &sType) == OK)
        {
            /* Setup value struct */
            pValue->type     = T_INT;
            pValue->side     = LHS;
            pValue->value.lv = (int *) value;
            /* Check if it is executable code */
            if ((sType & N_TEXT) == N_TEXT)
            {
                ret = T_SYMBOL;
            }
            else
            {
                ret = D_SYMBOL;
            }
        }
        else if ((taskId = taskNameToId(&s[1])) != ERROR)
        {
            /* Setup value struct */
            pValue->type     = T_INT;
            pValue->side     = LHS;
            pValue->value.lv = (int *) taskId;
            ret = T_SYMBOL;
        }
        else {
            /* Id unidentified */
            pValue->side     = RHS;
            pValue->type     = T_UNKNOWN;
            pValue->value.rv = (int) addTempString(str);
            ret = U_SYMBOL;
        }
    }
    return ret;
}
/******************************************************************************
 * typeCast - Check if scanned string is a typecast
 *
 * RETURNS: T_BYTE, T_WORD, T_INT, T_FLOAT, T_DOUBLE, or ERROR
 */
LOCAL int typeCast(
    char *str
    )
{
    static char *typeName[] =
    {
#ifdef INCLUDE_FLOAT
        "char", "short", "int", "long", "float", "double"
#else
        "char", "short", "int", "long"
#endif /* INCLUDE_FLOAT */
    };
    static TYPE typeType[] =
    {
#ifdef INCLUDE_FLOAT
        T_BYTE, T_WORD, T_INT, T_INT, T_FLOAT, T_DOUBLE
#else
        T_BYTE, T_WORD, T_INT, T_INT
#endif /* INCLUDE_FLOAT */
    };
    int i;
    int ret = ERROR;
    /* For all types */
    for (i = 0; i < NELEMENTS(typeType); i++)
    {
        if (strcmp(str, typeName[i]) == 0)
        {
            ret = (int) typeType[i];
            break;
        }
    }
    return ret;
}
