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
static holdingBuffer    holdingBuffer_0 = {{EMPTY},EMPTY};
static holdingBuffer    holdingBuffer_1 = {{EMPTY},EMPTY};

/*
 * @brief   Adds a character to the holding register
 * @param   char c: character to be added to holding register
 * @return  @return  int return used as a boolean value,
 *          if returns 1 the character was successfully added
 *          if returns 0 the holding buffer is full
 */
int addToBuffer_0(char c)
{
    if(holdingBuffer_0.writePtr<MAX_BUFFER)
    {
        holdingBuffer_0.buffer[holdingBuffer_0.writePtr++] = c;
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
int removeFromBuffer_0(void)
{
    if(holdingBuffer_0.writePtr>EMPTY)
    {
            holdingBuffer_0.writePtr--;
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
char* emptyBuffer_0(void)
{
   addToBuffer_0(NUL);
   holdingBuffer_0.writePtr=EMPTY;
   return holdingBuffer_0.buffer;
}

/*
 * @brief   Adds a character to the holding register
 * @param   char c: character to be added to holding register
 * @return  @return  int return used as a boolean value,
 *          if returns 1 the character was successfully added
 *          if returns 0 the holding buffer is full
 */
int addToBuffer_1(char c)
{
    if(holdingBuffer_1.writePtr<MAX_BUFFER)
    {
        holdingBuffer_1.buffer[holdingBuffer_1.writePtr++] = c;
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
int removeFromBuffer_1(void)
{
    if(holdingBuffer_1.writePtr>EMPTY)
    {
            holdingBuffer_1.writePtr--;
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
char* emptyBuffer_1(void)
{
   addToBuffer_1(NUL);
   holdingBuffer_1.writePtr=EMPTY;
   return holdingBuffer_1.buffer;
}
