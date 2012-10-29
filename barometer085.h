
#ifndef Barometer_h
#define Barometer_h

#include <Arduino.h>
#include <Wire.h>

#define BMP085_ADDRESS 0x77  // I2C address of BMP085
class TwoWire;

class Barometer{
  public:
    Barometer(int pin);
	void begin(TwoWire &wire);
    char read(unsigned char address);
    int readInt(unsigned char address);
	int readRegister(int deviceAddress, byte address);
	unsigned int readUT();
	unsigned long readUP();
	void writeRegister(int deviceAddress, byte address, byte val);
	void readRegiser(int deviceAddress, byte address);
	float calcAltitude(float pressure);
	byte readData();
	long getPressure();
	float getTemperature();
	void calibrate();
  private:
    TwoWire wirePtr;
	int ac1;
	int ac2;
	int ac3;
	unsigned int ac4;
	unsigned int ac5;
	unsigned int ac6;
	int b1;
	int b2;
	int mb;
	int mc;
	int md;
	const unsigned char OSS;  // Oversampling Setting   
	// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).

	long b5; 
 };
#endif