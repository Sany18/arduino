#ifndef WEBSERVER_H
#define WEBSERVER_H

// Initialize WiFi and WebServer
void initWebServer();

// Handle client requests (empty for AsyncWebServer - not required)
void handleWebServer();

// Set system stats (called from API endpoint)
void setSystemStats(int cpu, int ram, int battery, int network, int disk);

#endif
