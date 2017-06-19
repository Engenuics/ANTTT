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


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private Function Declarations.                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
static void softdevice_assert_callback(uint32_t ulPC, uint16_t usLineNum, const uint8_t *pucFileName);


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
bool SocIntegrationInitialize(void)
{
  uint32_t result = 0;
  
  result |= sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_SYNTH_250_PPM, softdevice_assert_callback);
  result |= sd_nvic_SetPriority(SD_EVT_IRQn, NRF_APP_PRIORITY_LOW);
  result |= sd_nvic_EnableIRQ(SD_EVT_IRQn);
  
  return result == 0;
}

void SocIntegrationHandler(void)
{
  // Check if pending event.
  if (G_u32SystemFlags & _SYSTEM_PROTOCOL_EVENT)
  {
    // Clear pending event and process protocol events.
    G_u32SystemFlags &= ~_SYSTEM_PROTOCOL_EVENT;
    ANTIntegrationHandler();
    BLEIntegrationHandler();
  }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @brief Handler for softdevice asserts
 */
void softdevice_assert_callback(uint32_t ulPC, uint16_t usLineNum, const uint8_t *pucFileName)
{
   while (1);
   NVIC_SystemReset();
}



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
