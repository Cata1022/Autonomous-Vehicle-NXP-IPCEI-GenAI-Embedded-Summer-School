#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// Set to 1 to enable the Wi-Fi and HTTP server (for /roi endpoint), 0 to disable for pure offline AI
#define ENABLE_WIFI_SERVER 1

#if ENABLE_WIFI_SERVER
void startWebServer();
#endif

#endif // WEB_SERVER_H
