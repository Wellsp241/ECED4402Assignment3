/*
 * @file    KernelCall.h
 * @brief   Contains functionality to identify command
 *          passed from holding buffer and call its respective
 *          function with arguments
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    17-Nov-2019 (edited)
 */
#pragma once

enum kernelcallcodes {GETID, NICE, SENDMSG, RECEIVEMSG, TERMINATE, BIND, UNBIND, BLOCK};
/*
 * @brief   Kernel Argument Structure
 * @details Holds all variables passed to kernel
 *          for when an SVC call is made
 */
typedef struct KernelCallArgs_
{
    unsigned long code;//kernel call code for SVC switch statement
    int rtnvalue;//stores the return value from SVC
    unsigned long arg1;//arguments to be passed
    unsigned long arg2;
}KernelArgs;

/*
 * @brief   Send Kernel Call Arguments
 * @details Holds all variables passed to kernel
 *          for when a send message call is made
 */
typedef struct SendMessage_
{
    int destinationMB;
    int fromMB;
    void * contents;
    int size;
}SendMessage;

/*
 * @brief   Receive Kernel Call Arguments
 * @details Holds all variables passed to kernel
 *          for when a  message is called
 */
typedef struct ReceiveMessage_
{
    int bindedMB;
    int * returnMB;
    void * contents;
    int maxSize;
}ReceiveMessage;

#ifndef GLOBAL_KERNELCALL
#define GLOBAL_KERNELCALL

extern int bind(unsigned int);
extern int unbind(unsigned int);
extern int getid(void);
extern int nice(unsigned int);
extern void terminate(void);
extern int sendMessage(int, int, void *, int);
extern int recvMessage(int, int*, void *, int);
extern void block(void);

#endif
