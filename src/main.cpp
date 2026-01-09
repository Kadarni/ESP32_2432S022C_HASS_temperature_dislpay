#include <Arduino.h>
#include <esp32_smartdisplay.h>

#include <WiFi.h>
#include "lv_conf.h"
#include  <WebServer.h>
#include <PubSubClient.h>

char ssid[] = "palfi3";
const char *password = "20170212a20140627b19861208c19860328d00";

const char* mqtt_server = "192.168.1.19"; //192.168.1.19

const char* mqttUser = "szenzorok"; //szenzorok

const char* mqttPassword ="Szenzorok1234"; //Szenzorok1234

double temperature = -999;
char msg[50] = "";

#define mqtt_port 1883

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void reconnect ();
void setup_wifi ();
void callback(char* topic, byte *payload, unsigned int length); 
void connectmqtt();

void publishSerialData(char *serialData);

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  // client.publish("outtopic", serialData);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32_clientID", mqttUser, mqttPassword)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      // client.publish("outTopic", "testing...");
      // ... and resubscribe
      client.subscribe("out_temp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void connectmqtt()
{
  client.connect("ESP32_clientID", mqttUser, mqttPassword);  // ESP will connect to mqtt broker with clientID
  {
    Serial.println("connected to MQTT");
    // Once connected, publish an announcement...

    // ... and resubscribe
    client.subscribe("out_temp"); //topic=Demo
    // client.publish("outTopic",  "connected to MQTT");

    if (!client.connected())
    {
      reconnect();
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    // char msg[50];    
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    strncpy(msg, (char*)payload, length);
    msg[length] = '\0';
    temperature = atof(msg);
    Serial.println();
}

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqtt_server, 1883);//connecting to mqtt server
    client.setCallback(callback);
    connectmqtt();
}





void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);
  log_i("Board: %s", BOARD_NAME);
  log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
  log_i("Free heap: %d bytes", ESP.getFreeHeap());
  log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
  log_i("SDK version: %s", ESP.getSdkVersion());

  smartdisplay_init();
  // Set display rotation, choose one of:
  lv_display_set_rotation(lv_display_get_default(), LV_DISPLAY_ROTATION_90);
  lv_obj_t* label_num = lv_label_create(lv_scr_act());
  lv_label_set_text(label_num, "Initializing ...");
  lv_obj_set_style_text_color(label_num, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_text_font(label_num, &lv_font_montserrat_28, 0);
  lv_obj_align(label_num, LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x00000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
  
  setup_wifi();
  delay(1000);
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
  delay(1000);

  // lv_obj_t* label_num = lv_label_create(lv_scr_act());
  lv_obj_t* label_num1 = lv_label_create(lv_scr_act());
  lv_label_set_text(label_num1, "Init DONE!");
  lv_obj_set_style_text_color(label_num1, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_text_font(label_num1, &lv_font_montserrat_28, 0);
  lv_obj_align(label_num1, LV_ALIGN_CENTER, 0, 80);
  // lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x00000), LV_PART_MAIN);
  // lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);

}

auto lv_last_tick = millis();



void loop() {
  // put your main code here, to run repeatedly:
  auto const now = millis();
  struct tm timeinfo;
  char timestring[] = "00:00:00";
  // Update the ticker
  lv_tick_inc(now - lv_last_tick);
  lv_last_tick = now;
  // Update the UI
  lv_timer_handler();
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();
  if (temperature == -999)
    return;
  lv_obj_clean(lv_scr_act());
  lv_obj_t* label_num = lv_label_create(lv_scr_act());
  lv_label_set_text(label_num, strcat(msg, " °C"));
  lv_color_t color;
    if (temperature < 10) {
        color = lv_palette_main(LV_PALETTE_BLUE);
    } else if (temperature > 29) {
        color = lv_palette_main(LV_PALETTE_RED);
    } else {
        color = lv_palette_main(LV_PALETTE_GREEN);
    }
  lv_obj_set_style_text_color(label_num, color, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_num, &lv_font_montserrat_48, 0);
  lv_obj_align(label_num, LV_ALIGN_CENTER, 0, -20);
  lv_obj_t* label_time = lv_label_create(lv_scr_act());
  getLocalTime(&timeinfo);
  strftime(timestring, 20, "%H:%M:%S", &timeinfo);
  // sprintf(timestring, "%02d:%02d:%02d", hours, minutes, seconds);
  lv_label_set_text(label_time,timestring) ;
  lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(label_time, &lv_font_montserrat_28, 0);
  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 80);
  temperature = -999;
  delay(1000);

}
