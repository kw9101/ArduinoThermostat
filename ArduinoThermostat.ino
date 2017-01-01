#include <IRremote.h>
#include "DHT.h"

const bool DEBUG_FLAG = true;
const int DHT_PIN = 2;     // what digital pin we're connected to
const int IR_PIN = 3;
const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 6;
const int LED_BRIGHT = 1;
const float HIGH_TEMPERATURE = 25.0f;
const float LOW_TEMPERATURE = 24.7f;
const unsigned long TEMPERATURE_CHECK_DURATION = 3L * 1000L;
bool isOnHeater = false;

DHT dht(DHT_PIN, DHT22);

IRsend irsend;

void analogLED(int r, int g, int b)
{
  analogWrite(RED_PIN, 255 - constrain(r, 0, 255));
  analogWrite(GREEN_PIN, 255 - constrain(g, 0, 255));
  analogWrite(BLUE_PIN, 255 - constrain(b, 0, 255));
}

void TurnOnHeater(bool isTurnOn)
{
  // 켜져 있는 데 키라고 하거나, 꺼져 있는데 끄라고 하면 무시
  if (isOnHeater == isTurnOn)
  {
    return;
  }

  if (DEBUG_FLAG == false)
  {
    Serial.print("Heater Turn ");
    Serial.println(isTurnOn ? "On" : "Off");
    Serial.println("");
  }

  const int khz = 38; // 38kHz carrier frequency for the NEC protocol

  // 한일 온풍기 전원 On/Off
  unsigned int irOnOffSignal[] = {4250, 4200, 600, 550, 600, 550, 600, 550, 550, 600, 550, 550, 550, 1600, 500, 1600, 500, 1600, 500, 500, 500, 600, 550, 500, 500, 1600, 500, 1550, 500, 550, 600, 550, 500, 550, 550, 1550, 450, 1600, 500, 1550, 500, 600, 500, 550, 450, 1650, 550, 1550, 550, 1600, 600};
  irsend.sendRaw(irOnOffSignal, sizeof(irOnOffSignal) / sizeof(irOnOffSignal[0]), khz); // Note the approach used to automatically calculate the size of the array.

  isOnHeater = !isOnHeater;
  digitalWrite(LED_BUILTIN, isOnHeater ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level)

  analogLED(LED_BRIGHT, LED_BRIGHT, 0);
}

void CheckTemperature()
{
  // dht22 에서 온습도를 얻어온다.
  float temperature = dht.readTemperature(false);
  float humidity = dht.readHumidity();
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  if (DEBUG_FLAG == false)
  {
    Serial.print("CheckTemperature: ");

    Serial.print(millis());
    Serial.print(" \tHumidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" *C\t");
    Serial.print("Heat index: ");
    Serial.print(heatIndex);
    Serial.println(" *C");
    Serial.println("");
    Serial.flush();
  }

  if (isnan(temperature)) // 온습도기 에러 났을 때
  {
    analogLED(LED_BRIGHT, 0, LED_BRIGHT);
  }
  else if (temperature < LOW_TEMPERATURE) // 추울 때
  {
    analogLED(0, 0, LED_BRIGHT);
    TurnOnHeater(true);
  }
  else if (HIGH_TEMPERATURE < temperature)   // 더울 때
  {
    analogLED(LED_BRIGHT, 0, 0);
    TurnOnHeater(false);
  }
  else   // 적정
  {
    analogLED(0, LED_BRIGHT, 0);
  }
}

void setup() {
  // LED 핀 셋팅
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  analogLED(0, 0, 0);
  // put your setup code here, to run once:
  if (DEBUG_FLAG == false)
  {
    Serial.begin(9600);   // Status message will be sent to PC at 9600 baud
    Serial.println("< Start >");
  }

  delay(500);
  dht.begin();
  delay(500);
}

void loop() {
  CheckTemperature();
  delay(TEMPERATURE_CHECK_DURATION);
}
