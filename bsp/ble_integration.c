/**********************************************************************************************************************
File: ble_integration.c                                                                

Description:
This is a ble_integration .c file new source code
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32BLEIntegrationFlags;                 /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SocInt_" and be declared as static.
***********************************************************************************************************************/
static u32 bleInt_u32Timeout;                      /* Timeout counter used across states */
static uint8_t m_evt_buffer[CEIL_DIV(sizeof(ble_evt_t) + (BLE_L2CAP_MTU_DEF), sizeof(uint32_t))];

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private Function Declarations.                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
static ble_evt_t* BLEIntegration_get_buffer(void);

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
bool BLEIntegrationInitialize(void)
{
  return true;
}

/**
*   Handle the Stack Event to determine if BLE message. If 
*   BLE Message, then dispatch to handlers.                       
*/
void BLEIntegrationHandler(void)
{
    // Fetch message.
    ble_evt_t* ble_evt = BLEIntegration_get_buffer();
    
    // Check if message was successfully fetched.
    if (ble_evt)
    {
      // Dispatch to all BLE specific handlers.
      blePeripheralEventHandler(ble_evt);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
static ble_evt_t* BLEIntegration_get_buffer(void)
{
   u16 evt_len = sizeof(m_evt_buffer);
   u32 err_code;

   err_code = sd_ble_evt_get(m_evt_buffer, &evt_len);
   if (err_code == NRF_ERROR_NOT_FOUND)
   {
      return NULL;
   }
   else if (err_code != NRF_SUCCESS)
   {
      // TODO: Add Appropriate Error Handler and repurcussions.
   }

   return (ble_evt_t*) m_evt_buffer;
}



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
