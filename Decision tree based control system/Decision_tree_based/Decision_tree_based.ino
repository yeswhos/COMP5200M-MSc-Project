#include <ArduinoJson.h>
#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4LCD lcd;
Zumo32U4ButtonA button;
Zumo32U4LineSensors sensors;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ProximitySensors proxSensors;

#define RA_SIZE 3  // number of readings to include in running average of accelerometer readings
#define XY_ACCELERATION_THRESHOLD 2000  //2400 for detection of contact (~16000 = magnitude of acceleration due to gravity)
#define NUM_SENSORS 5

#define QTR_THRESHOLD  1000 // microseconds
#define RIGHT 1
#define LEFT -1
#define FULL_SPEED_DURATION_LIMIT     250  // ms
enum ForwardSpeed { SearchSpeed, SustainedSpeed, FullSpeed };
ForwardSpeed _forwardSpeed;  // current forward speed setting

unsigned int sensor_values[NUM_SENSORS];
unsigned long full_speed_start_time;

unsigned long loop_start_time;
unsigned long last_turn_time;
unsigned long contact_made_time;
uint16_t lineSensorValues[5];
uint8_t lineSensor1;
uint8_t lineSensor2;
uint8_t lineSensor3;

boolean warnRight = false;
boolean warnLeft = false;
boolean warnCenter = false;
const uint8_t sensorThreshold = 1;

template <typename T>
class RunningAverage
{
  public:
    RunningAverage(void);
    RunningAverage(int);
    ~RunningAverage();
    void clear();
    void addValue(T);
    T getAverage() const;
    void fillValue(T, int);
  protected:
    int _size;
    int _cnt;
    int _idx;
    T _sum;
    T * _ar;
    static T zero;
};

// Accelerometer Class -- extends the LSM303 class to support reading and averaging the x-y acceleration
//   vectors from the onboard LSM303DLHC accelerometer/magnetometer
class Accelerometer : public LSM303
{
  typedef struct acc_data_xy
  {
    unsigned long timestamp;
    int x;
    int y;
    float dir;
  } acc_data_xy;

  public:
    Accelerometer() : ra_x(RA_SIZE), ra_y(RA_SIZE) {};
    ~Accelerometer() {};
    void enable(void);
    void getLogHeader(void);
    void readAcceleration(unsigned long timestamp);
    float len_xy() const;
    float dir_xy() const;
    int x_avg(void) const;
    int y_avg(void) const;
    long ss_xy_avg(void) const;
    float dir_xy_avg(void) const;
  private:
    acc_data_xy last;
    RunningAverage<int> ra_x;
    RunningAverage<int> ra_y;
};

Accelerometer lsm303;
boolean in_contact;  // set when accelerometer detects contact with opposing robot

// forward declaration
void setForwardSpeed(ForwardSpeed speed);
void turn(char direction, bool randomize);
void on_contact_lost();
bool check_for_contact();
void on_contact_made();
int getForwardSpeed();
void setup()
{
  sensors.initFiveSensors();
  proxSensors.initFrontSensor();
  randomSeed((unsigned int) millis());
  lineSensors.initFiveSensors();
  // Initialize the Wire library and join the I2C bus as a master
  Wire.begin();

  // Initialize LSM303
  lsm303.init();
  lsm303.enable();

#ifdef LOG_SERIAL
  lsm303.getLogHeader();
#endif

  ledYellow(1);
  waitForButtonAndCountDown(false);
}

void waitForButtonAndCountDown(bool restarting)
{
#ifdef LOG_SERIAL
  Serial.print(restarting ? "Restarting Countdown" : "Starting Countdown");
  Serial.println();
#endif

  ledRed(0);

  ledYellow(1);
  lcd.clear();
  lcd.print(F("Press A"));

  button.waitForButton();

  ledYellow(0);
  lcd.clear();

  // reset loop variables
  in_contact = false;  // 1 if contact made; 0 if no contact or contact lost
  contact_made_time = 0;
  last_turn_time = millis();  // prevents false contact detection on initial acceleration
  _forwardSpeed = SearchSpeed;
  full_speed_start_time = 0;
}
///////////////////////////////////
void turnRight()
{
  motors.setSpeeds(200, -200);
  in_contact = false;
//  turningLeft = false;
//  turningRight = true;
}

void turnLeft()
{
  motors.setSpeeds(-200, 200);
  in_contact = false;
//  turningLeft = true;
//  turningRight = false;
}

void Search(){
  motors.setSpeeds(200, -200);
}
void goCharge(){
    motors.setSpeeds(400, 400);
    delay(1000);
}
//////////////////////////////////
void loop()
{
  if (button.isPressed())
  {
    motors.setSpeeds(0, 0);
    button.waitForRelease();
    waitForButtonAndCountDown(true);
  }
  
  Serial.begin(9600);
  while (!Serial) continue;
  const size_t capacity = 0 + 390;
  DynamicJsonDocument doc(capacity);  
  char json[] = "{\"Contact\": [{\"0\": [{\"0\": [{\"0\": \"No object seeing\", \"1\": \"No object seeing\", \"2\": \"Object seeing\", \"3\": \"Object seeing\"}]}, {\"1\": [{\"0\": \"No object seeing\", \"1\": \"No object seeing\", \"2\": \"Object seeing\", \"3\": \"Object seeing\"}]}, {\"2\": \"Object seeing\"}, {\"3\": \"Object seeing\"} ]}, {\"1\": \"Charge\"} ]}";
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  //const char *value = doc["Contact"][0]["0"][reading_a][(String) reading_a][reading_b][(String) reading_b];
  uint8_t leftValue = proxSensors.countsFrontWithLeftLeds();
  uint8_t rightValue = proxSensors.countsFrontWithRightLeds();

  lineSensors.read(lineSensorValues);
  lineSensor1 = lineSensorValues[0];
  lineSensor2 = lineSensorValues[2];
  lineSensor3 = lineSensorValues[4];
  //Serial.println(lineSensor1);
  warnRight = (lineSensor1 <= sensorThreshold) && (lineSensor3 >= sensorThreshold);
  warnLeft = (lineSensor1 >= sensorThreshold) && (lineSensor3 <= sensorThreshold);
  warnCenter = (lineSensor1 <= sensorThreshold) && (lineSensor3 <= sensorThreshold);
  if(warnRight){
    //Serial.println("右侧");
    turn(RIGHT, true);
  }
  else if(warnLeft){
    //Serial.println("左侧");
    turn(LEFT, true);
  }
  
  proxSensors.read();
  uint8_t reading_a = map(proxSensors.countsFrontWithLeftLeds(), 0, 6, 0, 3);
  uint8_t reading_b = map(proxSensors.countsFrontWithRightLeds(), 0, 6, 0, 3);
  
  bool objectSeen = leftValue >= sensorThreshold || rightValue >= sensorThreshold;
  loop_start_time = millis();
  lsm303.readAcceleration(loop_start_time);
  sensors.read(sensor_values);
  
  const char *value = doc["Contact"][0]["0"][reading_a][(String) reading_a][reading_b][(String) reading_b];

  if(*value == 'Charge'){
    goCharge();  
  }
  
  if(*value == 'No object seeing'){
    //Serial.println("No object");
    Search();  
  }
  if((*value == 'Object seeing')||((sensor_values[0] < QTR_THRESHOLD))){
    Serial.println("Object");
    turn(RIGHT, true);
  }else if (sensor_values[NUM_SENSORS - 1] < QTR_THRESHOLD){
    turn(LEFT, true);
  }else{
    if (check_for_contact()) on_contact_made();
    if (objectSeen){

      if (leftValue < rightValue){
        //turn(RIGHT, true);
        turnRight();
  
      }else if (leftValue > rightValue){
        turnLeft();
      }else{
        if(objectSeen){
          int speed = getForwardSpeed();
          motors.setSpeeds(speed, speed);
          //goStraight();
        }
      }
    }
    
  }
  delay(100);
}


void turn(char direction, bool randomize)
{
#ifdef LOG_SERIAL
  Serial.print("turning ...");
  Serial.println();
#endif
  on_contact_lost();

  static unsigned int duration_increment = 300 / 4;

  // motors.setSpeeds(0,0);
  // delay(100);
  motors.setSpeeds(-200, -200);
  delay(200);
  motors.setSpeeds(200 * direction, -200 * direction);
  delay(randomize ? 300 + (random(8) - 2) * duration_increment : 300);
  int speed = getForwardSpeed();
  motors.setSpeeds(speed, speed);
  last_turn_time = millis();
}

void setForwardSpeed(ForwardSpeed speed)
{
  _forwardSpeed = speed;
  if (speed == FullSpeed) full_speed_start_time = loop_start_time;
}

int getForwardSpeed()
{
  int speed;
  switch (_forwardSpeed)
  {
    case FullSpeed:
      speed = 400;
      break;
    case SustainedSpeed:
      speed = 400;
      break;
    default:
      speed = 200;
      break;
  }
  return speed;
}

// check for contact, but ignore readings immediately after turning or losing contact
bool check_for_contact()
{
  static long threshold_squared = (long) XY_ACCELERATION_THRESHOLD * (long) XY_ACCELERATION_THRESHOLD;
  return (lsm303.ss_xy_avg() >  threshold_squared) && \
    (loop_start_time - last_turn_time > 400) && \
    (loop_start_time - contact_made_time > 1000);
}

// sound horn and accelerate on contact -- fight or flight
void on_contact_made()
{
#ifdef LOG_SERIAL
  Serial.print("contact made");
  Serial.println();
#endif
  in_contact = true;
  contact_made_time = loop_start_time;
  setForwardSpeed(FullSpeed);
//  buzzer.playFromProgramSpace(sound_effect);
  ledRed(1);
}

// reset forward speed
void on_contact_lost()
{
#ifdef LOG_SERIAL
  Serial.print("contact lost");
  Serial.println();
#endif
  in_contact = false;
  setForwardSpeed(SearchSpeed);
  ledRed(0);
}

// class Accelerometer -- member function definitions

// enable accelerometer only
// to enable both accelerometer and magnetometer, call enableDefault() instead
void Accelerometer::enable(void)
{
  // Enable Accelerometer
  // 0x27 = 0b00100111
  // Normal power mode, all axes enabled
  writeAccReg(LSM303::CTRL_REG1_A, 0x27);

  if (getDeviceType() == LSM303::device_DLHC)
  writeAccReg(LSM303::CTRL_REG4_A, 0x08); // DLHC: enable high resolution mode
}

void Accelerometer::getLogHeader(void)
{
  Serial.print("millis    x      y     len     dir  | len_avg  dir_avg  |  avg_len");
  Serial.println();
}

void Accelerometer::readAcceleration(unsigned long timestamp)
{
  readAcc();
  if (a.x == last.x && a.y == last.y) return;

  last.timestamp = timestamp;
  last.x = a.x;
  last.y = a.y;

  ra_x.addValue(last.x);
  ra_y.addValue(last.y);

#ifdef LOG_SERIAL
 Serial.print(last.timestamp);
 Serial.print("  ");
 Serial.print(last.x);
 Serial.print("  ");
 Serial.print(last.y);
 Serial.print("  ");
 Serial.print(len_xy());
 Serial.print("  ");
 Serial.print(dir_xy());
 Serial.print("  |  ");
 Serial.print(sqrt(static_cast<float>(ss_xy_avg())));
 Serial.print("  ");
 Serial.print(dir_xy_avg());
 Serial.println();
#endif
}

float Accelerometer::len_xy() const
{
  return sqrt(last.x*a.x + last.y*a.y);
}

float Accelerometer::dir_xy() const
{
  return atan2(last.x, last.y) * 180.0 / M_PI;
}

int Accelerometer::x_avg(void) const
{
  return ra_x.getAverage();
}

int Accelerometer::y_avg(void) const
{
  return ra_y.getAverage();
}

long Accelerometer::ss_xy_avg(void) const
{
  long x_avg_long = static_cast<long>(x_avg());
  long y_avg_long = static_cast<long>(y_avg());
  return x_avg_long*x_avg_long + y_avg_long*y_avg_long;
}

float Accelerometer::dir_xy_avg(void) const
{
  return atan2(static_cast<float>(x_avg()), static_cast<float>(y_avg())) * 180.0 / M_PI;
}

// RunningAverage class
// based on RunningAverage library for Arduino
// source:  http://playground.arduino.cc/Main/RunningAverage
// author:  Rob.Tillart@gmail.com
// Released to the public domain

template <typename T>
T RunningAverage<T>::zero = static_cast<T>(0);

template <typename T>
RunningAverage<T>::RunningAverage(int n)
{
  _size = n;
  _ar = (T*) malloc(_size * sizeof(T));
  clear();
}

template <typename T>
RunningAverage<T>::~RunningAverage()
{
  free(_ar);
}

// resets all counters
template <typename T>
void RunningAverage<T>::clear()
{
  _cnt = 0;
  _idx = 0;
  _sum = zero;
  for (int i = 0; i< _size; i++) _ar[i] = zero;  // needed to keep addValue simple
}

// adds a new value to the data-set
template <typename T>
void RunningAverage<T>::addValue(T f)
{
  _sum -= _ar[_idx];
  _ar[_idx] = f;
  _sum += _ar[_idx];
  _idx++;
  if (_idx == _size) _idx = 0;  // faster than %
  if (_cnt < _size) _cnt++;
}

// returns the average of the data-set added so far
template <typename T>
T RunningAverage<T>::getAverage() const
{
  if (_cnt == 0) return zero; // NaN ?  math.h
  return _sum / _cnt;
}

// fill the average with a value
// the param number determines how often value is added (weight)
// number should preferably be between 1 and size
template <typename T>
void RunningAverage<T>::fillValue(T value, int number)
{
  clear();
  for (int i = 0; i < number; i++)
  {
    addValue(value);
  }
}
