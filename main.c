/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 *
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    28-Nov-2019 (edited)
 */
#include "KernelCall.h"
#include <string.h>
#include "UART.h"
#include "SVC.h"
#include "SYSTICK.h"
#include "Messages.h"




/*
 * @brief   definition of idleProcess; the first process registered
 *          by the kernel. It must always idle and will only be run
 *          if there are no other processes in place.
 */
void idleProcess(void)
{
    /* Loop indefinitely */

    int mailBox = bind(ANY);
    int myID = getid();
    PCB* runningPCB = getRunningPCB();
    int k=0;
    int wait;
    int cursorPos = 0;
    while(1)
    {
    while(cursorPos<IDLE_SYMBOLS)
    {
    sendMessage(UART_OP_MB, mailBox, "*", CHAR_SEND);
    while(k<500000)
    {
        wait =0;
        k++;
    }
    k=0;
    cursorPos++;
    }
    cursorPos=0;
    runningPCB->xAxisCursorPosition=cursorPos;
    sendMessage(UART_OP_MB, mailBox, CLEAR_LINE, strlen(CLEAR_LINE) + 1);
    }
}

void defaultProcess(void)
{
    volatile int wait = 0;
    int mailBox = bind(ANY);
    int myID = getid();
    char idString[POSITION_DIGITS];
    printLineMarker(myID,mailBox);

    int i = 0;
    while (i < EXAMPLE_MESSAGES_AMOUNT)
    {
        sendMessage(UART_OP_MB, mailBox, idString, POSITION_DIGITS + 1);
        i++;
    }

    unbind(mailBox);
}

/*
 * @brief default process, except with a nice to
 *        show self premption
 * */
void defaultProcessNice(void)
{
    volatile int wait = 0;

    int mailBox = bind(ANY);
    int myID = getid();
    char idString[POSITION_DIGITS];
    printLineMarker(myID,mailBox);

    int i = 0;
    while (i < EXAMPLE_MESSAGES_AMOUNT)
    {

        if(i==1){nice(1);}
        sendMessage(UART_OP_MB, mailBox, idString, POSITION_DIGITS + 1);
        i++;
    }

    unbind(mailBox);
}

/*
 * @brief example process to send and recv
 *        with process 30
 * */
void Priority2Process10(void)
{

    int mailBox = bind(3);
    int myID = getid();
    printLineMarker(myID,mailBox);

    int toMB;
    int timerMB = TIMER_MB;
    char cont[EXAMPLE_MESSAGE_LENGTH];

        strcpy(cont, " Input to Process 10 ");
        sendMessage(UART_IP_MB, mailBox, cont, strlen(cont) + 1);
        recvMessage(mailBox, &toMB, cont, strlen(cont) + 1);
        sendMessage(UART_OP_MB, mailBox, cont, strlen(cont) + 1);
        sendMessage(TIMER_MB, mailBox,"60", 2);
        recvMessage(mailBox, &timerMB , cont, strlen(cont) + 1);
        sendMessage(UART_OP_MB, mailBox, cont, strlen(cont) + 1);

    unbind(mailBox);

}

/*
 * @brief   registers processes.
 *          Sets highest priority process as Running
 *          traps kernel with service call
 *
 * */
int main(void)
{
    initMessagePool();
    initMailBoxList();
    initReceiveLogs();

    int registerResult = 0;

    /* Register idle process first */
    registerResult |= registerProcess(idleProcess, 0, 0);
    registerResult |= registerProcess(uartOutputServer, 1, 4);
    registerResult |= registerProcess(uartInputServer, 2, 3);
    registerResult |= registerProcess(timeServer, 3, 4);


    /* Register other test processes */
    registerResult |= registerProcess(Priority2Process10, 10, 2);


    if (!registerResult)
    {
        /* Initialize required hardware + interrupts */
        initpendSV();
        UART0_Init();           // Initialize UART0
        InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
        UART0_IntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
        SysTickPeriod(HUNDREDTH_WAIT);//HUNDREDTH_WAIT
        SysTickIntEnable();
        systemPrintString(CLEAR_SCREEN);

        /* Trap to begin running processes */
        SVC();
    }

    return 0;
}
