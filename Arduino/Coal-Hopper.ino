//
// Coal Hopper Stepper Functions
//
// Arduino code to control the movement of a coal hopper for a model railway layout.
// 
// Version History
//
// v1.0 07/11/2024 First draft
// v2.0 10/11/2024 Some minor tweaks
// v3.0 20/11/2024 Added Analog inputs for Speed, Wait and Length. 
// v4.0 10/12/2024 Reconfigure for new PCB
//


//======== The following can be changed to suit the setup ===========================

// Define stepper motor connections
#define directionPin 10
#define stepPin 11
#define enablePin 12
#define homePin 7
#define awayPin 6
#define hopperFullPin 4
#define hopperBusyPin 5
#define homeLEDPin 2
#define awayLEDPin 3
#define homeTypePin 8
#define awayTypePin 9

// Define steps per revolution and size of travel
#define stepsPerRevolution (8 * 200)

// We always use the home switch so this is fixed for 500mm
#define MAX_TURNS_TO_HOME 55

#define DEFAULT_TURNS_TO_AWAY 40
#define MIN_TURNS_TO_AWAY 35
#define MAX_TURNS_TO_AWAY 55

// Specify if Home switch is Normally Open or Closed
#define DEFAULT_HOME_OPEN false 

// Specify if Away switch is Normally Open or Closed
#define DEFAULT_AWAY_OPEN false 

// Specify if Hopper Triger is Active High or Low 
#define TRIGGER_ACTIVE_HIGH false 

// Specify the default delay between steps in uS (will be double this) amd the max & min
#define DEFAULT_DELAY 200
#define MIN_DELAY 100
#define MAX_DELAY 300

// Specify the default wait time in mS at the top
#define DEFAULT_WAIT 10000
#define MIN_WAIT 5000
#define MAX_WAIT 15000

//===================================================================================


// Define the states the hopper can be in
#define HOPPER_INIT 0
#define HOPPER_HOME 1
#define HOPPER_MOVE_AWAY 2
#define HOPPER_AWAY 3
#define HOPPER_MOVE_HOME 4
#define HOPPER_ERROR 255


// Define if the motor is going to Home or Away
#define STEP_AWAY 0
#define STEP_HOME 1


// Set state to init
int hopperState = HOPPER_INIT;

// Set delay to 200uS
int defaultDelay = DEFAULT_DELAY;

// Set the default distance to away until we read the triimers
int turnsToAway = DEFAULT_TURNS_TO_AWAY;

// Set the time to wait at the top before returning.
int awayWaitTime = DEFAULT_WAIT;

// Variables to specify if home and away switch are normally open or closed. Read from pins 11 and 12 at setup.
bool awaySwitchNO = true;
bool homeSwitchNO = true;

// Declare functions
void InitialiseHopper();
void WaitForHopperTrigger();
void MoveHopperToAway();
void WaitForHopperToEmpty();
void MoveHopperToHome();
void HopperError();
bool IsHomeActive();
bool IsAwayActive();
bool IsTriggerActive();

void setup() 
{
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(hopperBusyPin, OUTPUT);
  pinMode(homeLEDPin, OUTPUT);
  pinMode(awayLEDPin, OUTPUT);

  // and the inputs with pullups
  pinMode(homePin, INPUT_PULLUP);
  pinMode(awayPin, INPUT_PULLUP);
  pinMode(homeTypePin, INPUT_PULLUP);
  pinMode(awayTypePin, INPUT_PULLUP);
  pinMode(hopperFullPin, INPUT_PULLUP);

  // Disable the controller for now
  digitalWrite(enablePin, HIGH);

  // Show we are busy 
  digitalWrite(hopperBusyPin, HIGH);

  // Clear the LEDs
  digitalWrite(homeLEDPin, LOW);
  digitalWrite(awayLEDPin, LOW);

  // Configure Serial for debugging
  Serial.begin(115200);

  // Set the initial state to init.
  hopperState = HOPPER_INIT;

}

void loop()
{
  // Loop around and call the appropriate function delepnding on the state.
  if (hopperState == HOPPER_INIT)
  {
    // First thing is to initialise and home the hopper
    InitialiseHopper();
  }
  else if (hopperState == HOPPER_HOME)
  {
    // Hopper is Home, wait to be told to move the hopper
    WaitForHopperTrigger();
  }
  else if (hopperState == HOPPER_MOVE_AWAY)
  {
    // Move the hopper to the away point
    MoveHopperToAway();
  }
  else if (hopperState == HOPPER_AWAY)
  {
    // Hopper is away, wait for it to empty.
    WaitForHopperToEmpty();
  }
  else if (hopperState == HOPPER_MOVE_HOME)
  {
    // Move the hopper back home
    MoveHopperToHome();
  }
  else
  {
    // Something has broken. Stop
    HopperError();
  }
}



bool
IsTriggerActive()
{
  // Check to see if its active high or low
  if (TRIGGER_ACTIVE_HIGH)
  {
    // It is active high
    if (digitalRead(hopperFullPin) == HIGH)
    {
      return(true);
    }
    else
    {
      return(false);
    }
  }
  else
  {
    // It is active low
    if (digitalRead(hopperFullPin) == LOW)
    {
      return(true);
    }
    else
    {
      return(false);
    }    
  }
}


//
// Is Home Switch Active
//
// Checks the state of the home switch.
// Caters for Normally Open and Normally Closed type switches
// Switch is between pin and ground with pullup enabled
//
bool
IsHomeActive()
{
  if (homeSwitchNO)
  {
    // Home switch is normally open
    if (digitalRead(homePin) == LOW)
    {
      // Active low so switch is pressed (NO)
      return(true);
    }
    else
    {
      // Its high, so not switched (NO)
      return(false);
    }
  }
  else
  {
    // Home switch is normally closed
    if (digitalRead(homePin) == LOW)
    {
      // Active low so its not switched (NC)
      return(false);
    }
    else
    {
      // Its high, so switch is pressed (NC)
      return(true);
    }
  }
}

//
// Is Away Switch Active
//
// Checks the state of the away switch.
// Caters for Normally Open and Normally Closed type switches
// Switch is between pin and ground with pullup enabled
//
bool
IsAwayActive()
{
  if (awaySwitchNO)
  {
    // Away switch is normally open
    if (digitalRead(awayPin) == LOW)
    {
      // Active low so switch is pressed (NO)
      return(true);
    }
    else
    {
      // Its high, so not switched (NO)
      return(false);
    }
  }
  else
  {
    // Away switch is normally closed
    if (digitalRead(awayPin) == LOW)
    {
      // Active low so its not switched (NC)
      return(false);
    }
    else
    {
      // Its high, so switch is pressed (NC)
      return(true);
    }
  }
}


//
// Move Stepper
//
// Move the stepper one step to Home or Away and wait.
//
void
MoveStepper(int stepDirection, int stepDelay)
{
  // Enable the board
  digitalWrite(enablePin, LOW);

  // Check direction
  if (stepDirection == STEP_AWAY)
  {
    // Move to Away point
    digitalWrite(directionPin, HIGH);
  }
  else
  {
    // Move to Home point
    digitalWrite(directionPin, LOW);
  }
  
  // Pulse line high
  digitalWrite(stepPin, HIGH);

  // Wait for passed time (uS)
  delayMicroseconds(stepDelay);
  
  // Pulse line low
  digitalWrite(stepPin, LOW);

  // Wait for passed time (uS)
  delayMicroseconds(stepDelay);
 
}


//
// Initialise Hopper
//
// At startup, move hopper back to home position.
// If the hopper is already home, move away and back to confirm switch.
// If the Home switch does not activate after a number of turns, move to ERROR state.
// If ok, set the HOME state.
//
void
InitialiseHopper()
{
  bool ledsAreOn = false;
  
  Serial.print(F("InitialiseHopper: Start\n"));

  // Flash the LEDs at the start so we dont move the motor as soon as the power is applied

  digitalWrite(homeLEDPin, HIGH);
  digitalWrite(awayLEDPin, HIGH);
  delay(2000);
  digitalWrite(homeLEDPin, HIGH);
  digitalWrite(awayLEDPin, LOW);
  delay(2000);
  digitalWrite(homeLEDPin, LOW);
  digitalWrite(awayLEDPin, LOW);
  delay(2000);

  // Check Type pins to see if switches are Normally Open or Closed type
  if (digitalRead(awayTypePin) == HIGH)
  {
    // Link not fitted so its Normally Open 
    awaySwitchNO = true;
    Serial.print(F("InitialiseHopper: Away Switch is Normally Open type\n"));
  }
  else
  {
    // Link is fitted so its Normally Closed
    awaySwitchNO = false;
    Serial.print(F("InitialiseHopper: Away Switch is Normally Closed type\n"));
  }

  if (digitalRead(homeTypePin) == HIGH)
  {
    // Link not fitted so its Normally Open 
    homeSwitchNO = true;
    Serial.print(F("InitialiseHopper: Home Switch is Normally Open type\n"));
  }
  else
  {
    // Link is fitted so its Normally Closed
    homeSwitchNO = false;
    Serial.print(F("InitialiseHopper: Home Switch is Normally Closed type\n"));
  }


  
  // First check if the home switch is already active (low)
  if (IsHomeActive() == true)
  {
    Serial.print(F("InitialiseHopper: Hopper at home position. Moving away.\n"));
    // Yes, move away a number of steps (4 rotations) to see if it goes high.
    for (int i = 0; i < 4; i++)
    {

      // Flash the LEDs at the same time
      if (ledsAreOn == true)
      {
        // Its on so turn it off
        ledsAreOn = false;
        digitalWrite(homeLEDPin, LOW);
        digitalWrite(awayLEDPin, LOW);
      }
      else
      {
        // Its off so turn it on
        ledsAreOn = true;
        digitalWrite(homeLEDPin, HIGH);
        digitalWrite(awayLEDPin, HIGH);
      }
      
      for (int j = 0; j < stepsPerRevolution; j++)
      {
        // Move at half speed.
        MoveStepper(STEP_AWAY, defaultDelay*2);        
      }
      Serial.print(F("InitialiseHopper: Full turn away complete.\n"));
    }

    // Check again
    if (IsHomeActive() == true)
    {
      // Its still active so hasn't moved. Go to Error state
      Serial.print(F("InitialiseHopper: *** Error *** Hopper failed to move away.\n"));
      hopperState = HOPPER_ERROR;
      return;
    }
  }

  // If we are here, the home switch must be inactive. Try and move it to Home.
  Serial.print(F("InitialiseHopper: Hopper away from home position. Moving to home.\n"));
  for (int i = 0; i < MAX_TURNS_TO_HOME; i++)
  {

    // Flash the LEDs at the same time
    if (ledsAreOn == true)
    {
      // Its on so turn it off
      ledsAreOn = false;
      digitalWrite(homeLEDPin, LOW);
      digitalWrite(awayLEDPin, LOW);
    }
    else
    {
      // Its off so turn it on
      ledsAreOn = true;
      digitalWrite(homeLEDPin, HIGH);
      digitalWrite(awayLEDPin, HIGH);
    }
    
    for (int j = 0; j < stepsPerRevolution; j++)
    {
      // Check to see if we have hit the home stop
      if (IsHomeActive() == true)
      {
        // Yes. Init is complete
        Serial.print(F("InitialiseHopper: Hopper successfully moved home.\n"));
        hopperState = HOPPER_HOME;
        return;
      }
      
      // Move at half speed.
      MoveStepper(STEP_HOME, defaultDelay*2);        
    }
    Serial.print(F("InitialiseHopper: Full turn home complete.\n"));
  }

  // If we reach here, it means we have not found the Home switch. So throw an error
  Serial.print(F("InitialiseHopper: *** Error *** Hopper failed to move home.\n"));
  hopperState = HOPPER_ERROR;
}


//
// Wait For Hopper Trigger (Home)
//
// Hopper is at home position and waiting for trigger to move.
// When triggered, move to MOVE_AWAY state
//
void
WaitForHopperTrigger()
{
  int count;
  long longScratch1;
  long longScratch2;
  long longScratch3;
  
  // Disable the stepper controller to save power
  digitalWrite(enablePin, HIGH);

  // Turn on Home LED and AWAY off
  digitalWrite(homeLEDPin, HIGH);
  digitalWrite(awayLEDPin, LOW);

  // At start, wait 5 seconds before doing anything
  Serial.print(F("WaitForHopperTrigger: At start, waiting 5 seconds.\n"));
  delay(5000);

  // Show we are not busy
  digitalWrite(hopperBusyPin, LOW);
  

  Serial.print(F("WaitForHopperTrigger: Waiting for trigger to move.\n"));

  // Keep track of how long we wait so we can read trimmers. First time read them at the start
  count = 0;
  
  // Check the trigger
  while(IsTriggerActive() == false)
  {
    // Sleep for 1/10 second if not
    delay(100);
    --count;
    
    // Timed out for trimmers
    if (count < 0)
    {
      // Set to wait 15 seconds before reading again
      count = 150;
      
      // Set delay time to govern speed of hopper
      // Bigger than int values so work it out long hand or it doesnt work very well.
      longScratch1 = MAX_DELAY - MIN_DELAY;
      longScratch2 = longScratch1 * analogRead(A1);
      longScratch3 = longScratch2 / 1024L;
      defaultDelay = int(longScratch3) + MIN_DELAY;
      Serial.print(F("WaitForHopperTrigger: Setting defaultDelay to "));
      Serial.print(defaultDelay);
      Serial.print(F(" \n"));

      // Set how many turnas to away
      turnsToAway = (((MAX_TURNS_TO_AWAY - MIN_TURNS_TO_AWAY) * analogRead(A2)) / 1024) + MIN_TURNS_TO_AWAY;
      Serial.print(F("WaitForHopperTrigger: Setting turnsToAway to "));
      Serial.print(turnsToAway);
      Serial.print(F(" \n"));

      // Set the time to wait at the top before returning.
      // Bigger than int values so work it out long hand or it doesnt work.
      longScratch1 = MAX_WAIT - MIN_WAIT;
      longScratch2 = longScratch1 * analogRead(A0);
      longScratch3 = longScratch2 / 1024L;
      awayWaitTime = int(longScratch3) + MIN_WAIT;

      Serial.print(F("WaitForHopperTrigger: Setting awayWaitTime to "));
      Serial.print(awayWaitTime);
      Serial.print(F(" \n"));
      
    }
  }

  // We have been triggered
  Serial.print(F("WaitForHopperTrigger: Got Trigger, state changed to Move-Away.\n"));

  // Turn off Home LED
  digitalWrite(homeLEDPin, LOW);

  // Show we are busy
  digitalWrite(hopperBusyPin, HIGH);

  hopperState = HOPPER_MOVE_AWAY;
}



//
// Move Hopper to Away position
//
// Trigger has been received and hopper is moved away.
// The speed to of the hopper is ramped up at the start and slowed down as the end.
// Hopper will stop when it hits the Away switch.
// At end, move to AWAY state.
//
void
MoveHopperToAway()
{
  long stepCount = 0;
  int  awayDelayTime;
  bool awayLEDIsOn = false;
  
  Serial.print(F("MoveHopperToAway: Moving Hopper to away point.\n"));

  for (int i = 0; i < turnsToAway; i++)
  {
    // Start off at half speed for a few turns and then normal speed until 75% and then back to half
    if ((i < 2) || (i > (turnsToAway * 0.75)))
    {
      awayDelayTime = defaultDelay*2;
    }
    else
    {
       awayDelayTime = defaultDelay;
    }

    // Flash the LED each turn
    if (awayLEDIsOn == true)
    {
      // Its on so turn it off
      awayLEDIsOn = false;
      digitalWrite(awayLEDPin, LOW);
    }
    else
    {
      // Its off so turn it on
      awayLEDIsOn = true;
      digitalWrite(awayLEDPin, HIGH);
    }


    for (int j = 0; j < stepsPerRevolution; j++)
    {
      // Check to see if we have hit the away stop
      if (IsAwayActive() == true)
      {
        // Yes. move is complete
        Serial.print(F("MoveHopperToAway: Hopper hit away switch.\n"));
        Serial.print(F("MoveHopperToAway: Hopper successfully moved away.\n"));
        Serial.print(F("MoveHopperToAway: "));
        Serial.print(stepCount);
        Serial.print(F(" steps taken.\n"));
        hopperState = HOPPER_AWAY;
        return;
      }

      MoveStepper(STEP_AWAY, awayDelayTime);  
      ++stepCount;      
    }

    // *** Oddly, with this debug code added, the motor clicks every turn ***
    //Serial.print(F("MoveHopperToAway: Full turn away complete.\n"));
    //Serial.print(F("MoveHopperToAway: Delay was "));
    //Serial.print(awayDelayTime);
    //Serial.print(F("uS\n"));
  }

  // If we reach here, it means we have made the requested number of turns. This may be less than the end switch so is ok.
  Serial.print(F("MoveHopperToAway: Hopper moved requested turns.\n"));
  Serial.print(F("MoveHopperToAway: Hopper successfully moved away.\n"));
  Serial.print(F("MoveHopperToAway: "));
  Serial.print(stepCount);
  Serial.print(F(" steps taken.\n"));
  hopperState = HOPPER_AWAY;
  return;
}


//
// Wait for Hopper to empty (AWAY)
//
// Sleeps for a short period to ensure hopper is empty.
// Next state is MOVE_HOME
// 
void
WaitForHopperToEmpty()
{
  // Disable the stepper controller to save power
  digitalWrite(enablePin, HIGH);

  // Turn on the LED
  digitalWrite(awayLEDPin, HIGH);
  
  // Sleep for a while 
  Serial.print(F("WaitForHopperToEmpty: Sleeping for "));
  Serial.print(awayWaitTime);
  Serial.print(F(" mS\n"));
  delay(awayWaitTime);

  // Turn off the LED
  digitalWrite(awayLEDPin, LOW);

  // Then move home
  Serial.print(F("WaitForHopperToEmpty: Moving to Home\n"));
  hopperState = HOPPER_MOVE_HOME;
}


//
// Move hoppper to Home position
//
// Hopper is emptied and moved home.
// The speed to of the hopper is ramped up at the start and slowed down as the end.
// Hopper will stop when it hits the Home switch.
// At end, move to HOME state.
//
void
MoveHopperToHome()
{
  long stepCount = 0;
  int  homeDelayTime;
  bool homeLEDIsOn = false;
  
  Serial.print(F("MoveHopperToHome: Moving Hopper to home point.\n"));

  for (int i = 0; i < MAX_TURNS_TO_HOME; i++)
  {
    // Start off at half speed for a few turns and then normal speed until 75% and then back to half
    if ((i < 4) || (i > (MAX_TURNS_TO_HOME * 0.75)))
    {
      homeDelayTime = defaultDelay*2;
    }
    else
    {
       homeDelayTime = defaultDelay;
    }

    // Flash the LED each turn
    if (homeLEDIsOn == true)
    {
      // Its on so turn it off
      homeLEDIsOn = false;
      digitalWrite(homeLEDPin, LOW);
    }
    else
    {
      // Its off so turn it on
      homeLEDIsOn = true;
      digitalWrite(homeLEDPin, HIGH);
    }

    for (int j = 0; j < stepsPerRevolution; j++)
    {
      // Check to see if we have hit the home stop
      if (IsHomeActive() == true)
      {
        // Yes. move is complete
        Serial.print(F("MoveHopperToHome: Hopper successfully moved home.\n"));
        Serial.print(F("MoveHopperToHome: "));
        Serial.print(stepCount);
        Serial.print(F(" steps taken.\n"));

        // Turn off the LED
        digitalWrite(homeLEDPin, LOW);

        hopperState = HOPPER_HOME;
        return;
      }

      MoveStepper(STEP_HOME, homeDelayTime);  
      ++stepCount;      
    }
    // *** Oddly, with this debug code added, the motor clicks every turn ***
    //Serial.print(F("MoveHopperToHome: Full turn home complete.\n"));
    //Serial.print(F("MoveHopperToHome: Delay was "));
   // Serial.print(homeDelayTime);
   // Serial.print(F("uS\n"));
  }

  // If we reach here, it means we have not found the Away switch. So throw an error
  Serial.print(F("MoveHopperToHome: *** Error *** Hopper failed to trigger home switch.\n"));
  hopperState = HOPPER_ERROR;  
}


//
// Error state
//
// Something has gone wrong. Kill the power and flash the Error LED.
// Possibly retry after a time ???
//
void
HopperError()
{
  Serial.print(F("HopperError: Waiting for Reset.\n"));

  // Disable the controller
  digitalWrite(enablePin, HIGH);

  // Show we are busy 
  digitalWrite(hopperBusyPin, HIGH);

  // loop forever until reset
  while(true)
  {
    // flash the LEDs alternatly
    digitalWrite(homeLEDPin, HIGH);
    digitalWrite(awayLEDPin, LOW);
    delay(1000);
    digitalWrite(homeLEDPin, LOW);
    digitalWrite(awayLEDPin, HIGH);
    delay(1000);
    
  }
}
