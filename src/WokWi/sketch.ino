#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiClientSecure.h>

// Define sensor pins
#define DHTPIN 14         // DHT22 Sensor Pin
#define DHTTYPE DHT22     // DHT22 Sensor Type
#define TURBIDITY_PIN 35   // Analog for turbidity
#define PH_PIN 36          // Analog pH sensor

// Initialize sensors
DHT dht(DHTPIN, DHTTYPE);

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// WiFi and MQTT configurations
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqttServer = "9fbc1787c51542aca42e34cc0bb2fbcd.s1.eu.hivemq.cloud";
const int mqttPort = 8883;
const char* mqttusername = "KarkheDam";
const char* mqttpassword = "Password1";

WiFiClientSecure espClient;
PubSubClient client(espClient);

bool openDam = false; // Variable to store dam status
float waterLevel = 220;

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected to Wokwi-GUEST!");
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT Broker at ");
    Serial.print(mqttServer);
    Serial.print(":");
    Serial.println(mqttPort);

    if (client.connect("ESP32Client", mqttusername, mqttpassword)) {
      Serial.println(" Connected to MQTT Broker!");
      client.subscribe("KarkheDam_Simulation/openDam");
    } else {
      Serial.print(" Failed to connect, state: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void publishData(const char* topic, float value) {
  char payload[50];
  sprintf(payload, "%.2f", value);
  if (client.publish(topic, payload)) {
    Serial.printf("Published %s: %.2f\n", topic, value);
  } else {
    Serial.printf("Failed to publish %s\n", topic);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (String(topic) == "KarkheDam_Simulation/openDam") {
    openDam = (message == "true");
    Serial.printf("Dam control received: %s\n", openDam ? "Open" : "Closed");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectToWiFi();
  espClient.setCACert(root_ca);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  connectToMQTT();
}
void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // Read sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pH = analogRead(PH_PIN) > 0 ? analogRead(PH_PIN) / 4095.0 * 0.7 + 7.5 : random(75, 82) / 10.0;
  int turbidityRaw = analogRead(TURBIDITY_PIN);
  // Map the raw analog value to NTU (Nephelometric Turbidity Units)
  float turbidity = (float)turbidityRaw / 4095.0 * 400.0; // Scale to a max of 400 NTU

  // Adjust turbidity based on other parameters
  if (temperature > 30) turbidity += 20;
  if (pH < 7.0 || pH > 8.0) turbidity += 15;
  if (humidity > 80) turbidity -= 10;
  if (waterLevel > 260) turbidity += 25;
  turbidity = fmax(fmin(turbidity, 400), 50); // Clamp turbidity between 50 and 400 NTU

  // Rainfall logic: dynamically adjust attributes if rainfall occurs
  if (temperature < 18) {
    float rainfallIncrease = random(5, 15) * 0.1f; // Increase by a small, random value
    waterLevel = fmin(waterLevel + rainfallIncrease, 280.0f); // Clamp water level to a max of 280 meters
    turbidity -= turbidity * 0.1;                             // Reduce turbidity by 10%
    pH += (7.5 - pH) * 0.1;                                  // Adjust pH towards neutral (7.5)
    humidity += (80 - humidity) * 0.1;                       // Gradually increase humidity towards 80%
  }

  // Calculate inflow and outflow rates dynamically
  float inflowRate = (humidity > 60) ? random(300, 400) : random(100, 250);
  float outflowRate = openDam ? random(400, 500) : random(100, 250); // Higher outflow if dam is open

  // Calculate actual water level
  waterLevel += (inflowRate - outflowRate) * 0.05; // Adjust scale for simulation realism
  waterLevel = fmax(fmin(waterLevel, 280.0f), 160.0f); // Clamp water level between 160 and 280 meters

  // Close dam if water level is below 160 meters
  if (waterLevel <= 185) {
    openDam = false;
    outflowRate = random(100, 250); // Reset to default outflow rate
  }

  // Calculate flood probability
  float floodProbability = 0;
  if (waterLevel > 260) floodProbability += 50;
  if (inflowRate > 300) floodProbability += 30;
  if (turbidity > 300) floodProbability += 20;
  floodProbability = std::min(floodProbability, 100.0f);
  bool floodWarning = floodProbability > 60;

  // Debugging output
  Serial.printf("Temperature: %.2f\n", temperature);
  Serial.printf("Humidity: %.2f\n", humidity);
  Serial.printf("Water Level: %.2f\n", waterLevel);
  Serial.printf("Turbidity: %.2f\n", turbidity);
  Serial.printf("Flood Probability: %.2f\n", floodProbability);
  Serial.printf("Flood Warning: %s\n", floodWarning ? "true" : "false");

  // Publish data to MQTT topics
  publishData("KarkheDam_Simulation/temperature", temperature);
  publishData("KarkheDam_Simulation/pH", pH);
  publishData("KarkheDam_Simulation/humidity", humidity);
  publishData("KarkheDam_Simulation/waterLevel", waterLevel);
  publishData("KarkheDam_Simulation/turbidity", turbidity);
  publishData("KarkheDam_Simulation/floodProbability", floodProbability);
  publishData("KarkheDam_Simulation/floodWarning", floodWarning ? 1 : 0);
  publishData("KarkheDam_Simulation/inflowRate", inflowRate);
  publishData("KarkheDam_Simulation/outflowRate", outflowRate);

  delay(5000); // Publish every 5 seconds
}
