/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 *
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    9-Dec-2019 (edited)
 */
#include "KernelCall.h"
#include <string.h>
#include "UART.h"
#include "SVC.h"
#include "SYSTICK.h"
#include "Messages.h"
#include "AppLayerMessage.h"
#include "DataLinkMessage.h"
#include "PhysLayerMessage.h"


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
    int cursorPos = 0;
    while(1)
    {
    while(cursorPos<IDLE_SYMBOLS)
    {
    sendMessage(UART0_OP_MB, mailBox, "*", CHAR_SEND);
    while(k<500000)
    {
        k++;
    }
    k=0;
    cursorPos++;
    }
    cursorPos=0;
    runningPCB->xAxisCursorPosition=cursorPos;
    sendMessage(UART0_OP_MB, mailBox, CLEAR_LINE, strlen(CLEAR_LINE) + 1);
    }
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
    int size = EXAMPLE_MESSAGE_LENGTH + 1;

    strcpy(cont, " Input to Process 10 ");
    sendMessage(UART0_IP_MB, mailBox, cont, size);
    recvMessage(mailBox, &toMB, cont, size);
    sendMessage(UART0_OP_MB, mailBox, cont, size);
    sendMessage(TIMER_MB, mailBox,"60", 2);
    recvMessage(mailBox, &timerMB , cont, size);
    sendMessage(UART0_OP_MB, mailBox, cont, size);

    unbind(mailBox);

}

/*
 * @brief   registers processes.
 *          Sets highest priority process as Running
 *          traps kernel with service call
 *
 */
int main(void)
{
    initMessagePool();
    initMailBoxList();
    initReceiveLogs();

    int registerResult = 0;

    /* Register idle process first */
    registerResult |= registerProcess(idleProcess, 0, 0);
    registerResult |= registerProcess(uart0_OutputServer, 1, 5);
    registerResult |= registerProcess(uart0_InputServer, 2, 4);
    registerResult |= registerProcess(uart1_OutputServer, 3, 5);
    registerResult |= registerProcess(uart1_InputServer, 4, 4);
    registerResult |= registerProcess(AppfromUART0Handler, 6, 1);
    registerResult |= registerProcess(AppfromDataLinkHandler, 5, 1);
    registerResult |= registerProcess(DataLinkfromAppHandler, 7, 2);
    registerResult |= registerProcess(DataLinkfromPhysHandler, 8, 2);
    registerResult |= registerProcess(PhysLayerFromDLHandler, 9, 3);
    registerResult |= registerProcess(PhysLayerFromUART1Handler, 10, 3);


    /* Register other test processes */

    if (!registerResult)
    {
        /* Initialize required hardware + interrupts */
        initpendSV();
        UART_Init();           // Initialize UART0
        InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
        InterruptEnable(INT_VEC_UART1);
        UARTIntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
        SysTickPeriod(HUNDREDTH_WAIT);//HUNDREDTH_WAIT
        SysTickIntEnable();
        systemPrintString(CLEAR_SCREEN);

        /* Trap to begin running processes */
        SVC();
    }

    return 0;
}
