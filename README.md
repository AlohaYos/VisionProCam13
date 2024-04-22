# VisionProCam13

Added a 13th camera to Vision Pro

## Want to get video in real time on Vision Pro
Trying to use Image recognition AI to analyze the video I'm watching on Vision Pro in real time.
However, with Vision Pro, it is not possible to programmatically obtain videos and photos from the camera, which was very easy with iPhones and iPads. Vision Pro is equipped with 12 cameras, but their use is prohibited because it violates privacy to see what users are looking at in real time.
So I decided to add a 13th camera (Cam13) myself.

## How Cam13 works
Add an external camera, receive the video via WKWebView via WiFi, and overlay it on the Vision Pro screen. My idea is to perform AI processing on images captured by Cam13 to obtain an effect similar to analyzing images from Vision Pro camera.

*Once the ban on camera footage is lifted at this year's WWDC24, I will abandon Cam13 and use the official API (lol)

## Connect the M5Stack camera unit
I could use a commercially available webcam, but I wanted to have more freedom in mounting it on Vision Pro, so I looked for an inexpensive, lightweight unit that could be controlled by programs and had an easy-to-handle power supply.
I chose the M5Stack series UnitCAMS3 equipped with ESP32S3. It costs less than $15.

<a href="https://shop.m5stack.com/products/unit-cams3-wi-fi-camera-ov2640" target=_blank><img src="https://github.com/AlohaYos/VisionProCam13/assets/4338056/841870ba-68ba-4bc7-958c-18fffcd1ea32" width="250"></a>


## Send real-time video from camera via WiFi
UnitCAMS3 ships with a demo sketch installed, but I have to manually initiate video transmission every time. I want it to start automatically, so I decided to program the sketch myself.
I installed VSCode and PlatformIO on my Mac and built the sketch below.

```cpp:main.cpp
// Access
//   http://192.168.4.1/stream
//   http://192.168.4.1/still

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
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
  IPAddress ip(192, 168, 1, 123);
  IPAddress gateway(192, 168, 1, 1);
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
```

Build → Run on PlatformIO, the program will wait in server mode. If isAccessPointMode is set to true, the server will wait at 192.168.4.1 in the access point mode. If set to false, it will wait at 192.168.1.123 in the station mode.
Access point mode works in 30fps, station mode 10fps. 
As Vision Pro can only connect to the internet via WiFi, so you will need to operate it in station mode to access the internet and camera at the same time.

## Receive video with visionOS app
The visionOS app is created using SwiftUI. WKWebView receives streaming from "http://192.168.1.123/stream".

```swift:ContentView.swift
import SwiftUI
import RealityKit
import RealityKitContent
import WebKit

#if os(macOS)
struct WebView: NSViewRepresentable {
	let loadUrl: URL
	func makeNSView(context: Context) -> WKWebView {
		return WKWebView()
	}
	func updateNSView(_ uiView: WKWebView, context: Context) {
		let request = URLRequest(url: loadUrl)
		uiView.load(request)
	}
}
#else
struct WebView: UIViewRepresentable {
	let loadUrl: URL
	func makeUIView(context: Context) -> WKWebView {
		return WKWebView()
	}
	func updateUIView(_ uiView: WKWebView, context: Context) {
		let request = URLRequest(url: loadUrl)
		uiView.load(request)
	}
}
#endif

struct ContentView: View {
	var body: some View {
		WebView(loadUrl: URL(string: "http://192.168.1.123/stream")!)
	}
}
```

If you do not add "Privacy - Local Network Usage Description" to Info.plist, you will not be able to communicate via LAN and WebView will show nothing.
Access "http://192.168.1.123/still" to obtain still footage.

## Try overlaying it on the Vision Pro screen
Now let's superimpose the Cam13 video onto the Vision Pro original screen.

https://github.com/AlohaYos/VisionProCam13/assets/4338056/1ce9bf8b-77aa-4ea4-a990-5ba45b4a972b

Image scaling and positioning are manual operation by myself (^^;  The next challenge is to automate this calibration, but I am optimistic that once the camera is mounted and the settings are made, there will be no major discrepancies.

## Next step
- Make a camera mount for Cam13 with a 3D printer and fix Cam13
- Automate the calibration of Vision Pro screen and Cam13 video overlapping
- Perform AI image analysis in real time
- Display analysis result information on the HUD which floats in the Vision Pro screen
- Do everything on-device of Vision Pro (considering privacy)
