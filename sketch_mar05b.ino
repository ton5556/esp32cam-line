#include <WiFi.h>
#include "esp_camera.h"
#include "esp_system.h"

hw_timer_t *timer = NULL;
void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}
#include <TridentTD_LineNotify.h>
#define SSID        "WaRi025308_2.4G"
#define PASSWORD    "3730601025"
#define LINE_TOKEN  "GXzDuLVNVNDKGeCdUjHrmquPtgKlIoGMZO0jD7MqdNb"

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

const int Led_Flash = 4;
const int Led_run = 13;
int PIR_Sensor = 12;
boolean startTimer = false;
unsigned long time_now = 0;
int time_capture = 0;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  pinMode(Led_Flash, OUTPUT);
  pinMode(Led_run, OUTPUT);
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);

  timer = timerBegin(0, 80, true); 
  timerAttachInterrupt(timer, &resetModule, true);
  timerAlarmWrite(timer, 20000000, false);
  timerAlarmEnable(timer); //enable interrupt

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
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
void loop() {
  timerWrite(timer, 0); 
  long tme = millis();
  if (digitalRead(PIR_Sensor) == 1 && startTimer != true) {
    Camera_capture();
    Serial.println("OK");
    startTimer = true;
  } else if (digitalRead(PIR_Sensor) == 0) {
    startTimer = false;
    time_capture = 0;
  }

  if (millis() > time_now + 1000) {
    time_now = millis();
    digitalWrite(Led_run, HIGH);
    delay(20);
    digitalWrite(Led_run, LOW);
  }
  tme = millis() - tme;
  if (digitalRead(PIR_Sensor) == 1) {
    if (++time_capture > 60) {
      time_capture = 0;
      Camera_capture();
      Serial.println("Over Time");
    }
  }

  Serial.println(digitalRead(PIR_Sensor));
  delay(200);
}


void Camera_capture() {
  digitalWrite(Led_Flash, HIGH);
  delay(100);
  digitalWrite(Led_Flash, LOW);
  delay(100);
  digitalWrite(Led_Flash, HIGH);
  camera_fb_t * fb = NULL;
  delay(200);
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  digitalWrite(Led_Flash, LOW);
  Send_line(fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void Send_line(uint8_t *image_data, size_t   image_size) {
  LINE.notifyPicture("ตรวจพบการเคลื่อนไหว", image_data, image_size);
}
