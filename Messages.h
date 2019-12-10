/*
 * @file    Message.h
 * @brief   Contains function prototypes allowing the kernel
 *          to utilize message queues.
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    29-Oct-2019 (created)
 * @date    26-Nov-2019 (edited)
 */
#pragma once
#include "Process.h"
#include "Utilities.h"

/* Maximum number of message queues allowed */
#define MAILBOX_AMOUNT 16
#define MAILBOX_MAX_INDEX MAILBOX_AMOUNT - 1


/* Structure containing information about messages */
typedef struct Message_
{

    /* ID of destination process */
    int from;

    /*Next pointer for linked list*/
    struct Message_* next;
    /* Size in bytes of message */
    int size;

    char contents[MESSAGE_SYS_LIMIT];

}Message;

typedef struct ReceiveLog_
{
    // Mailbox to check
    int mailbox;
    // Next receive log
    struct ReceiveLog_ * next;
    // Prev receive log
    struct ReceiveLog_ * prev;
    // Next receive log from same mailbox
    struct ReceiveLog_ * myNext;

}ReceiveLog;


/* Structure comprising a single message queue */
typedef struct MailBox_
{
    /* Owner of message queue */
    struct ProcessControlBlock_ * owner;
    /* Index of first occupied slot in message queue */
    Message* head;
    /* Index of next empty slot in message queue */
    Message* tail;

    // doubly linked list of free mailboxes
    struct MailBox_ * nextFree;

    struct MailBox_ * prevFree;

    int index;

    ReceiveLog * oldest;

    ReceiveLog * newest;

}MailBox;

#ifndef GLOBAL_MESSAGES
#define GLOBAL_MESSAGES

extern int kernelBind(unsigned int);
extern int kernelUnbind(unsigned int);
extern int kernelSend(int,int,void *, int);
extern int kernelReceive(int,int*,void*,int*);
extern void initMessagePool(void);
extern void initMailBoxList(void);
extern PCB * getOwnerPCB(int);
extern void initReceiveLogs(void);

#else

int kernelSend(int,int,void *, int);
int kernelReceive(int,int*,void*,int*);
void addToPool(Message *);
Message * retrieveFromPool(void);
void addReceiveLog(ReceiveLog *);
ReceiveLog * retrieveReceiveLog(void);

#endif /* GLOBAL_SVC */
