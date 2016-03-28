// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// MS5611_01BXXX
// This code is designed to work with the MS5611_01BXXX_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Analog-Digital-Converters?sku=MS5611-01BXXX_I2CS_A01#tabs-0-product_tabset-2

import com.pi4j.io.i2c.I2CBus;
import com.pi4j.io.i2c.I2CDevice;
import com.pi4j.io.i2c.I2CFactory;
import java.io.IOException;

public class MS5611_01BXXX
{
	public static void main(String args[]) throws Exception
	{
		// Create I2C bus
		I2CBus Bus = I2CFactory.getInstance(I2CBus.BUS_1);
		// Get I2C device, MS5611_01BXXX I2C address is 0x76(118)
		I2CDevice device = Bus.getDevice(0x76);

		// Reset command
		device.write((byte)0x1E);
		Thread.sleep(500);

		byte[] data = new byte[2];
		// Read 12 bytes of calibration data
		// Read pressure sensitivity
		device.read(0xA2, data, 0, 2);
		int C1 = ((data[0] & 0xFF) * 256 + (data[1] & 0xFF));

		// Read pressure offset
		device.read(0xA4, data, 0, 2);
		int C2 = ((data[0] & 0xFF) * 256 + (data[1] & 0xFF));

		// Read temperature coefficient of pressure sensitivity
		device.read(0xA6, data, 0, 2);
		int C3 = ((data[0] & 0xFF) * 256 + (data[1] & 0xFF));

		// Read temperature coefficient of pressure offset
		device.read(0xA8, data, 0, 2);
		int C4 = ((data[0] & 0xFF) * 256 + (data[1] & 0xFF));

		// Read reference temperature
		device.read(0xAA, data, 0, 2);
		int C5 = ((data[0] & 0xFF) * 256 + (data[1] & 0xFF));

		// Read temperature coefficient of the temperature
		device.read(0xAC, data, 0, 2);
		int C6 = ((data[0] & 0xFF) * 256 + (data[1] & 0xFF));

		// Send pressure conversion(OSR = 256) command
		device.write((byte)0x40);
		Thread.sleep(500);

		byte[] value = new byte[3];
		// Read digital pressure value
		// Read 3 bytes of data
		// D1 msb2, D1 msb1, D1 lsb
		device.read(0x00, value, 0, 3);
		long D1 = ((value[0] & 0xFF) * 65536 + (value[1] & 0xFF) * 256 + (value[2] & 0xFF));

		// Send temperature conversion(OSR = 256) command
		device.write((byte)0x50);
		Thread.sleep(500);

		// Read digital temperature value
		// Read 3 bytes of data
		// D2 msb2, D2 msb1, D2 lsb
		device.read(0x00, value, 0, 3);
		long D2 = ((value[0] & 0xFF) * 65536 + (value[1] & 0xFF) * 256 + (value[2] & 0xFF));

		long dT = D2 - C5 * 256;
		long TEMP = 2000 + dT * C6 / (long)8388608;
		long OFF = C2 * 65536L + (C4 * dT) / 128L;
		long SENS = C1 * 32768L + (C3 * dT) / 256L;
		long T2 = 0;
		long OFF2 = 0;
		long SENS2 = 0;

		if(TEMP >= 2000)
		{
			T2 = 0;
			OFF2 = 0;
			SENS2= 0;
		}
		else if(TEMP < 2000)
		{
			T2 = (dT * dT) / 2147483648L;
			OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
			SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
			if(TEMP < -1500)
			{
				OFF2 = OFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
				SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
			}
		}

		TEMP = TEMP - T2;
		OFF = OFF - OFF2;
		SENS = SENS - SENS2;
		double pressure = ((((D1 * SENS) / 2097152) - OFF) / 32768) / 100.0;
		double cTemp = TEMP / 100.0;
		double fTemp = cTemp * 1.8 + 32;

		// Output data to screen
		System.out.printf("Pressure : %.2f mbar %n", pressure);
		System.out.printf("Temperature in Celsius : %.2f C %n", cTemp);
		System.out.printf("Temperature in Fahrenheit : %.2f C %n", fTemp);
	}
}
