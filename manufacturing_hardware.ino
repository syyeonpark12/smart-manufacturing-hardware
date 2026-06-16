#include <Servo.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <avr/wdt.h>

// 핀 설정
#define IR_SENSOR_PIN 18
#define DHTPIN 13
#define DHTTYPE DHT22
#define EMERGENCY_BUTTON_PIN 2
#define BUZZER_PIN 11
#define SERVO1_PIN 8
#define SERVO2_PIN 12
#define MOTOR_A_PIN 7
#define MOTOR_B_PIN 4

// 시간 설정
const unsigned long PC_TIMEOUT_MS = 5000;
const unsigned long HEARTBEAT_INTERVAL_MS = 1000;
const unsigned long SENSOR_SEND_INTERVAL_MS = 500;
const unsigned long CONVEYOR_STOP_TIME_MS = 5000;
const unsigned long SENSOR_DEBOUNCE_TIME_MS = 30;
const unsigned long SERVO_RETURN_TIME_MS = 500;

// 동작값 설정
const int SERVO_WAIT_ANGLE = 20;
const int SERVO_ACTION_ANGLE = 110;
const int MOTOR_PWM = 150;
const char* FW_VERSION = "1.2.0";

// 장치 객체
DHT dht(DHTPIN, DHTTYPE);
Servo servo1;
Servo servo2;

// 서보모터 명령 저장 구조체
struct ServoTask {
  int servoId;
  int angle;
};

const int SERVO_QUEUE_SIZE = 10;
ServoTask servoQueue[SERVO_QUEUE_SIZE];

int queueHead = 0;
int queueTail = 0;

bool servoActive = false;
unsigned long servoStartTime = 0;

// 시스템 상태
int systemRunning = 1;
float temperatureValue = 0.0;

unsigned long lastCommandTime = 0;
unsigned long lastHeartbeatTime = 0;
unsigned long lastSensorSendTime = 0;
unsigned long sequenceNumber = 0;

bool pcTimeout = false;
bool emergencyStopped = false;
bool objectDetected = false;
bool sensorArmed = true;

unsigned long sensorLowStartTime = 0;
unsigned long conveyorStopStartTime = 0;

void setup() {
  Serial.begin(115200);

  dht.begin();

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  moveBothServos(SERVO_WAIT_ANGLE);

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(EMERGENCY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOTOR_A_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);

  lastCommandTime = millis();

  // 프로그램이 멈췄을 때 약 8초 후 자동 재시작
  wdt_enable(WDTO_8S);

  sendBootMessage();
}

void loop() {
  wdt_reset();

  unsigned long now = millis();

  processSerialCommunication(now);
  processEmergencyStop(now);
  processPcTimeout(now);
  processServoControl(now);
  processInfraredSensor(now);

  sendHeartbeat(now);
  sendSensorStatus(now);

  controlConveyorMotor();
}
