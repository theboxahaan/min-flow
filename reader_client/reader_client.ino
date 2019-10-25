/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SS_PIN 2
#define RST_PIN 0
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 
HTTPClient http;

// Init array that will store new NUID 
byte nuidPICC[4];

// Wifi Details
const char *ssid =  "Bharath";     // replace with your wifi ssid and wpa2 key
const char *pass =  "12345678";
WiFiClient client;


void read_data_from_block(MFRC522::StatusCode status, byte *buffer, byte size, byte trailerBlock, byte sector, byte blockAddr){
  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
      return;
  }

  // Show the whole sector as it currently is
//  Serial.println(F("Current data in sector:"));
//  rfid.PICC_DumpMifareClassicSectorToSerial(&(rfid.uid), &key, sector);
//  Serial.println();

  // Read data from the block
//  Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
//  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) rfid.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
  }
  
//  Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
//  dump_byte_array(buffer, 16); Serial.println();
//  Serial.println();
}

void write_data_to_block(MFRC522::StatusCode status, byte *buffer, byte size, byte trailerBlock, byte sector, byte blockAddr, byte *byteArray){
  // Authenticate using key B
  Serial.println(F("Authenticating again using key B..."));
  status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
      return;
  }

  // Write data to the block
  Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(byteArray, 16); Serial.println();
  status = (MFRC522::StatusCode) rfid.MIFARE_Write(blockAddr, byteArray, 16);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
  }
  Serial.println();

  // Read data from the block (again, should now be what we have written)
  Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) rfid.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(rfid.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();

  // Check that data in block is what we have written
  // by counting the number of bytes that are equal
  Serial.println(F("Checking result..."));
  byte count = 0;
  for (byte i = 0; i < 16; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (buffer[i] == byteArray[i])
          count++;
  }
  Serial.print(F("Number of bytes that match = ")); Serial.println(count);
  if (count == 16) {
      Serial.println(F("Success :-)"));
  } else {
      Serial.println(F("Failure, no match :-("));
      Serial.println(F("  perhaps the write didn't work properly..."));
  }
  Serial.println();

}

// Helper function to convert hextobytes
void hexCharacterStringToBytes(byte *byteArray, const char *hexString)
{
  bool oddLength = strlen(hexString) & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;

    if (oddLength)
    {
      // If the length is odd
      if (oddCharIndex)
      {
        // odd characters go in high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Even characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      // If the length is even
      if (!oddCharIndex)
      {
        // Odd characters go into the high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Odd characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

//Helper function to print bytearray to Serial
void dumpByteArray(const byte * byteArray, const byte arraySize)
{

for (int i = 0; i < arraySize; i++)
{
  Serial.print("0x");
  if (byteArray[i] < 0x10)
    Serial.print("0");
  Serial.print(byteArray[i], HEX);
  Serial.print(", ");
}
Serial.println();
}

byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

int byteArrayToHexString(uint8_t *byte_array, int byte_array_len,
                         char *hexstr, int hexstr_len)
{
    int off = 0;
    int i;

    for (i = 0; i < byte_array_len; i ++) {
        off += snprintf(hexstr + off, hexstr_len - off,
                           "%02x", byte_array[i]);
    }

    hexstr[off] = '\0';

    return off;
}


void setup() { 
  Serial.begin(9600);
  delay(10);
               
  Serial.println("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, pass); 
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  StaticJsonBuffer<200> jsonBuffer;
  char json[] = "{\"sak\":\"\",\"type\":\"\"}"; 
  JsonObject& json_data = jsonBuffer.parseObject(json);
}
 
void loop() {
  byte block;
  byte len;
  MFRC522::StatusCode status;

  if (WiFi.status() == WL_CONNECTED) {
    
    StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
    JsonObject& json_data = JSONbuffer.createObject(); 
    JsonArray& uid = json_data.createNestedArray("uid"); //JSON array
    JsonArray& unique_hash = json_data.createNestedArray("unique_hash"); //JSON array
    char JSONmessageBuffer[300];

    String SERVER = "192.168.43.171";
    int PORT = 8080;

    byte sector         = 1;
    byte blockAddr      = 5;
    byte trailerBlock   = 7;
    const byte MaxByteArraySize = 16;
    byte byteArray1[MaxByteArraySize] = {0};
    byte byteArray2[MaxByteArraySize] = {0};

    const char * first_half_hash = "01915ec5de4fb49c90978275147f0472";
    const char * second_half_hash = "e54b8cb9c3190b59a0fea3b0f8dc8b58";

    MFRC522::StatusCode status;
    byte buffer1[18];
    byte buffer2[18];
    byte size = sizeof(buffer1);
    char hexstr1[33];
    char hexstr2[33];

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! rfid.PICC_IsNewCardPresent())
      return;
  
    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
      return;
      
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    
    // Check is the PICC of Classic MIFARE type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      return;
    }

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
//      nuidPICC[i] = rfid.uid.uidByte[i];
      uid.add(rfid.uid.uidByte[i]);
    }

    json_data["type"] = rfid.PICC_GetTypeName(piccType);
    json_data["sak"] = rfid.uid.sak;

    // Converting the hash value to byte arrays    
    hexCharacterStringToBytes(byteArray1, first_half_hash);
    hexCharacterStringToBytes(byteArray2, second_half_hash);
    // Printing the hash value to serial
//    dumpByteArray(byteArray1, MaxByteArraySize);
//    dumpByteArray(byteArray2, MaxByteArraySize);
    // Read data from two consecutive blocks where the hash value is written at the time of registration
    read_data_from_block(status, buffer1, size, trailerBlock, sector, blockAddr);
    read_data_from_block(status, buffer2, size, trailerBlock, sector, blockAddr+1);
    // COnverting the read bytes back to hex strings to post them to the server for AUTH
    byteArrayToHexString(buffer1, MaxByteArraySize, hexstr1, 33);
    byteArrayToHexString(buffer2, MaxByteArraySize, hexstr2, 33);
    
    unique_hash.add(hexstr1);
    unique_hash.add(hexstr2);
    // Uncomment if you wish to write the hash values
//    write_data_to_block(status, buffer1, size, trailerBlock, sector, blockAddr, byteArray1);
//    write_data_to_block(status, buffer2, size, trailerBlock, sector, blockAddr+1, byteArray2);


    
    json_data.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
    // Connect to the server to send the data
    if (client.connect(SERVER, PORT))
      client.print(JSONmessageBuffer);
    else Serial.println("Couldnt connect to the server");
    client.stop();

  
    // Halt PICC
    rfid.PICC_HaltA();
  
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
  }
  else {
 
    Serial.println("Error in WiFi connection");
 
  }
  
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
