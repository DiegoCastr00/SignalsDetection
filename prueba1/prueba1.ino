#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>

// Configuración WiFi
const char* ssid = "INFINITUM";
const char* password = "TAymyAfGx5";
  
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

// Pines para el módulo L298N
const int PIN_IN1 = 15;  // Motor A
const int PIN_IN2 = 13;
const int PIN_IN3 = 14;  // Motor B
const int PIN_IN4 = 2;
const int PIN_ENA = 12;   // Enable Motor A
const int PIN_ENB = 4;   // Enable Motor B

const int LED_PIN = 33;

// Velocidad de los motores (0-255)
const int VELOCIDAD = 40;

// Variables globales
WebServer server(80);
String currentCommand = "stop";
unsigned long lastCommandTime = 0;

void setupMotores() {
  // Configurar pines de motores como salidas
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // Configurar pines como salidas
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  
  // Inicialmente detenido
  detener();
}

void adelante() {
 // Motor A adelante
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, VELOCIDAD);
  
  // Motor B adelante
  digitalWrite(PIN_IN3, HIGH);
  digitalWrite(PIN_IN4, LOW);
  analogWrite(PIN_ENB, VELOCIDAD);
}

void atras() {
  // Motor A atrás
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, HIGH);
  analogWrite(PIN_ENA, VELOCIDAD);
  
  // Motor B atrás
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, HIGH);
  analogWrite(PIN_ENB, VELOCIDAD);
}

void izquierda() {
  // Motor A adelante
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, 80);
  
  // Motor B atrás
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, HIGH);
  analogWrite(PIN_ENB, 10);
}

void derecha() {
  // Motor A atrás
  digitalWrite(PIN_IN1, HIGH);  // Cambiado de LOW a HIGH
  digitalWrite(PIN_IN2, LOW);   // Cambiado de HIGH a LOW
  analogWrite(PIN_ENA, 10);
 
  // Motor B adelante
  digitalWrite(PIN_IN3, HIGH);
  digitalWrite(PIN_IN4, LOW);
  analogWrite(PIN_ENB, 80);
}

void detener() {
  // Detener ambos motores
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
  analogWrite(PIN_ENA, 0);
  analogWrite(PIN_ENB, 0);
}

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
  
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 20;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST; // Tomar siempre el frame más reciente


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

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Connection", "close");
  server.setContentLength(fb->len);
  server.send(200, "image/jpeg", "");
  
  WiFiClient client = server.client();
  client.write(fb->buf, fb->len);
  
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
        adelante();
        delay(1000);

    }
    else if (cmd == "down") {
        Serial.println("Acción: Moviendo hacia atrás");
        atras();
        delay(100);
    }
    else if (cmd == "left") {
        Serial.println("Acción: Girando a la izquierda");
        derecha();
        delay(100);
    }
    else if (cmd == "right") {
        Serial.println("Acción: Girando a la derecha");
        izquierda();
        delay(100);
    }
    else if (cmd == "stop") {
        Serial.println("Acción: Deteniendo movimiento");
        detener();
        delay(100);
    }
    else if (cmd == "no signal") {
        Serial.println("Acción: Deteniendo movimiento");
        detener();
        delay(100);

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
  
  // Inicializar motores
  setupMotores();
  Serial.println("Motores inicializados");
  
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