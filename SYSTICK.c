/*
 * @file    SYSTICK.c
 * @brief   Contains SYSTICK timer handling functionality. Facilitates
 *          setup of the timer as well as its ISR requesting a context
 *          switch.
 * @author  Emad Khan (original, 2017)
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    29-Oct-2019 (created)
 * @date    1-Nov-2019 (edited)
 */
#define GLOBAL_SYSTICK
#include "SYSTICK.h"

/* Macro used to request a pendSV call */
#define CALLPENDSV (*((volatile unsigned long *)0xE000ED04) |= 0x10000000UL)

/*
 * @brief   Set the clock source to internal and enable the counter to interrupt
 */
void SysTickStart(void)
{
ST_CTRL_R |= ST_CTRL_CLK_SRC | ST_CTRL_ENABLE;
}

/*
 * @brief   Clear the enable bit to stop the counter
 */
void SysTickStop(void)
{
ST_CTRL_R &= ~(ST_CTRL_ENABLE);
}

/*
 * @brief   Set interrupt period
 */
void SysTickPeriod(unsigned long Period)
{
/*
 For an interrupt, must be between 2 and 16777216 (0x100.0000 or 2^24)
*/
ST_RELOAD_R = Period - 1;  /* 1 to 0xff.ffff */
}

/*
 * @brief   Enable Systick interrupts
 */
void SysTickIntEnable(void)
{
// Set the interrupt bit in STCTRL
ST_CTRL_R |= ST_CTRL_INTEN;
}

/*
 * @brief   disable Systick interrupts
 */
void SysTickIntDisable(void)
{
// Clear the interrupt bit in STCTRL
ST_CTRL_R &= ~(ST_CTRL_INTEN);
}

/*
 * @brief ISR of SYSTICK requesting a context switch
 *
 */
void SYSTICKHandler(void)
{
    /* Request a pendSV call */
    CALLPENDSV;
}
