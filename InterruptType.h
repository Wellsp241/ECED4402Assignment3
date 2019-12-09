/*
 * @file    InterruptType.h
 * @brief   Defines the interruptType structure
 */
#pragma once
/*
 * @brief   Interrupt Type structure
 * @details int type:   0 is a UART interrupt
 *                      1 is a SYSTICK interrupt
 *          char data:  where UART interrupts store
 *                      data being passed
 */
typedef struct interruptType_
{
    int type;
    char data;

}interruptType;

