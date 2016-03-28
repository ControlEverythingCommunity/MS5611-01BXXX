// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// MS5611_01BXXX
// This code is designed to work with the MS5611_01BXXX_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Analog-Digital-Converters?sku=MS5611-01BXXX_I2CS_A01#tabs-0-product_tabset-2

#include <Wire.h>

// MS5611_01BXXX I2C address is 0x76(118)
#define Addr 0x76

unsigned long Coff[6], Ti = 0, offi = 0, sensi = 0;
unsigned int data[3];
void setup()
{
  // Initialise I2C communication as MASTER
  Wire.begin();
  // Initialise Serial Communication, set baud rate = 9600
  Serial.begin(9600);

  // Read cofficients values stored in EPROM of the device
  for(int i = 0; i < 6; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write(0xA2 + (2 * i));
    // Stop I2C Transmission
    Wire.endTransmission();
  
    // Request 2 bytes of data
    Wire.requestFrom(Addr, 2);
      
    // Read 2 bytes of data
    // Coff msb, Coff lsb
    if(Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }  
      
    // Convert the data
    Coff[i] = ((data[0] * 256) + data[1]);
  }
  delay(300);
}

void loop() 
{
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Send reset command 
  Wire.write(0x1E);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(500);
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Refresh pressure with the OSR = 256 
  Wire.write(0x40);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(500);
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register 
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
  
  // Request 3 bytes of data
  Wire.requestFrom(Addr, 3);

  // Read 3 bytes of data
  // ptemp_msb1, ptemp_msb, ptemp_lsb
  if(Wire.available() == 3)
  {
     data[0] = Wire.read();
     data[1] = Wire.read();
     data[2] = Wire.read();
  }   
  
  // Convert the data 
  unsigned long ptemp = ((data[0] * 65536.0) + (data[1] * 256.0) + data[2]);
 
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Refresh temperature with the OSR = 256 
  Wire.write(0x50);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(500);
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
  
  // Request 3 bytes of data
  Wire.requestFrom(Addr, 3);

  // Read 3 bytes of data
  // temp_msb1, temp_msb, temp_lsb
  if(Wire.available() == 3)
  {
     data[0] = Wire.read();
     data[1] = Wire.read();
     data[2] = Wire.read();
  }
  
  // Convert the data
  unsigned long temp = ((data[0] * 65536.0) + (data[1] * 256.0) + data[2]);
  
  // Pressure and Temperature Calculations
  // 1st order temperature and pressure compensation
  // Difference between actual and reference temperature
  unsigned long dT = temp - ((Coff[4] * 256));
  temp = 2000 + (dT * (Coff[5] / pow(2, 23)));
  
  // Offset and Sensitivity calculation
  unsigned long long off = Coff[1] * 65536 + (Coff[3] * dT) / 128;
  unsigned long long sens = Coff[0] * 32768 + (Coff[2] * dT) / 256;

  // 2nd order temperature and pressure compensation
  if(temp >= 2000)
  {
     Ti = 0;
     offi = 0;
     sensi = 0;
  }
  else if(temp < 2000)
  {
    Ti = (dT * dT) / (pow(2,31));
    offi = 5 * ((pow((temp - 2000), 2))) / 2;
    sensi =  5 * ((pow((temp - 2000), 2))) / 4; 
    if(temp < -1500)
    {
       offi = offi + 7 * ((pow((temp + 1500), 2)));      
       sensi = sensi + 11 * ((pow((temp + 1500), 2))) / 2;
    }
  }
  
  // Adjust temp, off, sens based on 2nd order compensation   
  temp -= Ti;
  off -= offi;
  sens -= sensi;

  // Convert the final data
  ptemp = (ptemp * sens / 2097152 - off);
  ptemp /= 32768;
  float pressure = ptemp / 100.0;
  float ctemp = temp / 100.0;
  float fTemp = ctemp * 1.8 + 32.0;

  // Output data to serial monitor
  Serial.print("Temperature in Celsius : ");
  Serial.print(ctemp);
  Serial.println(" C");
  Serial.print("Temperature in Fahrenheit : ");
  Serial.print(fTemp);
  Serial.println(" F");
  Serial.print("Pressure : ");
  Serial.print(pressure);
  Serial.println(" mbar"); 
  delay(500); 
}

