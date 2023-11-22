// batUtils.h

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#ifndef _BATUTILS_h
#define _BATUTILS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

struct DATA
{
	int d1, d2, d3, d4;
	double gx, gy, gz;
};

void sendUDPPacket(WiFiUDP* Udp, IPAddress cl, uint16_t port, char* dados);
void initMPU(uint8_t mpu_addr, int sda_pin, int scl_pin);
void checkMPU(uint8_t mpu_addr);
void readMPU(uint8_t mpu_addr, float* Angle);

#endif

