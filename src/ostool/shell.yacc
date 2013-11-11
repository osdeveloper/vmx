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

/* shell.yacc - Yacc grammar for shell */

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <a.out.h>
#include <vmx.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <os/ioLib.h>

#include <ostool/private/shellLibP.h>

/* Imports */
IMPORT SYMTAB_ID sysSymTable;
IMPORT int       redirInFd;
IMPORT int       redirOutFd;

/* Locals */
LOCAL BOOL  semError;
LOCAL BOOL  usymUnprocessed;
LOCAL VALUE usymValue;
LOCAL int   nArgs;
LOCAL ARG   argStack[MAX_SHELL_ARGS];
LOCAL BOOL  spawnFlag;

LOCAL void yyerror(
    char *str
    );

LOCAL void printSym(
    int val,
    char *prefix,
    char *suffix
    );

LOCAL BOOL checkLHSValue(
    VALUE *pValue
    );

LOCAL BOOL checkRHSValue(
    VALUE *pValue
    );

LOCAL ARG getLHSValue(
    VALUE *pValue
    );

LOCAL VALUE* getRHSValue(
    VALUE *pValue,
    VALUE *pResult
    );

LOCAL void printLHSValue(
    VALUE *pValue
    );

LOCAL void printRHSValue(
    VALUE *pValue
    );

LOCAL void printValue(
    VALUE *pValue
    );

LOCAL int newString(
    char *str
    );

LOCAL VALUE newSym(
    char *name,
    TYPE type
    );

LOCAL void convUnknown(
    VALUE *pValue
    );

LOCAL void convByte(
    VALUE *pValue
    );

LOCAL void convWord(
    VALUE *pValue
    );

LOCAL void convInt(
    VALUE *pValue
    );

#ifdef INCLUDE_FLOAT
LOCAL void convFloat(
    VALUE *pValue
    );

LOCAL void convDouble(
    VALUE *pValue
    );
#endif /* INCLUDE_FLOAT */

LOCAL void typeConvert(
    VALUE *pValue,
    TYPE type,
    SIDE side
    );

LOCAL void assign(
    VALUE *pLHSValue,
    VALUE *pRHSValue
    );

LOCAL VALUE evalExp(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

LOCAL void setLHSValue(
    VALUE *pValue1,
    VALUE *pValue2
    );

LOCAL void setRHSValue(
    VALUE *pValue1,
    VALUE *pValue2
    );

LOCAL void binOp(
    VALUE *pResult,
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

LOCAL VALUE evalUnknown(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

LOCAL VALUE evalByte(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

LOCAL VALUE evalWord(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

LOCAL VALUE evalInt(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

#ifdef INCLUDE_FLOAT
LOCAL VALUE evalFloat(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );

LOCAL VALUE evalDouble(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    );
#endif /* INCLUDE_FLOAT */

LOCAL VALUE newArgList(
    void
    );

LOCAL void addArg(
    VALUE *pArgList,
    VALUE *pNewArg
    );

LOCAL VALUE funcCall(
    VALUE *pValue,
    VALUE *pArgList
    );

%}

%start line

%token NL 0
%token T_SYMBOL D_SYMBOL U_SYMBOL NUMBER CHAR STRING FLOAT
%token OR AND EQ NE GE LE INCR DECR ROT_LEFT ROT_RIGHT UMINUS PTR TYPECAST
%token ENDFILE LEX_ERROR

%left '=' MULA DIVA MODA ADDA SUBA SHLA SHRA ANDA ORA XORA
%right '?' ':'
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQ NE
%left GE LE '>' '<'
%left ROT_LEFT ROT_RIGHT
%left '+' '-'
%left '*' '/' '%'
%left INCR DECR
%left UNARY
%left PTR '[' '('
%left TYPECAST

%%

line    : stmt
        | stmt ';' line
        ;

stmt    :
        | expr                  { printValue(&$1);
                                  CHECK; }
        ;

expr    : D_SYMBOL
        | T_SYMBOL              { $1.side = RHS; setRHSValue(&$$, &$1); }
        | STRING                { $$ = $1;
                                  $$.value.rv = newString((char *) $1.value.rv);
                                  CHECK; }
        | CHAR
        | NUMBER
        | FLOAT
        | '(' expr ')'          { $$ = $2; }
        | expr '(' arglist ')'  { $$ = funcCall(&$1, &$3);
                                  CHECK; }
        | typecast expr %prec TYPECAST 
                                {
                                  typeConvert(&$2, $1.type, $1.side);
                                  $$ = $2;
                                  CHECK;
                                }
        | '*' expr %prec UNARY  { VALUE val;
                                  getRHSValue(&$2, &val);
                                  setLHSValue(&$$, &val);
                                  CHECK; }
        | '&' expr %prec UNARY  { $$.value.rv = (int) getLHSValue(&$2);
                                  $$.type = T_INT; $$.side = RHS; }
        | '-' expr %prec UNARY  { binOp(&$$, &$2, UMINUS, NULLVAL); }
        | '!' expr %prec UNARY  { binOp(&$$, &$2, '!', NULLVAL); }
        | '~' expr %prec UNARY  { binOp(&$$, &$2, '~', NULLVAL); }
        | expr '?' expr ':' expr { VALUE val;
                                   setRHSValue(&$1, &val);
                                   setRHSValue(&$$, val.value.rv ? &$3 : &$5); }
        | expr '[' expr ']'     { binOp(&$$, &$1, '+', &$3);
                                  typeConvert(&$$, T_INT, RHS);
                                  setLHSValue(&$$, &$$); }
        | expr PTR expr         { binOp(&$$, &$1, '+', &$3);
                                  typeConvert(&$$, T_INT, RHS);
                                  setLHSValue(&$$, &$$); }
        | expr '+' expr         { binOp(&$$, &$1, '+', &$3); }
        | expr '-' expr         { binOp(&$$, &$1, '-', &$3); }
        | expr '*' expr         { binOp(&$$, &$1, '*', &$3); }
        | expr '/' expr         { binOp(&$$, &$1, '/', &$3); }
        | expr '%' expr         { binOp(&$$, &$1, '%', &$3); }
        | expr ROT_LEFT expr    { binOp(&$$, &$1, ROT_LEFT, &$3); }
        | expr ROT_RIGHT expr   { binOp(&$$, &$1, ROT_RIGHT, &$3); }
        | expr '&' expr         { binOp(&$$, &$1, '&', &$3); }
        | expr '^' expr         { binOp(&$$, &$1, '^', &$3); }
        | expr '|' expr         { binOp(&$$, &$1, '|', &$3); }
        | expr AND expr         { binOp(&$$, &$1, AND, &$3); }
        | expr OR expr          { binOp(&$$, &$1, OR, &$3); }
        | expr EQ expr          { binOp(&$$, &$1, EQ, &$3); }
        | expr NE expr          { binOp(&$$, &$1, NE, &$3); }
        | expr GE expr          { binOp(&$$, &$1, GE, &$3); }
        | expr LE expr          { binOp(&$$, &$1, LE, &$3); }
        | expr '>' expr         { binOp(&$$, &$1, '>', &$3); }
        | expr '<' expr         { binOp(&$$, &$1, '<', &$3); }
        | INCR expr %prec UNARY { binOp(&$$, &$2, INCR, NULLVAL);
                                  assign(&$2, &$$);
                                  CHECK; }
        | DECR expr %prec UNARY { binOp(&$$, &$2, DECR, NULLVAL);
                                  assign(&$2, &$$);
                                  CHECK; }
        | expr INCR %prec UNARY { VALUE val;
                                  val = $1;
                                  binOp(&$$, &$1, INCR, NULLVAL);
                                  assign(&$1, &$$);
                                  CHECK;
                                  $$ = val; }
        | expr DECR %prec UNARY { VALUE val;
                                  val = $1;
                                  binOp(&$$, &$1, DECR, NULLVAL);
                                  assign(&$1, &$$);
                                  CHECK;
                                  $$ = val; }
        | expr ADDA expr        { binOp(&$$, &$1, ADDA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr SUBA expr        { binOp(&$$, &$1, SUBA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr ANDA expr        { binOp(&$$, &$1, ANDA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr ORA expr         { binOp(&$$, &$1, ORA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr MODA expr        { binOp(&$$, &$1, MODA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr XORA expr        { binOp(&$$, &$1, XORA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr MULA expr        { binOp(&$$, &$1, MULA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr DIVA expr        { binOp(&$$, &$1, DIVA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr SHLA expr        { binOp(&$$, &$1, SHLA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr SHRA expr        { binOp(&$$, &$1, SHRA, &$3);
                                  assign(&$1, &$$);
                                  CHECK; }
        | expr '=' expr         { assign(&$1, &$3); $$ = $1; }
        | U_SYMBOL
                                { usymUnprocessed = TRUE; usymValue = $1; }
          '=' expr              {
                                  if ($1.type != T_UNKNOWN) {
                                    fprintf(stderr, "typecast of lhs not allowed.\n");
                                    YYERROR;
                                  }
                                  else {
                                    $$ = newSym((char*) $1.value.rv, $4.type);
                                    CHECK;
                                    assign(&$$, &$4);
                                    CHECK;
                                  }
                                  usymUnprocessed = FALSE;
                                }
        ;

arglist :
                                { $$ = newArgList(); }
        | nArgList
        ;

nArgList: expr                  { $$ = newArgList(); addArg(&$$, &$1);
                                  CHECK; }
        | nArgList ',' expr     { addArg(&$1, &$3);
                                  CHECK; }
        ;

typecast: '(' TYPECAST ')'      { $2.side = RHS; $$ = $2; }
        | '(' TYPECAST '(' ')' ')' { $2.side = FHS; $$ = $2; }
        ;

%%

/******************************************************************************
 * yystart - Called before the shell is started
 *
 * RETURNS: N/A
 */

void yystart(
    char *line
    )
{
    lexNewLine(line);
    semError        = FALSE;
    usymUnprocessed = FALSE;
    nArgs           = 0;
    spawnFlag       = FALSE;
}

/******************************************************************************
 * yyerror - Called by yacc whenever an error occurs
 *
 * RETURNS: N/A
 */

LOCAL void yyerror(
    char *str
    )
{
    fprintf(stderr, "error: %s\n", str);
}

/******************************************************************************
 * printSym - Print symbol value
 *
 * RETURNS: N/A
 */

LOCAL void printSym(
    int val,
    char *prefix,
    char *suffix
    )
{
    ARG value;
    SYMBOL_ID symId;
    char *name;

    /* If val not minus one */
    if (val != -1)
    {
        /* Find symbol by value and get name and value */
        if (symFindSymbol(
                sysSymTable,
                NULL,
                (ARG) val,
                SYM_MASK_NONE,
                SYM_MASK_NONE,
                &symId) == OK
                )
        {
            /* Get symbol name */
            symNameGet(symId, &name);

            /* Get symbol value */
            symValueGet(symId, &value);

            /* If value range resonable */
            if ((value != (ARG) 0) && ((val - (int) value) < 0x1000))
            {
                printf(prefix);

                if (val == (int) value)
                {
                    printf("%s", name);
                }
                else
                {
                    printf("%s + 0x%x", name, val - (int) value);
                }

                printf(suffix);
            }
        }
    }
}

/******************************************************************************
 * checkLHSValue - Check if value is a left hand side value
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL checkLHSValue(
    VALUE *pValue
    )
{
    BOOL ret;

    /* If not left hand side */
    if (pValue->side != LHS)
    {
        fprintf(stderr, "invalid application of 'address of' operator.\n");
        SET_ERROR;
        ret = FALSE;
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}

/******************************************************************************
 * checkRHSValue - Check if value is a right hand side value
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL checkRHSValue(
    VALUE *pValue
    )
{
    BOOL ret;

    /* If left hand side */
    if (pValue->side == LHS)
    {
        ret = checkLHSValue(pValue);
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}

/******************************************************************************
 * getLHSValue - Get left hand side value
 *
 * RETURNS: Value or zero
 */

LOCAL ARG getLHSValue(
    VALUE *pValue
    )
{
    ARG ret;

    if (checkLHSValue(pValue) == TRUE)
    {
        ret = pValue->value.lv;
    }
    else
    {
        ret = (ARG) 0;
    }

    return ret;
}

/******************************************************************************
 * getRHSValue - Get right hand side value
 *
 * RETURNS: Pointer to value or zero
 */

LOCAL VALUE* getRHSValue(
    VALUE *pValue,
    VALUE *pResult
    )
{
    /* If it is a right hand side value */
    if (pValue->side == RHS)
    {
        *pResult = *pValue;
    }
    else
    {
        /* Setup result specifiers */
        pResult->side = RHS;
        pResult->type = pValue->type;

        /* Select type */
        switch(pValue->type)
        {
            case T_BYTE:
                pResult->value.byte = *(char *) pValue->value.lv;
                break;

            case T_WORD:
                pResult->value.word = *(short *) pValue->value.lv;
                break;

            case T_INT:
                pResult->value.rv = *pValue->value.lv;
                break;

#ifdef INCLUDE_FLOAT
            case T_FLOAT:
                pResult->value.fp = *(float *) pValue->value.lv;
                break;

            case T_DOUBLE:
                pResult->value.dp = *(double *) pValue->value.lv;
                break;
#endif /* INCLUDE_FLOAT */

            default:
                fprintf(stderr, "getRHSValue: invalid rhs.");
                SET_ERROR;
                break;
        }
    }

    return pResult;
}

/******************************************************************************
 * printLHSValue - Print left hand side value
 *
 * RETURNS: N/A
 */

LOCAL void printLHSValue(
    VALUE *pValue
    )
{
    ARG lv;

    /* Get left hand side value */
    lv = getLHSValue(pValue);

    /* Print symbol */
    printSym((int) lv, "", " = ");

    printf("0x%x", (unsigned int) lv);
}

/******************************************************************************
 * printRHSValue - Print right hand side value
 *
 * RETURNS: N/A
 */

LOCAL void printRHSValue(
    VALUE *pValue
    )
{
    VALUE value;
    int rv;
    BOOL intFlag;

    /* Get right hand side value */
    getRHSValue(pValue, &value);

    /* Select type */
    switch(pValue->type)
    {
        case T_BYTE:
            rv = (int) value.value.byte;
            intFlag = TRUE;
            break;

        case T_WORD:
            rv = (int) value.value.word;
            intFlag = TRUE;
            break;

        case T_INT:
            rv = value.value.rv;
            intFlag = TRUE;
            break;

#ifdef INCLUDE_FLOAT
        case T_FLOAT:
            printf("%g", value.value.fp);
            intFlag = FALSE;
            break;

        case T_DOUBLE:
            printf("%g", value.value.dp);
            intFlag = FALSE;
            break;
#endif /* INCLUDE_FLOAT */

        default:
            fprintf(stderr, "printRHSValue: bad type.\n");
            intFlag = FALSE;
            SET_ERROR;
            break;
    }

    /* Print integer value if needed */
    if (intFlag)
    {
        /* Print value */
        printf("%d = 0x%x", rv, rv);

        /* Print char code if it exists for value */
        if (__isascii(rv) && __isprint(rv))
        {
            printf(" = '%c'", rv);
        }

        /* Print symbol */
        printSym(rv, " = ", "");
    }
}

/******************************************************************************
 * printValue - Print value
 *
 * RETURNS: N/A
 */

LOCAL void printValue(
    VALUE *pValue
    )
{
    /* If left hand side */
    if (pValue->side == LHS) {
        if ((checkLHSValue(pValue) == TRUE) &&
            (checkRHSValue(pValue) == TRUE))
        {
            printLHSValue(pValue);
            printf(": value = ");
            printRHSValue(pValue);
            printf("\n");
        }
        else
        {
            fprintf(stderr, "invalid lhs.\n");
            SET_ERROR;
        }
    }
    else if (checkRHSValue(pValue) == TRUE)
    {
        printf("value = ");
        printRHSValue(pValue);
        printf("\n");
    }
    else
    {
        fprintf(stderr, "invalid rhs.\n");
        SET_ERROR;
    }
}

/******************************************************************************
 * newString - Create a new string
 *
 * RETURNS: Integer pointer to string
 */

LOCAL int newString(
    char *str
    )
{
    char *newStr;
    int len;

    /* Get length */
    len = strlen(str);

    /* Allocate storage */
    newStr = malloc(len);
    if (str == NULL)
    {
        fprintf(stderr, "not enough memory for string.\n");
        SET_ERROR;
    }
    else
    {
        strcpy(newStr, str);
    }

    return (int) newStr;
}

/******************************************************************************
 * newSym - Create a new symbol
 *
 * RETURNS: Symbol value
 */

LOCAL VALUE newSym(
    char *name,
    TYPE type
    )
{
    VALUE value;
    void *pAddr;

    pAddr = malloc(MAX_SYMBOL_SIZE);
    if (pAddr == NULL)
    {
        fprintf(stderr, "not enough memory for new symbol.\n");
        SET_ERROR;
    }
    else
    {
        if (symAdd(
                sysSymTable,
                name,
                pAddr,
                (N_BSS | N_EXT),
                symGroupDefault
                ) != OK)
        {
            free(pAddr);
            fprintf(
                stderr,
                "can't add '%s' to system symbol table - error = 0x%x.\n",
                name,
                errnoGet()
                );
            SET_ERROR;
        }

        printf("new symbol \"%s\" added to symbol table.\n", name);

        value.side     = LHS;
        value.type     = type;
        value.value.lv = (int *) pAddr;
    }

    return value;
}

/******************************************************************************
 * convUnknown - Convert value to unknown type
 *
 * RETURNS: N/A
 */

LOCAL void convUnknown(
    VALUE *pValue
    )
{
    fprintf(stderr, "convUnknown: bad type.\n");

    SET_ERROR;
}

/******************************************************************************
 * convByte - Convert value to byte type
 *
 * RETURNS: N/A
 */

LOCAL void convByte(VALUE *pValue)
{
    char value;

    /* If type is greater than byte */
    if (pValue->type > T_BYTE)
    {
        /* First convert type down to word */
        convWord(pValue);

        /* Get byte value from word value */
        value = (char) pValue->value.word;

        /* Set byte value */
        pValue->value.byte = value;

        /* Set type to byte */
        pValue->type = T_BYTE;
    }
}

/******************************************************************************
 * convWord - Convert value to word type
 *
 * RETURNS: N/A
 */

LOCAL void convWord(
    VALUE *pValue
    )
{
    short value;

    /* If type is smaller than word */
    if (pValue->type < T_WORD)
    {
        /* Get word value from byte value */
        value = (short) pValue->value.byte;

        /* Set word value */
        pValue->value.word = value;

        /* Set type to word */
        pValue->type = T_WORD;
    }
    else if (pValue->type > T_WORD)
    {
        /* First convert type down to int */
        convInt(pValue);

        /* Get word value from int value */
        value = (short) pValue->value.rv;

        /* Set word value */
        pValue->value.word = value;

        /* Set type to word */
        pValue->type = T_WORD;
    }
}

/******************************************************************************
 * convInt - Convert value to integer type
 *
 * RETURNS: N/A
 */

LOCAL void convInt(
    VALUE *pValue
    )
{
    int value;

    /* If type is smaller than int */
    if (pValue->type < T_INT)
    {
        /* First convert type up to word */
        convWord(pValue);

        /* Get int value from word value */
        value = (int) pValue->value.word;

        /* Set int value */
        pValue->value.rv = value;

        /* Set type to int */
        pValue->type = T_INT;
    }
    else if (pValue->type > T_INT)
    {
#ifdef INCLUDE_FLOAT
        /* First convert type down to float */
        convFloat(pValue);

        /* Get int value from float value */
        value = (int) pValue->value.fp;

        /* Set int value */
        pValue->value.rv = value;

        /* Set type to int */
        pValue->type = T_INT;
#endif /* INCLUDE_FLOAT */
    }
}

#ifdef INCLUDE_FLOAT
/******************************************************************************
 * convFloat - Convert value to floating point type
 *
 * RETURNS: N/A
 */

LOCAL void convFloat(
    VALUE *pValue
    )
{
    float value;

    /* If type is smaller than float */
    if (pValue->type < T_FLOAT)
    {
        /* First convert type up to int */
        convInt(pValue);

        /* Get float value from int value */
        value = (float) pValue->value.rv;

        /* Set float value */
        pValue->value.fp = value;

        /* Set type to float */
        pValue->type = T_FLOAT;
    }
    else if (pValue->type > T_INT)
    {
        /* First convert type down to float */
        convDouble(pValue);

        /* Get float value from double value */
        value = (float) pValue->value.dp;

        /* Set float value */
        pValue->value.fp = value;

        /* Set type to float */
        pValue->type = T_FLOAT;
    }
}

/******************************************************************************
 * convDouble - Convert value to double precision floating point type
 *
 * RETURNS: N/A
 */

LOCAL void convFloat(
    VALUE *pValue
    )
{
    double value;

    /* If type is smaller than double */
    if (pValue->type < T_DOUBLE)
    {
        /* First convert type up to float */
        convFloat(pValue);

        /* Get double value from float value */
        value = (double) pValue->value.fp;

        /* Set double value */
        pValue->value.dp = value;

        /* Set type to double */
        pValue->type = T_DOUBLE;
    }
}
#endif /* INCLUDE_FLOAT */

LOCAL VOIDFUNCPTR convType[] =
{
    convUnknown,
    convByte,
    convWord,
#ifdef INCLUDE_FLOAT
    convInt,
    convFloat,
    convDouble
#else
    convInt
#endif /* INCLUDE_FLOAT */
};

/******************************************************************************
 * typeConvert - Convert value to specified data type
 *
 * RETURNS: N/A
 */

LOCAL void typeConvert(
    VALUE *pValue,
    TYPE type,
    SIDE side
    )
{
    /* If request convert to right hand side value */
    if (side == FHS)
    {
        pValue->side = RHS;
        pValue->type = type;
    }
    else if (side == RHS)
    {
        /* Convert if needed */
        if (pValue->side == LHS)
        {
            pValue->type = type;
        }
        else
        {
            (*convType[type])(pValue);
        }
    }
    else if (pValue->side == LHS)
    {
        pValue->type = type;
    }
    else
    {
        fprintf(stderr, "typeConvert: bad type.\n");
        SET_ERROR;
    }
}

/******************************************************************************
 * assign - Assign a value
 *
 * RETURNS: N/A
 */

LOCAL void assign(
    VALUE *pLHSValue,
    VALUE *pRHSValue
    )
{
    VALUE value;

    /* If value sideness okay */
    if ((checkLHSValue(pLHSValue) == TRUE) &&
        (checkRHSValue(pRHSValue) == TRUE))
    {
        /* Get right hand side value */
        getRHSValue(pRHSValue, &value);

        /* Convert to correct type */
        pLHSValue->type = pRHSValue->type;
        typeConvert(&value, pLHSValue->type, RHS);

        /* Select type */
        switch(pLHSValue->type)
        {
            case T_BYTE:
                *(char *) getLHSValue(pLHSValue) = value.value.byte;
                break;

            case T_WORD:
                *(short *) getLHSValue(pLHSValue) = value.value.word;
                break;

            case T_INT:
                *(int *) getLHSValue(pLHSValue) = value.value.rv;
                break;

#ifdef INCLUDE_FLOAT
            case T_FLOAT:
                *(float *) getLHSValue(pLHSValue) = value.value.fp;
                break;

            case T_DOUBLE:
                *(double *) getLHSValue(pLHSValue) = value.value.dp;
                break;
#endif /* INCLUDE_FLOAT */

            default:
                fprintf(stderr, "bad assignment.\n");
                SET_ERROR;
                break;
        }
    }
    else
    {
        fprintf(stderr, "bad assignment.\n");
        SET_ERROR;
    }
}

/******************************************************************************
 * setLHSValue - Set right hand side value
 *
 * RETURNS: N/A
 */

LOCAL void setLHSValue(
    VALUE *pValue1,
    VALUE *pValue2
    )
{
    /* If value2 not left hand sided */
    if (pValue2->side == LHS)
    {
        fprintf(stderr, "setLHSValue: invalid lhs.\n");
        SET_ERROR;
    }

    /* If type not int */
    if (pValue2->type != T_INT)
    {
        fprintf(stderr, "setLHSValue: type confilict.\n");
        SET_ERROR;
    }

    /* Setup struct */
    pValue1->side     = LHS;
    pValue1->type     = pValue2->type;
    pValue1->value.lv = (int *) pValue2->value.rv;
}

/******************************************************************************
 * setRHSValue - Set right hand side value
 *
 * RETURNS: N/A
 */

LOCAL void setRHSValue(
    VALUE *pValue1,
    VALUE *pValue2
    )
{
    /* Setup value1 */
    pValue1->side = RHS;
    pValue1->type = pValue2->type;

    /* Select type */
    switch(pValue1->type)
    {
        case T_BYTE:
            if (pValue1->side == LHS)
            {
                pValue1->value.byte = *(char *) pValue2->value.lv;
            }
            else
            {
                pValue1->value.byte = pValue2->value.byte;
            }
            break;

        case T_WORD:
            if (pValue1->side == LHS)
            {
                pValue1->value.word = *(short *) pValue2->value.lv;
            }
            else
            {
                pValue1->value.word = pValue2->value.word;
            }
            break;

        case T_INT:
            if (pValue1->side == LHS)
            {
                pValue1->value.rv = *pValue2->value.lv;
            }
            else
            {
                pValue1->value.rv = pValue2->value.rv;
            }
            break;

#ifdef INCLUDE_FLOAT
        case T_FLOAT:
            if (pValue1->side == LHS)
            {
                pValue1->value.fp = *(float *) pValue2->value.lv;
            }
            else
            {
                pValue1->value.fp = pValue2->value.fp;
            }
            break;

        case T_DOUBLE:
            if (pValue1->side == LHS)
            {
                pValue1->value.dp = *(double *) pValue2->value.lv;
            }
            else
            {
                pValue1->value.dp = pValue2->value.dp;
            }
            break;
#endif /* INCLUDE_FLOAT */

        default:
            fprintf(stderr, "setRHSValue: bad type.\n");
            SET_ERROR;
            break;
    }
}

typedef struct
{
    VALUE (*eval)(
              VALUE *pValue1,
              int op,
              VALUE *pValue2
              );
} EVAL_TYPE;

LOCAL EVAL_TYPE evalType[] =
{
    { evalUnknown },
    { evalByte },
    { evalWord },
#ifdef INCLUDE_FLOAT
    { evalInt },
    { evalFloat },
    { evalDouble }
#else
    { evalInt }
#endif /* INCLUDE_FLOAT */
};

/******************************************************************************
 * evalExp - Evaluate expression
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalExp(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    VALUE *pVal1 = pValue1;
    VALUE *pVal2 = pValue2;

    /* If unary expression */
    if (pValue2 == NULLVAL)
    {
        pValue2 = pValue1;
        pVal2   = pValue1;
    }

    /* Make both value in the expression have the same type */
    if (pVal1->type > pVal2->type)
    {
        typeConvert(pVal2, pVal1->type, pVal1->side);
    }
    else
    {
        typeConvert(pVal1, pVal2->type, pVal2->side);
    }

    return (*evalType[pValue1->type].eval)(pVal1, op, pVal2);
}

/******************************************************************************
 * binOp - Do binary operator
 *
 * RETURNS: N/A
 */

LOCAL void binOp(
    VALUE *pResult,
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    VALUE tmpResult, tmpValue1, tmpValue2;
    VALUE result;

    getRHSValue(pValue1, &tmpValue1);

    /* If unary expression */
    if (pValue2 == NULLVAL)
    {
        tmpResult = evalExp(&tmpValue1, op, NULLVAL);
    }
    else
    {
        getRHSValue(pValue2, &tmpValue2);
        tmpResult = evalExp(&tmpValue1, op, &tmpValue2);
    }

    /* Set right hand side value value */
    setRHSValue(pResult, &tmpResult);
}

/******************************************************************************
 * evalUnknown - Evaluate unknown data type
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalUnknown(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    fprintf(stderr, "evalUnknown: bad evaluation.\n");

    SET_ERROR;

    return *pValue1;
}

/******************************************************************************
 * evalByte - Evaluate byte type value
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalByte(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    VALUE result;
    VALUE *pVal1 = pValue1;
    VALUE *pVal2 = pValue2;

    /* Convert both to int types */
    typeConvert(pVal1, T_INT, RHS);
    typeConvert(pVal2, T_INT, RHS);

    /* Evaluate int operator */
    result = evalInt(pVal1, op, pVal2);

    /* Convert result to word type */
    typeConvert(&result, T_BYTE, RHS);

    return result;
}

/******************************************************************************
 * evalWord - Evaluate word type value
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalWord(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    VALUE result;
    VALUE *pVal1 = pValue1;
    VALUE *pVal2 = pValue2;

    /* Convert both to int types */
    typeConvert(pVal1, T_INT, RHS);
    typeConvert(pVal2, T_INT, RHS);

    /* Evaluate int operator */
    result = evalInt(pVal1, op, pVal2);

    /* Convert result to word type */
    typeConvert(&result, T_WORD, RHS);

    return result;
}

/******************************************************************************
 * evalInt - Evaluate int type value
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalInt(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    VALUE result;
    int rv;
    int i1 = pValue1->value.rv;
    int i2 = pValue2->value.rv;

    /* Select operator */
    switch(op)
    {
        case  '+':
        case ADDA:
            rv = i1 + i2;
            break;

        case  '-':
        case SUBA:
            rv = i1 - i2;
            break;

        case  '*':
        case MULA:
            rv = i1 * i2;
            break;

        case  '/':
        case DIVA:
            rv = i1 / i2;
            break;

        case  '%':
        case MODA:
            rv = i1 % i2;
            break;

        case  '&':
        case ANDA:
            rv = i1 & i2;
            break;

        case '|':
        case ORA:
            rv = i1 | i2;
            break;

        case  '^':
        case XORA:
            rv = i1 ^ i2;
            break;

        case ROT_LEFT:
        case     SHLA:
            rv = i1 << i2;
            break;

        case ROT_RIGHT:
        case      SHRA:
            rv = i1 >> i2;
            break;

        case EQ:
            rv = i1 == i2;
            break;

        case NE:
            rv = i1 != i2;
            break;

        case LE:
            rv = i1 <= i2;
            break;

        case GE:
            rv = i1 >= i2;
            break;

        case AND:
            rv = i1 && i2;
            break;

        case OR:
            rv = i1 || i2;
            break;

        case '<':
            rv = i1 < i2;
            break;

        case '>':
            rv = i1 > i2;
            break;

        case INCR:
            rv = ++i1;
            break;

        case DECR:
            rv = --i1;
            break;

        case UMINUS:
            rv = -i1;
            break;

        case '~':
            rv = ~i1;
            break;

        case '!':
            rv = !i1;
            break;

        default:
            rv = 0;
            fprintf(stderr, "operads have incompatible types.\n");
            SET_ERROR;
            break;
    }

    /* Setup fieds in value struct */
    result.side     = RHS;
    result.type     = pValue1->type;
    result.value.rv = rv;

    return result;
}

#ifdef INCLUDE_FLOAT
/******************************************************************************
 * evalFloat - Evaluate float type value
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalFloat(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    return evalDouble(pValue1, op, pValue2);
}

/******************************************************************************
 * evalDouble - Evaluate double type value
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE evalDouble(
    VALUE *pValue1,
    int op,
    VALUE *pValue2
    )
{
    VALUE result = (VALUE) 0;

    /* Not implemented yet */
    return result;
}
#endif /* INCLUDE_FLOAT */

/******************************************************************************
 * newArgList - Create function agument list
 *
 * RETURNS: Value of first argument in list
 */

LOCAL VALUE newArgList(
    void
    )
{
    VALUE value;

    /* Setup value struct */
    value.side     = RHS;
    value.type     = T_INT;
    value.value.rv = nArgs;

    return value;
}

/******************************************************************************
 * addArg - Add argument to arglist
 *
 * RETURNS: N/A
 */

LOCAL void addArg(
    VALUE *pArgList,
    VALUE *pNewArg
    )
{
    SYMBOL_ID symId;
    SYM_TYPE sType;
    VALUE value;
    int rv;

    if ((nArgs == MAX_SHELL_ARGS) ||
        ((nArgs - pArgList->value.rv) == MAX_FUNC_ARGS))
    {
        fprintf(stderr, "too many arguments to functions.\n");
        SET_ERROR;
    }
    else
    {
        /* Get new argument right hand side value */
        getRHSValue(pNewArg, &value);

        /* If right hand value is okay */
        if (checkRHSValue(&value) == TRUE)
        {
            /* Select type */
            switch(value.type)
            {
                case T_BYTE:
                    rv = (int) value.value.byte;
                    break;

                case T_WORD:
                    rv = (int) value.value.word;
                    break;

                case T_INT:
                    rv = value.value.rv;
                    break;

                default:
                    rv = 0;
                    fprintf(stderr, "addArg: bad type.\n");
                    SET_ERROR;
                    break;
            }

            /* Store argument and advance */
            argStack[nArgs++] = (ARG) rv;
        }
    }
}

/******************************************************************************
 * funcCall - Call function
 *
 * RETURNS: Resulting value
 */

LOCAL VALUE funcCall(
    VALUE *pValue,
    VALUE *pArgList
    )
{
    static int funcErrno;
    VALUE value;
    FUNCPTR pFunc;
    ARG args[MAX_FUNC_ARGS];
    int i, argNum;
    int rv;
    int oldInFd, oldOutFd;

    /* Store old file descriptos */
    oldInFd  = ioGlobalStdGet(STDIN_FILENO);
    oldOutFd = ioGlobalStdGet(STDOUT_FILENO);

    /* Get function to call */
    if (pValue->side == LHS)
    {
        pFunc = (FUNCPTR) getLHSValue(pValue);
    }
    else
    {
        pFunc = (FUNCPTR) pValue->value.rv;
    }

    /* For all arguments on arglist */
    for (argNum = pArgList->value.rv, i = 0; i < MAX_FUNC_ARGS; argNum++, i++)
    {
        if (argNum < nArgs)
        {
            args[i] = argStack[argNum];
        }
        else
        {
            args[i] = (ARG) 0;
        }
    }

    /* Get redirections */
    if (redirInFd >= 0)
    {
        ioGlobalStdSet(STDIN_FILENO, redirInFd);
    }

    if (redirOutFd >= 0)
    {
        ioGlobalStdSet(STDOUT_FILENO, redirOutFd);
    }

    /* Setup return value struct */
    value.side = RHS;
    value.type = pValue->type;

    /* Select type */
    switch(pValue->type)
    {
        case T_BYTE:
        case T_WORD:
        case  T_INT:
            /* Call function */
            rv = (*pFunc)(
                     args[ 0],
                     args[ 1],
                     args[ 2],
                     args[ 3],
                     args[ 4],
                     args[ 5],
                     args[ 6],
                     args[ 7],
                     args[ 8],
                     args[ 9],
                     args[10],
                     args[11]
                     );

            /* Select return value type */
            switch(pValue->type)
            {
                case T_BYTE:
                    value.value.byte = (char) rv;
                    break;

                case T_WORD:
                    value.value.word = (short) rv;
                    break;

                case T_INT:
                    value.value.rv = rv;
                    break;

                default:
                    break;
            }
            break;

        default:
            fprintf(stderr, "funcCall: bad function type.\n");
            SET_ERROR;
            break;
    }

    /* Restore default filedescriptors */
    ioGlobalStdSet(STDIN_FILENO, oldInFd);
    ioGlobalStdSet(STDOUT_FILENO, oldOutFd);

    /* Store function errno */
    funcErrno = errnoGet();

    /* Restore args stack */
    nArgs = pArgList->value.rv;

    return value;
}

