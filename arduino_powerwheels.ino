// Direction
#define forward 1
#define stopped 0
#define reverse -1

// Commands
#define cmd_none   0
#define cmd_fwdlo  1
#define cmd_fwdhi  3
#define cmd_rev    4

// Input definitions
#define ON  LOW

// Input pin definitions
#define fwdpin1 4  // Low speed
#define fwdpin2 6  // High speed
#define revpin  2  // Reverse

// Output pin definitions
#define fwdpwm 9   // PWM for forward
#define revpwm 11  // PWM for reverse

// Variables
int currentmode=stopped;  // To store current direction of motion
int pwmspeed=0;           // To store current commanded speed: value may be from 0 to 255. Zero means stopped
byte command=0;           // to store Commands
byte prevcommand =0 ;     // for blinking debuging
bool blink = true;
bool turnonBlinking = false;

// Values for accel/decel/braking
#define accel_ratelo  8
#define accel_ratehi  14
#define decel_rate    10
#define brake_rate    13 

// Value for delay when changing direction without stopping first
#define directionchangedelay 500

// values for maximum commanded motor speed (maximum is 255)
#define maxfwd1 128
#define maxfwd2 255
#define maxrev  110

/* prototype to allow default value */
void slowdown(bool penalty=false);

/*****************************************************************************************************************************************/
 
void setup()
{
  // Set up input & output pin modes
  
  /* using pullup/down (like below) is a good idea as it keeps suprious voltage/signals 
  * from the enviroment causing pins to change state. When that happens, uncommanded 
  * movement of the car can happen resulting in possible dangerous things happening.
  * In that vein, since this is a "drive by wire" system, I would highly recommend
  * putting a switch on the console that turns everything off (disconnects battery)
  * and tell your kids to flip that switch if weird stuff starts happening.
  */
  
  pinMode(fwdpin1,INPUT_PULLUP);
  pinMode(fwdpin2,INPUT_PULLUP);
  pinMode(revpin,INPUT_PULLUP);
  pinMode(fwdpwm,OUTPUT);
  pinMode(revpwm,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  /************** SET PWM frequency. default is 450hz  **********************/

  // check out this site for lots more info: 
  // http://arduino-info.wikispaces.com/Arduino-PWM-Frequency

  //TCCR2B = TCCR2B & B11111000 | B00000010; //4 kHz uno
  //TCCR2B = TCCR2B & B11111000 | B00000001; //32 kHz uno
  
  TCCR2B = TCCR2B & B11111000 | B00000010; //4 khz mega 9&10

  //FOR PINS PWM PINS 5&6 only....
  //TCCR0B = TCCR0B & B11111000 | B00000010; 8 for PWM frequency of  7812.50 Hz
}
  
void loop()
{
   //read the pin statuses
   command=readCommand();
   if (command != prevcommand) //this basically sets it to only blink one time when the command state changes.
   {
      prevcommand=command;
      if (0 <= cmd_none && command <= cmd_rev)
      {
          blinkLight(command);
      }
   }
   switch(command)
   {
      case cmd_none:
         {
            if(currentmode != stopped)
            {
               slowdown();
            }
            break;
         }
      case cmd_fwdlo:
         {
            forwardlo();
            break;
         }
      case cmd_fwdhi:
         {
            forwardhi();
            break;
         }
      case cmd_rev:
         {
            reverselo();
            break;
         }
      default:
         {
            //Error! ALL STOP!
            allstop(); // stop *before* blinking the light
            blinkLight(5); 
            delay(3000); // delay before starting up again
            break;
         }
   }
}

/*****************************************************************************************************************************************/
int readCommand() // Read the input pins and return a value for the current input state
{
   int count=0;
   if(digitalRead(fwdpin1)==ON)
   {
      count+=1;
      
   }
   if(digitalRead(fwdpin2)==ON)
   {
      count+=2;
   }
   if(digitalRead(revpin)==ON)
   {
      count+=4;
   }
   return count;
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
void slowdown(bool penalty)
{
   pwmspeed-=decel_rate;
   if(pwmspeed<0)
   {
      pwmspeed=0;
   }

   commandMotor();

   // enforced when lever is changed direction and pedal is pressed before vehicle has come to a stop.
   if(penalty && pwmspeed==0) // add a delay (that'll teach 'em not to mess around with the lever!)
   {
      delay(directionchangedelay);
   }
}
/*****************************************************************************************************************************************/
void forwardlo() // moves vehicle foward up to low speed maximum
{
   
   if(currentmode==stopped) // go from stopped to fwd1
   { 
      currentmode=forward;
      acceleratehi();
   } 
   else if(currentmode==reverse) // go from reverse to fwd 1
   {
      slowdown(true);
   } 
   else if(pwmspeed>maxfwd1) // slow from fwd2 down to fwd1
   {
      slowdown();
   } 
   else if(pwmspeed<maxfwd1)
   {   // continue to accelerate to fwd1
      acceleratehi();
   }
}
/*****************************************************************************************************************************************/
void forwardhi()
{
   
   if(currentmode==stopped) // go from stopped to fwd2
   { 
      currentmode=forward;
      acceleratelo();
   } 
   else if(currentmode==reverse) // go from reverse to fwd2
   { 
      slowdown(true);
   } 
   else if(pwmspeed<maxfwd2)
   {   // continue to accelerate to fwd2
      acceleratelo();
   }
}
/*****************************************************************************************************************************************/
void reverselo()
{
   if(currentmode==stopped)   // go from stopped to reverse
   {
      currentmode=reverse;
      acceleratelo();           // change to acceratehi() if you want to reverse at a quicker rate!
   } 
   else if(currentmode==forward) // go from fwd1/2 to reverse
   {
      slowdown(true);
   } 
   else if(pwmspeed<maxrev)        // continue to accelerate to reverse
   {
      acceleratelo();           // change to acceratehi() if you want to reverse at a quicker rate!
   }
}
/*****************************************************************************************************************************************/
void allstop() // Emergency brake! Used when Error condition detected
{
   pwmspeed=0;
   commandMotor();
   currentmode=stopped;
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
void acceleratehi()// First gear! (also may be used for reverse if you want to reverse faster)
{
   
   pwmspeed+=accel_ratehi;
   commandMotor();
}
/*****************************************************************************************************************************************/
void acceleratelo() // Second gear! (hence lower acceleration); note, also used for reverse
{
   pwmspeed+=accel_ratelo;
   commandMotor();
}

void blinkLight(int times)
{
   if (turnonBlinking) // for debugging. turn off with global variable
   {
     for (int x=0;x<times;x++)
     {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);                       
        digitalWrite(LED_BUILTIN, LOW);  
        delay(150);
     }
   }
}



void commandMotor()
{
  
   /* Bounds check, if greater than max, set it to 255 max. 
   * This is need if the accel/decel rate is not a perfect multiple of the max speed(s)
   * Good safety measure anyways since the PWM is limited to a byte value of 255 max
   * yet the pwm is set to an int. 
   * Bizarre (aka unsafe) stuff will happen if you try to set the PWM value > 255. 
   */
   if (pwmspeed > 254) //bounds check, if greater than max, set it to max. This is need if the accel/decel rate is not a perfect multiple of the max speed(s)
   {
      
      pwmspeed=255;
   }
   
   // send the command to the motor
   if(pwmspeed==0)
   {   // All stopped
      analogWrite(fwdpwm,0);
      analogWrite(revpwm,0);
      currentmode=stopped;
   } 
   else if(currentmode==forward)
   {   // forward motion
      analogWrite(revpwm,0);
      analogWrite(fwdpwm,pwmspeed);
   } 
   else 
   {   // reverse motion
      analogWrite(fwdpwm,0);
      analogWrite(revpwm,pwmspeed);
   }
   delay(50); 
}
/*****************************************************************************************************************************************/

