/*
 * @file    Queue.c
 * @brief   Defines interrupt queues
 *          Contains enqueue and dequeue functionality
 * @author  Liam JA MacDonald
 * @date    23-Sep-2019 (Created)
 * @date    10-Oct-2019 (Modified)
 */
#include <string.h>
#define GLOBAL_QUEUES
#include "Queue.h"
#include "Utilities.h"
#include "UART.h"
#include "HoldingBuffer.h"
#include "Messages.h"
#include "SVC.h"
#include "KernelCall.h"

#define NUMBER_OF_QUEUES 2
/*Declare queue array for INPUT (0) and OUTPUT (1) interrupts*/
static queue uartInputQueue = {{NULL},NULL,NULL};
static queue systickInputQueue = {{NULL},NULL,NULL};

/*
 * @brief   Adds an interrupt to an interrupt queue.
 * @param   [in] int queueType: specifies which queue to add to
 *                              INPUT (0); OUTPUT(1)
 *          [in] interruptType it: incoming interrupt
 * @return  int return Used as a boolean value,
 *          if returns 1; interrupt successfully queued.
 *          if returns 0; queue full or empty.
 * @detail  if it's for OUTPUT queue and it's empty
 *          the data is forced out to the screen
 *          interrupts are disabled before changing the
 *          write pointer to avoid race conditions
 */
int enqueue(interruptType intType)
{
    queue * selectedQueue = (intType.type) ? &systickInputQueue : &uartInputQueue;
    disable();
    /* gives circular queue functionality*/
    unsigned int tmpPtr = (selectedQueue->writePtr+1)&(MAX_QUEUE_SIZE-1);
    if((tmpPtr == selectedQueue->readPtr)){return FULL;}
    /* put character in queue and increment write ptr */
    selectedQueue->fifo[selectedQueue->writePtr].data = intType.data;
    selectedQueue->writePtr =tmpPtr;
    enable();

    return SUCCESS;
}

/*
 * @brief   Removes an interrupt to an interrupt queue.
 * @param   [in] int queueType: specifies which queue to add to
 *                              INPUT (0); OUTPUT(1)
 *          [in] interruptType it: incoming interrupt
 * @return  int return Used as a boolean value,
 *          if returns 1; interrupt successfully queued.
 *          if returns 0; queue full or empty.
 * @detail  if it's for OUTPUT queue and it's empty
 *          the data is forced out to the screen
 *          interrupts are disabled before changing the
 *          write pointer to avoid race conditions
 */
int dequeue(interruptType * intType)
{
    queue * selectedQueue = (intType->type) ? &systickInputQueue : &uartInputQueue;
    if((selectedQueue->writePtr == selectedQueue->readPtr))//buffer is empty
    {
        return EMPTY;
    }

    disable();
    intType->data = selectedQueue->fifo[selectedQueue->readPtr].data;
    /* gives circular queue functionality*/
    selectedQueue->readPtr=(selectedQueue->readPtr+1)&(MAX_QUEUE_SIZE-1);
    enable();

    return SUCCESS;
}



