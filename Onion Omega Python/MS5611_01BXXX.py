# Distributed with a free-will license.
# Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
# MS5611_01BXXX
# This code is designed to work with the MS5611_01BXXX_I2CS I2C Mini Module available from ControlEverything.com.
# https://www.controleverything.com/content/Analog-Digital-Converters?sku=MS5611-01BXXX_I2CS_A01#tabs-0-product_tabset-2

from OmegaExpansion import onionI2C
import time

# Get I2C bus
i2c = onionI2C.OnionI2C()

# MS5611_01BXXX address, 0x76(118)
#		0x1E(30)	Reset command
data = [0x1E]
i2c.write(0x76, data)

time.sleep(0.5)

# Read 12 bytes of calibration data
# Read pressure sensitivity
data = i2c.readBytes(0x76, 0xA2, 2)
C1 = data[0] * 256 + data[1]

# Read pressure offset
data = i2c.readBytes(0x76, 0xA4, 2)
C2 = data[0] * 256 + data[1]

# Read temperature coefficient of pressure sensitivity
data = i2c.readBytes(0x76, 0xA6, 2)
C3 = data[0] * 256 + data[1]

# Read temperature coefficient of pressure offset
data = i2c.readBytes(0x76, 0xA8, 2)
C4 = data[0] * 256 + data[1]

# Read reference temperature
data = i2c.readBytes(0x76, 0xAA, 2)
C5 = data[0] * 256 + data[1]

# Read temperature coefficient of the temperature
data = i2c.readBytes(0x76, 0xAC, 2)
C6 = data[0] * 256 + data[1]

# MS5611_01BXXX address, 0x76(118)
#		0x40(64)	Pressure conversion(OSR = 256) command
data = [0x40]
i2c.write(0x76, data)

time.sleep(0.5)

# Read digital pressure value
# Read data back from 0x00(0), 3 bytes
# D1 MSB2, D1 MSB1, D1 LSB
value = i2c.readBytes(0x76, 0x00, 3)
D1 = value[0] * 65536 + value[1] * 256 + value[2]

# MS5611_01BXXX address, 0x76(118)
#		0x50(64)	Temperature conversion(OSR = 256) command
data = [0x50]
i2c.write(0x76, data)

time.sleep(0.5)

# Read digital temperature value
# Read data back from 0x00(0), 3 bytes
# D2 MSB2, D2 MSB1, D2 LSB
value = i2c.readBytes(0x76, 0x00, 3)
D2 = value[0] * 65536 + value[1] * 256 + value[2]

dT = D2 - C5 * 256
TEMP = 2000 + dT * C6 / 8388608
OFF = C2 * 65536 + (C4 * dT) / 128
SENS = C1 * 32768 + (C3 * dT ) / 256
T2 = 0
OFF2 = 0
SENS2 = 0

if TEMP >= 2000 :
	T2 = 0
	OFF2 = 0
	SENS2 = 0
elif TEMP < 2000 :
	T2 = (dT * dT) / 2147483648
	OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2
	SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4
	if TEMP < -1500 :
		OFF2 = OFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500))
		SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2

TEMP = TEMP - T2
OFF = OFF - OFF2
SENS = SENS - SENS2
pressure = ((((D1 * SENS) / 2097152) - OFF) / 32768.0) / 100.0
cTemp = TEMP / 100.0
fTemp = cTemp * 1.8 + 32

# Output data to screen
print "Pressure : %.2f mbar" %pressure
print "Temperature in Celsius : %.2f C" %cTemp
print "Temperature in Fahrenheit : %.2f F" %fTemp
