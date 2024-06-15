#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <BH1750.h>
BH1750 lightMeter;

#include <DHT.h>  // DHT 라이브러리 포함
#define DHT11_PIN 7  // DHT11의 디지털 핀 7 정의
DHT dht(DHT11_PIN, DHT11);  // DHT 객체 생성

#include <Servo.h>
Servo lr_servo; // 좌우 회전 서보
Servo ud_servo; // 상하 회전 서보

const byte interruptPin = 2;  // 버튼 핀 정의

int lr_angle = 90;  // 초기 각도 설정
int ud_angle = 10;  // 초기 각도 설정
int l_state = A0;  // 포토레지스터의 아날로그 입력 핀 정의
int r_state = A1;
int u_state = A2;
int d_state = A3;
const byte buzzer = 6;  // 버저 핀 정의
const byte lr_servopin = 9;  // 좌우 회전 서보 제어 신호 핀 정의
const byte ud_servopin = 10; // 상하 회전 서보 제어 신호 핀 정의

unsigned int light; // 빛 강도 변수
byte error = 15; // 진동 방지 오차 범위
byte m_speed = 10; // 서보 속도 조절 딜레이
byte resolution = 1;  // 서보 회전 정밀도
int temperature;  // 온도 변수
int humidity; // 습도 변수

void setup() {
  Serial.begin(9600); // 시리얼 통신 시작
  Wire.begin();
  lightMeter.begin();

  lr_servo.attach(lr_servopin);  // 서보 제어 핀 설정
  ud_servo.attach(ud_servopin);  // 서보 제어 핀 설정
  pinMode(l_state, INPUT); // 핀 모드 설정
  pinMode(r_state, INPUT);
  pinMode(u_state, INPUT);
  pinMode(d_state, INPUT);

  pinMode(interruptPin, INPUT_PULLUP);  // 버튼 핀을 풀업 모드로 설정
  attachInterrupt(digitalPinToInterrupt(interruptPin), adjust_resolution, FALLING); // 외부 인터럽트 설정

  lcd.init(); // LCD 초기화
  lcd.backlight(); // LCD 백라이트 켜기

  lr_servo.write(lr_angle); // 초기 각도로 서보 이동
  delay(1000);
  ud_servo.write(ud_angle);
  delay(1000);

  dht.begin(); // DHT 센서 초기화
}

void loop() {
  ServoAction();  // 서보 동작 수행
  read_light();   // BH1750 센서로부터 빛 강도 읽기
  read_dht11();   // DHT11 센서로부터 온도와 습도 읽기
  LcdShowValue(); // LCD에 값 표시
}

/**********서보 함수************/
void ServoAction(){
  int L = analogRead(l_state); // 아날로그 값 읽기
  int R = analogRead(r_state);
  int U = analogRead(u_state);
  int D = analogRead(d_state);
  /**********************좌우 조정**********************/
  if (abs(L - R) > error && L > R) {
    lr_angle -= resolution;
    if (lr_angle < 0) {
      lr_angle = 0;
    }
    lr_servo.write(lr_angle);
    delay(m_speed);
  }
  else if (abs(L - R) > error && L < R) {
    lr_angle += resolution;
    if (lr_angle > 180) {
      lr_angle = 180;
    }
    lr_servo.write(lr_angle);
    delay(m_speed);
  }
  else if (abs(L - R) <= error) {
    lr_servo.write(lr_angle);
  }
  /**********************상하 조정**********************/
  if (abs(U - D) > error && U >= D) {
    ud_angle -= resolution;
    if (ud_angle < 10) {
      ud_angle = 10;
    }
    ud_servo.write(ud_angle);
    delay(m_speed);
  }
  else if (abs(U - D) > error && U < D) {
    ud_angle += resolution;
    if (ud_angle > 90) {
      ud_angle = 90;
    }
    ud_servo.write(ud_angle);
    delay(m_speed);
  }
  else if (abs(U - D) <= error) {
    ud_servo.write(ud_angle);
  }
}

void LcdShowValue() {
  char str1[5];
  char str2[2];
  char str3[2];
  dtostrf(light, -5, 0, str1); // 문자열 형식으로 변환
  dtostrf(temperature, -2, 0, str2);
  dtostrf(humidity, -2, 0, str3);
  lcd.setCursor(0, 0);
  lcd.print("Light:");
  lcd.setCursor(6, 0);
  lcd.print(str1);
  lcd.setCursor(11, 0);
  lcd.print("lux");
  
  lcd.setCursor(0, 1);
  lcd.print(temperature);
  lcd.setCursor(2, 1);
  lcd.print("C");
  lcd.setCursor(5, 1);
  lcd.print(humidity);
  lcd.setCursor(7, 1);
  lcd.print("%");

  lcd.setCursor(11, 1);
  lcd.print("res:");
  lcd.setCursor(15, 1);
  lcd.print(resolution);
}

void read_light(){
  light = lightMeter.readLightLevel();  // BH1750 센서로부터 빛 강도 읽기
}

void read_dht11(){
  humidity = dht.readHumidity(); // 습도 읽기
  temperature = dht.readTemperature(); // 온도 읽기
}

/*********인터럽트 서비스 함수**************/
void adjust_resolution() {
  tone(buzzer, 800, 100);
  delay(10);  // 진동 제거 딜레이
  if (!digitalRead(interruptPin)){
    if(resolution < 5){
      resolution++;
    }else{
      resolution = 1;
    }
  }
}
