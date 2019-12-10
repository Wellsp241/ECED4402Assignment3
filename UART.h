/*
 * @file    UART.h
 * @details UART Constant Register Declarations
 *          UART constants
 *          UART Function Prototypes
 * @author  Emad Khan (original, 2017)
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    23-Sep-2019 (created)
 * @date    7-Dec-2019 (modified)
 */
#pragma once
#include "Process.h"
#define GPIO_PORTA_AFSEL_R  (*((volatile unsigned long *)0x40058420))   // GPIOA Alternate Function Select Register
#define GPIO_PORTA_DEN_R    (*((volatile unsigned long *)0x4005851C))   // GPIOA Digital Enable Register
#define GPIO_PORTA_PCTL_R   (*((volatile unsigned long *)0x4005852C))   // GPIOA Port Control Register

#define GPIO_PORTB_AFSEL_R  (*((volatile unsigned long *)0x40059420))   // GPIOB Alternate Function Select Register
#define GPIO_PORTB_DEN_R    (*((volatile unsigned long *)0x4005951C))   // GPIOB Digital Enable Register
#define GPIO_PORTB_PCTL_R   (*((volatile unsigned long *)0x4005952C))   // GPIOB Port Control Register

#define UART0_DR_R          (*((volatile unsigned long *)0x4000C000))   // UART0 Data Register
#define UART0_FR_R          (*((volatile unsigned long *)0x4000C018))   // UART0 Flag Register
#define UART0_IBRD_R        (*((volatile unsigned long *)0x4000C024))   // UART0 Integer Baud-Rate Divisor Register
#define UART0_FBRD_R        (*((volatile unsigned long *)0x4000C028))   // UART0 Fractional Baud-Rate Divisor Register
#define UART0_LCRH_R        (*((volatile unsigned long *)0x4000C02C))   // UART0 Line Control Register
#define UART0_CTL_R         (*((volatile unsigned long *)0x4000C030))   // UART0 Control Register
#define UART0_IFLS_R        (*((volatile unsigned long *)0x4000C034))   // UART0 Interrupt FIFO Level Select Register
#define UART0_IM_R          (*((volatile unsigned long *)0x4000C038))   // UART0 Interrupt Mask Register
#define UART0_MIS_R         (*((volatile unsigned long *)0x4000C040))   // UART0 Masked Interrupt Status Register
#define UART0_ICR_R         (*((volatile unsigned long *)0x4000C044))   // UART0 Interrupt Clear Register
#define UART0_CC_R          (*((volatile unsigned long *)0x4000CFC8))   // UART0 Clock Control Register

#define UART1_DR_R          (*((volatile unsigned long *)0x4000D000))   // UART1 Data Register
#define UART1_FR_R          (*((volatile unsigned long *)0x4000D018))   // UART1 Flag Register
#define UART1_IBRD_R        (*((volatile unsigned long *)0x4000D024))   // UART1 Integer Baud-Rate Divisor Register
#define UART1_FBRD_R        (*((volatile unsigned long *)0x4000D028))   // UART1 Fractional Baud-Rate Divisor Register
#define UART1_LCRH_R        (*((volatile unsigned long *)0x4000D02C))   // UART1 Line Control Register
#define UART1_CTL_R         (*((volatile unsigned long *)0x4000D030))   // UART1 Control Register
#define UART1_IFLS_R        (*((volatile unsigned long *)0x4000D034))   // UART1 Interrupt FIFO Level Select Register
#define UART1_IM_R          (*((volatile unsigned long *)0x4000D038))   // UART1 Interrupt Mask Register
#define UART1_MIS_R         (*((volatile unsigned long *)0x4000D040))   // UART1 Masked Interrupt Status Register
#define UART1_ICR_R         (*((volatile unsigned long *)0x4000D044))   // UART1 Interrupt Clear Register
#define UART1_CC_R          (*((volatile unsigned long *)0x4000DFC8))   // UART1 Clock Control Register

#define INT_VEC_UART0           5           // UART0 Rx and Tx interrupt index (decimal)
#define INT_VEC_UART1           6           // UART1 Rx and Tx interrupt index (decimal)
#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_RX_FIFO_ONE_EIGHT  0x00000038  // UART Receive FIFO Interrupt Level at >= 1/8
#define UART_TX_FIFO_SVN_EIGHT  0x00000007  // UART Transmit FIFO Interrupt Level at <= 7/8
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000301  // UART RX/TX Enable
#define UART_INT_TX             0x020       // Transmit Interrupt Mask
#define UART_INT_RX             0x010       // Receive Interrupt Mask
#define UART_INT_RT             0x040       // Receive Timeout Interrupt Mask
#define UART_CTL_EOT            0x00000010  // UART End of Transmission Enable
#define EN_RX_P0               0x00000001  // Enable Receive Function on PA0/PB0
#define EN_TX_P1               0x00000002  // Enable Transmit Function on PA1/PB1
#define EN_DIG_P0              0x00000001  // Enable Digital I/O on PA0/PB0
#define EN_DIG_P1              0x00000002  // Enable Digital I/O on PA1/PB1

// Clock Gating Registers
#define SYSCTL_RCGCGPIO_R      (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_RCGCUART_R      (*((volatile unsigned long *)0x400FE618))

#define SYSCTL_RCGCGPIO_UART      0x00000003   // UART0 and UART1 Clock Gating Control
#define SYSCTL_RCGCUART_GPIO      0x00000003   // Port A and B Clock Gating Control

// Clock Configuration Register
#define SYSCTRL_RCC_R           (*((volatile unsigned long *)0x400FE0B0))

#define CLEAR_USRSYSDIV     0xF83FFFFF  // Clear USRSYSDIV Bits
#define SET_BYPASS      0x00000800  // Set BYPASS Bit

#define NVIC_EN0_R      (*((volatile unsigned long *)0xE000E100))   // Interrupt 0-31 Set Enable Register
#define NVIC_EN1_R      (*((volatile unsigned long *)0xE000E104))   // Interrupt 32-54 Set Enable Register


#define NUL 0x00



/* Cursor position string */

/* Define the cursor position data structure */

typedef struct CUP
{
    char esc;
    char sqrbrkt;
    char line[2];   /* 01 through 24 */
    char semicolon;
    char col[2];    /* 01 through 80 */
    char cmdchar;
    char nul;
}Cursor;

#ifndef GLOBAL_UART
#define GLOBAL_UART


    extern void UART0_Init(void);
    extern void InterruptEnable(unsigned long);
    extern void UART0_IntEnable(unsigned long);
    extern void UART0_IntHandler(void);
    extern void forceOutput(char);
    extern int getDataRegister(char *);
    extern void printString(char*,PCB*);
    extern void systemPrintString(char*);
    extern void printWarning(int);
    extern void uart0_OutputServer(void);
    extern void uart0_InputServer(void);
    extern int get_UART0_InputState(void);



#else

    void systemPrintString(char*);
    void forceOutput(char);
    void printString(char*,PCB*);
    void printWarning(int);
    void dataRecieved(void);

#endif// GLOBAL_UART
