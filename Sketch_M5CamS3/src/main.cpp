//  Created by Yos on 2024
//
//  Console log 
//    Serial.println /dev/tty.usbmodem1411 115200 (MacBookPro)
//  Console
//    screen /dev/tty.usbmodem1101 115200 (MacBookAir)
//  Access
//    http://192.168.4.1/stream
//    http://192.168.4.1/still

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <esp32cam.h>
#include <esp32cam-asyncweb.h>
#include "apis/camera/api_cam.h"

// WiFi
#define WIFI_SSID "--YourSSID--"
#define WIFI_PASSWORD "--password--"
bool isAccessPointMode = false;

// Camera
esp32cam::Resolution initialResolution;
constexpr esp32cam::Pins UnitCamS3{
  D0: 6,  D1: 15,  D2: 16,  D3: 7,
  D4: 5,  D5: 10,  D6:  4,  D7: 13,
  XCLK: 11,  PCLK: 12,  VSYNC: 42,
  HREF: 18,  SDA: 17,  SCL: 41,
  RESET: 21,  PWDN: -1,
};

// Web server
static void serveStill(AsyncWebServerRequest *request);
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  IPAddress ipAp(192, 168, 4, 1);
  IPAddress ip(192, 168, 10, 123);
  IPAddress gateway(192, 168, 10, 1);
  IPAddress subnet(255, 255, 255, 0);

  if(isAccessPointMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("VisionProCam13-WiFi");
    delay(100);
    WiFi.softAPConfig(ipAp, ip, subnet);
  }
  else {
    if (!WiFi.config(ip,gateway,subnet)){
        Serial.println("Failed to configure!");
    }
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("Access 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("/stream' to connect webcam");
  }

  {
    using namespace esp32cam;

    initialResolution = Resolution::find(800, 600);
    Config cfg;
    cfg.setPins(UnitCamS3);
    cfg.setResolution(initialResolution);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }

  server.on("/still", HTTP_GET, serveStill);
  server.on("/stream", HTTP_GET, streamJpg);
  server.begin();
}

void loop() {
  delay(1);
}

// Photo
static void serveStill(AsyncWebServerRequest *request) {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("capture() failure");
    request->send(500, "text/plain", "still capture error\n");
    return;
  }
  AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", frame->data(), frame->size());
  request->send(response);
}
