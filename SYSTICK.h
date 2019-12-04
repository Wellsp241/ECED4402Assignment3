/*
 * @file    SYSTICK.h
 * @brief   Contains functionality to identify command
 *          passed from holding buffer and call its respective
 *          function with arguments
 * @author  Emad Khan (original, 2017)
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    13-Nov-2019 (edited)
 */

#pragma once

#define ST_CTRL_R   (*((volatile unsigned long *)0xE000E010))
// Systick Reload Value Register (STRELOAD)
#define ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))

// SysTick defines
#define ST_CTRL_COUNT      0x00010000  // Count Flag for STCTRL
#define ST_CTRL_CLK_SRC    0x00000004  // Clock Source for STCTRL
#define ST_CTRL_INTEN      0x00000002  // Interrupt Enable for STCTRL
#define ST_CTRL_ENABLE     0x00000001  // Enable for STCTRL

// Maximum period
#define MAX_WAIT           0x1000000   /* 2^24 */
#define HUNDREDTH_WAIT     0x27100 //(2^24)/100

#ifndef GLOBAL_SYSTICK
#define GLOBAL_SYSTICK

    extern void SysTickStart(void);
    extern void SysTickStop(void);
    extern void SysTickPeriod(unsigned long);
    extern void SysTickIntEnable(void);
    extern void SysTickIntDisable(void);
    extern void SysTickHandler(void);

#endif //GLOBAL_SYSTICK
