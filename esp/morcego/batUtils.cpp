// 
// 
// 

#include "batUtils.h"
#define A_R 2048.0
#define G_R 16.4

long tiempo_prev;
float dt;

int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
float A[2], G[3], An[3];

void sendUDPPacket(WiFiUDP* Udp, IPAddress cl, uint16_t port, char* dados) {
	Udp->beginPacket(cl, port);
	Udp->write(dados, (size_t) 40);
	Udp->endPacket();
}

void writeRegMPU(uint8_t mpu_addr, uint8_t reg, int val) {
	Wire.beginTransmission(mpu_addr);
	Wire.write(reg);                     
	Wire.write(val);                      
	Wire.endTransmission(true);           
}

uint8_t readRegMPU(uint8_t mpu_addr,uint8_t reg) {        // aceita um registro como parâmetro
	uint8_t data;
	Wire.beginTransmission(mpu_addr);     // inicia comunicação com endereço do MPU6050
	Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
	Wire.endTransmission(false);          // termina transmissão mas continua com I2C aberto (envia STOP e START)
	Wire.requestFrom(mpu_addr, (uint8_t) 1);        // configura para receber 1 byte do registro escolhido acima
	data = Wire.read();                   // lê o byte e guarda em 'data'
	return data;                          //retorna 'data'
}

void findMPU(uint8_t mpu_addr) {
	Wire.beginTransmission(mpu_addr);
	int data = Wire.endTransmission(true);

	if (data == 0)
	{
		Serial.print("Dispositivo encontrado no endereço: 0x");
		Serial.println(mpu_addr, HEX);
	}
	else
	{
		Serial.println("Dispositivo não encontrado!");
	}
}

void initMPU(uint8_t mpu_addr, int sda_pin, int scl_pin) {
	Wire.begin(sda_pin, scl_pin);
	writeRegMPU(mpu_addr, 0x6B, 0); // Register 107 – Power Management 1 - 0x6B
	writeRegMPU(mpu_addr, 0x1B, 24); // registro de configuração do giroscópio - 0x1B
	writeRegMPU(mpu_addr, 0x1C, 24); // registro de configuração do acelerômetro - 0x1C
}

void checkMPU(uint8_t mpu_addr) {
	findMPU(mpu_addr);

	int data = readRegMPU(mpu_addr,0x75); // Register 117 – Who Am I - 0x75

	if (data == 104)
	{
		Serial.println("MPU6050 Dispositivo respondeu OK! (104)");

		data = readRegMPU(mpu_addr,0x6B); // Register 107 – Power Management 1 - 0x6B

		if (data == 64) Serial.println("MPU6050 em modo SLEEP! (64)");
		else Serial.println("MPU6050 em modo ACTIVE!");
	}
	else Serial.println("Verifique dispositivo - MPU6050 NÃO disponível!");
}

void readMPU(uint8_t mpu_addr, float* Angle) {
	Wire.beginTransmission(mpu_addr);
	Wire.write(0x3B);                               //Pedir el registro 0x3B - corresponde al AcX
	Wire.endTransmission(false);
	Wire.requestFrom(mpu_addr, (size_t)14, true);   //A partir do 0x3B, ler 14 registros
	AcX = Wire.read() << 8 | Wire.read();           //Cada valor ocupa 2 registros
	AcY = Wire.read() << 8 | Wire.read();
	AcZ = Wire.read() << 8 | Wire.read();
	Wire.read();Wire.read();
	GyX = Wire.read() << 8 | Wire.read();
	GyY = Wire.read() << 8 | Wire.read();
	GyZ = Wire.read() << 8 | Wire.read();

	An[0] = AcX / A_R;
	An[1] = AcY / A_R;

	G[0] = GyX / G_R;
	G[1] = GyY / G_R;
	G[2] = GyZ / G_R;

	dt = (millis() - tiempo_prev) / 1000.0;
	tiempo_prev = millis();

	//Integración respecto del tiempo paras calcular el YAW
	An[2] = (An[2] + G[2] * dt) - .0061;

	memcpy(Angle, An, sizeof(An));
	
	Serial.print(An[0]);
	Serial.print(" ");
	Serial.print(An[1]);
	Serial.print(" ");
	Serial.print(An[2]);
	Serial.println(" ");

	delay(10);	


	

}

