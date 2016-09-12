/*

1. 블루투스 시리얼 통신
>> 소프트웨어 시리얼 통신 방식 사용.
>> 하드웨어 시리얼 통신 사용.
2. IR 리모트 컨트롤러 운행.

>> Ver 1.2 기초 코드.
*/

#include <IRremote.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

#include "MPU6050_6Axis_MotionApps20.h"


/*
하드웨어   시리얼 포트를 사용하는 경우 //#define __USE_BT_SOFTWARE_SERIAL__   
소프트웨어 시리얼 포트를 사용하는 경우 #define __USE_BT_SOFTWARE_SERIAL__   

%% 보다 안정적인 상태의 통신을 원하는 경우 하드웨어 시리얼 포트를 사용하도록 합니다.
*/


//#define __USE_BT_SOFTWARE_SERIAL__   

#ifdef __USE_BT_SOFTWARE_SERIAL__
#include <SoftwareSerial.h>
#endif

//
// 자동자 진행 방향 정의
//
#define CAR_DIR_FW  0   // 전진.
#define CAR_DIR_BK  1   // 후진.
#define CAR_DIR_LF  2   // 좌회전.
#define CAR_DIR_RF  3   // 우회전
#define CAR_DIR_ST  4   // 정지.

// 
// 차량 운행 방향 상태 전역 변수. // 정지 상태.
int g_carDirection = CAR_DIR_ST; // 

int g_carSpeed = 0; // 최대 속도의  60 % for testing.
bool is_back = false;

// 리모트 컨트롤러 관련 전역 변수.
decode_results decodedSignal; //stores results from IR sensor



bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
volatile bool mpuInterrupt = false;


// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
MPU6050 mpu;


int angle_level = 1;

void controllerByIRCommand(String& szIRCmd)
{
  if (szIRCmd == "2") // 전진
  {
    g_carDirection = CAR_DIR_FW;
    Serial.println("$Forward*");
    is_back = false;
  }
  else
  if (szIRCmd == "5")
  {
    g_carDirection = CAR_DIR_ST;
    g_carSpeed = 0;
    Serial.println("$Stop*");
  }
  else
  if (szIRCmd == "8")
  {
    g_carDirection = CAR_DIR_BK;
    Serial.println("$Backward*");
    is_back = true;
  }
}



//
// 주의:  ENA, ENB 는 PWM 지원 포트에 연결한다.
// 다음 업데이트시 변경합니다.
#define ENA   6
#define EN1   7
#define EN2   3

#define EN3   4
#define EN4   2
#define ENB   5


void init_car_controller_board()
{
  pinMode(ENA, OUTPUT);  // ENA
  pinMode(EN1, OUTPUT);  // EN1
  pinMode(EN2, OUTPUT);  // EN2

  pinMode(ENB, OUTPUT);  // ENB
  pinMode(EN3, OUTPUT);  // EN3
  pinMode(EN4, OUTPUT);  // EN4
}

//
// 전후좌우 4개의 함수는 테스트시
// DC 모터 연결에 맞게 고쳐서 정정해야 합니다.
// DC 모터 연결 (+)(-) 연결 변경하거나 코드를 변경합니다.
//
void car_forward()
{
  digitalWrite(EN1, HIGH);
  digitalWrite(EN2, LOW);
  analogWrite(ENA, g_carSpeed);

  digitalWrite(EN3, HIGH);
  digitalWrite(EN4, LOW);
  analogWrite(ENB, g_carSpeed);
}

void car_backward()
{

  digitalWrite(EN1, LOW);
  digitalWrite(EN2, HIGH);
  analogWrite(ENA, g_carSpeed);

  digitalWrite(EN3, LOW);
  digitalWrite(EN4, HIGH);
  analogWrite(ENB, g_carSpeed);

}
//
void car_left()
{
  digitalWrite(EN1, is_back? LOW : HIGH);
  digitalWrite(EN2, is_back?HIGH : LOW);
  analogWrite(ENA, g_carSpeed / angle_level);

  digitalWrite(EN3, is_back?LOW : HIGH);
  digitalWrite(EN4,  is_back?HIGH : LOW);
  analogWrite(ENB, g_carSpeed);
}

//
void car_right()
{
  digitalWrite(EN1, is_back?LOW : HIGH);
  digitalWrite(EN2,  is_back?HIGH : LOW);
  analogWrite(ENA, g_carSpeed);

  digitalWrite(EN3, is_back?LOW : HIGH);
  digitalWrite(EN4,  is_back?HIGH : LOW);
  analogWrite(ENB, g_carSpeed / angle_level);
}

void car_go() {
  is_back = g_carSpeed < 0;
  int left_speed = abs(angle_level > 0 ? g_carSpeed :  g_carSpeed / abs(angle_level));
  int right_speed = abs(angle_level > 0 ? g_carSpeed / angle_level : g_carSpeed);
  digitalWrite(EN1, is_back?LOW : HIGH);
  digitalWrite(EN2,  is_back?HIGH : LOW);
  analogWrite(ENA, left_speed);

  digitalWrite(EN3, is_back?LOW : HIGH);
  digitalWrite(EN4,  is_back?HIGH : LOW);
  analogWrite(ENB, right_speed);
}
//
//
void car_stop()
{
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

//
// 방향 전환값에 의해 차량 운행.
//
void car_update()
{
  // %%
  // 현재는 매번 loop 에서 호출되지만 차후에 커맨드가 있을 경우만 호출될 수 있도록
  // 변경해야 합니다.
  car_go();
}

void print_car_info()
{
  Serial.print("direction value ");
  Serial.println(g_carDirection);
  Serial.print("speed pwm value ");
  Serial.print(g_carSpeed);
  Serial.println("");
}



float Distance(const float x1,const float y1,const float x2, const float y2)
{
  float distance;
  distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));

  return distance;
}
int prevAngle = 0;
String str_buf = "";
int start_buf = 0;
void readString() {
  bool isEnd = false;
  char s;    
  while(Serial.available()) {
      s = (char)Serial.read();
     if(s == '@' || s == '/' || s== '#' || s== '%')
      isEnd = true;
     else
      str_buf += s;
  }
  if(isEnd) {
    if(s == '@') {
      setCarSpeed(str_buf);
    } else if(s == '/') {
      //set Angle
      //4 left
      //2 forward
      //6 right
      //8 back
      setDirection(str_buf);
     } else if( s== '#') {
      setCarSpeed("0");
      Serial.print("$" + String(prevAngle) + "#");
      Serial.flush();
     }else if( s== '%') {
      angle_level = str_buf.toInt();
     }
    str_buf = "";
  }
}
void setDirection(String dir) {
    controllerByIRCommand(dir); 
}
void setCarSpeed(String speed) {
  //Serial.println("$Speed*");
  g_carSpeed =  speed.toInt() / 10;
}
void dmpDataReady() {
    mpuInterrupt = true;
}


 void mpu_setup() {
  
    mpu.initialize();
    
    while (Serial.available()); // empty buffer again

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(30);
    mpu.setYGyroOffset(-66);
    mpu.setZGyroOffset(75);
    mpu.setZAccelOffset(1249);; // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
}

void mpu_print() {
 if (!dmpReady) return;
    //while (!mpuInterrupt && fifoCount < packetSize);
  //Serial.println("?");
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();
    fifoCount = mpu.getFIFOCount();
    
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        //while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        //fifoCount -= packetSize;
        //Serial.println(String(fifoCount) + "*");
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        //Serial.print("quat\t");
        int sign = q.w/abs(q.w);
        int angle = -int(sign * q.z * 180) / 10;
        angle *= 10;
        
        if(prevAngle != angle) {
          if(abs(abs(prevAngle) - abs(angle)) > 20) {
            mpu.resetFIFO();
            return;
          }
          Serial.flush();
          Serial.print("$" + String(angle) + "/");
          Serial.flush();
          prevAngle = angle; 
        }
    }
}
// 부팅 후 1회 실행되는 함수. 초기화 함수. Setup()
void setup()
{
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
  Serial.begin(9600);
  
  //mpu_setup();
  init_car_controller_board();

  //print_car_info();
  
}

// 계속 실행되는 함수. Loop()
void loop()
{
  car_update();
 // mpu_print();
  readString();
//  update_IRreceiverModule();
  
}



