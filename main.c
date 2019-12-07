/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    13-Nov-2019 (edited)
 */
#include <string.h>
#include "UART.h"
#include "Utilities.h"
#include "KernelCall.h"
#include "SVC.h"
#include "Process.h"
#include "SYSTICK.h"
#include "Messages.h"

/*
 * @brief   definition of idleProcess; the first process registered
 *          by the kernel. It must always idle and will only be run
 *          if there are no other processes in place.
 * */
void idleProcess(void)
{
    /* Loop indefinitely */

    while (1)
        ;

}

/*
 * @brief   Sample test process used to exercise inter-process communication
 */
void Priority3Process10(void)
{
    int mailBox = bind(ANY);
    int myID = getid();
    char cursorPosition[CURSOR_STRING];
    char idString[POSITION_DIGITS];
    getProcessCursor(myID,cursorPosition);
    formatLineNumber(myID, idString);
    sendMessage(UART_MB, mailBox, cursorPosition, CURSOR_STRING);
    sendMessage(UART_MB, mailBox, RED_TEXT, strlen(RED_TEXT) + 1);
    sendMessage(UART_MB, mailBox, idString, POSITION_DIGITS + 1);
    sendMessage(UART_MB, mailBox, CLEAR_MODE, strlen(CLEAR_MODE) + 1);
    sendMessage(UART_MB, mailBox, "  ", 3);
    int i = 0;
    int toMB =3;
    int size = 9;
    char cont[9];
    nice(2);
    nice(3);
    while (i < 5)
    {
        strcpy(cont, " *hi 20*\0");
        sendMessage(toMB, mailBox, cont, size);
        recvMessage(mailBox, &toMB, cont, size);
        getProcessCursor(myID,cursorPosition);
        sendMessage(UART_MB, mailBox, cursorPosition, CURSOR_STRING);
        sendMessage(UART_MB, mailBox, cont, size);
        i++;
    }

    unbind(mailBox);
}


/*
 * @brief   Sample test process used to exercise inter-process communication
 */
void Priority3Process20(void)
{
    int mailBox = bind(3);
    int myID = getid();
    char cursorPosition[CURSOR_STRING];
    char idString[POSITION_DIGITS];
    getProcessCursor(myID, cursorPosition);
    formatLineNumber(myID, idString);
    sendMessage(UART_MB, mailBox, cursorPosition, CURSOR_STRING);
    sendMessage(UART_MB, mailBox, RED_TEXT, strlen(RED_TEXT) + 1);
    sendMessage(UART_MB, mailBox, idString, POSITION_DIGITS + 1);
    sendMessage(UART_MB, mailBox, CLEAR_MODE, strlen(CLEAR_MODE) + 1);
    sendMessage(UART_MB, mailBox, "  ", 3);
    int i=0;
    int toMB;
    int size = 9;
    char cont[9];

    while (i < 5)
    {
        recvMessage(mailBox, &toMB, cont, size);
        getProcessCursor(myID,cursorPosition);
        sendMessage(UART_MB, mailBox, cursorPosition, CURSOR_STRING);
        sendMessage(UART_MB, mailBox, cont, size);
        strcpy(cont, " *hi 10*\0");
        sendMessage(toMB, mailBox, cont, size);
        i++;
    }

    unbind(mailBox);

}

/*
 * @brief   Registers processes.
 *          Configures hardware.
 *          Sets highest priority process as Running
 *          Traps kernel with service call
 *
 * */
int main(void)
{
    int registerResult = 0;

    initMessagePool();
    initMailBoxList();
    initReceiveLogs();

    /* Register idle process first */
    registerResult |= registerProcess(idleProcess, 0, 0);
    registerResult |= registerProcess(uartProcess, 1, 4);

    /* Register other test processes */
    registerResult |= registerProcess(Priority3Process10, 10, 3);
    registerResult |= registerProcess(Priority3Process20, 20, 3);


    if (!registerResult)
    {
        /* Initialize required hardware + interrupts */
        initpendSV();
        UART_Init();           // Initialize UART
        InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
        InterruptEnable(INT_VEC_UART1);
        UART_IntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
        SysTickPeriod(HUNDREDTH_WAIT);
        SysTickIntEnable();
        char *clearString = CLEAR_SCREEN;
        while (*clearString)
        {
            forceOutputUART0(*(clearString++));
        }
        /* Trap to begin running processes */
        SVC();
    }

    return 0;
}
