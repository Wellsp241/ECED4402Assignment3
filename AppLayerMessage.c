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
#include "PhysLayerMessage.h"
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
    DataLinkfromAppHandler(reply.recvAddr);

    return;
}

/*
 * @brief   Function dedicated to handling messages received
 *          from Data Link Layer (originating from train set)
 * @param   [in] char * message: Pointer to message received from data link
 *          layer
 */
void AppfromDataLinkHandler(char * message)
{
    struct RoutingTableEntry * path;
    int msgSize = sizeof(AppMessage);
    union AppFromMB received;
    received.recvAddr = message;
    /* Reserve space for reply message */
    char replyMsg[msgSize];
    union AppFromMB reply;
    reply.recvAddr = replyMsg;
    union Mag_Dir replySpeed;
    replySpeed.rawByte = &(reply.msgAddr->arg2);

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
            if(path->dir != TState.speed.direction)
            {
                /* Direction needs to be changed so send speed change request */
                reply.msgAddr->code = MAG_DIR_SET;
                reply.msgAddr->arg1 = 0;
                replySpeed.Speed->direction = path->dir;
                replySpeed.Speed->magnitude = TState.speed.magnitude;

                DataLinkfromAppHandler(reply.recvAddr);

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
                DataLinkfromAppHandler(reply.recvAddr);

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
        DataLinkfromAppHandler(reply.recvAddr);

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
    int fwdSize;
    struct RoutingTableEntry * path;
    /* Reserve space for received/sent messages */
    char received[MESSAGE_SYS_LIMIT];
    char toForward[(sizeof(DLMessage) * 2) + NUM_PHYSICAL_BYTES];
    char * DLStart = &toForward[1];
    union AppFromMB command;
    command.recvAddr = &toForward[3];
    union Mag_Dir commandSpeed;
    commandSpeed.rawByte = &(command.msgAddr->arg2);

    /* Bind to dedicated mailbox */
    Mailbox = bind(UART0APPMB);

    /* Ensure bind was successful */
    if(Mailbox == UART0APPMB)
    {
        /* Loop indefinitely while processing messages received from UART0 handler */
        while(1)
        {
            /* Receive message from dedicated mailbox */
            //recvSize = MESSAGE_SYS_LIMIT;
            //recvMessage(Mailbox, &senderMB, received, &recvSize);

            /* Get path from 1 to 10 */
            path = getPath(1, 10);

            /* Check whether train has to be stopped */
            if(path->stop == PATH_STOP)
            {
                command.msgAddr->code = MAG_DIR_SET;
                command.msgAddr->arg1 = TRAIN;
                *(commandSpeed.Speed) = STOP;

                //TODO: send this to UART1
                DataLinkfromAppHandler(&toForward[1]);
            }
            else
            {
                /* Check whether train's speed/direction must be changed */
                if((path->dir != TState.speed.direction) || (TState.stop == PATH_STOP))
                {
                    command.msgAddr->code = MAG_DIR_SET;
                    command.msgAddr->arg1 = TRAIN;
                    commandSpeed.Speed->direction = path->dir;
                    commandSpeed.Speed->magnitude = TState.speed.magnitude;

                    //TODO: Send command through UART1

                    /* Adjust train state */
                    TState.speed.direction = path->dir;
                    TState.stop = PATH_GO;
                }

                /* Check whether any switches need to be thrown */
                if((Switch_States & (1 << path->switchnum) != 0) != path->switchstate)
                {
                    command.msgAddr->code = SWITCH_THROW;
                    command.msgAddr->arg1 = path->switchnum;
                    command.msgAddr->arg2 = path->switchstate;

                    //TODO: Send command through UART1

                    /* Adjust switch state */
                    if(path->switchstate == SWITCH_DIVERGED)
                    {
                        Switch_States &= ~(1 << path->switchnum);
                    }
                    else
                    {
                        Switch_States |= (1 << path->switchnum);
                    }
                }
            }

            /* Send final packet to UART1 for output */
            sendMessage(UART1_OP_MB, Mailbox, toForward, fwdSize);
        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
    return;
}


