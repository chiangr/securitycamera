#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"          
#include "soc/rtc_cntl_reg.h"  
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

const char* ssid = "Chum House";
const char* password = "youngunicorn";
//const char* ssid = "Berkeley-IoT";
//const char* password = "no+1CteK";


AsyncWebServer server(80);

boolean new_photo = false;

#define photo_path "/image.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>

<!-- Required meta tags -->
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

<!-- Bootstrap CSS -->
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/css/bootstrap.min.css" integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm" crossorigin="anonymous">

<!-- Inter Font -->
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@100;200;300;400;500;600;700;800;900&display=swap" rel="stylesheet">

<title>TAP</title>

<style>

:root {
--bgd:#F4C095;
--main:#071E22;
--main-2:#1D7874;
--secondary:#679289;
--accent:#EE2E31;
}

* {
font-family: 'Inter', sans-serif;
color:whitesmoke;
}

html {
overflow-y:hidden;
}

body { 
text-align:center; 
background-color:var(--secondary);
}

h1 {
font-weight:800;
font-size:7em;
color:whitesmoke;
}

button {
background-color:var(--main-2);
border: none;
padding: 7px 10px;
text-align: center;
font-size: 10px;
border-radius: 2px;
color: white;
cursor: pointer;
font-weight:700;
font-size:large;
width:100%;
}

button:hover {
background-color:var(--bgd);
transition:0.3s;
}

.main-btn {
background-color:var(--main);
}

.title {
width:80vw;
text-align:left;
/* From https://css.glass */
background: rgba(255, 255, 255, 0.18);
border-radius: 16px;
box-shadow: 0 4px 30px rgba(0, 0, 0, 0.1);
backdrop-filter: blur(5.9px);
-webkit-backdrop-filter: blur(5.9px);
}

.main-ctn {
height:100vh;
width:100vw;
z-index:90;
}

.content-ctn {
/* From https://css.glass */
background: rgba(255, 255, 255, 0.18);
border-radius: 16px;
box-shadow: 0 4px 30px rgba(0, 0, 0, 0.1);
backdrop-filter: blur(5.9px);
-webkit-backdrop-filter: blur(5.9px);
width:30vw;
}

.camera-ctn {
width:50vw;
/* From https://css.glass */
background: rgba(255, 255, 255, 0.18);
border-radius: 16px;
box-shadow: 0 4px 30px rgba(0, 0, 0, 0.1);
backdrop-filter: blur(5.9px);
-webkit-backdrop-filter: blur(5.9px);
}

.main-bgd {
top:0px;
left:0px;
width:100vw;
height:100vh;
}
.second-bgd {
top:0px;
left:0px;
width:100vw;
height:100vh;
}

</style>
</head>
<body>
<div class="container d-flex flex-column align-items-center justify-content-center main-ctn">
<div class="d-flex flex-row p-2 m-4 title align-items-center">
<h1 class="m-3">TAP</h1>
<h2 class="m-3"> | Take A Picture!</h2>
</div>
<div class="d-flex flex-row">
<div class="col-md-4 p-4 m-2 content-ctn">
<div class="d-flex flex-column">
<p class="m-3"><button class="main-btn" onclick="capturePhoto()">CAPTURE PHOTO</button></p>
<p class="m-3"><button onclick="location.reload();">REFRESH</button></p>
</div>
</div>
<div class="col-md-8 m-2 camera-ctn">
<div class="m-2">
<img src="saved-photo" id="photo" width="90%">
</div>
</div>
</div>
</div>
<div class="position-fixed" style="z-index:-1">
<div class="position-fixed vw-100 vh-100 main-bgd">
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1440 320">
<path fill="#679289" fill-opacity="0.75" d="M0,256L48,256C96,256,192,256,288,234.7C384,213,480,171,576,176C672,181,768,235,864,240C960,245,1056,203,1152,186.7C1248,171,1344,181,1392,186.7L1440,192L1440,0L1392,0C1344,0,1248,0,1152,0C1056,0,960,0,864,0C768,0,672,0,576,0C480,0,384,0,288,0C192,0,96,0,48,0L0,0Z"></path>
</svg>
</div>
<div class="position-fixed vw-100 vh-100 main-bgd">
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1440 320">
<path fill="#1D7874" fill-opacity="1" d="M0,192L48,202.7C96,213,192,235,288,240C384,245,480,235,576,240C672,245,768,267,864,266.7C960,267,1056,245,1152,234.7C1248,224,1344,224,1392,224L1440,224L1440,0L1392,0C1344,0,1248,0,1152,0C1056,0,960,0,864,0C768,0,672,0,576,0C480,0,384,0,288,0C192,0,96,0,48,0L0,0Z"></path>
</svg>
</div>
<div class="position-fixed vw-100 vh-100 main-bgd">
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1440 320">
<path fill="#071E22" fill-opacity="1" d="M0,64L48,85.3C96,107,192,149,288,138.7C384,128,480,64,576,48C672,32,768,64,864,74.7C960,85,1056,75,1152,74.7C1248,75,1344,85,1392,90.7L1440,96L1440,0L1392,0C1344,0,1248,0,1152,0C1056,0,960,0,864,0C768,0,672,0,576,0C480,0,384,0,288,0C192,0,96,0,48,0L0,0Z"></path>
</svg>
</div>
</div>
<script>
/*var deg = 0;*/
function capturePhoto() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', "/capture", true);
  xhr.send();}
/*function rotatePhoto() {var img = document.getElementById("photo"); deg += 90;if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }else{ document.getElementById("container").className = "hori"; }img.style.transform = "rotate(" + deg + "deg)";}*/
/*function isOdd(n) {return Math.abs(n % 2) == 1;}*/
</script>
</body></html>)rawliteral";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    new_photo = true;
    request->send_P(200, "text/plain", "Capturing Photo using ESP32-CAM");
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, photo_path, "image/jpg", false);
  });


  server.begin();

}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Check if photo capture was successful
bool check_photo( fs::FS &fs ) {
  File f_pic = fs.open( photo_path );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void captureSave_photo( void ) {
  camera_fb_t * fb = NULL; 
  bool ok = 0;

  do {
    Serial.println("ESP32-CAMP capturing photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed");
      return;
    }

    Serial.printf("Picture file name: %s\n", photo_path);
    File file = SPIFFS.open(photo_path, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); 
      Serial.print("The picture has been saved in ");
      Serial.print(photo_path);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    file.close();
    esp_camera_fb_return(fb);

    ok = check_photo(SPIFFS);
  } while ( !ok );
}

void loop() {
  if (new_photo) {
    captureSave_photo();
    new_photo = false;
  }
  delay(1);
}
