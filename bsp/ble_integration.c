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
static u32 bleintegration_u32Timeout;                                            /* Timeout counter used across states */
static uint8_t m_evt_buffer[100];      // TODO: Determine exact value             /* Single BLE buffer used for incoming BLE messages */

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

/*----------------------------------------------------------------------------------------------------------------------
Function: BLEIntegrationHandler

Description:
Checks if a BLE buffer is present for processing.

Requires:
  - None

Promises:
  - Dispatches event to the all BLE Handlers
*/
void BLEIntegrationHandler(void)
{
    // Fetch message.
    ble_evt_t* ble_evt = BLEIntegration_get_buffer();
      
    // Check if message was successfully fetched.
    while (ble_evt)
    {
      // Dispatch to all BLE specific handlers.
      bleperipheralEventHandler(ble_evt);
      
      // Check if another message is pending.
      ble_evt = BLEIntegration_get_buffer();
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------
Function: BLEIntegration_get_buffer

Description:
Calls the BLE SoftDevice function to check if an appropriate message is available

Requires:
  - None

Promises:
  - Returns NULL if no ble_message is availble.
  - Copies the ble_message to m_evt_buffer if message is available and returns pointer to 
    the buffer.
*/
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
     return NULL;
   }

   
   return (ble_evt_t*) m_evt_buffer;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
