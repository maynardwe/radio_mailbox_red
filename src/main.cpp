//radio_mailbox_red

// rf69 demo tx rx.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem
// configuration

#include <Arduino.h>
#include <SPI.h>
#include <RH_RF69.h>

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

//#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_CS      8
#define RFM69_INT     7
#define RFM69_RST     4  
#define LED           13
#define testJumper   12
#define GREEN         5
//#define MAILBOX       6  ------------------------ 
#define speakerPin    10
//#endif

float rate, oldrate, ratio;
//int duration = 1000;   // how long the tone lasts
bool gotMail = false;
const int LDR = 0;

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

//-----------------------------------------------------------------------------------------------------
//     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK
//-----------------------------------------------------------------------------------------------------

void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i = 0; i < loops; i++)
  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}

//-----------------------------------------------------------------------------------------------------
//     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK     BLINK
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
//     TEST     TEST     TEST     TEST     TEST     TEST     TEST     TEST     TEST     TEST
//-----------------------------------------------------------------------------------------------------

void Test() {
  if (rf69.available()) {
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];  //look for msg
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {  //RX
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received ["); Serial.print((char*)buf); Serial.print(" RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);

      if (strstr((char *)buf, "Green")) {  // Send a reply to Green!
        uint8_t data[] = "Red";
        rf69.send(data, sizeof(data));
        rf69.waitPacketSent();
        Serial.println("Sent Red");
        //int freq = map(freq, 0, (rf69.lastRssi(), DEC), 440, 2440);
        //tone(speakerPin, freq, duration); // play the tone
        //Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
      }
    } else {
      Serial.println("Receive failed");
    }
  }
}

//-----------------------------------------------------------------------------------------------------
//     TEST     TEST     TEST     TEST     TEST     TEST     TEST     TEST     TEST     TEST
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
//     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP
//-----------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);
  Serial.println("mail_mailbox_red");
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  //pinMode(MAILBOX, INPUT);
  pinMode(testJumper, INPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(LDR, INPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  digitalWrite(LED, HIGH);

  Serial.println("Feather RFM69 RX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);

  //  pinMode(LED, OUTPUT);
  //  digitalWrite(LED,HIGH);
  //  delay(5000);
  //  digitalWrite(LED,LOW);

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

//-----------------------------------------------------------------------------------------------------
//     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP
//-----------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------
//     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
//-----------------------------------------------------------------------------------------------------

void loop() {
  if (digitalRead(testJumper) == HIGH) {
    Test();
    return;
  }

  delay(1000);  // Wait 1 second between transmits, could also 'sleep' here!
//  if ((digitalRead(MAILBOX) == HIGH) and !gotMail) { ----------------------------
//gotMail = true; -----------------------------------
  if (gotMail) {
    digitalWrite(LED, HIGH);
    uint8_t data[] = "Red";
    rf69.send(data, sizeof(data));
    rf69.waitPacketSent();
    Serial.println("TX Red - mailbox opened");
  }
  oldrate = rate;
  rate = analogRead(LDR);
  Serial.print("rate = ");
  Serial.print(rate);
  Serial.print(" ratio new to old rate = ");
    ratio = rate/oldrate;
    Serial.println(ratio);

  if (ratio > 1.2) {
    digitalWrite(LED,HIGH);
        gotMail = true; 
  }
  else {
    digitalWrite(LED,LOW);
  }
//  delay(3000);

    //---------------------------------------TX/RX---------------------------------
    if (gotMail and rf69.available()) {   // If message available
      uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (rf69.recv(buf, &len)) {
        if (!len) return;
        buf[len] = 0;
        Serial.print("RX ["); Serial.print(len); Serial.print("]: "); Serial.print((char*)buf);
        Serial.print(" - RSSI: ");  Serial.println(rf69.lastRssi(), DEC);

        if (strstr((char *)buf, "Green") and gotMail) {  // Send a reply to Green!
          uint8_t data[] = "Red";
          rf69.send(data, sizeof(data));
          rf69.waitPacketSent();
          Serial.println("TX to Green, set Green LED on for 20 seconds, turn off all LEDs, halt TX, await new mail delivery");
          digitalWrite(GREEN, HIGH);
          delay(20000);
          digitalWrite(GREEN, LOW);
          digitalWrite(LED, LOW);
          gotMail = false;
        } else {
          Serial.println("Receive failed");
        }
      }
    }
  }

  //-----------------------------------------------------------------------------------------------------
  //     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
  //-----------------------------------------------------------------------------------------------------