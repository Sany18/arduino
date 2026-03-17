#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "webserver.h"
#include "clock.h"

// WiFi credentials
const char* ssid = "troll2.4G";
const char* password = "F8MRYZLZ92";

// Create AsyncWebServer on port 80
AsyncWebServer server(80);

// Get MIME type based on file extension
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/pdf";
  else if (filename.endsWith(".zip")) return "application/zip";
  return "text/plain";
}

// Initialize WiFi and WebServer
void initWebServer() {
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  
  // Connect to WiFi
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup API routes
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"status\":\"ok\",\"uptime\":" + String(millis()) + "}";
    request->send(200, "application/json", json);
  });
  
  // Get current time
  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request){
    unsigned long currentTime = getCurrentTime();
    String json = "{\"time\":" + String(currentTime) + "}";
    request->send(200, "application/json", json);
  });
  
  // Set current time (POST)
  server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // Parse JSON body
      String body = "";
      for (size_t i = 0; i < len; i++) {
        body += (char)data[i];
      }
      
      Serial.println("Received time POST:");
      Serial.println(body);
      
      // Extract time value (simple parsing)
      int timeIndex = body.indexOf("\"time\":");
      if (timeIndex != -1) {
        int valueStart = timeIndex + 7;
        int valueEnd = body.indexOf(",", valueStart);
        if (valueEnd == -1) valueEnd = body.indexOf("}", valueStart);
        
        String timeStr = body.substring(valueStart, valueEnd);
        timeStr.trim();
        unsigned long epoch = timeStr.toInt();
        
        Serial.print("Setting time to: ");
        Serial.println(epoch);
        
        setCurrentTime(epoch);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
      } else {
        Serial.println("ERROR: Invalid JSON - no 'time' field found");
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      }
    }
  );
  
  // Receive system stats (POST)
  server.on("/api/stats", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // Parse JSON body
      String body = "";
      for (size_t i = 0; i < len; i++) {
        body += (char)data[i];
      }
      
      Serial.println("Received stats POST:");
      Serial.println(body);
      
      // Simple JSON parsing for each stat
      int cpu = 0, ram = 0, battery = 0, network = 0, disk = 0;
      
      // Extract CPU
      int idx = body.indexOf("\"cpu\":");
      if (idx != -1) {
        int start = idx + 6;
        int end = body.indexOf(",", start);
        if (end == -1) end = body.indexOf("}", start);
        cpu = body.substring(start, end).toInt();
      }
      
      // Extract RAM
      idx = body.indexOf("\"ram\":");
      if (idx != -1) {
        int start = idx + 6;
        int end = body.indexOf(",", start);
        if (end == -1) end = body.indexOf("}", start);
        ram = body.substring(start, end).toInt();
      }
      
      // Extract Battery
      idx = body.indexOf("\"battery\":");
      if (idx != -1) {
        int start = idx + 10;
        int end = body.indexOf(",", start);
        if (end == -1) end = body.indexOf("}", start);
        battery = body.substring(start, end).toInt();
      }
      
      // Extract Network
      idx = body.indexOf("\"network\":");
      if (idx != -1) {
        int start = idx + 10;
        int end = body.indexOf(",", start);
        if (end == -1) end = body.indexOf("}", start);
        network = body.substring(start, end).toInt();
      }
      
      // Extract Disk
      idx = body.indexOf("\"disk\":");
      if (idx != -1) {
        int start = idx + 7;
        int end = body.indexOf(",", start);
        if (end == -1) end = body.indexOf("}", start);
        disk = body.substring(start, end).toInt();
      }
      
      // Update servos with new stats
      setSystemStats(cpu, ram, battery, network, disk);
      
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
  );
  
  // Serve static files from SPIFFS
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  // Handle 404
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

// Handle client requests (not needed for AsyncWebServer)
void handleWebServer() {
  // AsyncWebServer handles requests automatically
  // No need to call handleClient()
}
