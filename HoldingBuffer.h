/*
 * @file    HoldingBuffer.h
 * @brief   Contains HoldingBuffer function prototypes
 *          and the HoldingBuffer structure definition
 * @author  Liam JA MacDonald
 * @date    24-Sep-2019 (Created)
 * @date    9-Oct-2019 (Last Modified)
 */
#pragma once
#define MAX_BUFFER 80

/*
 * @brief   Holding Buffer structure
 * @details char buffer[]: array for folding the received
 *                         characters from UART.c
 *          int writePtr:  the index where the next character
 *                         can be added
 */
typedef struct holdingBuffer_
{
    char buffer[MAX_BUFFER];
    int writePtr;
} holdingBuffer;

#ifndef GLOBAL_HOLDINGBUFFER
#define GLOBAL_HOLDINGBUFFER

        extern int addToBuffer(char);
        extern int removeFromBuffer(void);
        extern char* emptyBuffer(void);
#else

        int addToBuffer(char);
        int removeFromBuffer(void);
        char* emptyBuffer(void);
#endif// GLOBAL_HOLDINGBUFFER
