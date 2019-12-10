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
#include "KernelCall.h"
#include "Utilities.h"
#include "SVC.h"
#include "InterruptType.h"
#include "Queue.h"


static interruptType systickEvent = {SYSTICK,NUL};
static int timerBlocked = FALSE;
static int timerSet = FALSE;


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

int getTimerState(void)
{
    return timerSet;
}
int getTimerProcessState(void)
{
    return timerBlocked;
}
/*
 * @brief   set timer to delay with time variable that
 *          is in hundredths of a second
 *
 * @param   [in] int time: time in hundredths of a second
 *          to set timer
 *
 * @return int: SUCCESS-> Timer successfully set
 *              TAKEN-> Timer is in use
 */
void timeServer(void)
{
    bind(TIMER_MB);

    interruptType timerTrigger = {SYSTICK,NUL};
    int toMB;
    char cont[MESSAGE_SYS_LIMIT];
    int size = MESSAGE_SYS_LIMIT;
    volatile int time;
    while (1)
    {
        recvMessage(TIMER_MB, &toMB, cont, size);
        myAtoi(&time, cont);
        timerSet = TRUE;
        while(timerSet==TRUE)
        {
        if (dequeue(&timerTrigger))
        {
            if(time>0)
            {
                time--;
            }
            else
            {
                timerSet=FALSE;
            }
        }
        else
        {
             timerBlocked = TRUE;
             block();
             timerBlocked = FALSE;
        }
        }
        sendMessage(toMB, TIMER_MB," DONE ", 6);
    }
}
/*
 * @brief ISR of SYSTICK requesting a context switch
 *        if timer is set; decrement waitTime
 *
 */
void SYSTICKHandler(void)
{
    setPendType(CONTEXT);
    CALLPENDSV;

    if(getTimerState())
    {
        setPendType(TIMER);
        systickEvent.type=SYSTICK;
        enqueue(systickEvent);
    }

}
