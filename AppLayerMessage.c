/*
 * @file    AppLayerMessage.c
 * @brief   Contains process dedicated to processing messages
 *          received from Data Link Layer
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    7-Dec-2019 (edited)
 */
#include "KernelCall.h"
#include "AppLayerMessage.h"
#include "DataLinkMessage.h"


/*
 * @brief   Function used to send a request to stop all locomotives
 * @param   [in] int appMailbox: Index of mailbox used by message handler
 *          process
 */
inline void stopLocomotives(int appMailbox)
{
    unsigned int msgSize = sizeof(AppMessage);
    char replyMsg[msgSize];
    union AppFromMB reply;
    reply.recvAddr = replyMsg;

    /* Build locomotive stop request */
    reply.msgAddr->code = MAG_DIR_SET;
    reply.msgAddr->arg1 = ALL;
    reply.msgAddr->arg2 = STOP;

    /* Send request to data link layer */
    sendMessage(APPDATALINKMB, appMailbox, reply.recvAddr, msgSize);

    return;
}

/*
 * @brief   Process dedicated to handling messages received
 *          from Data Link Layer (originating from train set)
 */
void AppMessageHandler(void)
{
    int appMailbox;
    int senderMB;
    int msgSize = sizeof(AppMessage);
    /* Reserve space for messages */
    char message[msgSize];
    union AppFromMB received;
    received.recvAddr = message;

    /* Bind to dedicated mailbox */
    appMailbox = bind(APPLAYERMB);

    /* Ensure that mailbox bind was successful */
    if(appMailbox == APPLAYERMB)
    {
        /* Bind was successful so loop forever while processing messages
         * received from Data Link Layer.
         */
        while(1)
        {
            /* Receive message from mailbox */
            recvMessage(appMailbox, &senderMB, received.recvAddr, msgSize);

            /* Act based on message's code */
            switch(received.msgAddr->code)
            {
            /* Hall sensor has been triggered */
            case HALL_TRIGGERED:
                //TODO: Need to check state of each train to determine which train triggered this hall sensor
                //TODO: Then we need to check what this train needs to do to reach its destination. This might
                //      be best handled by a dedicated process.
                break;
            /* Reply to a hall sensor reset request */
            case HALL_RESET_ACK:
                /* Check whether reset was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(appMailbox);
                }
                break;
            /* Reply to a magnitude/direction set request */
            case MAG_DIR_ACK:
                /* Check whether speed set was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(appMailbox);
                }
                break;
            /* Reply to a switch-throw request */
            case SWITCH_THROW_ACK:
                /* Check whether switch throw was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(appMailbox);
                }
                break;
            /* If message code is not recognized, do not act on message */
            default:
                break;
            }
        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
    return;
}


