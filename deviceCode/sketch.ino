// Incluir bibliotecas
#include <WiFi.h> // https://www.arduinolibraries.info/libraries/wi-fi
#include <WiFiClientSecure.h> // Biblioteca para conexões seguras via TLS/SSL
#include <PubSubClient.h> // https://www.arduinolibraries.info/libraries/pub-sub-client
#include <DHTesp.h> // https://www.arduinolibraries.info/libraries/dht-sensor-library-for-es-px

// Definicoes e constantes
char SSIDName[] = "Wokwi-GUEST"; // nome da rede WiFi
char SSIDPass[] = ""; // senha da rede WiFI

const int DHT_PIN = 15; // terminal do sensor de temperatura e umidade

char BrokerURL[] = "8705b916cfd244dbbbd48c1f80e4d71a.s1.eu.hivemq.cloud"; // URL do broker MQTT
char BrokerUserName[] = "iot"; // nome do usuario para autenticar no broker MQTT
char BrokerPassword[] = "1a2b3c"; // senha para autenticar no broker MQTT
char MQTTClientName[] = "ESP32_Client_01"; // nome do cliente MQTT
int BrokerPort = 8883; // porta do broker MQTT

char Topico_01[] = "TemperaturaUmidade"; 

WiFiClientSecure espClient;
PubSubClient clienteMQTT(espClient);
DHTesp dhtSensor; 


void mqttReconnect() {
  while (!clienteMQTT.connected()) {
    Serial.println("Conectando-se ao broker MQTT...");
    Serial.println(MQTTClientName);
    if (clienteMQTT.connect(MQTTClientName, BrokerUserName, BrokerPassword)) {
      Serial.print(MQTTClientName);
      Serial.println(" conectado!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(" Tente novamente em 5 segundos.");
      delay(5000);
    }
  }
}

// Setup
void setup() {
  Serial.begin(9600);

  Serial.print("Conectando-se ao Wi-Fi");
  WiFi.begin(SSIDName, SSIDPass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure(); 
  clienteMQTT.setServer(BrokerURL, BrokerPort);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22); 
  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  if (!clienteMQTT.connected()) {
    mqttReconnect(); 
  }
  clienteMQTT.loop();

  TempAndHumidity temp_umid = dhtSensor.getTempAndHumidity(); 
  float temperatura = temp_umid.temperature;
  float umidade = temp_umid.humidity;

  // Criar o JSON com temperatura e umidade
  String jsonPayload = "{\"t\":\"" + String(temperatura, 1) + "\",\"u\":\"" + String(umidade, 1) + "\"}";

  clienteMQTT.publish(Topico_01, jsonPayload.c_str());
  Serial.print("Dados publicados: ");
  Serial.println(jsonPayload);

  delay(1000);
}
