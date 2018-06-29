// AirQualityMonitor
// Libraries needed:
// EasyNTPClient, Timezone, Time, MHZ19_uart

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <EasyNTPClient.h>
#include <dht.h>
#include <Timezone.h>
#include<string.h>

// SSID of your network
char ssid[] = ""; //SSID of your Wi-Fi router
char pass[] = ""; //Password of your Wi-Fi router

// NTP client and Timezone library
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "192.168.1.105");
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
unsigned long utc, localTime;

// DHT22 humidity and temperature sensor
dht DHT;
#define DHT22_PIN D4

struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0};

// DSM501A smog sensor
byte buff[2];
int pin = D8;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
int i=0;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(10);
    Serial.println("\r\n");

    // Connect to Wi-Fi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to...");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Wi-Fi connected successfully");
    Serial.println();

    // Start DHT22 sensor output
    Serial.print("LIBRARY VERSION: ");
    Serial.println(DHT_LIB_VERSION);
    Serial.println();
    Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");

    // Start DSM501A sensor
    pinMode(8,INPUT);
    starttime = millis();
}

void loop() {
    // put your main code here, to run repeatedly:
    utc = ntpClient.getUnixTime();
    localTime = CE.toLocal(utc);
    Serial.print("Epoch CET time: ");
    Serial.println(localTime);

    // READ DATA DHT22 sensor
    Serial.print("DHT22 sensor:\t");

    uint32_t start = micros();
    int chk = DHT.read22(DHT22_PIN);
    uint32_t stop = micros();

    stat.total++;
    switch (chk) {
      case DHTLIB_OK:
        stat.ok++;
        Serial.println("OK\t");
      break;
      case DHTLIB_ERROR_CHECKSUM:
        stat.crc_error++;
        Serial.println("Checksum error\t");
      break;
      case DHTLIB_ERROR_TIMEOUT:
        stat.time_out++;
        Serial.println("Time out error\t");
      break;
      default:
        stat.unknown++;
        Serial.println("Unknown error\t");
      break;
    }

    // DISPLAY DATA DHT22
    Serial.print("Humidity:\t");
    Serial.print(DHT.humidity, 1);
    Serial.println();
    Serial.print("Temperature:\t");
    Serial.print(DHT.temperature, 1);
    Serial.println();

    if (stat.total % 20 == 0) {
      Serial.println("\nTOT\tOK\tCRC\tTO\tUNK");
      Serial.print(stat.total);
      Serial.print("\t");
      Serial.print(stat.ok);
      Serial.print("\t");
      Serial.print(stat.crc_error);
      Serial.print("\t");
      Serial.print(stat.time_out);
      Serial.print("\t");
      Serial.print(stat.connect);
      Serial.print("\t");
      Serial.print(stat.ack_l);
      Serial.print("\t");
      Serial.print(stat.ack_h);
      Serial.print("\t");
      Serial.print(stat.unknown);
      Serial.println("\n");
    }

    // Display data DSM501A
    duration = pulseIn(pin, LOW);
    lowpulseoccupancy += duration;
    endtime = millis();
    if ((endtime-starttime) > sampletime_ms) {
      ratio = (lowpulseoccupancy-endtime+starttime + sampletime_ms)/(sampletime_ms*10.0);  // Integer percentage 0=>100
      concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
      Serial.print("lowpulseoccupancy:");
      Serial.print(lowpulseoccupancy);
      Serial.print("    ratio:");
      Serial.print(ratio);
      Serial.print("    DSM501A:");
      Serial.println(concentration);
      lowpulseoccupancy = 0;
      starttime = millis();
    }

    delay(5000); // wait for 5 seconds before refreshing.
}
