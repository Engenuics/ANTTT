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

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
extern volatile u32 G_u32BPEngenuicsFlags;             /* From bleperipheral_engenuics.h */

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_" and be declared as static.
***********************************************************************************************************************/
static u32 Anttt_u32Timeout;                             /* Timeout counter used across states */
static u32 Anttt_u32CyclePeriod;                         /* Current base time for Anttt modulation */
static u8 Anttt_rx_data[ANTTT_COMMAND_SIZE];
static uint16_t Anttt_home_state;
static uint16_t Anttt_away_state;

u16 winning_combos = 
{
   0b000000111,
   0b000111000,
   0b111000000,
   0b001001001,
   0b010010010,
   0b100100100,
   0b100010001,
   0b001010100
};


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
static void AntttSM_Idle(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
void AntttIncomingMessage(u8* data, u8 len)
{
   // Check length of the Command Size.
   if (len != ANTTT_COMMAND_SIZE)
      return;

   memcpy(&Anttt_rx_data, data, len);
}


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
   memset(Anttt_rx_data, 0xFF, ANTTT_COMMAND_SIZE);
   Anttt_home_state = Anttt_away_state = 0;
   ANTT_SM = &AntttSM_Idle;

   // Set up initial LEDs.
   LedOn(STATUS_YLW);
   LedOff(STATUS_RED);
   LedOff(STATUS_GRN);
} /* end AntttInitialize() */

void AntttHandleIncomingMessage(u8* data, u8 len)
{
   // Check the appropriate length.
   if (len == ANTTT_COMMAND_SIZE)
   {
      memcpy(Anttt_rx_data, data, len);
   }
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
static bool Anttt_is_game_over(void)
{
   // Check all 8 winning combinations.
   for (int i = 0; i < 8; i++)
   {
      if (Anttt_home_state & winning_combos[i])
      {
         Anttt_home_state = winning_combos[i];
         Anttt_home_state |= 0x200;       // Set this flag to indicate home won.

         return true;
      }
      else if (Anttt_away_state & winning_combos[i])
      {
         Anttt_away_state = winning_combos[i];
         Anttt_away_state |= 0x200;       // Set this flag to indicate away winning won.

         return true;
      }
   }

   return false;
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------
Function: AntttSM_Idle
*/
static void AntttSM_Idle(void)
{
   // Check if module is connected to client.
   if (G_u32BPEngenuicsFlags == BPENGENUICS_CONNECTED)
   {
      // Set LEDs and proceed to wait state.
      LedOn(STATUS_GRN);   // Connected to Client.
      LedOn(STATUS_RED);   // Waiting on Client to make first move.
      LedOff(STATUS_YLW);  // Status LED off

      ANTT_SM = &AntttSM_Wait;
   }
} 

static void AntttSM_Wait(void)
{
   // Check if module has established connection with client.
   if (G_u32BPEngenuicsFlags & BPENGENUICS_SERVICEENABLED)
   {
      // Wait for Client to make a move.
      if (Anttt_rx_data[ANTTT_COMMAND_ID_OFFSET] == ANTTT_COMMAND_ID_MOVE)
      {
         u8 temp[ANTTT_COMMAND_SIZE];
         u16  position = Anttt_rx_data[ANTTT_COMMAND_POSITION_OFFSET];
         
         // Check if position is already chosen.
         if (Anttt_away_state & (1 << (position-1)))
         {
            return;
         }

         // New Position.
         LedOn(position + 9);  // 9 is the offset for away LEDs.
         Anttt_away_state = 1 << (position - 1);
         
         // Send response.
         temp[ANTTT_COMMAND_ID_OFFSET] = ANTTT_COMMAND_ID_MOVE_RESP;
         BPEngenuicsSendData(temp, ANTTT_COMMAND_SIZE);

         if (anttt_is_game_over())
         {
            ANTT_SM = &AntttSM_Gameover;
            return;
         }
         
         memset(Anttt_rx_data, 0xFF, ANTTT_COMMAND_SIZE);   // Reset message.

         // Update State.
         ANTT_SM = &AntttSM_Acive;
         LedOff(STATUS_RED);
      }
   }
   else
   {
      // Disconnected from client.
      AntttInitialize();
   }
}

static void AntttSM_Active(void)
{
   // Check if module has established connection with client.
   if (G_u32BPEngenuicsFlags & BPENGENUICS_SERVICEENABLED)
   {
      // Make a move.
      // TODO:

      
      // Check if response received.
      if (Anttt_rx_data[ANTTT_COMMAND_ID_OFFSET] != ANTTT_COMMAND_ID_MOVE_RESP)
      {
         memset(Anttt_rx_data, 0xFF, ANTTT_COMMAND_SIZE);   // Reset message.

         // Check if game is over.
         if (anttt_is_game_over())
         {
            ANTT_SM = &AntttSM_Gameover;
            return;
         }
        
         // Update State.
         ANTT_SM = &AntttSM_Wait;
         LedOn(STATUS_RED);
      }
   }
   else
   {
      // Disconnected from client.
      AntttInitialize();
   }
}

static void AnttttSM_Gameover(void)
{   
   // Play Winning Sequence. 
   if (G_u32SystemTime1ms % 500)
   {
      // Toggle LED sequences.
      LedToggle(STATUS_GRN);
      LedToggle(STATUS_RED);
      LedToggle(STATUS_YLW);

      // Blink winning sequence.
      if (Anttt_home_state & 0x200)
      {
         // Home won.
         u8 three[3];
         u8 index = 0;

         // Convert LED bitmask to LED value.
         for (int i = 0; i < 9; i++)
         {
            if (Anttt_home_state & 0x01)
            {
               three[index++] = i + 1;
            }

            Anttt_home_state = Anttt_home_state >> 1;
         }

         LedToggle(three[0]);
         LedToggle(three[1]);
         LedToggle(three[2]);
      }
      else
      {
         // Away won.
         u8 three[3];
         u8 index = 0;

         // Convert LED bitmask to LED value.
         for (int i = 0; i < 9; i++)
         {
            if (Anttt_away_state & 0x01)
            {
               three[index++] = i + 1;
            }

            Anttt_away_state = Anttt_away_state >> 1;
         }

         LedToggle(three[0]);
         LedToggle(three[1]);
         LedToggle(three[2]);
      }
   }

   // TODO: Reset SM once a button is pressed.
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
