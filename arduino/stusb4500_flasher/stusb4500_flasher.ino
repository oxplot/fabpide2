// stusb4500_flasher.ino - flash STUSB4500 NVM with configuration
// Ported to Arduino from https://github.com/usb-c/STUSB4500 and further configuration
// builder implemented on top.
//
// 1. Connect to fabpide2 board as following:
//
//   Arduino   | fabpide2
//   --------------------
//   SDA         SDA
//   SCL         SCL
//   5v/3.3v     VCC
//   5v/3.3v     VPP
//   GND         GND
//
// SDA and SCL pins are defined further below by macros. They can be any
// pin on the arduino as software I2C is used.
//
// 2. Set SDA_PIN & SCL_PIN to the pin number of the SDA and SCL on your board.
// 3. Set SDA_PORT & SCL_PORT to the port name of the SDA and SCL pins on your board.
//    * The default config is for Arduino Nano board.
// 4. Set appropriate values for the configuration macros below.
// 5. Upload the sketch and set serial monitor baud to 115200.
// 6. Follow the prompt.
//
/////////////////////////////////////////////////////////////////
// Config Macros
/////////////////////////////////////////////////////////////////
// * There are three PDOs (1, 2 and 3) each defining a voltage range and
//   max current. The voltage is defined with 50mV resolution between
//   5-20V and a lower and upper percentage range from 5%-20%. For
//   instance, when configured with 12V with 5% lower and 20% upper
//   threshold, supply voltages between 11.4-14.4V are accepted.
//
// * If PDO_COUNT is set to 3, PDO3 will be tried to match first. If
//   supply doesn't support the range and/or max. current, PDO2 will be
//   tried. If PDO2 cannot be satisfied, PDO 1 is tried like the other two
//   only if PDO1_ENABLED is true.  Otherwise, the power path is disabled
//   and no power is supplied to downstream.
//
// * If number of PDOs is set to 2 or 1, the matching starts at the
//   corresponding PDO and goes down to 1, ignoring higher PDO profiles.
//
// * PDO1's voltage is 5V and cannot be changed. Max. current for all
//   PDOs can be set to range 0.5-5A in at specific values. It can
//   optionally be set to CURRENT_FLEX which can be defined on a more
//   granular level.
//
// * Other config parameters are explained in further comments below.
/////////////////////////////////////////////////////////////////

// Max. current - note increments change from 0.25A to 0.5A toward the
// higher currents.
#define CURRENT_FLEX 0
#define CURRENT_0_50 1
#define CURRENT_0_75 2
#define CURRENT_1_00 3
#define CURRENT_1_25 4
#define CURRENT_1_50 5
#define CURRENT_1_75 6
#define CURRENT_2_00 7
#define CURRENT_2_25 8
#define CURRENT_2_50 9
#define CURRENT_2_75 10
#define CURRENT_3_00 11
#define CURRENT_3_50 12
#define CURRENT_4_00 13
#define CURRENT_4_50 14
#define CURRENT_5_00 15

// If enabled poll source for supported current at set voltage and
// pick highest supported current. When enabled set PDOx_CURRENT to
// lowest current needed
#define REQ_SRC_CURRENT false

// Flexible max. current value (0.005A resolution)
// Instead of using predefined CURRENT_* macros to set maximum current
// for a PDO, you can set the PDOx_CURRENT = CURRENT_FLEX which will
// set the maximum current to FLEX_CURRENT value.
#define FLEX_CURRENT 3.83

// Number of active PDO configs - between 1 and 3 inclusive.
#define PDO_COUNT 2

// Define your power requirements below in order of priority.
// PDOX_VOLTAGE_X_PERCENT can be whole numbers b/w 5 and 20 inclusive.
// PDOX_VOLTAGE can be from 0-20V with 0.05V resolution.
#define PDO3_VOLTAGE 20.00
#define PDO3_VOLTAGE_LOWER_PERCENT 7
#define PDO3_VOLTAGE_UPPER_PERCENT 9
#define PDO3_CURRENT CURRENT_5_00

#define PDO2_VOLTAGE 15.00
#define PDO2_VOLTAGE_LOWER_PERCENT 10
#define PDO2_VOLTAGE_UPPER_PERCENT 10
#define PDO2_CURRENT CURRENT_1_00

// PDO1's voltage is 5v and it cannot be changed.
#define PDO1_CURRENT CURRENT_FLEX
#define PDO1_VOLTAGE_UPPER_PERCENT 11
#define PDO1_ENABLED false

// Consult datasheet for details.
#define GPIO_FUNCTION_SW_CTRL_GPIO
#define GPIO_FUNCTION_ERROR_RECOVERY
#define GPIO_FUNCTION_DEBUG
#define GPIO_FUNCTION_SINK_POWER

// Sets function of the GPIO pin.
#define GPIO_FUNCTION GPIO_FUNCTION_SINK_POWER

// Discharge time in milliseconds between 84-1260ms inclusive.
#define VBUS_TO_0V_DISCHARGE_TIME 1000
// Discharge time in milliseconds between 24-360ms inclusive.
#define VBUS_TO_LOWER_PDO_DISCHARGE_TIME 200

/////////////////////////////////////////////////////////////////

// Set to appropriate pin and port of your arduino board. SDA and SCL pins
// need not be I2C pins of arduino as software I2C is used. Default values below
// are for Arduino Nano.
//
// Pinout for current (default) configuration
// Board   | Nano | Uno | Mega 2650
// SDA pin |    4 |  A4 |        33
// SCL pin |    5 |  A5 |        32
//
#define SDA_PIN 4
#define SCL_PIN 5
#define SDA_PORT PORTC
#define SCL_PORT PORTC

// These sector values are derived from the config macros above.
uint8_t Sector[5][8] = {
  {0x00, 0x00, 0xB0, 0xAA, 0x00, 0x45, 0x00, 0x00},
  {
    0x10,
    0x40,
    0x00 | ((VBUS_TO_0V_DISCHARGE_TIME / 84) << 4) | (VBUS_TO_LOWER_PDO_DISCHARGE_TIME / 24),
    0x1C,
    0xFF,
    0x01,
    0x3C,
    0xDF
  },
  {0x02, 0x40, 0x0F, 0x00, 0x32, 0x00, 0xFC, 0xF1},
  {
    0x00,
    0x19,
    0x00 | (PDO1_CURRENT << 4) | (PDO_COUNT << 1),
    0x0F | ((PDO1_VOLTAGE_UPPER_PERCENT - 5) << 4),
    0x00 | (PDO2_CURRENT) | ((PDO2_VOLTAGE_LOWER_PERCENT - 5) << 4),
    0x00 | (PDO3_CURRENT << 4) | (PDO2_VOLTAGE_UPPER_PERCENT - 5),
    0x00 | ((PDO3_VOLTAGE_UPPER_PERCENT - 5) << 4) | (PDO3_VOLTAGE_LOWER_PERCENT - 5),
    0x00
  },
  {
    0x00 | ((int)(PDO2_VOLTAGE * 20) & 0b11) << 6,
    0x00 | ((int)(PDO2_VOLTAGE * 20) >> 2),
    0x90 | ((int)(PDO3_VOLTAGE * 20) & 0xFF),
    0x00 | (((int)(FLEX_CURRENT * 200) & 0b1111111) << 1) | ((int)(PDO3_VOLTAGE * 20) >> 8),
    0x40 | ((int)(FLEX_CURRENT * 200) >> 7),
    0x00,
    0x40 | (PDO1_ENABLED ? 0 : 8) | (REQ_SRC_CURRENT ? 16 : 0),
    0xFB
  }
};

#include <SoftWire.h>

#define FTP_CUST_PASSWORD_REG 0x95
#define FTP_CUST_PASSWORD 0x47
#define FTP_CTRL_0 0x96
#define FTP_CUST_PWR 0x80 
#define FTP_CUST_RST_N 0x40
#define FTP_CUST_REQ 0x10
#define FTP_CUST_SECT 0x07
#define FTP_CTRL_1 0x97
#define FTP_CUST_SER 0xF8
#define FTP_CUST_OPCODE 0x07
#define RW_BUFFER 0x53

#define READ 0x00
#define WRITE_PL 0x01
#define WRITE_SER 0x02
#define READ_PL 0x03
#define READ_SER 0x04
#define ERASE_SECTOR 0x05
#define PROG_SECTOR 0x06
#define SOFT_PROG_SECTOR 0x07

#define SECTOR_0 0x01
#define SECTOR_1 0x02
#define SECTOR_2 0x04
#define SECTOR_3 0x08
#define SECTOR_4 0x10

#define I2C_ADDRESS 0x28

#define DEBUG

SoftWire Wire = SoftWire();

void setup() {
  Wire.begin();
  Serial.begin(115200);

#ifdef DEBUG
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 8; j++) {
      printHex(Sector[i][j]);
      Serial.print("\t");
    }
    Serial.println();
  }
#endif
}

void loop() {
  uint8_t buf[40];

  delay(1000);
  Serial.println();
  Serial.println("Press 'f' to flash the configuration ...");
  while(!Serial.available() || Serial.read() != 'f');
  Serial.println("Flashing - hang on ...");

  if (nvm_flash() != 0) {
    Serial.println("FAILED flashing :(");
    return;
  }

  Serial.println("Verifying - hang on some more ...");
  if (nvmRead(buf) != 0) {
    Serial.println("Failed to read the flash :(");
    return;
  }
  for (int i = 0; i < 5; i++) {
    if (!verifySector(Sector[i], &buf[8 * i])) { 
      Serial.println("Verification failed :(");
      return;
    }
  }
  Serial.println("All done! Yayyyy! :)");
}

bool verifySector(uint8_t* target, uint8_t* actual) {
  for (int i = 0; i < 8; i++) {
    if (actual[i] != target[i]) {
      return false;
    }
  }
  return true;
}

int chipWrite(uint8_t reg, uint8_t* data, uint8_t len) {
  Wire.beginTransmission((uint8_t) I2C_ADDRESS);
  if (Wire.write(reg) != 1) {
    return 1;
  }
  if (Wire.write(data, len) != len) {
    return 1;
  }
  int endStatus = Wire.endTransmission(true);
  if (endStatus != 0) {
    Serial.print("chipWrite end tx failed (");
    Serial.print(endStatus, HEX);
    Serial.println(")");
    return 1;
  }
  return 0;
}

int chipRead(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission((uint8_t) I2C_ADDRESS);
  if (Wire.write((uint8_t) reg) != 1) {
    return 1;
  }
  if (Wire.endTransmission(false) != 0) {
    return 1;
  }
  Wire.requestFrom((uint8_t) I2C_ADDRESS, (uint8_t) len, (uint8_t) true);
  for (int i = 0; i < len; i++) {
    while (!Wire.available());
    buf[i] = Wire.read();
  }
  return 0;
}

int enterNVMReadMode() {
  uint8_t buf[2];
  buf[0] = FTP_CUST_PASSWORD;
  if (chipWrite(FTP_CUST_PASSWORD_REG, buf, 1) != 0) {
    Serial.println("Failed to write customer password");
    return 1;
  }
  buf[0] = 0;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0) {
    Serial.println("Failed to reset NVM internal controller");
    return 1;
  }
  buf[0] = FTP_CUST_PWR | FTP_CUST_RST_N;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0) {
    Serial.println("Failed to set PWR and RST_N bits");
    return 1;
  }
  return 0;
}

int enterNVMWriteMode(uint8_t erasedSector) {
  uint8_t buf[2];

  buf[0] = FTP_CUST_PASSWORD;
  if (chipWrite(FTP_CUST_PASSWORD_REG, buf, 1) != 0 ) {
    Serial.println("Failed to set user password");
    return 1;
  }

  buf[0] = 0 ;
  if (chipWrite(RW_BUFFER, buf, 1) != 0) {
    Serial.println("Failed to set null for partial erase feature");
    return 1;
  }

  buf[0] = 0;
  if (chipWrite(FTP_CTRL_0,buf,1) != 0 ) {
    Serial.println("Failed to reset NVM controller");
    return 1;
  }

  buf[0] = FTP_CUST_PWR | FTP_CUST_RST_N;
  if (chipWrite(FTP_CTRL_0,buf,1) != 0) {
    Serial.println("Failed to set PWR and RST_N bits");
    return 1;
  }
  
  
  buf[0] = ((erasedSector << 3) & FTP_CUST_SER) | ( WRITE_SER & FTP_CUST_OPCODE) ;
  if (chipWrite(FTP_CTRL_1, buf, 1) != 0) {
    Serial.println("Failed to write SER opcode");
    return 1;
  }
  
  buf[0] = FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0 ) {
    Serial.println("Failed to write SER optcode");
    return 1;
  }

  do {
    if (chipRead(FTP_CTRL_0, buf, 1) != 0 ) {
      Serial.println("Failed to wait for execution");
      return 1;
    }
  } while(buf[0] & FTP_CUST_REQ);

  buf[0] = SOFT_PROG_SECTOR & FTP_CUST_OPCODE;
  if (chipWrite(FTP_CTRL_1, buf, 1) != 0 ) {
    Serial.println("Failed to set soft prog opcode");
    return 1;
  }

  buf[0] = FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0 ) {
    Serial.println("Failed to load soft prog opcode");
    return 1;
  }

  do {
    if (chipRead(FTP_CTRL_0, buf, 1) != 0 ) {
      Serial.println("Failed waiting for execution");
      return 1;
    }
  } while(buf[0] & FTP_CUST_REQ);

  buf[0] = ERASE_SECTOR & FTP_CUST_OPCODE;
  if (chipWrite(FTP_CTRL_1, buf, 1) != 0) {
    Serial.println("Failed to set erase sector opcode");
    return 1;
  }

  buf[0] = FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ;
  if (chipWrite(FTP_CTRL_0, buf, 1)  != 0) {
    Serial.println("Failed to load erase sectors opcode");
    return 1;
  }

  do {
    if ( chipRead(FTP_CTRL_0, buf, 1) != 0 ) {
      Serial.println("Failed waiting for execution");
      return 1;
    }
  } while(buf[0] & FTP_CUST_REQ);
  
  return 0;
}

int writeNVMSector(uint8_t SectorNum, uint8_t *SectorData)
{
  uint8_t Buffer[2];

  if (chipWrite(RW_BUFFER, SectorData, 8) != 0) {
    return -1;
  }

  Buffer[0] = FTP_CUST_PWR | FTP_CUST_RST_N;
  if (chipWrite(FTP_CTRL_0, Buffer, 1) != 0) {
    return -1;
  }

  Buffer[0] = WRITE_PL & FTP_CUST_OPCODE;
  if (chipWrite(FTP_CTRL_1, Buffer, 1) != 0) {
    return -1;
  }

  Buffer[0] = FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;
  if (chipWrite(FTP_CTRL_0, Buffer, 1) != 0) {
    return -1;
  }

  do {
    if ( chipRead(FTP_CTRL_0,Buffer,1) != 0 )return -1;
  } while(Buffer[0] & FTP_CUST_REQ) ;

  Buffer[0] = PROG_SECTOR & FTP_CUST_OPCODE;
  if (chipWrite(FTP_CTRL_1, Buffer, 1) != 0) {
    return -1;
  }

  Buffer[0] = (SectorNum & FTP_CUST_SECT) | FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ;
  if (chipWrite(FTP_CTRL_0, Buffer, 1) != 0) {
    return -1;
  }

  do {
      if ( chipRead(FTP_CTRL_0,Buffer,1) != 0 )return -1;
  } while(Buffer[0] & FTP_CUST_REQ);

  return 0;
}

int nvm_flash() {
  if (enterNVMWriteMode(SECTOR_0 | SECTOR_1 | SECTOR_2 | SECTOR_3 | SECTOR_4) != 0) {
    return -1;
  }
  for (int i = 0; i < 5; i++) {
    if (writeNVMSector(i, Sector[i]) != 0) {
      return -1;
    }
  }
  if (exitNVMMode() != 0) {
    return -1;
  }
  return 0;
}

int exitNVMMode() {
  uint8_t buf[2];
  buf[0] = FTP_CUST_RST_N;
  buf[1] = 0;
  if (chipWrite(FTP_CTRL_0, buf, 2) != 0) {
    Serial.println("Failed to exit NVM mode");
    return 1;
  }
  buf[0] = 0;
  if (chipWrite(FTP_CUST_PASSWORD_REG, buf, 1) != 0) {
    Serial.println("Failed to reset customer password");
    return 1;
  }
  return 0;
}

int readNVMSector(uint8_t num, uint8_t* data) {
  uint8_t buf[2];
  buf[0] = FTP_CUST_PWR | FTP_CUST_RST_N;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0) {
    Serial.println("Failed to set PWR and RST_N bits");
    return 1;
  }
  buf[0]= (READ & FTP_CUST_OPCODE);
  if (chipWrite(FTP_CTRL_1, buf, 1) != 0) {
    Serial.println("Failed to set read sectors opcode");
    return 1;
  }

  buf[0] = (num & FTP_CUST_SECT) | FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0 ) {
    Serial.println("Failed to read sectors opcode");
    return 1;
  }
  do {
    if (chipRead(FTP_CTRL_0, buf, 1) != 0) {
      Serial.println("Failed waiting for execution");
      return 1;
    }
  } while(buf[0] & FTP_CUST_REQ);

  if (chipRead(RW_BUFFER, &data[0], 8) != 0) {
    Serial.println("NVM read failed");
    return 1;
  }

  buf[0] = 0;
  if (chipWrite(FTP_CTRL_0, buf, 1) != 0) {
    Serial.println("Resetting controller failed");
    return 1;
  }
  
  return 0;
}

int nvmRead(uint8_t* out) {
  if (enterNVMReadMode() != 0) {
    Serial.println("Failed to enter NVM read mode");
    return 1;
  }
  for (int i = 0; i < 5; i++) {
    if (readNVMSector(i, out + (i * 8)) != 0) {
      Serial.println("Failed to read sector");
      return 1;
    }
  }
  if (exitNVMMode() != 0) {
    Serial.println("Failed to exit NVM read mode");
    return 1;
  }
  return 0;
}

void printHex(int v) {
  char tmp[10];
  sprintf(tmp, "0x%.2X", v);
  Serial.print(tmp);
}
