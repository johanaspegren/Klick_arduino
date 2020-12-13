/*
 * Klick software for Adafruit Feather nRRF52
 * Features BLE (uart)
 * Connects to RFID reader (mf)rc522
 * Scans Mifare cards and writes to uart
 * 
 * By Scandinavian Design Thinking 20201213
 * version 1.0
 * 
 */

#include <bluefruit.h>
#include "SPI.h" // SPI library
#include "MFRC522.h" // RFID library (https://github.com/miguelbalboa/rfid)

// OTA DFU service
BLEDfu bledfu;

// Uart over BLE service
BLEUart bleuart;

// Packet buffer
extern uint8_t packetbuffer[];

// RFID stuff
#define RST_PIN         16         // Communication with mfrc522
#define SS_PIN          15         // Dito

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(115200); // open serial connection
  Serial.println(F("Klick 1.0 20201213"));
  Serial.println(F("Läser kort och talar över BLE uart"));

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Klick");

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and start the BLE Uart service
  bleuart.begin();

  // Set up and start advertising
  startAdv();

  SPI.begin(); // open SPI connection
  mfrc522.PCD_Init(); // Initialize Proximity Coupling Device (PCD)
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scanna PICC to see UID, SAK, type, and data blocks..."));

}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) { // (true, if RFID tag/card is present ) PICC = Proximity Integrated Circuit Card
    if(mfrc522.PICC_ReadCardSerial()) { // true, if RFID tag/card was read
        String cardUid = "{";
        uint8_t buf[64];
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          cardUid+= String(mfrc522.uid.uidByte[i], HEX);
        }
        cardUid += "}";
        char charBuf[20];
        cardUid.toCharArray(charBuf, 20);
        bleuart.write(charBuf);
        Serial.println(cardUid);
        delay(1000);
    }
  }
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}
