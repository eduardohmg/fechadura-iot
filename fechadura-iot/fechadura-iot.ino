#include <SPI.h>
#include <MFRC522.h>

//#include <Wire.h>
#include "SSD1306Wire.h"
#include <ESP8266WiFi.h>

#include <FirebaseArduino.h>

#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""

#define WIFI_SSID ""
#define WIFI_PASS ""

#define SS_PIN 2 // D4
#define RST_PIN 0 // D3

#define RELAY_PIN 15 // D8

SSD1306Wire display(0x3c, D1, D2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setupDisplay() {
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.clear();
}

void setupWifi() {
  Serial.print("WIFI Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWIFI Connected");
}

void setupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
 
void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  setupDisplay();
  setupWifi();
  setupFirebase();

  if (Firebase.failed()) {
      Serial.print("Initial error: ");
      Serial.println(Firebase.error());
  }
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  verifying();
  
  //Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  Serial.println("");
  conteudo.toUpperCase();
  String rfid = conteudo.substring(1);

  String user = Firebase.getString(rfid);

  if (Firebase.failed()) {
      Serial.print("error: ");
      Serial.println(Firebase.error());
      notAuthorized();
  }
  else if(user.length() == 0) {
      notAuthorized();
  }
  else {
    hello(user);
    digitalWrite(RELAY_PIN, HIGH);
    delay(2000);
    digitalWrite(RELAY_PIN, LOW);
  }
}

void hello(String name) {
  display.clear();
  display.drawString(63, 10, "Olá,");
  display.drawString(63, 26, name);
  display.display();
}

void notAuthorized() {
  display.clear();
  display.drawString(63, 10, "Não");
  display.drawString(63, 26, "Autorizado");
  display.display();
}

void verifying() {
  display.clear();
  display.drawString(63, 10, "Verificando");
  display.drawString(63, 26, "...");
  display.display();
}
