#include <DallasTemperature.h> // Sondes ds18b20
#include <OneWire.h>           // Gestion du bus 1-wire
#include <ESP8266WiFi.h>

#include "AdafruitIO_WiFi.h"

#include "TOKEN.h"
// W_SSID
// W_PWD
// IO_USERNAME
// IO_KEY
// TEMP_FEED
// RSSI_FEED

#define ONE_WIRE_BUS 13 // Où est connecté le bus
#define SLEEP_TIME 60   // in second

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
DeviceAddress *dev_addr;
uint dev_count=0;
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, W_SSID, W_PWD);
AdafruitIO_Feed *temperatureFeed = io.feed(TEMP_FEED);
AdafruitIO_Feed *rssiFeed = io.feed(RSSI_FEED);

void setup()
{
    pinMode(4, WAKEUP_PULLUP);
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    Serial.begin(74880);
    delay(10);

    io.connect();
    Serial.println();
    Serial.print("1-wire bus used :");
    Serial.println(ONE_WIRE_BUS);

    DS18B20.begin();
    Serial.print("sensor count : ");
    dev_count = DS18B20.getDeviceCount();
    Serial.println(dev_count);
    if (dev_count == 0)
    {
        Serial.println("No devices found. Going to sleep");
        ESP.deepSleep(0);
    }
    dev_addr = (DeviceAddress *)malloc(dev_count * sizeof(DeviceAddress));
    for (uint i = 0; i < dev_count; i++)
    {
        DS18B20.getAddress(dev_addr[i], i);
        DS18B20.setResolution(dev_addr[i], 12);
    }

    // AIO section
    Serial.print("Waiting for AIO connection");
    while (io.status() < AIO_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
}

void loop()
{
    digitalWrite(2, LOW);
    DS18B20.requestTemperatures();
    io.run();

    long rssi = WiFi.RSSI();
    Serial.print(rssi);
    Serial.print(" ");

    rssiFeed->save(rssi);
    for (uint i = 0; i < dev_count; i++)
    {
        float temp = DS18B20.getTempC(dev_addr[i]);
        if (i == 0)
        {
            temperatureFeed->save(temp);
        }
        Serial.print(temp);
        Serial.print(" ");
    }
    Serial.println();

    digitalWrite(2, HIGH);
    delay(SLEEP_TIME*1000);
}
