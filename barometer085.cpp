#include <Arduino.h>
#include <Wire.h>
#include "barometer085.h"


Barometer::Barometer(int pin) : OSS(0)
{
  //
  //OSS = 0;
}

void Barometer::begin(TwoWire &twowire)
{
  wirePtr = twowire;
  Barometer::calibrate();
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void Barometer::calibrate()
{
  ac1 = Barometer::readInt(0xAA);
  //Serial.print("test1");
  ac2 = Barometer::readInt(0xAC);
  ac3 = Barometer::readInt(0xAE);
  ac4 = Barometer::readInt(0xB0);
  ac5 = Barometer::readInt(0xB2);
  ac6 = Barometer::readInt(0xB4);
  b1 = Barometer::readInt(0xB6);
  b2 = Barometer::readInt(0xB8);
  mb = Barometer::readInt(0xBA);
  mc = Barometer::readInt(0xBC);
  md = Barometer::readInt(0xBE);
  //Serial.print("test2");
}

// Calculate temperature in deg C
float Barometer::getTemperature() 
{
  unsigned int ut = Barometer::readUT();
  long x1, x2;

  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long  Barometer::getPressure()
{
  unsigned long up = Barometer::readUP();
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  long temp = p;
  return temp;
}

// Read 1 byte from the BMP085 at 'address'
char Barometer::read(unsigned char address)
{
  unsigned char data;

  wirePtr.beginTransmission(BMP085_ADDRESS);
  wirePtr.write(address);
  wirePtr.endTransmission();

  wirePtr.requestFrom(BMP085_ADDRESS, 1);
  while(!wirePtr.available())
    ;

  return wirePtr.read();
}


// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int Barometer::readInt(unsigned char address)
{
  unsigned char msb, lsb;
  //Serial.println("test3");
  wirePtr.beginTransmission(BMP085_ADDRESS);
  wirePtr.write(address);
  wirePtr.endTransmission();
//Serial.println("test4");
  wirePtr.requestFrom(BMP085_ADDRESS, 2);
  while(wirePtr.available()<2);
  msb = wirePtr.read();
  lsb = wirePtr.read();
//Serial.println("test5");
  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int Barometer::readUT()
{
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  wirePtr.beginTransmission(BMP085_ADDRESS);
  wirePtr.write(0xF4);
  wirePtr.write(0x2E);
  wirePtr.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = Barometer::readInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long Barometer::readUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  wirePtr.beginTransmission(BMP085_ADDRESS);
  wirePtr.write(0xF4);
  wirePtr.write(0x34 + (OSS<<6));
  wirePtr.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = Barometer::read(0xF6);
  lsb = Barometer::read(0xF7);
  xlsb = Barometer::read(0xF8);

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  return up;
}

void Barometer::writeRegister(int deviceAddress, byte address, byte val)
{
  wirePtr.beginTransmission(deviceAddress); // start transmission to device 
  wirePtr.write(address);       // send register address
  wirePtr.write(val);         // send value to write
  wirePtr.endTransmission();     // end transmission
}

int Barometer::readRegister(int deviceAddress, byte address)
{
  int v;
  wirePtr.beginTransmission(deviceAddress);
  wirePtr.write(address); // register to read
  wirePtr.endTransmission();

  wirePtr.requestFrom(deviceAddress, 1); // read a byte

  while(!wirePtr.available()) {
    // waiting
  }

  v = wirePtr.read();
  return v;
}

float Barometer::calcAltitude(float pressure)
{
  float A = pressure/101325;
  float B = 1/5.25588;
  float C = pow(A,B);
  C = 1 - C;
  C = C /0.0000225577;

  return C;
}