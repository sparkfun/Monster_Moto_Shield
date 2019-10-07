/*  MonsterMoto Shield Example Sketch
  date: 5/24/11
  code by: Jim Lindblom
  hardware by: Nate Bernstein
  SparkFun Electronics

 This is really simple example code to get you some basic
 functionality with the MonsterMoto Shield. The MonsterMote uses
 two VNH2SP30 high-current full-bridge motor drivers.
 
 Use the motorGo(uint8_t motor, uint8_t direct, uint8_t pwm) 
 function to control the motors. Available options are CW, CCW, 
 BRAKEVCC, or BRAKEGND. Use motorOff(int motor) to turn a specific motor off.
 
 The motor variable in each function should be either a 0 or a 1.
 pwm in the motorGo function should be a value between 0 and PWM_MAX.
 
This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
 */
#define BRAKEVCC 0
#define CW 1
#define CCW 2
#define BRAKEGND 3
#define CS_THRESHOLD 100

#define PWM_MAX  255
#define PWM_HALF 127

/*  VNH2SP30 pin definitions
 xxx[0] controls motor '1' outputs
 xxx[1] controls motor '2' outputs */
int inApin[2] = {7, 4}; // INA: Clockwise input
int inBpin[2] = {8, 9}; // INB: Counter-clockwise input
int pwmpin[2] = {5, 6}; // PWM input
int cspin[2] = {2, 3};  // CS: Current sense ANALOG input
int enpin[2] = {0, 1};  // EN: Status of switches output (Analog pin)

int statpin = 13;

void setup()
{
  Serial.begin(9600);

  pinMode(statpin, OUTPUT);

  // Initialize digital pins as outputs
  for (int i = 0; i < 2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
  }
  // Initialize with enabled brake
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
}

void loop()
{
  motorGo(0, CW, PWM_MAX);
  motorGo(1, CCW, PWM_MAX);
  delay(5000);

  motorGo(0, CCW, PWM_MAX);
  motorGo(1, CW, PWM_MAX);
  delay(5000);

  if ((analogRead(cspin[0]) < CS_THRESHOLD) && (analogRead(cspin[1]) < CS_THRESHOLD))
    digitalWrite(statpin, HIGH);
}

void motorOff(int motor)
{
  // Initialize brake to Vcc
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  analogWrite(pwmpin[motor], 0);
}

/* motorGo() controls the motors. It allows control of their speed
 and direction. Once set the motor will continue with its previous
 settings indefinitely
 
 motor: Selects the motor to be controlled.
 Motor 1: 0
 Motor 2: 1
 
 direct: Controls the direction in which the motor spins. Range is 0 - 3
 Settings have the following effect:
 0: Brake to VCC
 1: Spin Clockwise (CW)
 2: Spin Counter-Clockwise (CCW)
 3: Brake to GND
 
 pwm: Range is between 0 (slowest)- 255 (fastest).

 Invalid values are ignored.
 */
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)
{

  if (0 <= motor <= 1)
  {
    if (direct == 0 || 3)
    { // Brake to Vcc
      digitalWrite(inApin[motor], HIGH);
      digitalWrite(inBpin[motor], HIGH);
    }
    else if (direct == 1)
    { // Turn Clockwise
      digitalWrite(inApin[motor], HIGH);
      digitalWrite(inBpin[motor], LOW);
    }
    else if (direct == 2)
    { // Turn Counter-Clockwise
      digitalWrite(inApin[motor], LOW);
      digitalWrite(inBpin[motor], HIGH);
    }
    else
    {
      return;
    }
    analogWrite(pwmpin[motor], pwm);
  }
  return;
}