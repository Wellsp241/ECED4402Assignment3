/*
 * @file    AppLayerMessage.c
 * @brief   Contains process dedicated to processing messages
 *          received from Data Link Layer
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    9-Dec-2019 (edited)
 */
#include "KernelCall.h"
#include "AppLayerMessage.h"
#include "DataLinkMessage.h"
#include "TrainRouting.h"


/*
 * @brief   Function used to send a request to stop any/all locomotives
 * @param   [in] unsigned char train: Index of train to be stopped
 * @param   [in] int appMailbox: Index of mailbox used by message handler
 *          process
 */
void stopLocomotives(unsigned char train, int appMailbox)
{
    unsigned int msgSize = sizeof(AppMessage);
    char replyMsg[msgSize];
    union AppFromMB reply;
    reply.recvAddr = replyMsg;

    /* Build locomotive stop request */
    reply.msgAddr->code = MAG_DIR_SET;
    reply.msgAddr->arg1 = train;
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
    struct RoutingTableEntry * path;
    int msgSize = sizeof(AppMessage);
    /* Reserve space for messages */
    char message[msgSize];
    union AppFromMB received;
    received.recvAddr = message;
    char replyMsg[msgSize];
    union AppFromMB reply;
    reply.recvAddr = replyMsg;
    union Mag_Dir replySpeed;
    replySpeed.Speed = &(reply.msgAddr->arg2);

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
            recvMessage(appMailbox, &senderMB, received.recvAddr, &msgSize);

            /* Act based on message's code */
            switch(received.msgAddr->code)
            {
            /* Hall sensor has been triggered */
            case HALL_TRIGGERED:
                //TODO: Need to check state of each train to determine which train triggered this hall sensor
                path = getPath(received.msgAddr->arg1, TState.destination);

                /* Check whether train must be stopped */
                if(path->stop == PATH_STOP)
                {
                    /* Locomotive is at its destination so it must be stopped */
                    stopLocomotives(0, appMailbox);
                }
                else
                {
                    /* Determine whether any messages need to be sent to adjust the train's course.
                     * Start by checking for a needed magnitude/direction set message.
                     */
                    if(path->dir != TState.speed.direction)
                    {
                        /* Direction needs to be changed so send speed change request */
                        reply.msgAddr->code = MAG_DIR_SET;
                        reply.msgAddr->arg1 = 0;
                        replySpeed.Speed->direction = path->dir;
                        replySpeed.Speed->magnitude = TState.speed.magnitude;

                        sendMessage(APPDATALINKMB, appMailbox, reply.recvAddr, msgSize);
                    }

                }

                /* Must also send acknowledgment message */
                reply.msgAddr->code = HALL_TRIGGERED_ACK;
                reply.msgAddr->arg1 = received.msgAddr->arg1;
                reply.msgAddr->arg2 = 0;
                sendMessage(APPDATALINKMB, appMailbox, reply.recvAddr, msgSize);

                break;
            /* Reply to a hall sensor reset request */
            case HALL_RESET_ACK:
                /* Check whether reset was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(ALL, appMailbox);
                }
                break;
            /* Reply to a magnitude/direction set request */
            case MAG_DIR_ACK:
                /* Check whether speed set was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(ALL, appMailbox);
                }
                break;
            /* Reply to a switch-throw request */
            case SWITCH_THROW_ACK:
                /* Check whether switch throw was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(ALL, appMailbox);
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


