/*
 * #  RFID 3.3v
 *      MISO: D6
 *      MOSI: D7
 *      SCK:  D5
 *      SDA:  RFID_SDA_PIN
 *      RST:  RFID_RST_PIN
 * 
 * #  LED
 *      Vermelho: LED_VRM_PIN
 *      Verde:    LED_VRD_PIN
 *      
 * #  SERVO
 *      Servo:    SERVO_PIN
 */

#define __ASSERT_USE_STDERR
 
#include <assert.h>
#include <ESP8266WiFi.h>
#include <Servo.h> 
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>

 
/* ## Configuracões da rede Wifi ## */
 
const char* ssid     = "WiFi-SSID";
const char* password = "WiFi-Senha";

/* ## Configuracões do Servidor ## */

const char* SERVER_URL = "http://Server-URL:8000/check/";

HTTPClient http;

/* ## Configuracões do leitor RFID ## */

#define RFID_SDA_PIN D4
#define RFID_RST_PIN D3

MFRC522 mfrc522(RFID_SDA_PIN, RFID_RST_PIN);

/* ## Configuracões dos LEDs ## */

#define LED_VRM_PIN D0
#define LED_VRD_PIN D1

/* ## Configuracões do Servo ## */

#define SERVO_PIN D8

Servo myservo;

/* ## Setup ## */

void setup() {
  Serial.begin(115200);        
  conectarWifi();
  setupRfid();
  setupLed();
  setupServo();
}
 
void conectarWifi() {
  WiFi.persistent(false);       // Workaround bug da conexão Wifi da placa ESP Wemos - NÃO APAGAR!
  delay(10);
 
  Serial.print("\nConectando a rede wifi...");
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) { // Aguarda a conexão ser feita
    delay(1000);
    Serial.print('.');
  }
 
  Serial.println("\nConexão estabelecida com sucesso!");  
  Serial.print("IP:\t");
  Serial.println(WiFi.localIP());
}
 
void setupRfid() {
  Serial.println("Iniciando leitor RFID...");
  
  SPI.begin();
  mfrc522.PCD_Init();
  
  Serial.println("Leitor RFID iniciado com sucesso!");
}

void setupLed() {
  pinMode(LED_VRM_PIN, OUTPUT);
  digitalWrite(LED_VRM_PIN, LOW);
  
  pinMode(LED_VRD_PIN, OUTPUT);
  digitalWrite(LED_VRD_PIN, LOW);
}

void setupServo() {
  myservo.attach(SERVO_PIN);
  myservo.write(180);
}

/* ## Loop ## */

String lerIdCartao() {
  Serial.println("Lendo cartão RFID...");
  int lidoComSucesso = mfrc522.PICC_ReadCardSerial();
 
  assert(lidoComSucesso);
 
  String cartaoId = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cartaoId.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    cartaoId.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
 
  cartaoId.toUpperCase();
  cartaoId.trim();
 
  mfrc522.PICC_HaltA();
 
  return cartaoId;
}
 
int checarPassagem(String cartaoId) {
  String urlCheck = SERVER_URL + cartaoId + "/";
  Serial.println(urlCheck);
  
  http.begin(urlCheck);
  int httpCode = http.GET();
  http.end();
  
  return httpCode == 200;
}

void liberarPassagem() {
  Serial.println("Passagem Liberada!");
  
  digitalWrite(LED_VRD_PIN, HIGH);
  myservo.write(90);
  delay(5000);
  
  digitalWrite(LED_VRD_PIN, LOW);
  delay(2000);
  
  myservo.write(180);
}

void negarPassagem() {
  Serial.println("Passagem Negada!");
  
  digitalWrite(LED_VRM_PIN, HIGH);
  delay(4000);
  
  digitalWrite(LED_VRM_PIN, LOW);
}

void loop() {
  if (! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
 
  String cartaoId = lerIdCartao();
  Serial.print("Cartão ID: ");
  Serial.println(cartaoId);
 
  int passagemPermitida = checarPassagem(cartaoId);
  
  if (passagemPermitida) {
    liberarPassagem();
  } else {
    negarPassagem();
  }
}
 
 
/* Para a execução do programa se alguma asserção falhar */
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
   
    abort();
}
