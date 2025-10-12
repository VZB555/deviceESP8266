#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h> // Librairie ArduinoJson
#include <DHT.h>
#define VERSION "1.1.4"
#define DURATION_PING 300000

bool Command_FirmwareUpgrade = false;
bool b_SendAcknowledgement = false;

float temperature;
float humidity;

unsigned long Last_Time_ping = 0;
unsigned long Last_etat_sent = 0;
unsigned long Last_Time_receivedfromServer=0;

int etat = LOW;

const char* ssid = "SFR_11B0";
const char* password = "nxdf9ujc69aydp2amsr3";


String mac;
String Str_SocketAcknoledgement = "";

const int pinEntree = D6;  // ou directement 14

// --- DHT ---
#define DHTPIN D7       // Pin du capteur DHT
#define DHTTYPE DHT22   // DHT11 ou DHT22 selon ton capteur
//DHT dht(DHTPIN, DHTTYPE);

WebSocketsClient webSocket;
volatile bool signalDetecte = false; // flag mis à jour par l'interruption

void IRAM_ATTR detectSignal() {
  signalDetecte = true;
}


void envoi(){
  if (webSocket.isConnected()) {

 //   temperature = dht.readTemperature(); // °C
 //   humidity = dht.readHumidity();

    StaticJsonDocument<300> doc;
    doc["type"] = "sensor_update";
    doc["ring"] = etat;
    doc["Ack"] = Str_SocketAcknoledgement;
    doc["mac"] = mac;
    doc["V"] = VERSION;

    String jsonMessage;
    
    serializeJson(doc, jsonMessage);

    // Envoi au serveur
    webSocket.sendTXT(jsonMessage);

    Serial.println("Payload envoyé");
    
    Last_Time_ping = millis();
    if (etat == HIGH) Last_etat_sent = millis();
    if (b_SendAcknowledgement == true ) {Str_SocketAcknoledgement =""; b_SendAcknowledgement = false;}
  } 
  else {
       Last_Time_ping = 0;
       Serial.println("WebSocket non connecté, en attente de reconnexion automatique...");
//       connexion(); 
//       delay(1000);
  }


}

void connexion(){
  webSocket.beginSSL("server-chj7.onrender.com", 443, "/");
  webSocket.setReconnectInterval(5000);
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket connecté !");
      break;

    case WStype_TEXT:
      Serial.printf("Message reçu: %s\n", payload);

      // Crée un buffer mémoire pour stocker l'arbre JSON
      StaticJsonDocument<300> doc2;

      // Parse la chaîne
      DeserializationError error = deserializeJson(doc2, payload);

      // Vérifie les erreurs
      if (error) {
        Serial.print(F("Erreur de parsing: "));
        Serial.println(error.f_str());
        break; 
      }
  
      // Récupère les champs
      const char* type =    doc2["type"]; 
      const char* payload  = doc2["payload"];

      // Affiche les résultats
      Serial.print("Type : ");    Serial.println(type);
      Serial.print("payload : "); Serial.println(payload);


      if (strcmp(type,"command") == 0) { Serial.println("***** RECEPTION DU SERVEUR ********** "); Last_Time_receivedfromServer = millis(); }

      if (strcmp(payload,"FIRM_UPG") == 0) { Command_FirmwareUpgrade = true; b_SendAcknowledgement = true; Str_SocketAcknoledgement = "FIRM_UPG"; Serial.println("FIRMWARE UPGRADE RECEIVED FROM BROWSER");}

      break;
  
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
  }
  Serial.println("\nWi-Fi connecté !");
  mac=WiFi.macAddress();
  Serial.println(mac);
//  dht.begin();
//  pinMode(pinEntree, INPUT);  // ou INPUT_PULLUP selon ton montage


  pinMode(pinEntree, INPUT); // Entrée avec résistance interne
  attachInterrupt(digitalPinToInterrupt(pinEntree), detectSignal, RISING);
  // Modes possibles : RISING, FALLING, CHANGE, ONLOW, ONHIGH

  connexion(); 
//  delay(1000);
//  webSocket.loop();
//  envoi();

}




void loop() {
  webSocket.loop();

  if (Command_FirmwareUpgrade) {envoi(); FOTA_Check();} 

  if (signalDetecte) {
    signalDetecte = false;
    etat = digitalRead(pinEntree);
    if (etat == HIGH) {
      Serial.println("HIGH");
      if ( (millis() - Last_etat_sent) > 3000 ) 
           envoi();
    }     
  }


 if ( (millis() - Last_Time_ping) > DURATION_PING ) {envoi();}

 if ( (millis() - Last_Time_receivedfromServer) > (DURATION_PING + 2000) )
  {
    Serial.println("** TIMEOUT RECEPTION DU SERVER ** ");
    // connexion(); 
    // delay(1000); 
    envoi(); 
  }

 if (b_SendAcknowledgement) 
  envoi();     

}
