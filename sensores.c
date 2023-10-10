#include <Arduino.h>
#include "MQTT_Client.h" //Arquivo com as funções de mqtt
#include <WiFi.h>
#include <PubSubClient.h> //Biblioteca para as publicações via mqtt
#include <Wire.h>

#define SDA1 21 // Pino SDA para o primeiro barramento I2C
#define SCL1 22 // Pino SCL para o primeiro barramento I2C
#define SDA2 23 // Pino SDA para o segundo barramento I2C
#define SCL2 18 // Pino SCL para o segundo barramento I2C

TwoWire I2Cone = TwoWire(0); // Cria o primeiro barramento I2C
TwoWire I2Ctwo = TwoWire(1); // Cria o segundo barramento I2C

#define WIFISSID "iPhone Ari" //Coloque seu SSID de WiFi aqui
#define PASSWORD "arv123456" //Coloque seu password de WiFi aqui
#define TOKEN "BBFF-INwAsEa8RG21RKI3AwSyf7GqzAknMG" //Coloque seu TOKEN do Ubidots aqui
#define VARIABLE_LABEL_TEMPERATURE_INT "VARIABLE_LABEL_TEMPERATURE_INT" //Label referente a variável de temperatura interna  criada no ubidots
#define VARIABLE_LABEL_HUMIDITY_INT "VARIABLE_LABEL_HUMIDITY_INT" //Label referente a variável de umidade interna criada no ubidots
#define VARIABLE_LABEL_TEMPERATURE_EXT "VARIABLE_LABEL_TEMPERATURE_EXT" //Label referente a variável de temperatura externa criada no ubidots
#define VARIABLE_LABEL_HUMIDITY_EXT "VARIABLE_LABEL_HUMIDITY_EXT" //Label referente a variável de umidade externa criada no ubidots
#define DEVICE_ID "651765120ffc07000d2baa6a" //ID do dispositivo (Device id, também chamado de client name)
#define SERVER "things.ubidots.com" //Servidor do Ubidots (broker)

//Porta padrão
#define PORT 1883

//Tópico aonde serão feitos os publish, "esp32-dht" é o DEVICE_LABEL
#define TOPIC "/v1.6/devices/control"

//Objeto WiFiClient usado para a conexão wifi
WiFiClient ubidots;
//Objeto PubSubClient usado para publish–subscribe
PubSubClient client(ubidots);

float tempInt; 
float umidInt;
float tempExt; 
float umidExt;

void reconnect() 
{  
  //Loop até que o MQTT esteja conectado
  while (!client.connected()) 
  {
    Serial.println("Attempting MQTT connection...");
    
    //Tenta conectar
    if (client.connect(DEVICE_ID, TOKEN,"")) 
      Serial.println("connected");
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      //Aguarda 2 segundos antes de retomar
      delay(2000);
    }
  }
}

bool mqttInit()
{
  //Inicia WiFi com o SSID e a senha
  WiFi.begin(WIFISSID, PASSWORD);
 
  //Loop até que o WiFi esteja conectado
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }
 
  //Exibe no monitor serial
  Serial.println("Connected to network");

  //Seta servidor com o broker e a porta
  client.setServer(SERVER, PORT);
  
  //Conecta no ubidots com o Device id e o token, o password é informado como vazio
  while(!client.connect(DEVICE_ID, TOKEN, ""))
  {
      Serial.println("MQTT - Connect error");
      return false;
  }

  Serial.println("MQTT - Connect ok");
  return true;
}

bool sendValues(float tempInt, float umidInt, float tempExt, float umidExt)
{
  
  tempInt = readTemperature(&I2Cone);
  umidInt = readHumidity(&I2Cone);
  tempExt = readTemperature(&I2Ctwo);
  umidExt = readHumidity(&I2Ctwo);

  char json[250];

  //Atribui para a cadeia de caracteres "json" os valores referentes a temperatura e os envia para a variável do ubidots correspondente
  sprintf(json,  "{\"%s\":{\"value\":%02.02f, \"context\":{\"tempExt\":%02.02f, \"umidExt\":%02.02f}}}", VARIABLE_LABEL_TEMPERATURE_EXT, tempExt, tempExt, umidExt);  

  if(!client.publish(TOPIC, json))
    return false;

  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  sprintf(json,  "{\"%s\":{\"value\":%02.02f, \"context\":{\"tempExt\":%02.02f, \"umidExt\":%02.02f}}}", VARIABLE_LABEL_HUMIDITY_EXT, umidExt, tempExt, umidExt);  
      
  if(!client.publish(TOPIC, json))
    return false;
 
  //Atribui para a cadeia de caracteres "json" os valores referentes a temperatura e os envia para a variável do ubidots correspondente
  sprintf(json,  "{\"%s\":{\"value\":%02.02f, \"context\":{\"tempInt\":%02.02f, \"umidInt\":%02.02f}}}", VARIABLE_LABEL_TEMPERATURE_INT, tempInt, tempInt, umidInt);  

  if(!client.publish(TOPIC, json))
    return false;

  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  sprintf(json,  "{\"%s\":{\"value\":%02.02f, \"context\":{\"tempInt\":%02.02f, \"umidInt\":%02.02f}}}", VARIABLE_LABEL_HUMIDITY_INT, umidInt, tempInt, umidInt);  
      
  if(!client.publish(TOPIC, json))
    return false;

  //Se tudo der certo retorna true
  return true;
}

void setup() 
{
  // Inicia os barramentos I2C
  I2Cone.begin(SDA1, SCL1, 400000); // Inicia o primeiro barramento I2C com velocidade de 400kHz
  I2Ctwo.begin(SDA2, SCL2, 400000); // Inicia o segundo barramento I2C com velocidade de 400kHz
  
  //Para debug, iniciamos a comunicação serial com 115200 bps
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);

  //Inicializa mqtt (conecta o esp com o wifi, configura e conecta com o servidor da ubidots)
  if(!mqttInit())
  {        
    delay(3000);
    Serial.println("Failed!");
    ESP.restart();
  }
  Serial.println("OK");
}


void loop() 
{
  tempInt = readTemperature(&I2Cone); // Somente para mostrar no monitor serial
  umidInt = readHumidity(&I2Cone); // Somente para mostrar no monitor serial
  tempExt = readTemperature(&I2Ctwo); // Somente para mostrar no monitor serial
  umidExt = readHumidity(&I2Ctwo); // Somente para mostrar no monitor serial

  Serial.print("Temperatura 1: "); Serial.print(tempInt); Serial.println(" *C");
  Serial.print("Umidade 1: "); Serial.print(umidInt); Serial.println(" %");
  Serial.print("Temperatura 2: "); Serial.print(tempExt); Serial.println(" *C");
  Serial.print("Umidade 2: "); Serial.print(umidExt); Serial.println(" %");

  delay(2000);

  //Se o esp foi desconectado do ubidots, tentamos reconectar
  if(!client.connected())
    reconnect();

  delay(2500);  
  if(sendValues(tempInt, umidInt, tempExt, umidExt))
  {      
    Serial.println("Successfully sent data");
  }
  else
  {      
    Serial.println("Failed to send sensor data");

  }    
    
  //Esperamos 2.5s para dar tempo de ler as mensagens acima
  delay(2500);    
}

float readTemperature(TwoWire *wire) {

  wire->beginTransmission(0x40); // Endereço do HTU21D
  wire->write(0xE3); // Comando para ler temperatura
  wire->endTransmission();
  delay(50); // Tempo para conversão
  
  wire->requestFrom(0x40, 2); // Solicita 2 bytes (MSB e LSB)
  if (wire->available() < 2) return NAN; // Retorna NaN se não receber os 2 bytes
  
  uint16_t rawTemp = wire->read() << 8 | wire->read();
  return -46.85 + 175.72 * (rawTemp / 65536.0);
}

float readHumidity(TwoWire *wire) {

  wire->beginTransmission(0x40); // Endereço do HTU21D
  wire->write(0xE5); // Comando para ler umidade
  wire->endTransmission();
  delay(50); // Tempo para conversão
  
  wire->requestFrom(0x40, 2); // Solicita 2 bytes (MSB e LSB)
  if (wire->available() < 2) return NAN; // Retorna NaN se não receber os 2 bytes
  
  uint16_t rawHum = wire->read() << 8 | wire->read();
  return -6.0 + 125.0 * (rawHum / 65536.0);
}