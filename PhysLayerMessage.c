/*
 * @file    PhysLayerMessage.c
 * @brief   Contains handlers for incoming messages to physical
 *          layer from data link layer and UART1 handler
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    8-Dec-2019 (created)
 * @date    9-Dec-2019 (edited)
 */
#include "string.h"
#include "KernelCall.h"
#include "PhysLayerMessage.h"

/* Define number of bytes added to data link message by physical layer */
#define NUMPHYSICALBYTES    (3)

/*
 * @brief   Routine used by physical layer processes to calculate a message's checksum
 */
unsigned char getChecksum(char * DLMsg, int msgSize)
{
    char checksum = 0;
    int i = 0;

    /* Loop through each byte of message */
    for(i = 0; i < msgSize; i++)
    {
        /* Add byte to checksum */
        checksum += DLMsg[i];
    }

    /* Return the one's compliment of calculated checksum value */
    return ~checksum;
}


/*
 * @brief   Handler of messages to physical layer from
 *          data link layer.
 * @param   [in] char * message: Pointer to data link message to be forwarded
 * @param   [in] unsigned int recvSize: Size of received message in bytes
 */
void PhysLayerFromDLHandler(char * message, unsigned int recvSize)
{
    int i;
    /* Reserve space for a physical layer format message.
     * received points to first byte of container after
     * start byte (STX)
     *
     * Field:       |Start|Message|Checksum|End|
     * Pointers:           received
     */
    char toForward[(sizeof(DLMessage) * 2) + NUMPHYSICALBYTES];
    char * received = &toForward[1];
    char * checksum;

    /* Set start character of message to forward */
    toForward[0] = STX;

    /* Receive message from mailbox. These messages follow the DataLinkMessage format */
    memcpy(received, message, recvSize);

    /* Add extra DLEs to message where necessary */
    for(i = 0; i < recvSize; i++)
    {
        /* Check whether current character is a special character */
        if((received[i] == STX) ||
           (received[i] == ETX) ||
           (received[i] == DLE))
        {
            /* First move contents of message to make room for extra character */
            memmove(&received[i + 1], &received[i], recvSize - i);

            /* Insert extra DLE character */
            received[i] = DLE;

            /* Adjust iterator and message size */
            i++;
            recvSize++;
        }
    }

    /* Update checksum pointer */
    checksum = received + recvSize;

    /* Calculate checksum of message to forward */
    *checksum = getChecksum(received, recvSize);

    /* Add ETX character after checksum */
    *(checksum + 1) = ETX;

    /* Send this packet to UART1 handler for transmission */
    //TODO: Get mailbox number of UART1 handler here
    //sendMessage(##, DLtoPhysMB, toForward, recvSize + NUMPHYSCIALBYTES);

    return;
}


/*
 * @brief   Handler of messages to physical layer
 *          received from UART1 handler
 */
void PhysLayerFromUART1Handler(void)
{
    int UART1toPhysMB;
    int senderMB;
    int maxrecvSize = (sizeof(DLMessage) * 2) + NUMPHYSICALBYTES;
    int recvSize;
    int fwdSize;
    int i;
    /* Reserve space for a physical layer format message.
     * toForward points to first byte of container after
     * start byte (STX)
     *
     * Field:       |Start|Message|Checksum|End|
     * Pointers:           toForward
     */
    char received[maxrecvSize];
    char * toForward = &received[1];
    char * checksum;

    /* Bind to dedicated mailbox */
    UART1toPhysMB = bind(UART1PHYSMB);

    /* Ensure that mailbox bind was successful */
    if(UART1toPhysMB == UART1PHYSMB)
    {
        /* Loop indefinitely while processing messages received from
         * UART1 handler
         */
        while(1)
        {
            /* Receive message from mailbox. These messages follow the PhysLayerMessage format */
            recvSize = maxrecvSize;
            recvMessage(UART1toPhysMB, &senderMB, received, &recvSize);
            fwdSize = recvSize - NUMPHYSICALBYTES;

            /* Remove extra DLEs from message to forward */
            for(i = 0; i < fwdSize; i++)
            {
                /* Check if current character is a DLE */
                if(toForward[i] == DLE)
                {
                    /* Current character is a DLE so remove it */
                    memmove(&toForward[i], &toForward[i + 1], fwdSize - i);
                    fwdSize--;
                }
            }

            /* Adjust checksum pointer */
            checksum = toForward + fwdSize;

            /* Calculate expected checksum of message to forward */
            if(getChecksum(toForward, fwdSize) == *checksum)
            {
                /* Forward message to data link layer only if checksum is valid */
                DataLinkfromPhysHandler(toForward);
            }
        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
    return;
}
