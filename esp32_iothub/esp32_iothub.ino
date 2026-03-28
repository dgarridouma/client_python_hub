#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID     = "YOURSSID";
const char* WIFI_PASSWORD = "***";

const char* IOT_HUB_HOST  = "YOURHUB.azure-devices.net";
const char* DEVICE_ID     = "YOURDEVICE";
const char* SAS_TOKEN     = "SharedAccessSignature sr=...";
const int   MQTT_PORT     = 8883;
const char* MQTT_USERNAME = "YOURHUB.azure-devices.net/YOURDEVICE/?api-version=2021-04-12";

// Periodo de envío (segundos)
int period = 10;

char telemetryTopic[128];
char c2dTopic[128];

const char* ROOT_CA =
"-----BEGIN CERTIFICATE-----\n"
"...DigiCert Global Root G2...\n"
"-----END CERTIFICATE-----\n";

WiFiClientSecure wifiClient;
PubSubClient     mqttClient(wifiClient);

// Timestamp via NTP
void syncNTP() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Sincronizando NTP");
  time_t now = time(nullptr);
  while (now < 1000000000L) { delay(500); Serial.print("."); now = time(nullptr); }
  Serial.println(" OK");
}

void getTimestamp(char* buf, size_t len) {
  time_t now = time(nullptr);
  struct tm* t = gmtime(&now);
  strftime(buf, len, "%Y-%m-%dT%H:%M:%SZ", t);
}

// Handler C2D
// Espera: {"period": 5, "message": "period changed"}
void message_handler(char* topic, byte* payload, unsigned int length) {
  Serial.println("\nMensaje recibido desde la nube:");

  char message[256];
  memcpy(message, payload, min(length, sizeof(message) - 1));
  message[length] = '\0';

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, message)) {
    Serial.println("Error al parsear JSON");
    return;
  }

  period = doc["period"] | period;          // actualiza periodo si viene el campo
  const char* msg = doc["message"] | "";    // imprime el mensaje si viene
  if (strlen(msg) > 0) Serial.println(msg);
}

// WiFi
void connectWiFi() {
  Serial.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" OK");
}

// MQTT
void connectMQTT() {
  wifiClient.setCACert(ROOT_CA);
  mqttClient.setServer(IOT_HUB_HOST, MQTT_PORT);
  mqttClient.setCallback(message_handler);
  mqttClient.setBufferSize(1024);

  snprintf(telemetryTopic, sizeof(telemetryTopic),
    "devices/%s/messages/events/$.ct=application%%2Fjson&$.ce=utf-8", DEVICE_ID);
  snprintf(c2dTopic, sizeof(c2dTopic),
    "devices/%s/messages/devicebound/#", DEVICE_ID);

  Serial.print("Conectando a IoT Hub");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(DEVICE_ID, MQTT_USERNAME, SAS_TOKEN)) {
      Serial.println(" OK");
      mqttClient.subscribe(c2dTopic);
    } else {
      Serial.print(" Error: "); Serial.println(mqttClient.state());
      delay(3000);
    }
  }
}

// Telemetría
void sendTelemetry() {
  int temperature = random(25, 30);
  int humidity    = random(50, 100);
  int pressure    = random(900, 1100);
  char when[32];
  getTimestamp(when, sizeof(when));

  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"]    = humidity;
  doc["pressure"]    = pressure;
  doc["when"]        = when;

  char payload[200];
  serializeJson(doc, payload);

  Serial.print(temperature); Serial.print(" ");
  Serial.print(humidity);    Serial.print(" ");
  Serial.println(pressure);

  mqttClient.publish(telemetryTopic, payload);
}

// Setup / Loop
void setup() {
  Serial.begin(115200);
  connectWiFi();
  syncNTP();
  connectMQTT();
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();   // procesa mensajes C2D entrantes
  sendTelemetry();
  delay(period * 1000);
}
