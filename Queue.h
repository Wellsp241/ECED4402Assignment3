/*
 * @file    Queue.h
 * @brief   Contains queue function prototypes
 *          and the queue structure definition
 * @author  Liam JA MacDonald
 * @date    23-Sep-2019 (Created)
 * @date    10-Oct-2019 (Last Modified)
 */
#pragma once
#include "InterruptType.h"

#define MAX_QUEUE_SIZE 128

enum inputQueue {UART, SYSTICK};
/*
 * @brief   queue structure
 * @details interruptType fifo[]: array for folding the interrupt structures
 *          int writePtr:         the index where the next interrupt
 *                                can be added
 *          int readPtr:          the index where the next interrupt
 *                                can be read from
 */
typedef struct queue_
{
    interruptType fifo[MAX_QUEUE_SIZE];
    unsigned int writePtr, readPtr;

} queue;

#ifndef GLOBAL_QUEUES
#define GLOBAL_QUEUES

        extern int enqueue(interruptType);
        extern int dequeue(interruptType *);

#endif// GLOBAL_QUEUES
