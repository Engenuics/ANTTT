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
static bool bPendingResponse;

u16 winning_combos[] = 
{
  0x0007,        // 0b000000111
  0x0038,        // 0b000111000
  0x01C0,        // 0b111000000
  0x0049,        // 0b001001001
  0x0092,        // 0b010010010
  0x0124,        // 0b100100100
  0x0111,        // 0b100010001
  0x0054         // 0b001010100
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
  Anttt_reset_rx_buffer();
  Anttt_home_state = Anttt_away_state = 0;
  ANTT_SM = &AntttSM_Idle;
  bPendingResponse = false;
  
  // Set up initial LEDs.
  LedOn(STATUS_RED);
  LedOff(STATUS_YLW);
  LedOff(STATUS_GRN);
  
  for (int led = 0; led < TOTAL_BUTTONS * 2; led++)
    LedOff((LedNumberType) led);
  
  nrf_gpio_pin_clear(16);
  
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
    if ((Anttt_home_state & winning_combos[i]) == winning_combos[i])
    {
      Anttt_home_state = winning_combos[i];
      Anttt_home_state |= 0x200;       // Set this flag to indicate home won.
      
      return true;
    }
    else if ((Anttt_away_state & winning_combos[i]) == winning_combos[i])
    {
      Anttt_away_state = winning_combos[i];
      Anttt_away_state |= 0x200;       // Set this flag to indicate away winning won.
      
      return true;
    }
  }
  
  // Check if draw.
  if ((Anttt_home_state | Anttt_away_state) == 0x1FF)
    return true;
  
  return false;
}

static void Anttt_reset_rx_buffer(void)
{
  u8 status;
  SystemEnterCriticalSection(&status);
  memset(Anttt_rx_data, 0xFF, ANTTT_COMMAND_SIZE);   // Reset message.
  SystemExitCriticalSection(status);
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
    
    // Reset any pending button states incase the user was pressing while waiting.
    ButtonInitialize();
    
    ANTT_SM = &AntttSM_Wait;
  }
  // TEST CODE TO TEST BUTTONS
  //     for (int button = 0; button < TOTAL_BUTTONS; button++)
  //     {
  //       if (WasButtonPressed(button))
  //       {
  //         LedToggle(0xFF);
  //         ButtonAcknowledge(button);
  //         return;
  //       }
  //     }
} 

static void AntttSM_Wait(void)
{
  // Check if module has established connection with client.
  if (G_u32BPEngenuicsFlags & BPENGENUICS_CONNECTED)
  {
    // Wait for Client to make a move.
    if (Anttt_rx_data[ANTTT_COMMAND_ID_OFFSET] == ANTTT_COMMAND_ID_MOVE)
    {
      u8 temp[ANTTT_COMMAND_SIZE];
      u8  position = Anttt_rx_data[ANTTT_COMMAND_POSITION_OFFSET];
      
      // Check if position is already chosen or incorrect position sent.
      if ((Anttt_home_state & (1 << position)) || (Anttt_away_state & (1 << position))
          || (position >= TOTAL_BUTTONS))
      {
        return;
      }
      
      // New Position.
      LedOn((LedNumberType) (position + 9));  // 9 is the offset for away LEDs.
      Anttt_away_state |= 1 << (position);
      
      // Send response.
      temp[ANTTT_COMMAND_ID_OFFSET] = ANTTT_COMMAND_ID_MOVE_RESP;
      BPEngenuicsSendData(temp, ANTTT_COMMAND_SIZE);
      
      // Check if game is over.
      if (Anttt_is_game_over())
      {
        // Set up initial LEDs.
        LedOff(STATUS_RED);
        LedOff(STATUS_YLW);
        LedOff(STATUS_GRN);
  
        for (int led = 0; led < TOTAL_BUTTONS * 2; led++)
          LedOff((LedNumberType) led);
        
        ANTT_SM = &AntttSM_Gameover;
        return;
      }
      
      Anttt_reset_rx_buffer();
      
      // Update State.
      ANTT_SM = &AntttSM_Active;
      LedOn(STATUS_YLW);
    }
  }
  else
  {
    // Disconnected from client.
    AntttInitialize();
  }
  
  // User may be pressing buttons in this state, Ack the button presses to 
  // disable queuing of the presses as valid game presses.
  for (int button = 0; button < TOTAL_BUTTONS; button++)
  {
    if (WasButtonPressed(button) & !bPendingResponse)
    {
      ButtonAcknowledge(button);    // Ack Button Press.
    }
  }
}

static void AntttSM_Active(void)
{
  // Check if module has established connection with client.
  if (G_u32BPEngenuicsFlags & BPENGENUICS_SERVICEENABLED)
  {
    // Make a move.
    // Check if a button was pressed, then update UI and send message.  
    for (int button = 0; button < TOTAL_BUTTONS; button++)
    {
      if (WasButtonPressed(button) & !bPendingResponse)
      {
        u8 temp[ANTTT_COMMAND_SIZE];
        
        // Check if a valid press. Check if position already chosen via client
        // or itself or incorrect button index reported
        if ((Anttt_home_state & (1 << button)) || (Anttt_away_state & (1 << button))
            || (button >= TOTAL_BUTTONS))
        {
          continue;
        }
        
        // Button index directly corresponds to Home LED Index.
        // First Update UI.
        LedOn((LedNumberType) button);
        Anttt_home_state |= 1 << (button);
        
        // Send message to client.
        temp[ANTTT_COMMAND_ID_OFFSET] = ANTTT_COMMAND_ID_MOVE;
        temp[ANTTT_COMMAND_POSITION_OFFSET] = button;
        temp[ANTTT_COMMAND_SOURCE_OFFSET] = 0;
        BPEngenuicsSendData(temp, ANTTT_COMMAND_SIZE);
        
        ButtonAcknowledge(button);    // Ack Button Press.
        bPendingResponse = true;
        return;
      }
      else if (bPendingResponse)
      {
        // User may be pressing buttons in this state, Ack the button presses to 
        // disable queuing of the presses as valid game presses.
        ButtonAcknowledge(button);    // Ack Button Press.
      }
    }
    
    // Check if response received.
    if (Anttt_rx_data[ANTTT_COMMAND_ID_OFFSET] == ANTTT_COMMAND_ID_MOVE_RESP)
    {
      Anttt_reset_rx_buffer();
      bPendingResponse = false;
      
      // Check if game is over.
      if (Anttt_is_game_over())
      {
        // Set up initial LEDs.
        LedOff(STATUS_RED);
        LedOff(STATUS_YLW);
        LedOff(STATUS_GRN);
  
        for (int led = 0; led < TOTAL_BUTTONS * 2; led++)
          LedOff((LedNumberType) led);
        
        ANTT_SM = &AntttSM_Gameover;
        return;
      }
      
      // Update State.
      ANTT_SM = &AntttSM_Wait;
      LedOff(STATUS_YLW);
    }
  }
  else
  {
    // Disconnected from client.
    AntttInitialize();
  }
}

static void AntttSM_Gameover(void)
{   
  // Play Winning Sequence. 
  if ((G_u32SystemTime1ms % 500) == 0)
  {
    nrf_gpio_pin_toggle(16);
    
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
      u16 temp = Anttt_home_state;  
      
      // Convert LED bitmask to LED value.
      for (int i = 0; i < 9; i++)
      {
        if (temp & 0x01)
        {
          three[index++] = i;
        }
        
        temp = temp >> 1;
      }
      
      LedToggle((LedNumberType)three[0]);
      LedToggle((LedNumberType)three[1]);
      LedToggle((LedNumberType)three[2]);
    }
    else if (Anttt_away_state & 0x200)
    {
      // Away won.
      u8 three[3];
      u8 index = 0;
      u16 temp = Anttt_away_state;  
      
      // Convert LED bitmask to LED value.
      for (int i = 0; i < 9; i++)
      {
        if (temp & 0x01)
        {
          three[index++] = i;
        }
        
        temp = temp >> 1;
      }
      
      LedToggle((LedNumberType)(three[0] + 9));
      LedToggle((LedNumberType)(three[1] + 9));
      LedToggle((LedNumberType)(three[2] + 9));
    }
    else
    {
      // Play Draw Sequence.
      LedToggle((LedNumberType)(0));
      LedToggle((LedNumberType)(1));
      LedToggle((LedNumberType)(2));
      LedToggle((LedNumberType)(3));
      LedToggle((LedNumberType)(5));
      LedToggle((LedNumberType)(6));
      LedToggle((LedNumberType)(7));
      LedToggle((LedNumberType)(8));
      LedToggle((LedNumberType)(0 + 9));
      LedToggle((LedNumberType)(1 + 9));
      LedToggle((LedNumberType)(2 + 9));
      LedToggle((LedNumberType)(3 + 9));
      LedToggle((LedNumberType)(5 + 9));
      LedToggle((LedNumberType)(6 + 9));
      LedToggle((LedNumberType)(7 + 9));
      LedToggle((LedNumberType)(8 + 9));
    }
  }
  
  // Check if any button was pressed and return to initialized state.
  for (int button = 0; button < TOTAL_BUTTONS; button++)
  {
    if (WasButtonPressed(button))
    {
      AntttInitialize();
      ButtonAcknowledge(button);
      return;
    }
  }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
