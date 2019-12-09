/*
 * @file    Utilities.c
 * @brief   Contains functions frequently used
 *          over several modules used to format
 *          convert values and printing multiple
 *          characters in varying formats
 *
 * @author  Liam JA MacDonald
 * @date    20-Oct-2019 (created)
 * @date    28-Nov-2019 (modified)
 */
#define GLOBAL_UTILITIES
#define TWO_DIGITS 10

#include "KernelCall.h"
#include <stdlib.h>
#include "Utilities.h"
#include "Messages.h"
#include "SVC.h"
#include "UART.h"
#include <string.h>
/*
 * @brief   adds a leading zero if a single digit number
 *          converts digit to a string
 *
 * @param   [in] int val: value to be converted to a string
 *          [out] char* rtn: to return the formatted string
 */
void formatLineNumber(int val, char* rtn)
{
    if(val<TWO_DIGITS)//only values less than ten have zeros added to the tens placement
    {
        sprintf(rtn,"0%d",val);
    }
    else
    {
        sprintf(rtn,"%d",val);
    }
}

/*
 * @brief   converts ascii to decimal value
 * @param   [in] char* str: string that will be converted from ascii
 *          [out] int* num: integer passed reference to store decimal value
 * @return  int return used as a boolean value,
 *          if returns 1 string was successfully converted
 *          if returns 0 characters were not digits or the string
 *          too long
 */
int myAtoi(int * num, char* str)
{
    int total = 0;

    while(*str)
    {
        if((*str>='0')&&(*str<='9')&&(strlen(str)<TIME_STRING))
        {
          total = total*10+(*(str++)-'0') ;
        }
        else
        {return FAILURE;}
    }
    *num = total;
    return SUCCESS;
}

/*
 * @brief   formats cursor escape sequence with process id and
 *          horizontal process cursor position
 *
 * @param   [in] int lineNumber: vertical value of cursor
 *          [out] char* cursorString: to return the formatted cursor string
 */
void getProcessCursor( PCB* printingProcess, char *cursorString)
{
    char printLine[POSITION_DIGITS];
    char cursorPosition[POSITION_DIGITS];

    formatLineNumber(printingProcess->pid, printLine);
    formatLineNumber(printingProcess->xAxisCursorPosition, cursorPosition);
    Cursor formattedString = {ESC, '[',printLine[0] , printLine[1], ';', cursorPosition[0], cursorPosition[1], 'H',NUL};
    memcpy(cursorString, (char*)&formattedString, CURSOR_STRING);

}

/*
 * @brief   to print the processes terminal line marker
 *          used at the start of non system processes,
 *          ie idle and uart
 *
 * @param   [in] int myID: process PID
 *          [in] int mailBox: process MB
 *          [out] char* cursorPosition: pass out formatted cursor
 *          [out] char* idString: pass out id string
 *
 *
 * */
void printLineMarker(int myID, int mailBox)
{
    char idString[POSITION_DIGITS];
    sendMessage(UART_OP_MB, mailBox, RED_TEXT, strlen(RED_TEXT)+1);
    formatLineNumber(myID, idString);
    sendMessage(UART_OP_MB, mailBox, idString, POSITION_DIGITS + 1);
    sendMessage(UART_OP_MB, mailBox, LINE_MARK, LINE_MARK_LENGTH);
    sendMessage(UART_OP_MB, mailBox, CLEAR_MODE, strlen(CLEAR_MODE)+1);
}
