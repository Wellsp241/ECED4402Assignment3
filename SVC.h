/*
 * @file    SVC.h
 * @brief   SVC function declarations
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    28-Nov-2019 (edited)
 */
#pragma once
#include "Process.h"

enum pendType {CONTEXT,INPUT_0,INPUT_1,TIMER};

/* Macro used to set the priority of the pendSV interrupt */
#define SETPENDSVPRIORITY ((*(volatile unsigned long *)0xE000ED20) |= 0x00E00000UL)

#ifndef GLOBAL_SVC
#define GLOBAL_SVC

extern int registerProcess(void (*)(void), unsigned int,int );
extern int addPCB(PCB *,int);
extern PCB * removePCB(void);
extern void initpendSV(void);
extern PCB * getRunningPCB(void);
extern void setPendType(int);


#else

void decrementPriority(void);
int addPCB(PCB *, int);
PCB * removePCB(void);
void initpendSV(void);
void SVCall(void);
void SVCHandler(StackFrame*);

#endif /* GLOBAL_SVC */
