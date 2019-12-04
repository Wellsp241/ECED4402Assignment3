/*
 * @file    UART.c
 * @details Contains initialization routines to set
 *          a UART interrupts for transmission and receive.
 *          Definition of the UART ISR
 * @author  Emad Khan (original, 2017)
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    23-Sep-2019 (created)
 * @date    26-Oct-2019 (modified)
 */

#define GLOBAL_UART
#include "UART.h"
#include "KernelCall.h"
#include "Utilities.h"
#include "SVC.h"
#include "Process.h"
#include "Messages.h"


#define TRUE    1
#define FALSE   0
/*UART interrupt buffer */
static char dataRegister;
static int gotData = FALSE;
static PCB * printingProcess;

void uartProcess(void)
{
    bind(UART_MB);
    int toMB;
    char cont[MESSAGE_SYS_LIMIT];
    int size = MESSAGE_SYS_LIMIT;
    while(1)
    {
        recvMessage(ANY, &toMB, cont, size);
        printingProcess = getOwnerPCB(toMB);
        printString(cont);
    }
}

int getDataRegister(char * data)
{
    if (gotData)
    {
    *data = dataRegister;
    }
    return gotData;
}

void dataRecieved(void)
{
    gotData = FALSE;
}
/*
 * @brief initialize UART0
 *        with BAUD-RATE:       115200
 *             Data Bits:       8
 *             Parity Bits:     0
 *             Stop Bits:       1
 */
void UART0_Init(void)
{
    volatile int wait;

    /* Initialize UART0 */
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCUART_GPIOA;   // Enable Clock Gating for UART0
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCGPIO_UART0;   // Enable Clock Gating for PORTA
    wait = 0; // give time for the clocks to activate

    UART0_CTL_R &= ~UART_CTL_UARTEN;        // Disable the UART
    wait = 0;   // wait required before accessing the UART config regs

    // Setup the BAUD rate
    UART0_IBRD_R = 8;   // IBRD = int(16,000,000 / (16 * 115,200)) = 8.680555555555556
    UART0_FBRD_R = 44;  // FBRD = int(.680555555555556 * 64 + 0.5) = 44.05555555555556

    UART0_LCRH_R = (UART_LCRH_WLEN_8);  // WLEN: 8, no parity, one stop bit, without FIFOs)

    GPIO_PORTA_AFSEL_R = 0x3;        // Enable Receive and Transmit on PA1-0
    GPIO_PORTA_PCTL_R = (0x01) | ((0x01) << 4);         // Enable UART RX/TX pins on PA1-0
    GPIO_PORTA_DEN_R = EN_DIG_PA0 | EN_DIG_PA1;        // Enable Digital I/O on PA1-0

    UART0_CTL_R = UART_CTL_UARTEN;        // Enable the UART
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
void UART0_IntEnable(unsigned long flags)
{
    /* Set specified bits for interrupt */
    UART0_IM_R |= flags;
}

/*
 * @brief   Force character into the data register
 * @param   [in] char data: character to be put into
 *          data register
 */
void forceOutput(char data)
{
        while(UART0_FR_R & UART_FR_TXFF);//wait until not busy
        UART0_DR_R = data;
}

/*
 * @brief   Handles receive and transmit interrupts
 * @detail  check if receive interrupt has been set
 *          if it has load enqueue it in the input queue
 *          if the the output queue isn't empty force
 *          next available data out
 */
void printString(char* string)
{
    int increaseCursor = (*string == ESC)? FALSE : TRUE;

    while(*string)
    {
        forceOutput(*(string++));
        printingProcess->xAxisCursorPosition+=increaseCursor;
    }
}


void printWarning(int returnValue)
{
    if(returnValue<NULL)
    {
        switch(returnValue)
        {
        case DEFAULT_FAIL:
            printString("DEFAULT FAILURE");
        break;
        case SEND_FAIL:
            printString("SEND FAILURE");
        break;
        case RECV_FAIL:
            printString("RECEIVE FAILURE");
        break;
        case BIND_FAIL:
            printString("BIND FAILURE");
        break;
        case UNBIND_FAIL:
            printString("UNBIND FAILURE");
        break;
        }
    }
}

/*
 * @brief   Handles receive and transmit interrupts
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
        gotData = TRUE;
        dataRegister = UART0_DR_R;
    }

    if(UART0_MIS_R & UART_INT_TX)
    {
        UART0_ICR_R |= UART_INT_TX;
    }

}
