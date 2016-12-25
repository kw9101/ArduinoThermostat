#include <IRremote.h>
#include "DHT.h"

const int DHT_PIN = 2;     // what digital pin we're connected to
const int IR_PIN = 3;
const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 6;
const int YELLOW_PIN = 8;
const int LED_BRIGHT = 3;
const float HIGH_TEMPERATURE = 24.5f;
const float LOW_TEMPERATURE = 23.5f;
const unsigned long TEMPERATURE_CHECK_DURATION = 5L * 1000L;
unsigned long checkTemperatureMillis;
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
  Serial.print("<<< TurnOnHeater ================================================================: ");
  Serial.println(millis());

  Serial.print("Heater TurnOn milliseconds: ");
  Serial.println(millis());

  Serial.print("Heater State: ");
  Serial.println(isOnHeater ? "On" : "Off");

  Serial.print("Heater Turn On: ");
  Serial.println(isTurnOn ? "On" : "Off");

  // 켜져 있는 데 키라고 하거나, 꺼져 있는데 끄라고 하면 무시
  if (isOnHeater == isTurnOn)
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

  isOnHeater = !isOnHeater;
  digitalWrite(LED_BUILTIN, isOnHeater ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level)

  Serial.println(">>> TurnOnHeater ================================================================");
  Serial.println("");
  Serial.println("");

  digitalWrite(YELLOW_PIN, HIGH);
  delay(1000);
  digitalWrite(YELLOW_PIN, LOW);
}

void CheckTemperature()
{
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

  if (temperature < LOW_TEMPERATURE) // 추울 때
  {
    TurnOnHeater(true);
    analogLED(0, 0, LED_BRIGHT);
  }
  else if (HIGH_TEMPERATURE < temperature)   // 더울 때
  {
    TurnOnHeater(false);
    analogLED(LED_BRIGHT, 0, 0);
  }
  else   // 적정
  {
    analogLED(0, LED_BRIGHT, 0);
  }

  Serial.println(">>> CheckTemperature ================================================================");
  Serial.println("");
  Serial.println("");
}

void setup() {
  // LED 핀 셋팅
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // put your setup code here, to run once:
  // Serial.begin(9600);   // Status message will be sent to PC at 9600 baud
  Serial.println("Start");
  dht.begin();

  checkTemperatureMillis = millis() - TEMPERATURE_CHECK_DURATION;
  CheckTemperature();
}

void loop() {
  CheckTemperature();
}
