/*
 * @file    Messages.c
 * @brief   Contains all kernel send and receive functionality,
 *          Mailbox and Message pool initialization
 *          Mailbox bind and unbind functions
 *          Message pool retrieve and add functions
 *
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    29-Oct-2019 (created)
 * @date    26-Nov-2019 (edited)
 */

#define GLOBAL_MESSAGES
#include "Messages.h"
#include "SVC.h"
#include "KernelCall.h"
#include "Utilities.h"
#include <stdio.h>
#include <stdlib.h>

#define  NEXT i+1
#define  PREV i-1
#define  STARTING_INDEX 0

/*Pointer to the head of the message pool*/
static Message * messagePool = NULL;

/*Mailbox List*/
static MailBox mailboxList[MAILBOX_AMOUNT];

/*free Mail Box pointer used for bind any*/
static MailBox * freeMailBox;

static ReceiveLog * receiveLogPool = NULL;

/*
 * @brief   Initializes the doubly linked list connecting unowned
 *          mailboxs allowing bind any in constant time
 */
void initMailBoxList(void)
{
    int i;

    mailboxList[i].index = i;

    //initialize free to mailbox at starting index
    freeMailBox = &mailboxList[i];

    //first and last mailboxes must be initialized to point
    //to eachother to give circular doubly linked list functionality

    mailboxList[MAILBOX_MAX_INDEX].nextFree = &mailboxList[i];
    mailboxList[i].prevFree = &mailboxList[MAILBOX_MAX_INDEX];
    mailboxList[i].nextFree = &mailboxList[NEXT];

    for(i; i<MAILBOX_MAX_INDEX;i++)
    {
        mailboxList[i].index = i;
        mailboxList[i].nextFree = &mailboxList[NEXT];
        mailboxList[i].prevFree = &mailboxList[PREV];
    }

    mailboxList[i].index = i;
    mailboxList[i].prevFree = &mailboxList[PREV];
}

/*
 * @brief   To maintain process cursor horizontal
 *          position
 * @param   [in] int MB: the index of the mailbox who's PCB
 *                       is desired
 * @return  PCB * : address of mailbox owners PCB
 */
PCB * getOwnerPCB(int MB)
{
    return (PCB *)mailboxList[MB].owner;
}

/*
 * @brief   To return a message structure to the pool
 * @param   [in/out]  Message * newMsg: address of message
 *          structure being returned to the pool
 */
void addToPool(Message * newMsg)
{
    newMsg->from =NULL;
    newMsg->size= NULL;
    *(newMsg->contents)=NULL;
    newMsg->next = messagePool;
    messagePool = newMsg;
}

/*
 * @brief   To retrieve a message structure from the pool
 * @return  Message *: address of message structure retrieved
 */
Message * retrieveFromPool(void)
{
    Message * newPtr = messagePool;
    // Fault protection
    messagePool = (newPtr) ? newPtr->next : NULL;
    return newPtr;
}

/*
 * @brief   Initializes the linked list connecting the
 *          free message structures
 */
void initMessagePool(void)
{
    int i;
    for(i=0;i<MESSAGE_SYS_LIMIT;i++)
    {
        addToPool(malloc(sizeof(Message)));
    }
}

/*
 * @brief   To return a receive log structure to the pool
 * @param   [in/out]   ReceiveLog * newLog: address of receive log
 *          structure being returned to the pool
 */
void addReceiveLog(ReceiveLog * newLog)
{
    newLog->from =NULL;
    newLog->next = receiveLogPool;
    receiveLogPool = newLog;
}

/*
 * @brief   To retrieve a receive log structure from the pool
 * @return  ReceiveLog *: address of message structure retrieved
 */
ReceiveLog * retrieveReceiveLog(void)
{
    ReceiveLog * newPtr = receiveLogPool;
    // Fault protection
    receiveLogPool = (newPtr) ? newPtr->next : NULL;
    return newPtr;
}

/*
 * @brief   Initializes the linked list connecting the
 *          free receive log structures
 */
void initReceiveLogs(void)
{
    int i;
    for(i=0;i<RECEIVE_LOG_AMOUNT;i++)
    {
        addReceiveLog(malloc(sizeof(ReceiveLog)));
    }
}

void addReceiveLogToPCB(PCB* owner, ReceiveLog* newLog)
{
    if(owner->receiveAnyHead)
    {
        ReceiveLog * temp = owner->receiveAnyTail;
        owner->receiveAnyTail = newLog;
        owner->receiveAnyTail->next = NULL;

        if (temp)
        {
            temp->next = owner->receiveAnyTail;
            owner->receiveAnyTail->prev = temp;
        }
        else
        {
            owner->receiveAnyTail->prev = owner->receiveAnyHead;
        }
    }
    else
    {

        //first message in mailbox

        owner->receiveAnyHead = newLog;
        owner->receiveAnyTail = NULL;
        owner->receiveAnyHead->next = owner->receiveAnyTail;
        owner->receiveAnyHead->prev = NULL;
    }
}

int getOldestMessageMB(PCB* owner)
{
    int toReturn = ANY;
    if(owner->receiveAnyHead)
    {
        toReturn = owner->receiveAnyHead->from;
    }
    return toReturn;
}

/*
 * @brief   Allow processes to bind to a mailbox
 * @param   int desiredMB: Mailbox that the process
 *          wants to bind to. If desiredMB == 16
 *          it will bind to the MB pointed to by
 *          freeMailBox
 * @return  Bind Fail = -4 or Mailbox Number that
 *          was binded to
 *
 * */
int kernelBind(int desiredMB)
{
    int result = SUCCESS;

    if(!(STARTING_INDEX<=desiredMB&&desiredMB<=MAILBOX_AMOUNT))
    {return BIND_FAIL;}

    if(desiredMB == ANY)
    {
        // BIND ANY

        if(freeMailBox)
        {
            desiredMB = freeMailBox->index;
            freeMailBox->nextFree->prevFree=freeMailBox->prevFree;
            freeMailBox->prevFree->nextFree=freeMailBox->nextFree;
            freeMailBox = (freeMailBox->nextFree==freeMailBox)? NULL : freeMailBox->nextFree;

            mailboxList[desiredMB].owner = (struct ProcessControlBlock_*)getRunningPCB();
            mailboxList[desiredMB].head = mailboxList[desiredMB].tail = NULL;
            result = desiredMB;
        }
        else
        {
            result = BIND_FAIL;
        }
    }
    else
    {
        // BIND SPECIFIC

        if(!(mailboxList[desiredMB].owner))
        {
            mailboxList[desiredMB].owner = (struct ProcessControlBlock_*)getRunningPCB();
            mailboxList[desiredMB].prevFree->nextFree = mailboxList[desiredMB].nextFree;
            mailboxList[desiredMB].nextFree->prevFree = mailboxList[desiredMB].prevFree;
            mailboxList[desiredMB].head =mailboxList[desiredMB].tail = NULL;

            if(desiredMB==freeMailBox->index)
            {
                freeMailBox = (freeMailBox->nextFree==freeMailBox)? NULL : freeMailBox->nextFree;
            }
        }
        else
        {
            result = BIND_FAIL;
        }
    }
    return result;
}

/*
 * @brief   Allow processes to unbind from a mailbox
 * @param   int releasedMB: Mailbox number the process
 *          wants to release
 * @return  Unbind Fail = -5 or Success = 1
 *
 * */
int kernelUnbind(int releaseMB)
{
    int result = UNBIND_FAIL;

    if(mailboxList[releaseMB].owner == getRunningPCB()||!(STARTING_INDEX<=releaseMB&&releaseMB<=MAILBOX_AMOUNT))
    {
        mailboxList[releaseMB].owner = NULL;

        mailboxList[releaseMB].nextFree = (freeMailBox)? freeMailBox : &mailboxList[releaseMB];
        mailboxList[releaseMB].prevFree = (freeMailBox)? freeMailBox->prevFree : &mailboxList[releaseMB];

        freeMailBox = &mailboxList[releaseMB];
        freeMailBox->nextFree->prevFree =  &mailboxList[releaseMB];

        return SUCCESS;
    }
    return result;
}

/*
 * @brief   Adds message to a mailbox, if destination process is blocked; it transfers message
 *          and unblocks
 * @param   [in] int destinationMB: MB # of the destination process
 *          [in] int fromMB: MB # of the sending process
 *          [in] void* contents: data to be sent
 *          [in] int size: amount of data measured in bytes
 * @return  int: 1->success, -1->failure
 */
int kernelSend(int destinationMB, int fromMB, void * contents, int size)
{

   PCB * runningPCB = (struct ProcessControlBlock_*) getRunningPCB();

   //check the validity of arguments
   if((mailboxList[fromMB].owner != runningPCB)||
      (!(mailboxList[destinationMB].owner))||
      (MESSAGE_SYS_LIMIT<size))
   {return SEND_FAIL;}

   //check if the destination process is blocked
   if(mailboxList[destinationMB].owner->contents)
   {
       /* If the owner's PCB is blocked*/

     *(mailboxList[destinationMB].owner->from) = fromMB;
     int copySize = (mailboxList[destinationMB].owner->size< size)?
                     mailboxList[destinationMB].owner->size :
                     size;
      memcpy(mailboxList[destinationMB].owner->contents, contents, copySize);
      addPCB(mailboxList[destinationMB].owner, mailboxList[destinationMB].owner->priority);
      *(mailboxList[destinationMB].owner->returnValue) = copySize;
      mailboxList[destinationMB].owner->contents = NULL;

   }
   else
   {
       //if not blocked, fill a message structure from the
       //message pool and put it in the mailbox
       ReceiveLog * newRecv = retrieveReceiveLog();
       Message * newMessage = retrieveFromPool();

       if(newMessage)
       {
           newRecv->from = destinationMB;
           newMessage->from = fromMB;
           newMessage->size = size;
           memcpy(newMessage->contents, contents, size);
           if(mailboxList[destinationMB].head)
           {
               ReceiveLog * tempLog = mailboxList[destinationMB].newest;
               mailboxList[destinationMB].newest= newRecv;
               mailboxList[destinationMB].newest->myNext = NULL;
               if (tempLog)
               {
                    tempLog->myNext = mailboxList[destinationMB].newest;
               }

               Message * temp = mailboxList[destinationMB].tail;
               mailboxList[destinationMB].tail = newMessage;
               mailboxList[destinationMB].tail->next = NULL;
               if(temp)
               {
                   temp->next = mailboxList[destinationMB].tail;
               }
           }
           else
           {

               //first message in mailbox

               mailboxList[destinationMB].oldest = newRecv;
               mailboxList[destinationMB].oldest->myNext = mailboxList[destinationMB].newest;

               mailboxList[destinationMB].head = newMessage;
               mailboxList[destinationMB].tail=NULL;
               mailboxList[destinationMB].head->next = mailboxList[destinationMB].tail;
           }
           addReceiveLogToPCB(runningPCB, newRecv);
       }
       else{ return SEND_FAIL;}

   }
   return SUCCESS;
}

/*
 * @brief   Take message from a mailbox, blocks if mailbox is empty
 *          and unblocks
 * @param   [in] int bindedMB: MB # of the receiving process
 *          [out] int* returnMB: MB # of the process that sent the message
 *          [in/out] void* contents: address where data is stored
 *          [in/out] int* maxSize: [in]maximum amount of bytes the process will take
 *                                 [out] amount of bytes that were copied
 * @return  int: -1->failure, 1->success
 */
int kernelReceive(int bindedMB, int* returnMB, void * contents, int * maxSize)
{
    int result;
    PCB * runningPCB = (struct ProcessControlBlock_*) getRunningPCB();

    if(bindedMB == ANY)
    {
        bindedMB = getOldestMessageMB(runningPCB);
    }

    if(bindedMB!=ANY)
    {
        if ((mailboxList[bindedMB].owner != runningPCB)
                || !(STARTING_INDEX <= bindedMB && bindedMB < MAILBOX_AMOUNT)
                || (MESSAGE_SYS_LIMIT < *maxSize))
        {return RECV_FAIL;}


        if (mailboxList[bindedMB].head)
        {
            // Mailbox contains at least one message

            if( runningPCB->receiveAnyHead == mailboxList[bindedMB].oldest)
            {
                runningPCB->receiveAnyHead = runningPCB->receiveAnyHead->next;

                if(runningPCB->receiveAnyHead)
                {
                    runningPCB->receiveAnyHead->prev = NULL;
                }
            }
            else if(runningPCB->receiveAnyTail == mailboxList[bindedMB].oldest)
            {
                if(runningPCB->receiveAnyTail->prev == runningPCB->receiveAnyHead)
                {
                    runningPCB->receiveAnyTail=NULL;
                }
                else
                {
                    runningPCB->receiveAnyTail = runningPCB->receiveAnyTail->prev;
                    runningPCB->receiveAnyTail->next=NULL;
                }

            }
            else
            {
                mailboxList[bindedMB].oldest->next->prev =mailboxList[bindedMB].oldest->prev;
                mailboxList[bindedMB].oldest->prev->next = mailboxList[bindedMB].oldest->next;
            }

            mailboxList[bindedMB].oldest = mailboxList[bindedMB].oldest->myNext;

            *returnMB = mailboxList[bindedMB].head->from;

            int copySize =
                    (mailboxList[bindedMB].head->size < *maxSize) ?
                            mailboxList[bindedMB].head->size : *maxSize;

            memcpy(contents, mailboxList[bindedMB].head->contents, copySize);
            Message * temp = mailboxList[bindedMB].head;
            mailboxList[bindedMB].head = mailboxList[bindedMB].head->next;
            addToPool(temp);
            return SUCCESS;
        }
    }
    // BLOCK
    removePCB();
    runningPCB->from = returnMB;
    runningPCB->contents = contents;
    runningPCB->size = *maxSize;
    runningPCB->returnValue = maxSize;
    runningPCB->sp = get_PSP();
    runningPCB = (struct ProcessControlBlock_*) getRunningPCB();
    set_PSP(runningPCB->sp);

    return SUCCESS;
}

