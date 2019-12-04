/*
 * Utilities.c
 *
 *  Created on: Nov 6, 2019
 *      Author: LiamMacDonald
 */
#define GLOBAL_UTILITIES
#define TWO_DIGITS 10
#include <stdlib.h>
#include "Utilities.h"
#include "Messages.h"
#include "SVC.h"
#include "UART.h"


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

void getProcessCursor(int lineNumber, char *cursorString)
{
    char printLine[POSITION_DIGITS];
    char cursorPosition[POSITION_DIGITS];
    PCB* runningPCB = getRunningPCB();

    formatLineNumber(lineNumber, printLine);
    formatLineNumber(runningPCB->xAxisCursorPosition, cursorPosition);
    cursor formattedString = {ESC, '[',printLine[0] , printLine[1], ';', cursorPosition[0], cursorPosition[1], 'H',NUL};
    memcpy(cursorString, (char*)&formattedString, 9);

}

