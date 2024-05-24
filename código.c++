#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

#define PUBLISH_DELAY 500

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

// Configurações de WiFi
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";  // Substitua pelo sua senha

// Configurações de MQTT
const char *BROKER_MQTT = "broker.hivemq.com";
const int BROKER_PORT = 1883;
const char *ID_MQTT = "BEA_mqtt";
const char *TOPIC_SUBSCRIBE_BUTTON = "fiap/iot/melody";

const char* tagoToken = "3b478277-a9bf-489a-8507-b94736b12ad5";
const char* tagoEndpoint = "https://api.tago.io/data";

// Variáveis globais
WiFiClient espClient;
PubSubClient MQTT(espClient);
unsigned long publishUpdate = 0;
const int TAMANHO = 200;

// Protótipos de funções
void updateSensorValues();
void initWiFi();
void initMQTT();
void callbackMQTT(char *topic, byte *payload, unsigned int length);
void reconnectMQTT();
void reconnectWiFi();
void checkWiFIAndMQTT();

void initWiFi() {
  Serial.print("Conectando com a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso: ");
  Serial.println(SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(callbackMQTT);
}

void callbackMQTT(char *topic, byte *payload, unsigned int length) {
  String msg = String((char*)payload).substring(0, length);
  
  Serial.printf("Mensagem recebida via MQTT: %s do tópico: %s\n", msg.c_str(), topic);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar com o Broker MQTT: ");
    Serial.println(BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
      MQTT.subscribe(TOPIC_SUBSCRIBE_BUTTON);
    } else {
      Serial.println("Falha na conexão com MQTT. Tentando novamente em 2 segundos.");
      delay(2000);
    }
  }
}

void checkWiFIAndMQTT() {
  if (WiFi.status() != WL_CONNECTED) reconnectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

void reconnectWiFi(void) {
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wifi conectado com sucesso");
  Serial.print(SSID);
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

int b1 = 16;
int b2 = 21;
int b3 = 15;
int e1 = 0;
int e2 = 0;
int e3 = 0;
int led_amarelo = 32;
int led_verde = 33;
int trava = 0;
int tempo = 140;
int buzzer = 22;

int melody[] = {
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-4, NOTE_E5,8, NOTE_FS5,4,
  NOTE_D5,-2,
};

int notes = sizeof(melody) / sizeof(melody[0]) / 2;

int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;

void setup(){
  pinMode(b1, INPUT);
  pinMode(b2, INPUT);
  pinMode(b3, INPUT);
  pinMode(led_amarelo, OUTPUT);
  pinMode(led_verde, OUTPUT);

  initWiFi();
  initMQTT();
  
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    
    divider = melody[thisNote + 1];
    if (divider > 0) {
      
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; 
    }

    tone(buzzer, melody[thisNote], noteDuration * 0.9);

    delay(noteDuration);
    noTone(buzzer);
  }

  Serial.begin(115200);
}

void loop (){
  
  checkWiFIAndMQTT();
  MQTT.loop();
 
  e1 = digitalRead(b1);
  e2 = digitalRead(b2);
  e3 = digitalRead(b3);

  if((e1 == 1) && (trava == 0)){
    StaticJsonDocument<TAMANHO> doc;
    digitalWrite(led_amarelo, 1);
    trava = 1;

    String message = "Time Amarelo respondendo!";
    Serial.println(message);
    doc["message"] = message;
    sendToTago(message);

    char buffer[TAMANHO];
    serializeJson(doc, buffer);
    MQTT.publish(TOPIC_SUBSCRIBE_BUTTON, buffer);

  }
  if((e2 == 1) && (trava == 0)){
    StaticJsonDocument<TAMANHO> doc;
    digitalWrite(led_verde, 1);
    trava = 1;

    String message = "Time Verde respondendo!";
    Serial.println(message);
    doc["message"] = message;
    sendToTago(message);

    char buffer[TAMANHO];
    serializeJson(doc, buffer);
    MQTT.publish(TOPIC_SUBSCRIBE_BUTTON, buffer);
  }
  if(e3 == 1 ){
    StaticJsonDocument<TAMANHO> doc;
    digitalWrite(led_amarelo, 0);
    digitalWrite(led_verde, 0);
    trava = 0;
    
    String message = "Jogo resetando...";
    Serial.println(message);
    doc["message"] = message;
    sendToTago(message);

    char buffer[TAMANHO];
    serializeJson(doc, buffer);
    MQTT.publish(TOPIC_SUBSCRIBE_BUTTON, buffer);
  }
}

void sendToTago(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(tagoEndpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device-Token", tagoToken);

    String jsonData = "{\"variable\": \"message\", \"value\": \"" + message + "\"}";
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
   
