/*
 * @file    TrainsetInit.c
 * @brief   Contains high-priority application layer process
 *          that performs initial setup of the train set to
 *          a known safe state
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    9-Dec-2019 (created)
 * @date
 */
#include "KernelCall.h"
#include "DataLinkMessage.h"
#include "TrainRouting.h"

/* Define mailbox used by initialization progress */
#define INITMB  (16)

/*
 * @brief   Process meant to run prior to any other train set
 *          activity to "reset" the train set to a known state
 */
void InitTrainset(void)
{
    unsigned char i;
    int msgSize = sizeof(AppMessage);
    /* Reserve space for application layer message */
    char Msg[msgSize];
    union AppFromMB toForward;
    toForward.recvAddr = Msg;

    /* Set all switches to the "straight" position */
    for(i = 0; i < NUM_SWITCHES; i++)
    {
        toForward.msgAddr->code = SWITCH_THROW;
        toForward.msgAddr->arg1 = i;
        toForward.msgAddr->arg2 = SW_STRAIGHT;

        DataLinkfromAppHandler(toForward.recvAddr);
    }

    /* Make the train go from sensor 1 to sensor 10 */
    Go(1, 10);

    /* Initialization should be done now so terminate */
    return;
}
