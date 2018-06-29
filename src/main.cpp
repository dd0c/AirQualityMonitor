// AirQualityMonitor
// Libraries needed:
// EasyNTPClient, Timezone, Time, MHZ19_uart

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <EasyNTPClient.h>
#include <Timezone.h>
#include <MHZ19_uart.h>
/*
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
*/
// MH-Z19 CO2 sensor
const int rx_pin = D2;  //Serial rx pin no
const int tx_pin = D1;  //Serial tx pin no
const int waitingMinutes = 30;  //waiting 30 minutes
MHZ19_uart mhz19;
long cnt = 0;
const long waitingSeconds = waitingMinutes * 60L;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(10);
    Serial.println("\r\n");
/*
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
*/
    // MH-Z19 CO2 sensor setup
    mhz19.begin(rx_pin, tx_pin);
    mhz19.setAutoCalibration(false);
    while ( mhz19.isWarming() ) {
      Serial.print("MH-Z19 now warming up...  status:"); Serial.println(mhz19.getStatus());
      delay(1000);
    }
    Serial.println();
}

void loop() {
    // put your main code here, to run repeatedly:
    /*
    utc = ntpClient.getUnixTime();
    localTime = CE.toLocal(utc);
    Serial.print("Epoch CET time: ");
    Serial.println(localTime);
*/
    // MH-Z19 CO2 sensor loop
    if ( ++cnt % 60 == 0) {
      Serial.print(cnt / 60); Serial.println("min.");
      Serial.print("co2: "); Serial.print(mhz19.getPPM()); Serial.println("ppm now.");
    } else {
      Serial.print(".");
    }
    delay(1000);

    if (cnt > waitingSeconds) {
      Serial.println("");
      mhz19.calibrateZero();
      Serial.println("1st zero calibration now .");

      delay(60000);
      mhz19.calibrateZero();  // Just in case
      Serial.println("2nd zero calibration now .");

      for ( int i = 0; i < 10; i++) {
        Serial.print("co2: "); Serial.print(mhz19.getPPM()); Serial.println("ppm now.");
        delay(10000);
      }
      cnt = 0;
      return;
    }
    delay(5000); // wait for 5 seconds before refreshing.
}
