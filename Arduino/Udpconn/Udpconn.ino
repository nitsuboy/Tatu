#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#ifndef APSSID
#define APSSID "Morcego"
#define APPSK "tatudobolado"
#endif

// Set these to your desired credentials.
const char *ssid = APSSID;
const char *password = APPSK;

// UDP 

unsigned int localport = 2000;

char packetBuffer[255];
char replyBuffer[512];

WiFiUDP Udp;
IPAddress cl;

bool connOn = false;

// US sensor

const int trigPin = D6; 
const int echoPin = D5; 

long duration;
int distance;

// gyro / acc sensor

//Ratios de conversion
#define A_R 16384.0 // 32768/2
#define G_R 131.0 // 32768/250
 
//Conversion de radianes a grados 180/PI
#define RAD_A_DEG = 57.295779

const int MPU_ADDR =      0x68; // definição do endereço do sensor MPU6050 (0x68)
const int WHO_AM_I =      0x75; // registro de identificação do dispositivo
const int PWR_MGMT_1 =    0x6B; // registro de configuração do gerenciamento de energia
const int GYRO_CONFIG =   0x1B; // registro de configuração do giroscópio
const int ACCEL_CONFIG =  0x1C; // registro de configuração do acelerômetro
const int ACCEL_XOUT =    0x3B; // registro de leitura do eixo X do acelerômetro
 
const int sda_pin = D2; // definição do pino I2C SDA
const int scl_pin = D1; // definição do pino I2C SCL

int16_t AcX, AcY, AcZ, GyX, GyY, GyZ; 

//Angulos
float Acc[2];
float Gy[3];
float Angle[3];

long tiempo_prev;
float dt;

void writeRegMPU(int reg, int val){     //aceita um registro e um valor como parâmetro  
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                      // escreve o valor no registro
  Wire.endTransmission(true);           // termina a transmissão
}
 
uint8_t readRegMPU(uint8_t reg){        // aceita um registro como parâmetro
  uint8_t data;
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.endTransmission(false);          // termina transmissão mas continua com I2C aberto (envia STOP e START)
  Wire.requestFrom(MPU_ADDR, 1);        // configura para receber 1 byte do registro escolhido acima
  data = Wire.read();                   // lê o byte e guarda em 'data'
  return data;                          //retorna 'data'
}
 
void findMPU(int mpu_addr){
  Wire.beginTransmission(MPU_ADDR);
  int data = Wire.endTransmission(true);
 
  if(data == 0)
  {
    Serial.print("Dispositivo encontrado no endereço: 0x");
    Serial.println(MPU_ADDR, HEX);
  }
  else
  {
    Serial.println("Dispositivo não encontrado!");
  }
}
 
void checkMPU(int mpu_addr){
  findMPU(MPU_ADDR);
     
  int data = readRegMPU(WHO_AM_I); // Register 117 – Who Am I - 0x75
   
  if(data == 104) 
  {
    Serial.println("MPU6050 Dispositivo respondeu OK! (104)");
 
    data = readRegMPU(PWR_MGMT_1); // Register 107 – Power Management 1-0x6B
 
    if(data == 64) Serial.println("MPU6050 em modo SLEEP! (64)");
    else Serial.println("MPU6050 em modo ACTIVE!"); 
  }
  else Serial.println("Verifique dispositivo - MPU6050 NÃO disponível!");
}
 
void initMPU(){
  Wire.begin(sda_pin, scl_pin);
  writeRegMPU(PWR_MGMT_1, 0);
  writeRegMPU(GYRO_CONFIG, 0);
  writeRegMPU(ACCEL_CONFIG, 0);
}

void readMPU(){  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT); //Pedir el registro 0x3B - corresponde al AcX
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR,14,true);   //A partir del 0x3B, se piden 6 registros
  AcX=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  Wire.read();Wire.read();
  GyX=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
  GyY=Wire.read()<<8|Wire.read();
  GyZ=Wire.read()<<8|Wire.read();
 
  Acc[1] = atan(-1*(AcX/A_R)/sqrt(pow((AcY/A_R),2) + pow((AcZ/A_R),2)))*RAD_TO_DEG;
  Acc[0] = atan((AcY/A_R)/sqrt(pow((AcX/A_R),2) + pow((AcZ/A_R),2)))*RAD_TO_DEG;
 
   //Calculo del angulo del Giroscopio
  Gy[0] = GyX/G_R;
  Gy[1] = GyY/G_R;
  Gy[2] = GyZ/G_R;
 
  delay(50);                                        
}

void setup() {
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.print("Iniciando WAP...");
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  // http://192.168.4.1
  Serial.println(myIP);
  Udp.begin(localport);
  Serial.println("Socket Udp pronto");


  initMPU();
  checkMPU(MPU_ADDR);
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
  if(connOn){
    readMPU();
    sprintf(replyBuffer, "D1 = %d cm,GyX = %.1f,GyY = %.1f,GyZ = %.1f", 0,AcX/A_R,AcY/A_R,AcZ/A_R);

    Udp.beginPacket(Udp.remoteIP(), localport);
    Udp.write(replyBuffer);
    Udp.endPacket();
  }
  if(packetSize){
    Serial.printf("Recevied packet from %s:%d\n",
                  Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Conteudo : ");
    Serial.println(packetBuffer);

    readMPU();
    sprintf(replyBuffer, "D1 = %d cm,GyX = %.1f,GyY = %.1f,GyZ = %.1f", 0,AcX/A_R,AcY/A_R,AcZ/A_R);

    Udp.beginPacket(Udp.remoteIP(), localport);
    Udp.write(replyBuffer);
    Udp.endPacket();
    cl = Udp.remoteIP();
    connOn = true;
  }
  
}