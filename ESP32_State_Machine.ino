#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MFRC522.h>
#include <Wire.h>

/*Pinout 
zone1 -D34
RC522 - SDA -- D16
RC522 - SCK -- D18
RC522 - MOSI - D23
RC522 - MISO - D19
RC522 - IRQ - Not connected
RC522 - GND - G
RC522 - RST -- D4
RC522 - 3.3v - V

NOKIA 5110 - RST -- D25
NOKIA 5110 - CE -- D26
NOKIA 5110 - DC -- D27
NOKIA 5110 - DIN -- MOSI -- D13
NOKIA 5110 - CLK -- D14
NOKIA 5110 - VCC -- V
NOKIA 5110 - LIGHT -- Not connected
NOKIA 5110 - GND -- G
*/

#define SS_PIN 16 
#define RST_PIN 4 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
bool uid = 0;
bool armed = 0; 
bool zone1 = 0;
bool zone2 = 0;
bool zone3 = 0;
bool toneBuzzer = 0;
bool siren = 0;
unsigned long startMillis; 
unsigned long currentMillis;
unsigned long timerValue;
bool latch;
const unsigned long exitDelay = 15000; // Exit delay
const unsigned long disableDelay = 10000; // Entry delay
const unsigned long alarmDelay = 240000; // Stop siren after delay

enum {systemReady, systemExit, systemArmed, systemDelayed, systemIntrusion}; // system states
unsigned char systemState;
String State;

/* Declare LCD object for SPI
 Adafruit_PCD8544(CLK,DIN,D/C,CE,RST); */
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 27, 26, 25);
int contrastValue = 60; // Default Contrast Value
const int adcPin = 34;
int adcValue = 0;

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(34, INPUT);
  Serial.begin(115200);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  /* Initialize the Display*/
  display.begin();
  /* Change the contrast using the following API*/
  display.setContrast(contrastValue);
  /* Clear the buffer */
  display.clearDisplay();
  display.display();
  delay(1000);

  /* Now let us display some text */
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,1);
  display.setTextSize(2);
  display.println("|ESP32|");
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(22,20);
  display.println("|Nokia|");
  display.setCursor(22,32);
  display.println("|5110|");
  display.display();
  delay(2000);

}

void zones(){
  int frontDoor = digitalRead(34);
  if (frontDoor== 1){
    zone1 = 1;
   }
  else if (frontDoor==0) {
    zone1 = 0;
  }
  display.clearDisplay(); display.setCursor(0,1); 
  display.println(State); 
  display.print("Zone 1: "); display.println(zone1); 
  display.print("Alarme: "); display.println(siren);
  display.print("Buzzer: "); display.println(toneBuzzer);
  display.print("Armed: "); display.println(armed);
  display.print("Timer "); display.println(timerValue);
  display.display();
}


void TCA9548A(uint8_t bus)
{
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.println("TCA9548A");
}

void rfid() {
  TCA9548A(0);
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();
  Serial.print("Identification... "); Serial.println(content); // Open serial monitor to READ the UID
  display.clearDisplay(); display.setCursor(0,4); display.println("UID..."); display.println(); display.println("APPROVED"); display.display();
  

  if (content.substring(1) == "56 AC FA 1F") //Main access
  { uid=1;}
  if (content.substring(1) == "C4 A6 23 D5") //change here the UID of the card/cards that you want to give access
  { uid=1;}
  if (content.substring(1) == "7B 52 24 D5") //change here the UID of the card/cards that you want to give access
  { uid=1;}
  else { }

  if (uid == 1) {
    Serial.println("Carte lue");
    Serial.print("armed: "); Serial.println(armed);
    if (armed == 1 ){
      armed = 0;
      Serial.print("armed = 0");
      delay(2000);  
    }
      else if (armed == 0){
      armed = 1;
      Serial.print("armed = 1");
      delay(2000);
    }
    uid=0;
  }
}



void timer(int state) {
  if (state==0){
   currentMillis = millis();
   startMillis = millis();
   timerValue = 0; 
  }
  if (state==1){
    if (latch==0){
      startMillis = millis();
      latch = 1;
    }
  currentMillis = millis();
  timerValue = currentMillis - startMillis;
  }
}

void loop()
{
  switch (systemState) {
    case systemReady: // Waiting
      State = "SystemReady";
      Serial.println(State); //delay(1000);
      zones();
      rfid();
      timer(0); // Reset timer
      toneBuzzer = 0; siren = 0;
      if ((armed==1) && (zone1==1)){
        systemState = systemExit;
        break;
      }
      if ((armed==1) && (zone1==0)) {
        toneBuzzer = 1; siren = 0;
        display.clearDisplay(); display.setCursor(0,1); display.println("Zone ouverte"); display.display();
        delay(2000);
        break;
      }
      else {
        break;
      }

    case systemExit: // Exit delay
      State = "SystemExit";
      Serial.println(State); //delay (1000);
      rfid();
      zones();
      timer(1); // Start timer
      Serial.println("debug0");
      if (armed==0) {
        systemState = systemReady; Serial.println("debug2");
        break;
      }
      if (timerValue<exitDelay) {
        toneBuzzer=1; siren=0; Serial.println("debug1");
      }
      if ((armed==1) && (timerValue>=exitDelay)){
        systemState = systemArmed;
        break;
      }
      else {
        break;
      }
      
    case systemArmed: // Armed
      State = "SystemArmed";
      Serial.println(State); //delay (1000);
      zones();
      timer(0); // Reset timer
      rfid();
      toneBuzzer=0; siren=0;
      if ((armed==1) && (zone1==0)) { //Main door is opened
        //wifiMessage(1); //zone 1
        systemState = systemDelayed; Serial.println("Main door is opened");
        break;
      }
      /*
      if ((armed==1) && (zone2==0)) { //patio door is opened
        wifiMessage(2); //zone 2
        systemState = systemIntrusion;
        break;
      }
      if((armed==1) && (zone3==0)) { //IR sensor detection
        wifiMessage(3); //zone 3
        systemState = systemIntrusion;
        break;
      }
      */
      else {
        break;
      }

    case systemDelayed: // Front door intrusion delay
      State = "SystemDelayed";
      Serial.println(State); //delay (1000);
      zones();
      timer(1); // Start timer
      rfid();
      if (armed==0) {
        systemState = systemReady;
        break;
      }
      if (timerValue<disableDelay) {
        toneBuzzer=1; siren=0;
      }
      if ((armed==1) && (timerValue>=disableDelay)){
        timer(0); // Reset timer
        systemState = systemIntrusion;
        break;
      }
      else {
        break;
      }

    case systemIntrusion: // Siren activated
      State = "Intrusion";
      Serial.println(State); //delay (1000);
      zones();
      timer(1); // Start timer
      rfid();
      toneBuzzer=1; siren=1;
      // wifiMessage(4) //Siren activated
      if ((armed==1) && (timerValue<alarmDelay)){
        break;
      }
      else if ((armed==1) && (timerValue>=alarmDelay)) {
      systemState = systemArmed; // stop siren after intrusion delay
      }
      else if (armed==0) {
      systemState = systemReady;
      break;
      }
      else {
        break;
      }

    default:
      systemState = systemReady;
      break;
  } 
}