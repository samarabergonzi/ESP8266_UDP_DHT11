#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>

const char* ssid = "ESP8266_AP";     // Nome da rede WiFi que o ESP8266 vai criar
const char* password = "12345678";   // Senha da rede WiFi

WiFiUDP udp;
unsigned int localUdpPort = 4210;    // Porta UDP que o ESP8266 vai escutar
char incomingPacket[255];            // Buffer para os dados recebidos
char replyPacket[] = "Dados recebidos";  // Resposta que será enviada de volta

#define DHTPIN D4  // Pino conectado ao DHT11
#define DHTTYPE DHT11  // Tipo do sensor DHT11

DHT dht(DHTPIN, DHTTYPE);  // Inicializa o sensor DHT

float temperatura;  // Variável para armazenar a temperatura
float umidade;      // Variável para armazenar a umidade

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configura o ESP8266 como ponto de acesso
  WiFi.softAP(ssid, password);
  
  // Exibe o IP do Access Point
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Access Point iniciado. IP do AP: ");
  Serial.println(myIP);

  // Inicia o servidor UDP
  udp.begin(localUdpPort);
  Serial.printf("Servidor UDP iniciado na porta %d\n", localUdpPort);
}

void loop() {
  int packetSize = udp.parsePacket();  // Verifica se há pacotes disponíveis
  if (packetSize) {
    // Recebe o pacote UDP
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Adiciona o terminador nulo ao final
    }
    Serial.printf("Pacote UDP recebido: %s\n", incomingPacket);
    Serial.printf("De IP: %s, Porta: %d\n", udp.remoteIP().toString().c_str(), udp.remotePort());

    // Se o pacote recebido for "REQUEST_DATA", envia temperatura e umidade
    if (String(incomingPacket) == "REQUEST_DATA") {
      // Lê a temperatura e a umidade
      temperatura = dht.readTemperature();
      umidade = dht.readHumidity();

      // Formata os dados a serem enviados de volta
      String response = "Temperatura: " + String(temperatura) + " C, Umidade: " + String(umidade) + " %";

      // Envia a resposta de volta ao cliente
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write(response.c_str());
      udp.endPacket();
    } else {
      // Se outro comando for recebido, envia uma resposta genérica
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write(replyPacket);
      udp.endPacket();
    }
  }
}
