/*
 * @file    Process.h
 * @brief   Process structure definitions
 *          Process function declarations
 *
 * @author  Larry Hughes (Original)
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    13-Nov-2019 (edited)
 */
#pragma once

#define TRUE    1
#define FALSE   0
#define PRIVATE static

#define PRIORITY_LEVELS 5

#define SVC()   __asm(" SVC #0")
#define disable()   __asm(" cpsid i")
#define enable()    __asm(" cpsie i")
#define STACKSIZE   1024
#define MSP_RETURN 0xFFFFFFF9    //LR value: exception return using MSP as SP
#define PSP_RETURN 0xFFFFFFFD    //LR value: exception return using PSP as SP

/* Cortex default stack frame */

typedef struct StackFrame_
{
/* Registers saved by s/w (explicit) */
/* There is no actual need to reserve space for R4-R11, other than
 * for initialization purposes.  Note that r0 is the h/w top-of-stack.
 */
unsigned long r4;
unsigned long r5;
unsigned long r6;
unsigned long r7;
unsigned long r8;
unsigned long r9;
unsigned long r10;
unsigned long r11;
/* Stacked by hardware (implicit)*/
unsigned long r0;
unsigned long r1;
unsigned long r2;
unsigned long r3;
unsigned long r12;
unsigned long lr;
unsigned long pc;
unsigned long psr;
} StackFrame;


/* Process control block */

typedef struct ProcessControlBlock_
{
/* Stack pointer - r13 (PSP) */
unsigned long sp;
unsigned long topOfStack;
/* Process ID number */
unsigned int pid;
/* Links to adjacent PCBs */
struct ProcessControlBlock_ *next;
struct ProcessControlBlock_ *prev;
/* Priority of process */
unsigned char priority;
/* Pointer to message storing space */
int * returnValue;

int xAxisCursorPosition;
// Blocked Message variables
int* from;
int size;
void* contents;

struct ReceiveLog_ * receiveAnyHead;
struct ReceiveLog_ * receiveAnyTail;

} PCB;


#ifndef GLOBAL_KERNELCALL
#define GLOBAL_KERNELCALL

extern void set_LR(volatile unsigned long);
extern unsigned long get_PSP();
extern void set_PSP(volatile unsigned long);
extern unsigned long get_MSP(void);
extern void set_MSP(volatile unsigned long);
extern unsigned long get_SP();
extern void volatile save_registers();
extern void volatile restore_registers();

#endif
