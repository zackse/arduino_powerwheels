/*****************************************************************************************************************************************
Power wheels drive-by-wire controller code
By Mom's Mechanic
*****************************************************************************************************************************************/
// Variables, Defines
/*****************************************************************************************************************************************/
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
#define fwdpin1 5  // Low speed
#define fwdpin2 4  // High speed
#define revpin  2  // Reverse

// Output pin definitions
#define fwdpwm 9   // PWM for forward
#define revpwm 11  // PWM for reverse

// Variables
byte currentmode=stopped;  // To store current direction of motion
int pwmspeed=0;            // To store current commanded speed: value may be from -255 (reverse) to 255 (forward). Zero means stopped
byte command;              // to store Commands


// Values for accel/decel/braking
#define accel_ratelo  2
#define accel_ratehi  4
#define decel_rate    3
#define brake_rate    6

// Value for delay when changing direction without stopping first
#define directionchangedelay 1000

// values for maximum commanded motor speed (maximum is 255 for forward directions, and -255 for reverse)
#define maxfwd1  128
#define maxfwd2  180
#define maxrev  -128

/*****************************************************************************************************************************************/
 
void setup()
{
  // Set up input & output pin modes
  pinMode(fwdpin1,INPUT_PULLUP);
  pinMode(fwdpin2,INPUT_PULLUP);
  pinMode(revpin,INPUT_PULLUP);
  pinMode(fwdpwm,OUTPUT);
  pinMode(revpwm,OUTPUT);
}
  
void loop()
{
  //read the pin statuses
  command=readCommand();
  switch(command)
  {
    case cmd_none:
    {
      if(currentmode != stopped){
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
      allstop();
      break;
    }
  }
}
 
/*****************************************************************************************************************************************/
int readCommand(){
  // Read the input pins and return a value for the current input state
  int count=0;
  if(digitalRead(fwdpin1)==ON){
    count+=1;
  }
  if(digitalRead(fwdpin2)==ON){
    count+=2;
  }
  if(digitalRead(revpin)==ON){
    count+=4;
  }
  return count;
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
void slowdown(){
  // slows vehicle down when pedal is released
  if(pwmspeed>0){ // motor is currently set to forward
    pwmspeed-=decel_rate;
    if(pwmspeed<0){
      pwmspeed=0;
    }
  } else
  if(pwmspeed<0){ // motor is current set to reverse 
    pwmspeed+=decel_rate;
    if(pwmspeed>0){
      pwmspeed=0;
    }
  }
  commandMotor();
}
/*****************************************************************************************************************************************/
void forwardlo(){
  // moves vehicle foward up to low speed maximum
  if(currentmode==stopped){ // go from stopped to fwd1
    currentmode=forward;
    acceleratehi();
  } else 
  if(currentmode==reverse){ // go from reverse to fwd 1
    brake();
  } else
  if(pwmspeed>maxfwd1){ // slow from fwd2 down to fwd1
    slowdown();
  } else
  if(pwmspeed<maxfwd1){ // continue to accelerate to fwd1
    acceleratehi();
  }
}
/*****************************************************************************************************************************************/
void forwardhi(){
  if(currentmode==stopped){ // go from stopped to fwd2
    currentmode=forward;
    acceleratelo();
  } else 
  if(currentmode==reverse){ // go from reverse to fwd2
    brake();
  } else
  if(pwmspeed<maxfwd2){  // continue to accelerate to fwd2
    acceleratelo();
  }
}
/*****************************************************************************************************************************************/
void reverselo(){
  if(currentmode==stopped){   // go from stopped to reverse
    currentmode=reverse;
    acceleratelo();           // change to acceratehi() if you want to reverse at a quicker rate!
  } else 
  if(currentmode==forward){   // go from fwd1/2 to reverse
    brake();
  } else
  if(pwmspeed>maxrev){        // continue to accelerate to reverse
    acceleratelo();           // change to acceratehi() if you want to reverse at a quicker rate!
  }
}
/*****************************************************************************************************************************************/
void allstop(){
  // Emergency brake! Used when Error condition detected
  pwmspeed=0;
  commandMotor();
  delay(3000); // delay before starting up again
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
void acceleratehi(){
  // First gear! (also may be used for reverse if you want to reverse faster)
  if(currentmode==forward){ 
    pwmspeed+=accel_ratehi;
  } else {
    pwmspeed-=accel_ratehi;
  }
  commandMotor();
}
/*****************************************************************************************************************************************/
void acceleratelo(){
  // Second gear! (hence lower acceleration); note, also used for reverse
  if(currentmode==forward){
    pwmspeed+=accel_ratelo;
  } else {
    pwmspeed-=accel_ratelo;
  }
  commandMotor();
}
/*****************************************************************************************************************************************/
void brake(){
  // Stop at high rate, used when lever is changed direction and pedal is pressed before vehicle has come to a stop.
  if(pwmspeed>0){  // slow from forward direction
    pwmspeed-=brake_rate;
    if(pwmspeed<0){
      pwmspeed=0;
    }
  } else
  if(pwmspeed<0){  // slow from reverse direction
    pwmspeed+=brake_rate;
    if(pwmspeed>0){
      pwmspeed=0;
    }
  }
  commandMotor();
  if(pwmspeed==0){    // add a delay (that'll teach 'em not to mess around with the lever!)
    delay(directionchangedelay);
  }
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
void commandMotor(){
  // send the command to the motor
  if(pwmspeed==0){ // All stopped
    analogWrite(fwdpwm,0);
    analogWrite(revpwm,0);
    currentmode=stopped;
  } else 
  if(pwmspeed>0){ // forward motion
    analogWrite(revpwm,0);
    analogWrite(fwdpwm,pwmspeed);
  } else {  // reverse motion
    analogWrite(fwdpwm,0);
    analogWrite(revpwm,-1*pwmspeed);
  }
  delay(50); // forgot to include this line originally, added June 30, 2015
}
/*****************************************************************************************************************************************/

