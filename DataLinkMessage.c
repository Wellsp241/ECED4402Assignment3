/*
 * @file    DataLinkMessage.c
 * @brief   Contains handlers for incoming messages to data link
 *          layer from application layer and physical layer
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 */
#include "DataLinkMessage.h"

/* Definition of maximum sequence number */
#define MAX_SEQUENCE    (8)

/* Macro used to increment sequence/expected numbers */
#define INCREMENT_SEQUENCE(x)   ((x + 1) % MAX_SEQUENCE)

/* Sequence number and expected number of data link layer */
DLControl DLState = {0, 0, DATA};

/*
 * @brief   Handler of messages to data link layer
 *          from application layer
 */
void DataLinkfromAppHandler(void)
{
    int ApptoDLMB;
    int senderMB;
    int recvSize = sizeof(AppMessage);
    /* Reserve space for a data link format message */
    DLMessage toForward;

    /* Bind to dedicated mailbox */
    ApptoDLMB = bind(APPDATALINKMB);

    /* Ensure that mailbox bind was successful */
    if(ApptoDLMB == APPDATALINKMB)
    {
        /* Loop indefinitely while processing messages received from
         * application layer
         */
        while(1)
        {
            /* Receive message from mailbox. These messages follow the AppLayerMessage format */
            recvMessage(ApptoDLMB, &senderMB, toForward->message, &recvSize);

            /* Fill control field of message to forward with current data link state */
            DLState.type = DATA;
            toForward.control = DLState;

            /* Assemble message to forward */
            toForward.length = *recvSize;

            //TODO: Find a way to not send arg2 of AppLayerMessages if it is not needed

            /* Forward message to physical layer */
            sendMessage();
        }
    }

    /* If this return statement is reached, the process terminates because
     * mailbox bind was unsuccessful
     */
    return;
}
