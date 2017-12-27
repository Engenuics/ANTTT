/**********************************************************************************************************************
File: anttt.c                                                                

Description:
Implements TIC-TAC-TOE using data input from ANT or BLE.



**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
u32 G_u32AntttFlags;                                     /* Global state flags */
fnCode_type ANTT_SM;

enum ANTTT_STATE
{
   ANTTT_STATE_IDLE,
   ANTTT_STATE_WAIT,
   ANTTT_STATE_ACTIVE,
   ANTTT_STATES
};

enum ANTTT_STATE anttt_state;

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_" and be declared as static.
***********************************************************************************************************************/
static u32 Anttt_u32Timeout;                             /* Timeout counter used across states */
static u32 Anttt_u32CyclePeriod;                         /* Current base time for Anttt modulation */

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

static void AntttSM_Idle(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: AntttInitialize

Description:
Initializes the State Machine and its variables.

Requires:

Promises:
*/
void AntttInitialize(void)
{
   // Initialize the SM to idle. 
  ANTT_SM = &AntttSM_Idle;
  anttt_state = ANTTT_STATE_IDLE;

  // Set up the LEDs as module is now connected.
  LedOn(STATUS_YLW);   // Turn Red LED indicating that it is powered but in disconnected state. 
} /* end AntttInitialize() */

void AntttHandleIncomingMessage(u8* data, u8 len)
{
   if (anttt_state == ANTTT_STATE_WAIT)
   {
      
   }
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: AntttSM_Idle
*/
static void AntttSM_Idle(void)
{
   // In idle state, wait for BLE Connection to progress to next state.
   if (bleperipheralIsConnectedandEnabled())
   {
      ANTT_SM = &AntttSM_Wait;
      LedOn(STATUS_GRN):
      LedOff(STATUS_YLW);
      LedOn(STATUS_RED);   // By default, let the client start first.
      anttt_state = ANTTT_STATE_WAIT;
   }
} /* end AntttInitialize() */

static void AntttSM_Wait(void)
{
   // Waiting for client to make a move. Nothing to do.
   // Handled in AntttHandleIncomingMessages() 
}

static void AntttSM_Active(void)
{
   
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
