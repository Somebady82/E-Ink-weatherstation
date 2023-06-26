// homeassistant/sensor/esp32-woonkamer/<sensor_type>/config
// Sensor type: temperature, humidity, voc, co2, etc
// {
//     "device_class": "temperature",
//     "enabled_by_default": true,
//     "name": "Meterkast Temperatuur",
//     "state_class": "measurement",
//     "state_topic": "home/sensor/temp-meterkast",
//     "unique_id": "esp_temp_meterkast_temperature",
//     "unit_of_measurement": "°C",
//     "value_template": "{{ value_json.temperature }}"
// }

#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "sensor_data.h"

class MQTT {
  private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    const char* stateTopic;
    const char* id;

    void anounceSensor(const char* topic, String name, String uniqueId, String deviceClass, String unit, String templ) {
      StaticJsonDocument<400> doc;
      char output[400];

      doc["enabled_by_default"] = true;
      doc["state_class"] = "measurement";
      doc["device_class"] = deviceClass;
      doc["name"] = name;
      doc["unique_id"] = uniqueId;
      doc["unit_of_measurement"] = unit;
      doc["value_template"] = templ;
      doc["state_topic"] = MQTT::stateTopic;

      serializeJson(doc, output);
      delay(20);

      if (!mqttClient.publish(topic, output)) {
        log_i("Could not announce %s", name);
      }
    }

  public:
    MQTT(const char* state_topic, const char* mqtt_id, IPAddress mqtt_server) : mqttClient(wifiClient)
    {
      stateTopic = state_topic;
      id = mqtt_id;
      mqttClient.setServer(mqtt_server, 1883);
      mqttClient.setBufferSize(500);
    }

    /**
     * Ensure a connection to the mqtt server is made.
     */
    void ensureConnection() {
      // Loop until we're reconnected
      while (!mqttClient.connected()) {
        log_i("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient.connect(id, "mqtt_login", "mqtt_password")) {
          log_i("mqtt connected");
        } else {
          log_i("failed, rc= %d", mqttClient.state());
          delay(1000);
        }
      }
    }

    void announceSensorToHomeAssistant() {
      anounceSensor("homeassistant/sensor/ws_esp32_room/temperature/config", "Room temperature", "ws_esp32_room_temp", "temperature", "°C", "{{ value_json.room_temp }}");
      anounceSensor("homeassistant/sensor/ws_esp32_room/humidity/config", "Room humidity", "ws_esp32_room_hum", "humidity", "%", "{{ value_json.room_humidity }}");
      anounceSensor("homeassistant/sensor/ws_esp32_room/pressure/config", "Room pressure", "ws_esp32_room_pressure", "pressure", "Pa", "{{ value_json.room_pressure }}");
      anounceSensor("homeassistant/sensor/ws_esp32_outside/temperature/config", "Outside temperature", "ws_esp32_outside_temp", "temperature", "°C", "{{ value_json.outside_temp }}");
      anounceSensor("homeassistant/sensor/ws_esp32_outside/humidity/config", "Outside humidity", "ws_esp32_outside_hum", "humidity", "%", "{{ value_json.outside_humidity }}");
    }

    double round2(double value) {
      return (int)(value * 100 + 0.5) / 100.0;
    }

    void publishSensorData(SensorData* sensorData) {
      StaticJsonDocument<128> doc;
      char output[128];
      log_i("start mqtt publisher");

      doc["room_temp"]        = round2(sensorData->room_temp);
      doc["room_humidity"]    = round2(sensorData->room_humidity);
      doc["room_pressure"]    = round2(sensorData->room_pressure);
      doc["outside_temp"]     = round2(sensorData->outside_temp);
      doc["outside_humidity"] = round2(sensorData->outside_humidity);
//      doc["humidity"] = round2(sensorData->humidity);
//      doc["error"]    = sensorData->error;

      size_t n = serializeJson(doc, output);
      log_i("%s", output);

      mqttClient.publish(MQTT::stateTopic, output, n);
      log_i("mqtt published");

    }
};
