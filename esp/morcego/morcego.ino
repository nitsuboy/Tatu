/*
    Name:       morcego.ino
    Created:	28/10/2023 23:35:07
    Author:     Nitsuboy
*/

#include "batUtils.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

// Soft ap and udp socket
#define APSSID "Morcego"
#define APPASS "tatudobolado"

char packetBuffer[255];
char dataBuffer[sizeof(DATA)];

bool connOn = false;

WiFiUDP Udp;
uint16_t port = 2000;
DATA mydata;
IPAddress cl;

// Sensors and stuff

const uint8_t mpu_addr = 0x68;
const int sda_pin = D2;
const int scl_pin = D1;
const int pingPin = D6;
const int echoPin = D7;

float Angle[3];

void setup()
{
    Serial.begin(115200);

    // MPU init and check
    initMPU(mpu_addr, sda_pin, scl_pin);
    checkMPU(mpu_addr);

    // AP setup
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APSSID, APPASS);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    // Default ip http://192.168.4.1
    Serial.println(myIP);
    Udp.begin(port);
    Serial.println("Socket Udp ready");
}

void loop()
{
    readMPU(mpu_addr,Angle);
    readUS(pingPin, echoPin);
    mydata = { readUS(pingPin, echoPin),1,1,1, Angle[0], Angle[1], Angle[2] };
    int packetSize = Udp.parsePacket();

    if (connOn) {

        if (packetSize) {
            Serial.printf("Recevied packet from %s:%d\n",
                Udp.remoteIP().toString().c_str(), Udp.remotePort());
            int n = Udp.read(packetBuffer, 255); 
            packetBuffer[n] = 0;
            Serial.println("Conteudo : ");
            Serial.println(packetBuffer);
        }

        memcpy(dataBuffer, &mydata, sizeof(DATA));
        sendUDPPacket(&Udp, Udp.remoteIP(), port, dataBuffer);
        if (WiFi.softAPgetStationNum() < 1) {
            Serial.println("client disconected");
            connOn = false;
        }
    }
    if (packetSize && connOn == false) {

        Serial.printf("Recevied packet from %s:%d\n",
            Udp.remoteIP().toString().c_str(), Udp.remotePort());
        int n = Udp.read(packetBuffer, 255);
        packetBuffer[n] = 0;
        Serial.println("Conteudo : ");
        Serial.println(packetBuffer);

        cl = Udp.remoteIP();
        connOn = true;
    }
}
