#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>

// Configuración WiFi
const char* ssid = "INFINITUMFE49";
const char* password = "VpG2DNv9Vg";

// Pines ESP32-CAM
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

// Variables globales
WebServer server(80);
String currentCommand = "stop";
unsigned long lastCommandTime = 0;

void startCamera() {
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
  
  // Configuración para imágenes más pequeñas
  config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al inicializar la cámara: 0x%x\n", err);
    return;
  }
}

void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Error al capturar imagen");
    return;
  }

  server.setContentLength(fb->len);
  server.send(200, "image/jpeg", "");
  server.sendContent((const char*)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

void executeCommand(String cmd) {
    Serial.println("--------------------");
    Serial.print("Comando recibido: ");
    Serial.println(cmd);
    Serial.print("Tiempo desde último comando: ");
    Serial.print((millis() - lastCommandTime) / 1000.0);
    Serial.println(" segundos");
    
    if (cmd == "up") {
        Serial.println("Acción: Moviendo hacia adelante");
    }
    else if (cmd == "down") {
        Serial.println("Acción: Moviendo hacia atrás");
    }
    else if (cmd == "left") {
        Serial.println("Acción: Girando a la izquierda");
    }
    else if (cmd == "right") {
        Serial.println("Acción: Girando a la derecha");
    }
    else if (cmd == "stop") {
        Serial.println("Acción: Deteniendo movimiento");
    }
    else if (cmd == "no signal") {
        Serial.println("Acción: Deteniendo movimiento");
    }
    
    lastCommandTime = millis();
}

void handleCommand() {
  if (server.hasArg("cmd")) {
    currentCommand = server.arg("cmd");
    executeCommand(currentCommand);
    server.send(200, "text/plain", "Command received: " + currentCommand);
  } else {
    server.send(400, "text/plain", "No command received");
  }
}

void handleGetCommand() {
  server.send(200, "text/plain", currentCommand);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando ESP32-CAM Robot");
  
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  startCamera();
  Serial.println("Cámara inicializada");

  // Configurar rutas del servidor
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/cmd", HTTP_GET, handleCommand);
  server.on("/getcmd", HTTP_GET, handleGetCommand);
  
  server.begin();
  Serial.println("Servidor web iniciado");
  Serial.println("\nEsperando comandos...");
}

void loop() {
  server.handleClient();
}