/*  MonsterMoto Shield Example Sketch
  date: 10/7/19
  code by: Jim Lindblom & Dennis Irrgang
  hardware by: Nate Bernstein
  SparkFun Electronics

 This is really simple example code to get you some basic
 functionality with the MonsterMoto Shield. The MonsterMote uses
 two VNH2SP30 high-current full-bridge motor drivers.
 
 Use the motorGo(Motor motor, MotorMode mode, uint8_t speed) 
 function to control the motors. Available options are CW, CCW, 
 BRAKEVCC, or BRAKEGND. Use motorOff(Motor motor) to turn a specific motor off.
 
 The motor variable in each function should be a Motor enum, alternatively 0 or a 1.
 pwm in the motorGo function should be a value between 0 and 255.
 
This code is beerware; if you see the authors (or any SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given. */

enum MotorMode
{
  BRAKEVCC,
  CW,
  CCW,
  BRAKEGND,
};

enum Motor
{
  MOTOR_A,
  MOTOR_B,
};

const uint8_t PWM_MAX = 255;
const uint8_t PWM_HALF = PWM_MAX / 2;

const int kCurrentSensingThreshhold = 100;

/* Voltage controlled input pins with hysteresis, CMOS compatible. These two pins
control the state of the bridge in normal operation according to the truth table (brake
to VCC , brake to GND, clockwise and counterclockwise).

Table 1.    Truth table in normal operating conditions
+------+-----+------+-----+-------+-------+---------------------+------------------------+
| INA  | INB | ENA  | ENB | OUTA  | OUTB  |         CS          |          MODE          |
+------+-----+------+-----+-------+-------+---------------------+------------------------+
|    1 |   1 |    1 |   1 | H     | H     | High Imp.           | Brake to VCC           |
|    1 |   0 |    1 |   1 | H     | L     | I_Sense = I_OUT / K | Clockwise (CW)         |
|    0 |   1 |    1 |   1 | L     | H     | I_SENSE = I_OUT / K | Counterclockwise (CCW) |
|    0 |   0 |    1 |   1 | L     | L     | High Imp.           | Brake to GND           |
+------+-----+------+-----+-------+-------+---------------------+------------------------+
For more information, see the VNH2SP30-E motor driver datasheet */
const int kInAPin[2] = {7, 4};
const int kInBPin[2] = {8, 9};

/* Voltage controlled input pins with hysteresis, CMOS compatible. Gates of low side
FETs are modulated by the PWM signal during their ON phase allowing speed
control of the motors. */
const int kPwmPin[2] = {5, 6};

/* When pulled low, disables the half-bridge of the VNH2SP30-E of the motor. In case of 
fault detection (thermal shutdown of a high side FET or excessive ON state voltage drop 
across a low side FET), this pin is pulled low by the device. 

Table 2.    Truth table in fault conditions (detected on OUTA)
+------+-----+------+-----+-------+-------+------------+
| INA  | INB | ENA  | ENB | OUTA  | OUTB  |     CS     |
+------+-----+------+-----+-------+-------+------------+
| 1    | 1   |    1 |   1 | OPEN  | H     | High Imp.  |
| 1    | 0   |    1 |   1 | OPEN  | L     | High Imp.  |
| 0    | 1   |    1 |   1 | OPEN  | H     | I_OUTB / K |
| 0    | 0   |    1 |   1 | OPEN  | L     | High Imp.  |
| X    | X   |    0 |   0 | OPEN  | OPEN  | High Imp.  |
| X    | 1   |    0 |   1 | OPEN  | H     | I_OUTB / K |
| X    | 0   |    0 |   1 | OPEN  | L     | High Imp.  |
+------+-----+------+-----+-------+-------+------------+
For more information, see the VNH2SP30-E motor driver datasheet */
const int kEnPin[2] = {0, 1};

/* Analog current sense input. This input senses a current proportional to the motor
current. The information can be read back as an analog voltage. */
const int kCspPin[2] = {2, 3};

// On-Board LED used as status indication
const int kStatPin = 13;

void motorSetup()
{
  pinMode(kStatPin, OUTPUT);

  // Initialize digital pins as outputs
  for (int i = 0; i < 2; i++)
  {
    pinMode(kInAPin[i], OUTPUT);
    pinMode(kInBPin[i], OUTPUT);
    pinMode(kPwmPin[i], OUTPUT);
  }
  // Initialize with brake applied
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(kInAPin[i], LOW);
    digitalWrite(kInBPin[i], LOW);
  }
}

void motorOff(int motor)
{
  // Initialize brake to Vcc
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(kInAPin[i], LOW);
    digitalWrite(kInBPin[i], LOW);
  }
  analogWrite(kPwmPin[motor], 0);
}

/* motorGo() will set a motor going in a specific direction
 the motor will continue going in that direction, at that speed
 until told to do otherwise.
 
 motor: this should be either MOTOR_A (0) or MOTOR_B (1), will select 
 which of the two motors to be controlled. Invalid values are ignored.
 
 mode: Should be one of the following values
  BRAKEVCC (0): Brake to VCC
  CW (1): Turn Clockwise
  CCW (2): Turn Counter-Clockwise
  BRAKEGND (3): Brake to GND
 
 speed: should be a value between 0 and PWM_MAX (255), higher the number, the faster
 */
void motorGo(Motor motor, MotorMode mode, uint8_t speed)
{

  if (motor == MOTOR_A || motor == MOTOR_B)
  {
    switch (mode)
    {
    case BRAKEVCC: // Brake to VCC
      digitalWrite(kInAPin[motor], HIGH);
      digitalWrite(kInBPin[motor], HIGH);
      break;
    case CW: // Turn Clockwise
      digitalWrite(kInAPin[motor], HIGH);
      digitalWrite(kInBPin[motor], LOW);
      break;
    case CCW: // Turn Counter-Clockwise
      digitalWrite(kInAPin[motor], LOW);
      digitalWrite(kInBPin[motor], HIGH);
      break;
    case BRAKEGND: // Brake to GND
      digitalWrite(kInAPin[motor], LOW);
      digitalWrite(kInBPin[motor], LOW);
      break;

    default:
      // Invalid mode does not change the PWM signal
      return;
    }
    analogWrite(kPwmPin[motor], speed);
  }
  return;
}

void setup()
{
  Serial.begin(9600);

  motorSetup();
}

void loop()
{
  uint8_t speed;
  float motorCurrent;

  while (true)
  {
    motorGo(MOTOR_A, CW, PWM_MAX);
    motorGo(MOTOR_B, CCW, PWM_MAX);
    delay(5000);

    motorOff(MOTOR_A);
    motorOff(MOTOR_B);
    delay(1000);

    motorGo(MOTOR_A, CCW, PWM_MAX);
    motorGo(MOTOR_B, CW, PWM_MAX);
    delay(5000);

    if ((analogRead(kCspPin[0]) < kCurrentSensingThreshhold) && (analogRead(kCspPin[1]) < kCurrentSensingThreshhold))
    {
      digitalWrite(kStatPin, HIGH);
    }
    else
    {
      digitalWrite(kStatPin, LOW);
      break;
    }
  }
  motorOff(MOTOR_A);
  motorOff(MOTOR_B);
}