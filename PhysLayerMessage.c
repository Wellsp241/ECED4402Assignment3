/*
 * @file    PhysLayerMessage.c
 * @brief   Contains handlers for incoming messages to physical
 *          layer from data link layer and UART1 handler
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    8-Dec-2019 (created)
 * @date
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
 */
void PhysLayerFromDLHandler(void)
{
    int DLtoPhysMB;
    int senderMB;
    int maxrecvSize = sizeof(DLMessage) * 2;
    int maxfwdSize = maxrecvSize + NUMPHYSICALBYTES;
    int recvSize;
    int i;
    unsigned char specialchar = 0;
    /* Reserve space for a physical layer format message.
     * received points to first byte of container after
     * start byte (STX)
     *
     * Field:       |Start|Message|Checksum|End|
     * Pointers:           received
     */
    char toForward[maxfwdSize];
    char * received = &toForward[1];
    char * checksum;

    /* Bind to dedicated mailbox */
    DLtoPhysMB = bind(DLPHYSMB);

    /* Ensure that mailbox bind was successful */
    if(DLtoPhysMB == DLPHYSMB)
    {
        /* Set start character of message to forward */
        toForward[0] = STX;

        /* Loop indefinitely while processing messages received from
         * data link layer
         */
        while(1)
        {
            /* Receive message from mailbox. These messages follow the DataLinkMessage format */
            recvSize = maxrecvSize;
            recvMessage(DLtoPhysMB, &senderMB, received, &recvSize);

            /* Remove extra DLEs from received message */
            for(i = 0; i < recvSize; i++)
            {
                /* Check whether previous character was a DLE */
                if(specialchar == 0)
                {
                    /* Previous character was not a DLE so remove DLE if spotted */
                    if(received[i] == DLE)
                    {
                        memmove(&received[i], &received[i + 1], recvSize - i);
                        specialchar = 1;
                        recvSize--;
                    }
                }
                else
                {
                    /* Do not remove this character since it was preceded by DLE */
                    specialchar = 0;
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
        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
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

        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
    return;
}
