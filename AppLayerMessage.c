/*
 * @file    AppLayerMessage.c
 * @brief   Contains process dedicated to processing messages
 *          received from Data Link Layer
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    10-Dec-2019 (edited)
 */
#include "string.h"
#include "KernelCall.h"
#include "AppLayerMessage.h"
#include "DataLinkMessage.h"
#include "TrainRouting.h"
#include "Utilities.h"


/*
 * @brief   Function used to send a request to stop any/all locomotives
 * @param   [in] unsigned char train: Index of train to be stopped
 */
void stopLocomotives(unsigned char train)
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
    sendMessage(APPDATALINKMB, DATALINKAPPMB, reply.recvAddr, msgSize);

    return;
}

/*
 * @brief   Function dedicated to handling messages received
 *          from Data Link Layer
 */
void AppfromDataLinkHandler(void)
{
    int Mailbox;
    int senderMB;
    struct RoutingTableEntry * path;
    int msgSize = sizeof(AppMessage);
    /* Reserve space for received and reply messages */
    char Msg[msgSize];
    union AppFromMB received;
    received.recvAddr = Msg;
    char replyMsg[msgSize];
    union AppFromMB reply;
    reply.recvAddr = replyMsg;
    union Mag_Dir replySpeed;
    replySpeed.rawByte = &(reply.msgAddr->arg2);

    /* Bind to dedicated mailbox */
    Mailbox = bind(DATALINKAPPMB);

    /* Ensure bind was successful */
    if(Mailbox == SUCCESS)
    {
        /* Loop indefinitely while processing message received from data link layer */
        while(1)
        {
            /* Receive message from dedicated mailbox */
            recvMessage(DATALINKAPPMB, &senderMB, received.recvAddr, &msgSize);

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
                    stopLocomotives(0);
                }
                else
                {
                    /* Determine whether any messages need to be sent to adjust the train's course.
                     * Start by checking for a needed magnitude/direction set message.
                     */
                    if((path->dir != TState.speed.direction) || (TState.stop == PATH_STOP))
                    {
                        /* Direction needs to be changed so send speed change request */
                        reply.msgAddr->code = MAG_DIR_SET;
                        reply.msgAddr->arg1 = TRAIN;
                        replySpeed.Speed->direction = path->dir;
                        replySpeed.Speed->magnitude = TState.speed.magnitude;

                        sendMessage(APPDATALINKMB, DATALINKAPPMB, reply.recvAddr, msgSize);

                        /* Update train's state */
                        TState.speed.direction = path->dir;
                    }

                    /* Check whether a switch-throw request must be sent */
                    if(((Switch_States & (1 << path->switchnum)) == 0) != path->switchstate)
                    {
                        /* Build and send switch-throw request message */
                        reply.msgAddr->code = SWITCH_THROW;
                        reply.msgAddr->arg1 = path->switchnum;
                        reply.msgAddr->arg2 = path->switchstate;
                        sendMessage(APPDATALINKMB, DATALINKAPPMB, reply.recvAddr, msgSize);

                        /* Update switch's state */
                        if(path->switchstate == SW_DIVERGED)
                        {
                            Switch_States &= ~(1 << path->switchnum);
                        }
                        else
                        {
                            Switch_States |= 1 << path->switchnum;
                        }
                    }
                }

                /* Must also send acknowledgment message */
                reply.msgAddr->code = HALL_TRIGGERED_ACK;
                reply.msgAddr->arg1 = received.msgAddr->arg1;
                reply.msgAddr->arg2 = 0;
                sendMessage(APPDATALINKMB, DATALINKAPPMB, reply.recvAddr, msgSize);

                break;
            /* Reply to a hall sensor reset request */
            case HALL_RESET_ACK:
                /* Check whether reset was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(ALL);
                }

                break;
            /* Reply to a magnitude/direction set request */
            case MAG_DIR_ACK:
                /* Check whether speed set was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(ALL);
                }
                break;
            /* Reply to a switch-throw request */
            case SWITCH_THROW_ACK:
                /* Check whether switch throw was successful */
                if(received.msgAddr->arg2 != 0)
                {
                    /* Stop all locomotives */
                    stopLocomotives(ALL);
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


/*
 * @brief   Process giving directions to trains based on user input
 */
void AppfromUART0Handler(void)
{
    int Mailbox;
    int senderMB;
    int recvSize;
    int fwdSize = sizeof(AppMessage);
    unsigned char start;
    unsigned char end;
    struct RoutingTableEntry * path;
    /* Reserve space for received and reply messages */
    char received[MESSAGE_SYS_LIMIT];
    char Msg[fwdSize];
    union AppFromMB reply;
    reply.recvAddr = Msg;
    union Mag_Dir replySpeed;
    replySpeed.rawByte = &(reply.msgAddr->arg2);

    /* Bind to dedicated mailbox */
    Mailbox = bind(UART0APPMB);

    /* Ensure bind was successful */
    if(Mailbox == SUCCESS)
    {
        /* Loop indefinitely while processing messages received from UART0 handler */
        while(1)
        {
            /* Receive message from dedicated mailbox */
            //TODO: This input only works for single-digit sensors
            strcpy(received, "Input Start Point: ");
            sendMessage(UART0_IP_MB, UART0APPMB, received, 19);
            recvSize = MESSAGE_SYS_LIMIT;
            recvMessage(UART0APPMB, &senderMB, received, &recvSize);
            start = *received - '0';

            strcpy(received, "Input End Point: ");
            sendMessage(UART0_IP_MB, UART0APPMB, received, 17);
            recvSize = MESSAGE_SYS_LIMIT;
            recvMessage(UART0APPMB, &senderMB, received, &recvSize);
            end = *received - '0';
            TState.destination = end;
            path = getPath(start, end);

            /* Check whether train must be stopped */
            if(path->stop == PATH_STOP)
            {
                /* Locomotive is at its destination so it must be stopped */
                stopLocomotives(0);
            }
            else
            {
                /* Determine whether any messages need to be sent to adjust the train's course.
                 * Start by checking for a needed magnitude/direction set message.
                 */
                if((path->dir != TState.speed.direction) || (TState.stop == PATH_STOP))
                {
                    /* Direction needs to be changed so send speed change request */
                    reply.msgAddr->code = MAG_DIR_SET;
                    reply.msgAddr->arg1 = TRAIN;
                    replySpeed.Speed->direction = path->dir;
                    replySpeed.Speed->magnitude = TState.speed.magnitude;

                    sendMessage(APPDATALINKMB, UART0APPMB, reply.recvAddr, fwdSize);

                    /* Update train's state */
                    TState.speed.direction = path->dir;
                    TState.stop = PATH_GO;
                }

                /* Check whether a switch-throw request must be sent */
                if(path->switchnum != NO_SWITCH)
                {
                    if((Switch_States & (1 << (path->switchnum - 1)) != 0) != path->switchstate)
                    {
                        /* Build and send switch-throw request message */
                        reply.msgAddr->code = SWITCH_THROW;
                        reply.msgAddr->arg1 = path->switchnum;
                        reply.msgAddr->arg2 = path->switchstate;
                        sendMessage(APPDATALINKMB, UART0APPMB, reply.recvAddr, fwdSize);

                        /* Update switch's state */
                        if(path->switchstate - 1 == SW_DIVERGED)
                        {
                            Switch_States &= ~(1 << (path->switchnum - 1));
                        }
                        else
                        {
                            Switch_States |= 1 << (path->switchnum - 1);
                        }
                    }
                }
            }
        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
    return;
}


