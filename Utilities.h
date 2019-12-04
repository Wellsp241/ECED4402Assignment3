/*
 * @file    Utilities.h
 * @brief   frequently used constants
 *          Utilities.c function prototypes
 * @author  Liam JA MacDonald
 * @date    23-Sep-2019 (created)
 * @date    10-Oct-2019 (modified)
 */
#pragma     once

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
#define     POSITION_DIGITS 2
#define     TRUE        1   // For using Integers as booleans
#define     FALSE       0
#define     SUCCESS     1   // To clarify returns values where possible
#define     FAILURE     -1
#define     NULL        0
#define     EMPTY       0       //Queue return values
#define     FULL        0
#define     ANY         16      //signals bind any
#define     SEND_FAIL   -2
#define     RECV_FAIL   -3
#define     BIND_FAIL   -4
#define     UNBIND_FAIL -5
#define     DEFAULT_FAIL FAILURE
#define     MESSAGE_SYS_LIMIT 32
#define     RECEIVE_LOG_AMOUNT MESSAGE_SYS_LIMIT
#define     UART_MB     0
#define     CURSOR_STRING   9


#ifndef     GLOBAL_UTILITIES
#define     GLOBAL_UTILITIES

extern void formatLineNumber(int,char*);
extern void getProcessCursor(int,char*);

#else

#endif /* UTILITIES */
