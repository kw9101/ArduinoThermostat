#include <MsTimer2.h>
#include <IRremote.h>
#include "CircularBuffer.h"
#include "DHT.h"

const int DHT_PIN = 2;     // what digital pin we're connected to
const int IR_PIN = 3;
const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

const float highTemperature = 25.f;
const float lowTemperature = 24.f;
const unsigned long TEMPERATURE_CHECK_DURATION = 10L * 1000L;
const unsigned long TURN_ON_DURATION = 150L * 1000L;
unsigned long turnOnMillis;
unsigned long checkTemperatureMillis;

CircularBuffer < float, TURN_ON_DURATION / TEMPERATURE_CHECK_DURATION > temperatures;

DHT dht(DHT_PIN, DHT22);

IRsend irsend;

void analogLED(int r, int g, int b)
{
  analogWrite(RED_PIN, 255 - constrain(r, 0, 255));
  analogWrite(GREEN_PIN, 255 - constrain(g, 0, 255));
  analogWrite(BLUE_PIN, 255 - constrain(b, 0, 255));
}

// 10초에 한 번씩 온도를 젠다.
// 1분 전에 젠 온도와 비교하여 온도가 올라갔다면 켜진 것, 내려갔다면 꺼진 것이다.
bool IsOnHeater()
{
  Serial.print("<<< IsOnHeater ================================================================: ");
  Serial.println(millis());

  Serial.print(temperatures.old());
  Serial.print(" : ");
  Serial.print(temperatures.recent());
  Serial.println(">>> IsOnHeater ================================================================");
  Serial.println("");
  Serial.println("");

  return temperatures.old() < temperatures.recent();
}

void TurnOnHeater(bool isTurnOn)
{
  Serial.print("<<< TurnOnHeater ================================================================: ");
  Serial.println(millis());

  Serial.print("Heater TurnOn milliseconds: ");
  Serial.println(turnOnMillis);

  Serial.print("Heater State: ");
  Serial.println(IsOnHeater() ? "On" : "Off");

  Serial.print("Heater Turn On: ");
  Serial.println(isTurnOn ? "On" : "Off");

  // 켜져 있는 데 키라고 하거나, 꺼져 있는데 끄라고 하면 무시
  if (IsOnHeater() == isTurnOn)
  {
    Serial.println(">>> TurnOnHeater pass ============================================================");
    Serial.println("");
    Serial.println("");
    return;
  }
  
  const int khz = 38; // 38kHz carrier frequency for the NEC protocol

  // 한일 온풍기 전원 On/Off
  unsigned int irOnOffSignal[] = {4250, 4200, 600, 550, 600, 550, 600, 550, 550, 600, 550, 550, 550, 1600, 500, 1600, 500, 1600, 500, 500, 500, 600, 550, 500, 500, 1600, 500, 1550, 500, 550, 600, 550, 500, 550, 550, 1550, 450, 1600, 500, 1550, 500, 600, 500, 550, 450, 1650, 550, 1550, 550, 1600, 600};
  irsend.sendRaw(irOnOffSignal, sizeof(irOnOffSignal) / sizeof(irOnOffSignal[0]), khz); // Note the approach used to automatically calculate the size of the array.

  turnOnMillis = millis();
  
  Serial.println(">>> TurnOnHeater ================================================================");
  Serial.println("");
  Serial.println("");
}

void CheckTemperature()
{
    // 전원이 변경 된후 1분이 지났는지 확인
  if (millis() < checkTemperatureMillis + TEMPERATURE_CHECK_DURATION)
  {
    return;
  }

  checkTemperatureMillis = millis();
  
  // dht22 에서 온습도를 얻어온다.
  float temperature = dht.readTemperature(false);
  float humidity = dht.readHumidity();
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  Serial.print("<<< CheckTemperature ================================================================: ");
  Serial.println(millis());

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C\t");
  Serial.print("Heat index: ");
  Serial.print(heatIndex);
  Serial.println(" *C");

  if (temperature < lowTemperature) // 추울 때
  {
    analogLED(0, 0, 10);
  }
  else if (highTemperature < temperature)   // 더울 때
  {
    analogLED(10, 0, 0);
  }
  else   // 적정
  {
    analogLED(0, 10, 0);
  }
  temperatures.push(temperature);

  Serial.println(">>> CheckTemperature ================================================================");
  Serial.println("");
  Serial.println("");
}

void ProcessThermostat()
{
  // 전원이 변경 된후 1분이 지났는지 확인
  if (millis() < turnOnMillis + TURN_ON_DURATION)
  {
    return;
  }

  Serial.print("<<< ProcessThermostat ================================================================: ");
  Serial.println(millis());
  float temperature = temperatures.recent();

  // 추울 때
  if (temperature < lowTemperature)
  {
    TurnOnHeater(true);
  }
  // 더울 때
  else if (highTemperature < temperature)
  {
    TurnOnHeater(false);
  }

  Serial.println(">>> ProcessThermostat ================================================================");
  Serial.println("");
  Serial.println("");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);   // Status message will be sent to PC at 9600 baud
  Serial.println("Start");

  dht.begin();

  turnOnMillis = millis();
  checkTemperatureMillis = millis() - TEMPERATURE_CHECK_DURATION;
  CheckTemperature();
}

void loop() {
  CheckTemperature();
  ProcessThermostat();
}
