#include <Arduino.h>

#define RL_EN  10
#define R_PWM  5
#define L_PWM  6

#define OUTPUT_A  2 // PE5   - Khai báo chân A encoder của motor DC
#define OUTPUT_B  3 // PE4   - Khai báo chân B encoder của motor DC

#define REF_OUT_A 18 // PD3  - Khai báo chân A encoder của con lắc
#define REF_OUT_B 19 // PD2  - Khai báo chân B encoder của con lắc

#define SWITCH_RIGHT  21     // - Khai báo chân công tắc hành trình phải
#define SWITCH_LEFT  20      // - Khai báo chân công tắc hành trình trái

#define MAX_STALL_U 50
#define PPR  2400            // - Khai báo xung của encoder motor
#define SHAFT_R 0.00612     // - Khai báo đường kính rulo (D = 12.24 mm)
#define PENDULUM_ENCODER_PPR  10000   // - Khai báo xung của encoder con lắc

// #define A 11.3
// #define B 0.7              // Bộ hệ số của công thức chuyển đổi lực - điện áp cấp vào động cơ
// #define C -0.1

#define A 11.43
#define B 0.69              // Bộ hệ số của công thức chuyển đổi lực - điện áp cấp vào động cơ
#define C -0.9

#define Kx -10.00
#define Kv -10.29 // Bộ tham số bộ điều khiển
#define Kth 32.00
#define Kw  6.85


#define STATE_CALIBRATE 0        // Trạng thái hiệu chuẩn
#define STATE_SWING_UP  1        // Trạng thái swing up - Du lên
#define STATE_BALANCE   2        // Trạng thái thăng bằng
#define STATE__STOP 3        // Trạng thái ngừng hẳn

const float THETA_THRESHOLD = 10*(PI/180);        // Khai báo ngưỡng ~ 15 độ
const float SU_THETA_THRESHOLD = 90*(PI/180); 
const float PI2 = 2.0 * PI;                   // 360 độ

volatile long encoderValue = 0L;              // Khai báo biến encoder động cơ kiểu long
volatile long lastEncoded = 0L;

volatile long refEncoderValue = 0;            // Khai báo biến encoder con lắc kiểu long
volatile long lastRefEncoded = 0;

volatile boolean leftSwitchPressed = 0;   // Biến nhấn công tắc trái
volatile boolean rightSwitchPressed = 0;  // Biến nhấn công tắc phải
volatile boolean leftPress = 0;
volatile boolean rightPress = 0;

unsigned long now = 0L;
unsigned long lastTimeMicros = 0L;

int state;
char userInput;

float x, last_x, v, dt;
float theta, last_theta, w;
float control, u;
double time;


volatile bool debounce = false;
volatile long debounceTimestampMillis;

long rightSwitchPulses;

float integralError = 0.0;
float lastTarget = 0.0;

void encoderHandler();     // Người xử lý mã hóa động cơ
void refEncoderHandler(); 
void calibrate();

void leftSwitchHandler() { 
  leftSwitchPressed = true && debounce;
  if (leftSwitchPressed){
    leftPress = true;
  }
  debounce = false;
  
}

void rightSwitchHandler() {
  rightSwitchPressed = true && !debounce;
  debounce = true;
}

void calibrate() {
  long lastReading;
  do {
    lastReading = refEncoderValue;
    delay(1000);
  } while (lastReading != refEncoderValue);
  cli();
  refEncoderValue = 0;
  sei();
}
double sign(double x) {
    if (x > 0) {
        return 1.0;
    } else if (x < 0) {
        return -1.0;
    } else {
        return 0.0;
    }
}


void setup() {
  Serial.begin(9600);

  pinMode(OUTPUT_A, INPUT_PULLUP);
  pinMode(OUTPUT_B, INPUT_PULLUP);

  pinMode(REF_OUT_A, INPUT_PULLUP);
  pinMode(REF_OUT_B, INPUT_PULLUP);

  pinMode(SWITCH_LEFT, INPUT_PULLUP);
  pinMode(SWITCH_RIGHT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(OUTPUT_A), encoderHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(OUTPUT_B), encoderHandler, CHANGE);

  attachInterrupt(digitalPinToInterrupt(REF_OUT_A), refEncoderHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(REF_OUT_B), refEncoderHandler, CHANGE);

  attachInterrupt(digitalPinToInterrupt(SWITCH_LEFT), leftSwitchHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(SWITCH_RIGHT), rightSwitchHandler, RISING);

  pinMode(RL_EN, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);

  digitalWrite(R_PWM, HIGH);
  digitalWrite(L_PWM, HIGH);

  lastTimeMicros = 0L;
  state = STATE_CALIBRATE;
}

float avoidStall(float u) {
  if (fabs(u) < MAX_STALL_U) {
    // return u > 0 ? u +  MAX_STALL_U : u -  MAX_STALL_U;
    return u > 0 ? 50 : -50;
  }
  return u;
}

float getCartDistance(long pulses, long ppr) {
  return 2.0 * PI * pulses / PPR * SHAFT_R;
}

float getAngle(long pulses, long ppr) {  
  float angle = PI2 * pulses / ppr;
  return angle;
}

void driveMotor(float speed){
    if(speed > 0.0){
      digitalWrite(R_PWM, LOW);
      digitalWrite(L_PWM, HIGH); // clockwise
    }
    else{
      digitalWrite(R_PWM, HIGH);
      digitalWrite(L_PWM, LOW); // counterclockwise
    }
    analogWrite(RL_EN, fabs(speed));
}

float saturate(float v, float maxValue) {
  if (fabs(v) > maxValue) {
    return (v > 0) ? maxValue : -maxValue;
  } else {
    return v;
  }
}

boolean isControllable(float theta, float w, float x) {
  return fabs(fabs(fmod(theta, PI2))-PI) < THETA_THRESHOLD && fabs(w) < 15.0 && fabs(x) < 0.2;
}
boolean su_isControllable(float theta, float w, float x) {
  return fabs(fabs(fmod(theta, PI2))-PI) > SU_THETA_THRESHOLD && fabs(w) < 7.5 && fabs(x) < 0.20;
}


float getBalancingControl(float x, float v, float theta, float w) {
  return -(Kx * x + Kv * v + Kth * (-copysignf(PI, theta) + fmod(theta, PI2)) + Kw * w);
}

void driveMotorWithControl(float control, float v) {
  u = (3.13185 * control + A * v - copysignf(C, v)) / B;
  u = 255.0 * u / 12.0;
  driveMotor(saturate(avoidStall(u), 255));
}

float getSwingUpControl(float x, float v, float theta, float w) {

  float ksu = 1.5;
  float kcw = 1.5;
  float kvw = 0.75;
  float kww = 2.5;
  float kem = 0.5;
  float n = 1.05;

  float c = 0.32 * kvw * log(1.0 - 2.0 * abs(v)) * sign(v) - 1.8 * cos(theta) * (0.18 * sin(theta) + cos(theta) * (0.018 * kvw * log(1.0 - 2.0 * abs(v)) * sign(v) - 0.018 * ksu * sign(w * cos(theta)) + 0.018 * kcw * log(1.0 - 5.0 * abs(x)) * sign(x) + 0.018 * kww * log(1.0 - 0.1 * abs(w)) * sign(w) - 0.018 * kem * sign(w * cos(theta)) * sign(0.18 * cos(theta) + 0.17) * (exp(abs(0.36 * n + 0.18 * cos(theta) - 0.18)) - 1.0))) - 0.32 * ksu * sign(w * cos(theta)) - 0.018 * pow(w, 2) * sin(theta) + 0.32 * kcw * log(1.0 - 5.0 * abs(x)) * sign(x) + 0.32 * kww * log(1.0 - 0.1 * abs(w)) * sign(w) - 0.32 * kem * sign(w * cos(theta)) * sign(0.18 * cos(theta) + 0.17) * (exp(abs(0.36 * n + 0.18 * cos(theta) - 0.18)) - 1.0);
  return c;
}

float getPositionControl(float x, float v, float target) {
  float pKx = 20.0; //285.4;
  float pKv = 10.0; //37.1;
  return - (pKx * (x - target) + pKv * v);
}

void loop() {


  now = micros();
  dt = 1.0 * (now - lastTimeMicros) / 1000000;
  x = getCartDistance(encoderValue, PPR);
  v = (x - last_x) / dt;

  theta = getAngle(refEncoderValue, PENDULUM_ENCODER_PPR);
  w = (theta - last_theta) / dt;


  switch (state) {
    case STATE_CALIBRATE:
      //Serial.println("Waiting for the pendulum to come to rest...");  // Chờ con lắc dừng lại
      calibrate();
      //Serial.println("Pendulum is at rest");   // Con lắc đang đứng yên
      time = millis();
      state = STATE_SWING_UP;
      break;
    case STATE_SWING_UP:
      if (isControllable(theta, w, x)) {
        state = STATE_BALANCE;
      } else {
        control = getSwingUpControl(x, v, theta, w);
        driveMotorWithControl(control, v);
      }
      break;
    case STATE_BALANCE:
      if (isControllable(theta, w, x)) {
      control = getBalancingControl(x, v, theta, w);
      driveMotorWithControl(control, v);
      }
      else 
        state = STATE__STOP;
      break;
    case STATE__STOP:
      if (su_isControllable(theta, w, x)){
        state = STATE_SWING_UP;
      }
      else {
        control = getPositionControl(x, v, 0.0);
        driveMotorWithControl(control, v);
      }
      break;
    }
    
    if(Serial.available()> 0){ 
    
    userInput = Serial.read();               // read user input
      
      if(userInput == 'g'){                  // if we get expected value 
            Serial.print((millis()-time)*0.001);
            Serial.print(",");
            Serial.print(fmod(theta, PI2)*(180/PI), 2);
            Serial.print(",");
            Serial.print(x*1000);
            Serial.print("\n");     
            
      } // if user input 'g' 
  }
     //Serial.print(millis()-time);
      //Serial.print(",");
      //Serial.print(x,6);
      //Serial.print(",");
      //Serial.print(theta, 6);
      //Serial.print(",");
      //Serial.print(v);
      //Serial.print(",");
      //Serial.print(w);
      //Serial.print("\n");
  last_x = x;
  last_theta = theta;
  lastTimeMicros = now;
  delay(10);
}


void refEncoderHandler() {
  int MSB = (PIND & (1 << PD3)) >> PD3; //MSB = most significant bit
  int LSB = (PIND & (1 << PD2)) >> PD2; //LSB = least significant bit
  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastRefEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    refEncoderValue--; //CW
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    refEncoderValue++; //CCW
  }

  lastRefEncoded = encoded; //store this value for next time  
}

void encoderHandler() {
  int MSB = (PINE & (1 << PE5)) >> PE5; //MSB = most significant bit
  int LSB = (PINE & (1 << PE4)) >> PE4; //LSB = least significant bit
  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderValue--; 
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue++; 
  }

  lastEncoded = encoded; //store this value for next time  
}
