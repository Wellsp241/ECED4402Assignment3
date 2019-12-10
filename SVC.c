/*
 * @file    SVC.c
 * @brief   Contains Service Call (SVC) trap handling functionality.
 *          Declares waitingToRun queues and the running PCB pointer;
 *          contains initialization and manipulation for both.
 *
 * @author  Larry Hughes (original)
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    28-Nov-2019 (edited)
 */
#define GLOBAL_SVC
#include "SVC.h"
#include "Process.h"
#include "KernelCall.h"
#include "Messages.h"
#include "Utilities.h"
#include "SYSTICK.h"
#include "UART.h"



#define HIGH_PRIORITY 4
#define LOW_PRIORITY 0
#define PRIORITY_LEVELS 5
#define RUNNING waitingToRun[currentPriority]
#define STACK_SIZE 1024*sizeof(unsigned long)
#define INIT_SP (1024-16)*sizeof(unsigned long)
#define THUMB_MODE 0x01000000
static int currentPriority = 0;

#define MAX_STACK_SIZE (1024U)
#define STARTING_PSR (0x01000000U)

/* Macro used to set the priority of the pendSV interrupt */

extern void terminate(void);

static PCB * waitingToRun[PRIORITY_LEVELS];
static volatile int pendType = CONTEXT;
/*
 * @brief   returns PCB of running process
 * @return  PCB *: address of running processes
 *          PCB
 * */
PCB * getRunningPCB(void)
{
    return RUNNING;
}

/*
 * @brief   Allocates a new process stack frame and PCB
 *          for the process being registered.
 *          sets PCB sp and pid.
 *          calls addPCB to add PCB to waitingToRun with
 *          respective priority
 * @param   [in] void (*code)(void): pointer to the start of the process code
 *          [in] unsigned int pid: Process ID of process being registered
 *          [in] unsigned char priority: Process' initial priority
 * @return  int: if sucessful, will return 0. Otherwise, return 1, in this case
 *               the desired process will not be registered and the program will
 *               continue to run.
 *
 */
int registerProcess(void (*code)(void), unsigned int pid, int priority)
{
   int result = 0;

   /* First must check to ensure the requested priority is valid */
   if((priority >= LOW_PRIORITY) && (priority <= HIGH_PRIORITY))
   {

       /* Requested priority is valid so continue with process registration */
       PCB * newProcess = (PCB*)malloc(sizeof(PCB));
       newProcess->topOfStack = (unsigned long)malloc(STACK_SIZE);
       StackFrame *processSP = (StackFrame*) (newProcess->topOfStack+(INIT_SP));
       processSP -> psr = THUMB_MODE;
       processSP -> pc = (unsigned long)code;
       processSP -> lr = (unsigned long)terminate;
       newProcess -> sp = (unsigned long) processSP;
       newProcess -> pid = pid;

       newProcess->contents=NULL;
       newProcess->size=NULL;
       newProcess->from=NULL;
       newProcess->xAxisCursorPosition=1;
       newProcess->receiveAnyHead=newProcess->receiveAnyTail=NULL;
       addPCB(newProcess, priority);
   }
   else
   {
       /* Requested an invalid priority so must reject process */
       result = 1;
   }
   return result;
}

/*
 * @brief   Adds a PCB to the end of a waitingToRun queue.
 *          If its the first process in the queue its next
 *          and prev pointers are set to itself.
 *          Otherwise, its added to the end, and pointers are
 *          reassigned accordingly
 * @param   [in] PCB *newPCB: PCB being added to the queue
 * @param   [in] unsigned int newPriority: Priority of queue to which
 *          newPCB will be added
 * */
int addPCB(PCB *newPCB, int newPriority)
{
    /* Must check whether desired queue is empty */
    if(waitingToRun[newPriority] != NULL)
    {
        /* Must add process to tail of priority queue */
        newPCB->next = waitingToRun[newPriority];
        waitingToRun[newPriority] -> prev -> next = newPCB;
        newPCB->prev = waitingToRun[newPriority] -> prev;
        waitingToRun[newPriority] -> prev = newPCB;
    }
    else
    {   /* Desired queue is empty so the process becomes its only entry */
        waitingToRun[newPriority] = newPCB;
        waitingToRun[newPriority]->next = newPCB;
        waitingToRun[newPriority]->prev = newPCB;
    }

    /* Set new priority of process and adjust current operating priority */
    newPCB->priority = newPriority;
    currentPriority = (currentPriority < newPriority)? newPriority: currentPriority;
    return currentPriority;
}

/*
 * @brief   Function used to remove a process from a waiting to run queue
 * @return  PCB *: Pointer to PCB of process removed from queue
 */
PCB * removePCB()
{
    PCB * toRemove = RUNNING;

    /* Check whether process is the queue's only entry */
    if (RUNNING == RUNNING -> next )
    {
        /* This waiting to run queue is now empty so
         * must move to the next highest priority.
         */
        RUNNING = NULL;
        decrementPriority();
    }
    else
    {
        /* There are other entries in this queue so make adjacent PCBs point
         * to each other and advance running process pointer.
         */
        RUNNING -> next -> prev = RUNNING -> prev;
        RUNNING -> prev -> next = RUNNING ->next;
        RUNNING = RUNNING -> next;
    }

    return toRemove;
}

/*
 * @brief   Decrements operating priority until a non-empty queue is found
 */
void decrementPriority(void)
{
    /* Decrement operating priority if current waiting to run queue is empty */
    while((RUNNING == NULL) && (currentPriority >= 0))
    {
        currentPriority--;
    }

    return;
}

/*
 * @brief   Configures pendSV interrupt by setting it to the lowest
 *          possible priority allowing other kernel calls to trigger
 *          the pendSV routine upon finishing their business.
 */
void initpendSV(void)
{
    /* Set pendSV to lowest possible priority */
    SETPENDSVPRIORITY;

    return;
}

void setPendType(int newPendState)
{
    pendType = newPendState;
}

/*
 * @brief   pendSV ISR that carries out context switches
 */
void pendSV(void)
{
    PCB* callerPCB;

    switch(pendType)
    {
    case INPUT_0:
    if (get_UART0_InputState())
    {
        save_registers();
        callerPCB = RUNNING;
        addPCB(getOwnerPCB(UART0_IP_MB),3);
        if(RUNNING != callerPCB)
        {
            callerPCB -> sp = get_PSP();
            set_PSP(RUNNING -> sp);
        }
        restore_registers();
    }
    break;
    case TIMER:
    if(getTimerProcessState()&&getTimerState())
    {
        save_registers();
        callerPCB = RUNNING;
        addPCB(getOwnerPCB(TIMER_MB),4);
        if(RUNNING != callerPCB)
        {
            callerPCB -> sp = get_PSP();
            set_PSP(RUNNING -> sp);
        }
        restore_registers();
    }
    break;
   case CONTEXT:

        disable();
        save_registers();
        RUNNING -> sp = get_PSP();
        RUNNING = RUNNING -> next;
        set_PSP(RUNNING -> sp);
        restore_registers();
        enable();
    break;
    }
}

/*
 * @brief   Entry point of SVC routine
 */
void SVCall(void)
{
/* Supervisor call (trap) entry point
 * Using MSP - trapping process either MSP or PSP (specified in LR)
 * Source is specified in LR: F1 (MSP) or FD (PSP)
 * Save r4-r11 on trapping process stack (MSP or PSP)
 * Restore r4-r11 from trapping process stack to CPU
 * SVCHandler is called with r0 equal to MSP or PSP to access any arguments
 */

/* Save LR for return via MSP or PSP */
__asm("     PUSH    {LR}");

/* Trapping source: MSP or PSP? */
__asm("     TST     LR,#4");    /* Bit #3 (0100b) indicates MSP (0) or PSP (1) */
__asm("     BNE     RtnViaPSP");

/* Trapping source is MSP - save r4-r11 on stack (default, so just push) */
__asm("     PUSH    {r4-r11}");
__asm("     MRS r0,msp");
__asm("     BL  SVCHandler");   /* r0 is MSP */
__asm("     POP {r4-r11}");
__asm("     POP     {PC}");

/* Trapping source is PSP - save r4-r11 on psp stack (MSP is active stack) */
__asm("RtnViaPSP:");
__asm("     mrs     r0,psp");
__asm("     stmdb   r0!,{r4-r11}"); /* Store multiple, decrement before */
__asm("     msr psp,r0");
__asm("     BL  SVCHandler");   /* r0 Is PSP */

/* Restore r4..r11 from trapping process stack  */
__asm("     mrs     r0,psp");
__asm("     ldmia   r0!,{r4-r11}"); /* Load multiple, increment after */
__asm("     msr psp,r0");
__asm("     POP     {PC}");

}

/*
 * @brief   Supervisor call handler
 *          Handle startup of initial process
 *          Handle all other SVCs such as getid, terminate, etc.
 */
void SVCHandler(StackFrame *argptr)
{
/*
 * Assumes first call is from startup code
 * Argptr points to (i.e., has the value of) either:
   - the top of the MSP stack (startup initial process)
   - the top of the PSP stack (all subsequent calls)
 * Argptr points to the full stack consisting of both hardware and software
   register pushes (i.e., R0..xPSR and R4..R10); this is defined in type
   stack_frame
 * Argptr is actually R0 -- setup in SVCall(), above.
 * Since this has been called as a trap (Cortex exception), the code is in
   Handler mode and uses the MSP
 */
static int firstSVCcall = TRUE;
KernelArgs *kcaptr;
PCB * callerPCB;
SendMessage * sendMsg;
ReceiveMessage * recvMsg;

if (firstSVCcall)
{
/*
 * Force a return using PSP
 * This will be the first process to run, so the eight "soft pulled" registers
   (R4..R11) must be ignored otherwise PSP will be pointing to the wrong
   location; the PSP should be pointing to the registers R0..xPSR, which will
   be "hard pulled"by the BX LR instruction.
 * To do this, it is necessary to ensure that the PSP points to (i.e., has) the
   address of R0; at this moment, it points to R4.
 * Since there are eight registers (R4..R11) to skip, the value of the sp
   should be increased by 8 * sizeof(unsigned int).
 * sp is increased because the stack runs from low to high memory.
*/
    SysTickStart();
    enable();     // Enable Master (CPU) Interrupts

    set_PSP(RUNNING-> sp + 8 * sizeof(unsigned int));

    firstSVCcall = FALSE;

    /*
     - Change the current LR to indicate return to Thread mode using the PSP
     - Assembler required to change LR to FFFF.FFFD (Thread/PSP)
     - BX LR loads PC from PSP stack (also, R0 through xPSR) - "hard pull"
    */
    __asm(" movw    LR,#0xFFFD");  /* Lower 16 [and clear top 16] */
    __asm(" movt    LR,#0xFFFF");  /* Upper 16 only */
    __asm(" bx  LR");          /* Force return to PSP */
}
else /* Subsequent SVCs */
{
/*
 * kcaptr points to the arguments associated with this kernel call
 * argptr is the value of the PSP (passed in R0 and pointing to the TOS)
 * the TOS is the complete stack_frame (R4-R10, R0-xPSR)
 * in this example, R7 contains the address of the structure supplied by
    the process - the structure is assumed to hold the arguments to the
    kernel function.
 * to get the address and store it in kcaptr, it is simply a matter of
   assigning the value of R7 (arptr -> r7) to kcaptr
 */

    kcaptr = (KernelArgs *) argptr -> r7;
    switch(kcaptr -> code)
    {
    case GETID:
        kcaptr -> rtnvalue = RUNNING -> pid;
    break;
    case NICE:
        callerPCB = RUNNING;
        kcaptr -> rtnvalue = addPCB(removePCB(),kcaptr->arg1);
        /* Here, RUNNING has been changed to the PCB of the process that is to be
         * run next. If RUNNING does not point to the process that requested a nice()
         * then a context switch is required. Note that no registers are pushed/pulled
         * because the caller's registers have been pushed prior to arriving here and
         * the new RUNNING's registers will be pulled once this service call is concluded.
         */
        if(RUNNING != callerPCB)
        {
            callerPCB -> sp = get_PSP();
            set_PSP(RUNNING -> sp);
        }

        /* Set the returned value to be the ending priority of the calling process */
        kcaptr -> rtnvalue = callerPCB->priority;
    break;
    case SENDMSG:
        callerPCB = RUNNING;
        sendMsg = (SendMessage *)kcaptr ->arg1;
        kcaptr ->rtnvalue =
                kernelSend(sendMsg->destinationMB,sendMsg->fromMB,
                           sendMsg->contents, sendMsg->size);
        if(RUNNING != callerPCB)
        {
            callerPCB -> sp = get_PSP();
            set_PSP(RUNNING -> sp);
        }
    break;
    case RECEIVEMSG:
        recvMsg = (ReceiveMessage *)kcaptr ->arg1;
        kcaptr->rtnvalue = recvMsg->maxSize;
        if(kernelReceive(recvMsg->bindedMB,recvMsg->returnMB,
                      recvMsg->contents, &(kcaptr->rtnvalue)) < 0)
        {
            kcaptr->rtnvalue = FAILURE;
        }
    break;
    case TERMINATE:
        callerPCB = removePCB();
        free(&(callerPCB->sp));
        free(callerPCB);
        /* RUNNING must have changed here so the process stack pointer must be
         * changed accordingly. No registers are pulled here since they will all
         * be pulled once this service call is exited.
         */
        set_PSP(RUNNING -> sp);
    break;
    case BIND:
        kcaptr->rtnvalue= kernelBind( kcaptr->arg1);
    break;
    case UNBIND:
        kcaptr->rtnvalue= kernelUnbind( kcaptr->arg1);
    break;
    case BLOCK:
           callerPCB = removePCB();
           callerPCB -> sp = get_PSP();
           set_PSP(RUNNING -> sp);
    break;
    default:
        kcaptr -> rtnvalue = -1;
    }
}
}
