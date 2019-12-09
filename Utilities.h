/*
 * @file    Utilities.h
 * @brief   frequently used constants
 *          Utilities.c function prototypes
 * @author  Liam JA MacDonald
 * @date    20-Oct-2019 (created)
 * @date    28-Nov-2019 (modified)
 */
#pragma     once
#include "Process.h"
#include <stdio.h>
#include <stdlib.h>
#define     disable()   __asm(" cpsid i") //disable interrupts
#define     enable()    __asm(" cpsie i")//enable interrupts
#define     ENTER       0x0d //ASCII Characters
#define     BS          0x08
#define     NUL         0x00
#define     ESC         '\x1b'
#define     CLEAR_SCREEN "\x1b[2J\x0"
#define     RED_TEXT   "\x1b[1;31m\x0"
#define     CLEAR_MODE  "\x1b[m\x0"
#define     CLEAR_LINE  "\x1b[2K\x0"
#define     POSITION_DIGITS 2
#define     TRUE        1   // For using Integers as booleans
#define     FALSE       0
#define     SUCCESS     1   // To clarify returns values where possible
#define     FAILURE     -1
#define     NULL        0
#define     EMPTY       0       //Queue return values
#define     FULL        0
#define     ANY         16      //signals bind any
#define     SEND_FAIL   -2      //Kernel Warnings
#define     RECV_FAIL   -3
#define     BIND_FAIL   -4
#define     UNBIND_FAIL -5
#define     DEFAULT_FAIL FAILURE
#define     MESSAGE_SYS_LIMIT 32
#define     RECEIVE_LOG_AMOUNT MESSAGE_SYS_LIMIT
#define     UART_OP_MB     0   //uart always mailbox 0
#define     UART_IP_MB     1
#define     TIMER_MB       2
#define     CURSOR_STRING   9
#define     FREE            1   //Timer states
#define     TAKEN           0
#define     EXAMPLE_MESSAGES_AMOUNT 3
#define     EXAMPLE_MESSAGE_LENGTH 21
#define     LINE_MARK " |"
#define     LINE_MARK_LENGTH 3
#define     IDLE_SYMBOLS 10
#define     ONE_SECOND 100
#define     HALF_SECOND 50
#define     CHAR_SEND 1
#define     TIME_STRING     3   //time string length including nul
#define     CALLPENDSV (*((volatile unsigned long *)0xE000ED04) |= 0x10000000UL)


#ifndef     GLOBAL_UTILITIES
#define     GLOBAL_UTILITIES

extern void formatLineNumber(int,char*);
extern void getProcessCursor(PCB*,char*);
extern void printLineMarker(int, int);
extern int myAtoi(int *, char*);

#else

#endif /* UTILITIES */
