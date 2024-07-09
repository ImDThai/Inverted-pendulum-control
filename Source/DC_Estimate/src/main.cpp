#include <Arduino.h>

int RL_EN = 10;
int R_PWM = 5;
int L_PWM = 6;
int v = -250;
float Limit_Time = 0.5;
int counter = 0;

#define OUTPUT_A  2 // PE5   - Khai báo chân A encoder của motor DC
#define OUTPUT_B  3 // PE4   - Khai báo chân B encoder của motor DC

#define REF_OUT_A 18 // PD3  - Khai báo chân A encoder của con lắc
#define REF_OUT_B 19 // PD2  - Khai báo chân B encoder của con lắc



volatile long encoderValue = 0L;              // Khai báo biến encoder động cơ kiểu long
volatile long lastEncoded = 0L;

volatile long refEncoderValue = 0;            // Khai báo biến encoder con lắc kiểu long
volatile long lastRefEncoded = 0;

volatile bool wait = false;
volatile bool start = false;

long begin_x;

unsigned long time;


void encoderHandler();     // Người xử lý mã hóa động cơ
void refEncoderHandler(); 

void motor(int speed){
  if(speed == 0){
    digitalWrite(R_PWM, HIGH);
    digitalWrite(L_PWM, HIGH);
  }
  else{
    if(speed > 0){
      digitalWrite(R_PWM, LOW);
      digitalWrite(L_PWM, HIGH); // clockwise
      analogWrite(RL_EN, speed);
    }
    else{
      digitalWrite(R_PWM, HIGH);
      digitalWrite(L_PWM, LOW); // counterclockwise
      analogWrite(RL_EN, abs(speed));
    }
  }
}

float x_parameter(long prr){
  float resuilts = 0.011280*PI*(prr/2400.0);
  return resuilts;
}

void setup() {
 Serial.begin(19200);

  pinMode(OUTPUT_A, INPUT_PULLUP);
  pinMode(OUTPUT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(OUTPUT_A), encoderHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(OUTPUT_B), encoderHandler, CHANGE);


  pinMode(RL_EN, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);

  digitalWrite(R_PWM, HIGH);
  digitalWrite(L_PWM, HIGH);
}

void loop() {
    if(!wait){
      if(millis() >= 5000){
        time = micros();
        start = true;
        wait = true;
      }
    }
    if(start){
      
      if((micros() - time) <= Limit_Time*1000000){
      motor(v);
      Serial.print(counter);
      Serial.print(",");
      Serial.print(v);
      Serial.print(",");
      Serial.print(micros()-time);
      Serial.print(",");
      Serial.print(x_parameter(encoderValue),6);
      Serial.print("\n");
      counter += 1;
      }
    else
      motor(0);
    }
    else motor(0);
    
}


void refEncoderHandler() {
  int MSB = (PIND & (1 << PD3)) >> PD3; //MSB = most significant bit
  int LSB = (PIND & (1 << PD2)) >> PD2; //LSB = least significant bit
  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastRefEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    refEncoderValue++; //CW
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    refEncoderValue--; //CCW
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
