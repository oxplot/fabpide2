// stusb4500_flasher.ino - flash STUSB4500 NVM with configuration
// Ported to Arduino from https://github.com/usb-c/STUSB4500
//
// 1. Using the official GUI configuration tool, generate an .h sector
// file and paste its content at the appropriate place in this file
// further below.
//
// 2. Connect to fabpide2 board as following:
//
//   Arduino   | fabpide2
//   --------------------
//   SDA         SDA
//   SCL         SCL
//   5v          VCC
//   5v          VPP (pull-up power)
//   GND         GND
//
// Look at the pinout of your arduino board to locate the SDA and SCL
// pins which vary across boards.
//
// 3. Set the serial monitor to 115200 baud and follow prompts.
// 4. Set SDA_PIN & SCL_PIN to the pin number of the SDA and SCL on your board.
// 5. Set SDA_PORT & SCL_PORT to the port name of the SDA and SCL pins on your board.
//    * The default config is for Arduino Nano board.

#define SDA_PIN 4
#define SCL_PIN 5
#define SDA_PORT PORTC
#define SCL_PORT PORTC

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

#define ADDRESS 0x28


SoftWire Wire = SoftWire();
/////////////////////////////////////////////////////////////////
// Replace these with .h output file from GUI config editor:
//  https://github.com/usb-c/STUSB4500/tree/master/GUI
/////////////////////////////////////////////////////////////////
uint8_t Sector0[8] = {0x00,0x00,0xFF,0xAA,0x00,0x45,0x00,0x00};
uint8_t Sector1[8] = {0x00,0x40,0x11,0x1C,0xF0,0x01,0x00,0xDF};
uint8_t Sector2[8] = {0x02,0x40,0x0F,0x00,0x32,0x00,0xFC,0xF1};
uint8_t Sector3[8] = {0x00,0x19,0x34,0x0F,0x53,0x35,0x55,0x00};
uint8_t Sector4[8] = {0x00,0x2D,0x90,0x21,0x03,0x00,0x40,0xFB};
/////////////////////////////////////////////////////////////////

void setup() {
  Wire.begin();
  Serial.begin(115200);
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
  if (
    !verifySector(Sector0, &buf[8 * 0])
    || !verifySector(Sector1, &buf[8 * 1])
    || !verifySector(Sector2, &buf[8 * 2])
    || !verifySector(Sector3, &buf[8 * 3])
    || !verifySector(Sector4, &buf[8 * 4])
  ) {
    Serial.println("Verification failed :(");
    return;
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
  Wire.beginTransmission((uint8_t) ADDRESS);
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
  Wire.beginTransmission((uint8_t) ADDRESS);
  if (Wire.write((uint8_t) reg) != 1) {
    return 1;
  }
  if (Wire.endTransmission(false) != 0) {
    return 1;
  }
  Wire.requestFrom((uint8_t) ADDRESS, (uint8_t) len, (uint8_t) true);
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
  if (enterNVMWriteMode(SECTOR_0 | SECTOR_1  | SECTOR_2 | SECTOR_3  | SECTOR_4 ) != 0 ) return -1;
  if (writeNVMSector(0,Sector0) != 0 ) return -1;
  if (writeNVMSector(1,Sector1) != 0 ) return -1;
  if (writeNVMSector(2,Sector2) != 0 ) return -1;
  if (writeNVMSector(3,Sector3) != 0 ) return -1;
  if (writeNVMSector(4,Sector4) != 0 ) return -1;
  if (exitNVMMode() != 0 ) return -1;
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
