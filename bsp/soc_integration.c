/**********************************************************************************************************************
File: soc_integration.c                                                                

Description:
This is a soc_integration .c file new source code
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SocIntegrationFlags;                 /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SocInt_" and be declared as static.
***********************************************************************************************************************/
static u32 SocInt_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Interrupt handler: SD_EVT_IRQHandler

Description:
Processes soft device events.

Requires:
  -

Promises:
  - 
*/
void SD_EVT_IRQHandler(void)
{
   u32 u32Event;

  /* Read out all current SOC events */
  while (sd_ant_event_get(&u32Event) != NRF_ERROR_NOT_FOUND)
  {
    /* Flag if there are any ANT events */
    G_u32SystemFlags |= _SYSTEM_ANT_EVENT; 
  }
}

/**
 * @brief Handler for softdevice asserts
 */
void softdevice_assert_callback(uint32_t ulPC, uint16_t usLineNum, const uint8_t *pucFileName)
{
   UNUSED_PARAMETER(ulPC);
   assert_nrf_callback(usLineNum, pucFileName);
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
