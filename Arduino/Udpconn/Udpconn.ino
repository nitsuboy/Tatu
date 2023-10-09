#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef APSSID
#define APSSID "Tatu"
#define APPSK "tatudobolado"
#endif

// Set these to your desired credentials.
const char *ssid = APSSID;
const char *password = APPSK;

// UDP 

unsigned int localport = 2000;

char packetBuffer[255];
char replyBuffer[255];

WiFiUDP Udp;

// US sensor

const int trigPin = D5; 
const int echoPin = D6; 

long duration;
int distance;

void setup() {
  delay(1000);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  // http://192.168.4.1
  Serial.println(myIP);
  Serial.println("HTTP server started");
  Udp.begin(localport);
}

int US(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  return duration*0.034/2;
}

void loop(){
  int packetSize = Udp.parsePacket();
  if(packetSize){
    Serial.printf("Recevied packet of size %d from %s:%d\n    to %s:%d, free heap = %d B\n",
                  packetSize,
                  Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                  Udp.destinationIP().toString().c_str(), Udp.localPort(),
                  ESP.getFreeHeap());
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Conteudo : ");
    Serial.println(packetBuffer);

    sprintf(replyBuffer, "D1 = %d cm", US());

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyBuffer);
    Udp.endPacket();
  }
}