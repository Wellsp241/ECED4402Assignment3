/*
 * @file    HoldingBuffer.c
 * @brief   Defines the holding buffer used for
 *          holding characters before the ENTER
 *          character is received; contains all
 *          functionality to use the holding buffer
 * @author  Liam JA MacDonald
 * @date    24-Sep-2019 (Created)
 * @date    9-Oct-2019 (Modified)
 */

#include "Utilities.h"
#define GLOBAL_HOLDINGBUFFER
#include "HoldingBuffer.h"
/* Define an empty holding buffer*/
static holdingBuffer holdingBuf={{EMPTY},EMPTY};

/*
 * @brief   Adds a character to the holding register
 * @param   char c: character to be added to holding register
 * @return  @return  int return used as a boolean value,
 *          if returns 1 the character was successfully added
 *          if returns 0 the holding buffer is full
 */
int addToBuffer(char c)
{
    if(holdingBuf.writePtr<MAX_BUFFER)
    {
        holdingBuf.buffer[holdingBuf.writePtr++] = c;
        return SUCCESS;
    }
    return FULL;
}

/*
 * @brief   Removes a character from the holding register
 * @return  int return used as a boolean value,
 *          if returns 1 the character was successfully removed
 *          if returns 0 the holding buffer is empty
 */
int removeFromBuffer(void)
{
    if(holdingBuf.writePtr>EMPTY)
    {
            holdingBuf.writePtr--;
            return SUCCESS;
    }
    return EMPTY;
}

/*
 * @brief   Adds a NUL character to the end of
 *          the holding register, returns a pointer
 *          to the start
 * @return  char*: pointer to the first character in the
 *          holding register
 */
char* emptyBuffer(void)
{
   addToBuffer(NUL);
   holdingBuf.writePtr=EMPTY;
   return holdingBuf.buffer;
}
