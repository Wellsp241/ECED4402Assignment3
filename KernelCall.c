/*
 * @file    KernelCall.c
 * @brief   Module contains process kernel calls
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    21-Nov-2019 (edited)
 */
#include <stdio.h>
#define GLOBAL_KERNELCALL
#include "KernelCall.h"
#include "Process.h"
#include "Messages.h"

/*
 * @brief   Used to set R7, to point to Kernel Argument passed to SVC
 * @param   [in] volatile unsigned long data: passes address of argument
 *              structure
 */
void assignR7(volatile unsigned long data)
{
    __asm("     mov     r7,r0");
}

/*
 * @brief   called by messaging kernel calls to push their
 *          special structures on the stack and trap the
 *          kernel
 * @param   [in] int code: enumeration of repective kernel call
 *          [in/out] void* messageStruct: structure that contains
 *          arguments needed for interprocess communication
 * @return  int: returns status of operation for send
 *               size of message for receive
 */
int procKernelCall(int code, void* messageStruct)
{
    volatile KernelArgs argList;
    argList.code = code;
    argList.arg1 = (unsigned long)messageStruct;
    assignR7((unsigned long) &argList);

    SVC();

    return argList.rtnvalue;

}

/*
 * @brief   called to bind a process to a mailbox
 * @param   [in] int desiredMB: # of MB the process wants
 *          to bind to; if equal to 16 its a bind any call
 * @return  int: -4-> bind failure; 1 -> success
 */
int bind(unsigned int desiredMB)
{
    int result;

    /* First check whether a valid queue was requested */
    if((desiredMB >= MAILBOX_AMOUNT) && (desiredMB != ANY))
    {
        result = FAILURE;
    }
    else
    {
        volatile KernelArgs bindArg; /* Volatile to actually reserve space on stack */
        bindArg.code = BIND;
        bindArg.arg1 = desiredMB;

        assignR7((unsigned long) &bindArg);

        SVC();

        result = bindArg.rtnvalue;
    }

    return result;
}

/*
 * @brief   called to unbind a process from a mailbox
 * @param   [in] int releaseMB: # of MB to be released
 * @return int: -5-> unbind failure; 1 -> success
 */
int unbind(unsigned int releaseMB)
{
    /* First check whether a valid queue was requested */
    if(releaseMB >= MAILBOX_AMOUNT)
    {
        return FAILURE;
    }

    volatile KernelArgs unbindArg; /* Volatile to actually reserve space on stack */
    unbindArg.code = UNBIND;
    unbindArg.arg1 = releaseMB;

    assignR7((unsigned long) &unbindArg);

    SVC();

    return unbindArg.rtnvalue;
}

/*
 * @brief   Called from a process to retrieve it's PID
 *          from kernel
 * @return int: returns running process PID
 */
int getid(void)
{
    volatile KernelArgs getIdArg; /* Volatile to actually reserve space on stack */
    getIdArg . code = GETID;

    /* Assign address of getidarg to R7 */
    assignR7((unsigned long) &getIdArg);

    SVC();

    return getIdArg.rtnvalue;
}

/*
 * @brief   The address of this function is loaded into the processes
 *          LR at initialization. This is called when a process is completed
 *          for its' PCB and stack to be free'd
 */
void terminate(void)
{
    volatile KernelArgs terminateArg; /* Volatile to actually reserve space on stack */
    terminateArg.code = TERMINATE;

    /* Assign address of terminateArg to R7 */
    assignR7((unsigned long) &terminateArg);

    SVC();
}

/*
 * @brief   Process calls nice function to change its priority level
 * @param   [in] int newPriority: the priority level the process is
 *          changing to
 * @return  int: New priority of calling process. If this value is the same
 *          as the process' priority from before this call, then the priority
 *          change has failed.
 */
int nice(unsigned int newPriority)
{
    /* First check whether requested priority is valid */
    if(newPriority >= PRIORITY_LEVELS)
    {
        return FAILURE;
    }

    volatile KernelArgs niceArgs; /* Volatile to actually reserve space on stack */

    niceArgs.code = NICE;
    niceArgs.arg1 = newPriority;

    /* Assign address of niceArgs to R7 */
    assignR7((unsigned long) &niceArgs);

    SVC();
    return niceArgs.rtnvalue;
}


/*
 * @brief   Invokes the kernel to send a message to a desired Mailbox
 * @param   [in] int destinationMB: MB # of the destination process
 *          [in] int fromMB: MB # of the sending process
 *          [in] void* contents: data to be sent
 *          [in] int size: amount of data measured in bytes
 * @return  int:  -2->send failure; 1->success
 */
int sendMessage(int destinationMB, int fromMB, void * contents, int size)
{
    int result;

    /* First check if valid mailbox was requested */
    if((destinationMB >= MAILBOX_AMOUNT) || (fromMB >= MAILBOX_AMOUNT))
    {
        /* Invalid mailbox was requested */
        result = FAILURE;
    }
    else
    {
        /* Valid mailbox was requested so perform a send */
        SendMessage sendArgs;
        sendArgs.destinationMB = destinationMB;
        sendArgs.fromMB = fromMB;
        sendArgs.contents = contents;
        sendArgs.size = size;
        result = procKernelCall(SENDMSG, &sendArgs);
    }

    return result;
}


/*
 * @brief   Invokes the kernel to receive a message from a MB that the running
 *          process is binded to
 * @param   [in] int bindedMB: MB # of the receiving process
 *          [out] int* returnMB: MB # of the process that sent the message
 *          [in/out] void* contents: address where data is stored
 *          [in/out] int* maxSize: [in]maximum amount of bytes the process will take
 *                                 [out] amount of bytes that were copied
 * @return  int: -3->receive failure; 1->success
 *
 */
int recvMessage(unsigned int bindedMB, int * returnMB, void * contents, int * maxSize)
{
    int result;

    /* First check if valid mailbox was requested */
    if((bindedMB >= MAILBOX_AMOUNT) && (bindedMB != ANY))
    {
        /* Invalid mailbox was requested */
        result = FAILURE;
    }
    else
    {
        /* Valid mailbox was requested to perform a receive */
        ReceiveMessage recvArgs;
        recvArgs.bindedMB = bindedMB;
        recvArgs.returnMB = returnMB;
        recvArgs.contents =contents;
        recvArgs.maxSize = maxSize;
        result = procKernelCall(RECEIVEMSG, &recvArgs);
    }

    return result;
}

void block(void)
{
    volatile KernelArgs blockArg; /* Volatile to actually reserve space on stack */
    blockArg . code = BLOCK;

    /* Assign address of getidarg to R7 */
    assignR7((unsigned long) &blockArg);

    SVC();
}





