/*
 * @file    UART.c
 * @details Contains initialization routines to set
 *          a UART interrupts for transmission and receive.
 *          Definition of the UART ISR
 *
 * @author  Liam JA MacDonald
 * @date    20-Oct-2019 (created)
 * @date    28-Nov-2019 (modified)
 */

#define GLOBAL_UART
#include "KernelCall.h"
#include "Process.h"
#include "HoldingBuffer.h"
#include "UART.h"
#include "Utilities.h"
#include "SVC.h"
#include "Messages.h"
#include "Queue.h"
#include <ctype.h>

static interruptType uart0_ReceiveBuffer = {UART0,NUL};
static interruptType uart1_ReceiveBuffer = {UART1,NUL};

static int input0_Blocked = FALSE;
static int input1_Blocked = FALSE;


//for accessing the processes horizontal possition

/*
 * @brief uart process dedicated to outputing messages
 *        received from other processes
 */
void uart0_OutputServer(void)
{
    bind(UART0_OP_MB);
    int toMB;
    char cont[MESSAGE_SYS_LIMIT];
    int size = MESSAGE_SYS_LIMIT;
    while(1)
    {
        recvMessage(UART0_OP_MB, &toMB, cont, size);
        printString(cont, getOwnerPCB(toMB));
    }
}

int get_UART0_InputState(void)
{
    return input0_Blocked;
}

void uart0_InputServer(void)
{
    bind(UART0_IP_MB);
    int myID = getid();
    int inputEntered;
    interruptType inputBuffer ={UART0,NUL};
    char * cmd;
    int toMB;
    char cont[MESSAGE_SYS_LIMIT];
    int size = MESSAGE_SYS_LIMIT;
    while (1)
    {
        recvMessage(UART0_IP_MB, &toMB, cont, size);
        sendMessage(UART0_OP_MB, UART0_IP_MB, cont, size);
        cmd = NULL;
        size = NULL;
        inputEntered = FALSE;
        while (!inputEntered)
        {
            if (dequeue(&inputBuffer))
            {
                switch (inputBuffer.data)
                {
                case ENTER:

                    cmd = emptyBuffer_0();
                    inputEntered = TRUE;
                    sendMessage(toMB, UART0_IP_MB, cmd, size);

                    break;
                case BS:

                    if (removeFromBuffer_0() == SUCCESS)
                    {
                        sendMessage(UART0_OP_MB, UART0_IP_MB, &inputBuffer.data, CHAR_SEND);
                    }

                    break;
                default:
                    if (addToBuffer_0(toupper(inputBuffer.data)) == SUCCESS)
                    {
                        sendMessage(UART0_OP_MB, UART0_IP_MB, &inputBuffer.data, CHAR_SEND);
                        size++;
                    }
                }
            }
            else
            {
                input0_Blocked = TRUE;
                block();
                input0_Blocked = FALSE;
            }
        }

    }
}


//for accessing the processes horizontal possition

/*
 * @brief uart process dedicated to outputing messages
 *        received from other processes
 */
void uart1_OutputServer(void)
{
    bind(UART1_OP_MB);
    int toMB;
    char cont[MESSAGE_SYS_LIMIT];
    int size = MESSAGE_SYS_LIMIT;
    while(1)
    {
        recvMessage(UART1_OP_MB, &toMB, cont, size);
        printString(cont, getOwnerPCB(toMB));
    }
}

int get_UART1_InputState(void)
{
    return input1_Blocked;
}

void uart1_InputServer(void)
{
    bind(UART1_IP_MB);
    int myID = getid();
    int inputEntered;
    interruptType inputBuffer ={UART1,NUL};
    char * cmd;
    int toMB;
    char cont[MESSAGE_SYS_LIMIT];
    int size = MESSAGE_SYS_LIMIT;
    while (1)
    {
        recvMessage(UART1_IP_MB, &toMB, cont, size);
        sendMessage(UART1_OP_MB, UART1_IP_MB, cont, size);
        cmd = NULL;
        size = NULL;
        inputEntered = FALSE;
        while (!inputEntered)
        {
            if (dequeue(&inputBuffer))
            {
                switch (inputBuffer.data)
                {
                case ENTER:

                    cmd = emptyBuffer_1();
                    inputEntered = TRUE;
                    sendMessage(toMB, UART1_IP_MB, cmd, size);

                    break;
                case BS:

                    if (removeFromBuffer_1() == SUCCESS)
                    {
                        sendMessage(UART1_OP_MB, UART1_IP_MB, &inputBuffer.data, CHAR_SEND);
                    }

                    break;
                default:
                    if (addToBuffer_1(toupper(inputBuffer.data)) == SUCCESS)
                    {
                        sendMessage(UART1_OP_MB, UART1_IP_MB, &inputBuffer.data, CHAR_SEND);
                        size++;
                    }
                }
            }
            else
            {
                input1_Blocked = TRUE;
                block();
                input1_Blocked = FALSE;
            }
        }

    }
}
/*
 * @brief initialize UART0 and UART1
 *        with BAUD-RATE:       115200
 *             Data Bits:       8
 *             Parity Bits:     0
 *             Stop Bits:       1
 */
void UART_Init(void)
{
    volatile int wait;

    /* Initialize UART0/UART1 */
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCUART_GPIO;   // Enable Clock Gating for UART0/UART1
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCGPIO_UART;   // Enable Clock Gating for PORTA/PORTB
    wait = 0; // give time for the clocks to activate

    UART0_CTL_R &= ~UART_CTL_UARTEN;        // Disable the UART
    UART1_CTL_R &= ~UART_CTL_UARTEN;
    wait = 0;   // wait required before accessing the UART config regs

    // Setup the BAUD rate
    UART0_IBRD_R = 8;   // IBRD = int(16,000,000 / (16 * 115,200)) = 8.680555555555556
    UART0_FBRD_R = 44;  // FBRD = int(.680555555555556 * 64 + 0.5) = 44.05555555555556

    UART1_IBRD_R = 8;
    UART1_FBRD_R = 44;

    UART0_LCRH_R = (UART_LCRH_WLEN_8);  // WLEN: 8, no parity, one stop bit, without FIFOs)
    UART1_LCRH_R = (UART_LCRH_WLEN_8);

    GPIO_PORTA_AFSEL_R = 0x3;        // Enable Receive and Transmit on PA1-0
    GPIO_PORTA_PCTL_R = (0x01) | ((0x01) << 4);         // Enable UART RX/TX pins on PA1-0
    GPIO_PORTA_DEN_R = EN_DIG_P0 | EN_DIG_P1;        // Enable Digital I/O on PA1-0

    GPIO_PORTB_AFSEL_R = 0x3;
    GPIO_PORTB_PCTL_R = (0x01) | ((0x01) << 4);
    GPIO_PORTB_DEN_R = EN_DIG_P0 | EN_DIG_P1;

    UART0_CTL_R = UART_CTL_UARTEN;        // Enable the UART
    UART1_CTL_R = UART_CTL_UARTEN;
    wait = 0; // wait; give UART time to enable itself.
}


/*
 * @brief   Enable UART0 to interrupt
 * @param   [in] unsigned long InterruptIndex:
 *          UART0 address in interrupt table
 */
void InterruptEnable(unsigned long InterruptIndex)
{
/* Indicate to CPU which device is to interrupt */
if(InterruptIndex < 32)
    NVIC_EN0_R |= 1 << InterruptIndex;       // Enable the interrupt in the EN0 Register
else
    NVIC_EN1_R |= 1 << (InterruptIndex - 32);    // Enable the interrupt in the EN1 Register
}

/*
 * @brief   Enable UART0 receive and transmit interrupts in UART0
 * @param   [in] unsigned long Flags:
 *          bit mask to specify conditions for interrupt
 */
void UARTIntEnable(unsigned long flags)
{
    /* Set specified bits for interrupt */
    UART0_IM_R |= flags;
    UART1_IM_R |= flags;
}

/*
 * @brief   Force character into the data register
 * @param   [in] char data: character to be put into
 *          data register
 */
void force_UART0_Output(char data)
{
        while(UART0_FR_R & UART_FR_TXFF);//wait until not busy
        UART0_DR_R = data;
}

/*
 * @brief   for printing from a string from process
 *
 * @param   [in] char* string: string to print
 *
 */
void printString(char* string, PCB * printingProcess)
{
    //check if its and escape sequence
    char cursorPosition[CURSOR_STRING];
    getProcessCursor(printingProcess, cursorPosition);
    systemPrintString(cursorPosition);

    int increaseCursor = (*string == ESC)? FALSE : TRUE;
    while(*string)
    {
        force_UART0_Output(*(string++));
        //update process cursor if not an escape sequence
        printingProcess->xAxisCursorPosition+=increaseCursor;
    }
}

/*
 * @brief   for printing from a string from system
 *          mostly for clearing screen printing warnings
 *          will be used more in future work
 *
 * @param   [in] char* string: string to print
 *
 */
void systemPrintString(char* string)
{
    while(*string)
    {
        force_UART0_Output(*(string++));
    }
}

/*
 * @brief   Handles receive and transmit interrupts
 *
 * @detail  check if receive interrupt has been set
 *          if it has set gotData in the input queue
 *          if the the output queue isn't empty force
 *          next available data out
 */
void UART0_IntHandler(void)
{
/*
 * Simplified UART ISR - handles receive and xmit interrupts
 * Application signalled when data received
 */
    if(UART0_MIS_R & UART_INT_RX)
    {
        /* RECV done - clear interrupt and make char available to application */
        UART0_ICR_R |= UART_INT_RX;
        uart0_ReceiveBuffer.data = UART0_DR_R;
        enqueue(uart0_ReceiveBuffer);
        if(get_UART0_InputState())
        {
            setPendType(INPUT_0);
            CALLPENDSV;
        }
    }

    if(UART0_MIS_R & UART_INT_TX)
    {
        UART0_ICR_R |= UART_INT_TX;
    }

}



/*
 * @brief   Force character into the data register
 * @param   [in] char data: character to be put into
 *          data register
 */
void force_UART1_Output(char data)
{
        while(UART0_FR_R & UART_FR_TXFF);//wait until not busy
        UART0_DR_R = data;
}

/*
 * @brief   Handles receive and transmit interrupts for UART1
 * @detail  check if receive interrupt has been set
 *          if it has set gotData in the input queue
 *          if the the output queue isn't empty force
 *          next available data out
 */
void UART1_IntHandler(void)
{
/*
 * Simplified UART ISR - handles receive and xmit interrupts
 * Application signalled when data received
 */
    if (UART1_MIS_R & UART_INT_RX)
    {
        /* RECV done - clear interrupt and make char available to application */
        UART1_ICR_R |= UART_INT_RX;
        uart1_ReceiveBuffer.data = UART0_DR_R;
        enqueue(uart1_ReceiveBuffer);
        if (get_UART1_InputState())
        {
            setPendType(INPUT_1);
            CALLPENDSV;
        }
    }

    if(UART1_MIS_R & UART_INT_TX)
    {
        UART1_ICR_R |= UART_INT_TX;
    }

}
