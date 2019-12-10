/*
 * @file    DataLinkMessage.c
 * @brief   Contains handlers for incoming messages to data link
 *          layer from application layer and physical layer
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    9-Dec-2019 (edited)
 */
#include "stdlib.h"
#include "string.h"
#include "KernelCall.h"
#include "DataLinkMessage.h"
#include "PhysLayerMessage.h"


/* Definition of sliding window size */
#define MAX_SEQUENCE    (8)

/* Macro used to increment sequence/expected numbers */
#define INCREMENT_SEQUENCE(x)   ((x + 1) % MAX_SEQUENCE)

/* Sequence number and expected number of data link layer:
 * {Ns, Nr, Type}
 */
DLControl DLState = {0, 0, DATA};

/* Circular queue of sent messages. This container
 * is meant to preserve sent messages in case of
 * failure (NACK from trainset).
 */
DLMessage sentQueue[MAX_SEQUENCE];


/*
 * @brief   Routine used to re-send all failed messages
 * @param   [in] unsigned char start: Starting index to start
 *          re-sending from
 */
inline void forwardMessages(unsigned char start)
{
    unsigned char i;
    int fwdSize = sizeof(DLMessage);
    union DLFromMB toForward;

    //TODO: What if two processes call this function at once?
    //      No harm can be done from this but it could be really
    //      inefficient to not handle it. Though it is unlikely to
    //      occur in the first place.

    /* Loop through all messages to re-send */
    for(i = start; i != DLState.sequenceNum; i = INCREMENT_SEQUENCE(i))
    {
        /* Set forwarded pointer to message to re-send */
        toForward.msgAddr = &sentQueue[i];

        /* Send this message to the physical layer */
        PhysLayerFromDLHandler(toForward.recvAddr, fwdSize);
    }

    return;
}


/*
 * @brief   Handler of messages to data link layer
 *          from application layer. Prepares these messages
 *          to be forwarded through the physical layer.
 * @param   [in] char * message: Pointer to message received from
 *          application layer
 */
void DataLinkfromAppHandler(char * message)
{
    int recvSize = sizeof(AppMessage);
    int fwdSize = sizeof(DLMessage);
    /* Reserve space for a data link format message.
     * toForward points to start of DLMessage
     * received points to AppMessage field of DLMessage
     *
     * Field:       |Ctrl|Length|AppMessage|
     * Pointers: toForward    received
     */
    char Msg[fwdSize];
    union DLFromMB toForward;
    toForward.recvAddr = Msg;
    union AppFromMB received;
    received.msgAddr = &(toForward.msgAddr->appMessage);

    /* Copy application layer packet into message to forward */
    memcpy(received.recvAddr, message, recvSize);

    /* Fill control field of message to forward with current data link state.
     * Note that the type field of our saved DLState is not ever changed from DATA.
     */
    toForward.msgAddr->control = DLState;

    /* Assemble message to forward */
    toForward.msgAddr->length = fwdSize;

    /* Forward message to physical layer */
    PhysLayerFromDLHandler(toForward.recvAddr, fwdSize);

    /* Copy this message to the sent queue in case of failure */
    sentQueue[DLState.sequenceNum] = *(toForward.msgAddr);

    /* Increment Sequence Number of Current State */
    DLState.sequenceNum = INCREMENT_SEQUENCE(DLState.sequenceNum);

    return;
}


/*
 * @brief   Handler of messages to data link layer
 *          from physical layer. Prepares these messages
 *          to be forwarded to the application layer, if applicable.
 * @param   [in] char * message: Pointer to message received from physical layer
 */
void DataLinkfromPhysHandler(char * message)
{
    int ctlSize = sizeof(DLState);
    /* Reserve space for a data link format message.
     * received points to start of DLMessage
     * toForward points to AppMessage field of DLMessage
     *
     * Field:       |Ctrl|Length|AppMessage|
     * Pointers: received     toForward
     */
    union DLFromMB received;
    received.recvAddr = message;
    union AppFromMB toForward;
    toForward.msgAddr = &(received.msgAddr->appMessage);

    /* Act on received message's type */
    switch(received.msgAddr->control.type)
    {
    /* Data message; forward this to the application layer */
    case DATA:
        /* First check that attached sequence number matches what we expect */
        if(received.msgAddr->control.sequenceNum != DLState.receivedNum)
        {
            /* Sequence number mismatch has occurred so must send NACK reply and
             * discard the received packet.
             */
            received.msgAddr->control = DLState;
            received.msgAddr->control.type = NACK;

            /* Send this reply to the physical layer for forwarding to the train set */
            PhysLayerFromDLHandler(received.recvAddr, ctlSize);
        }
        else
        {
            /* Sequence number matches so first increment received number of current state */
            DLState.receivedNum = INCREMENT_SEQUENCE(DLState.receivedNum);

            /* Since the Nr field of this received message implies an ACK, need to
             * reset timer on ACKed messages.
             */
            //TODO: Do this once time server has been finished

            /* Build control field to send to physical layer */
            received.msgAddr->control = DLState;
            received.msgAddr->control.type = ACK;
            PhysLayerFromDLHandler(received.recvAddr, ctlSize);

            /* Send non-data link portion of received message to application layer */
            AppfromDataLinkHandler(toForward.recvAddr);
        }
        break;
    /* Acknowledgment message; can discard acknowledged messages */
    case ACK:
        //TODO: When time server is in place, need to reset timer on acknowledged messages
        break;
    /* Negative Acknowledgment message; must forward each missed message */
    case NACK:
        forwardMessages(received.msgAddr->control.receivedNum);
        break;
    default:
        break;
    }

    return;
}


/*
 * @brief   Process that checks for timeouts on messages that
 *          have yet to be acknowledged.
 */
void DataLinkTimeoutHandler(void)
{
    /* Loop indefinitely while looking for a timeout */
    while(1)
    {
        //TODO: Once time server is added, check earliest
        //      sent message that has yet to be ACKed.
        /*
        if(###)
        {
            forwardMessages(##, PHYSDATALINKMB);
        }
        */
    }

    /* This return statement should never be reached;
     * but if it does, terminate the process
     */
    return;
}
