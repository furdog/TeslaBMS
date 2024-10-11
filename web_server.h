#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <DNSServer.h>

#include "web_interface.h"

// Налаштування WiFi
const char* ssid = "TeslaBOX";
const char* password = "0123456789";

// Ім'я хосту
const char* hostname = "DetecBOX";

// Ініціалізація серверів
#if defined(ESP32)
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif

DNSServer dnsServer;

// Обробка запитів на сервері
void handleRoot() {
	server.sendHeader("Content-Encoding", "gzip");
	server.sendHeader("Content-Type", "text/html");

	server.send_P(200, "text/html", (const char *)build_index_html_gz, build_index_html_gz_len);
}

void send_ok() { server.send(200); }

void download_firmware()
{
	uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
	HTTPUpload& upload = server.upload();
	
	if (upload.status == UPLOAD_FILE_START) {
		if (Update.begin(free_space)) {
			Serial.println("Update begin success");
		} {
			Serial.println("Update begin fail");
		}
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (Update.write(upload.buf, upload.currentSize))
			Serial.println("Update write ok...");
	} else if(upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) {
			Serial.println("Update end success");			
			ESP.restart();
		} else {
			Serial.println("Update end fail");
		}
	}
}

void tbox_web_server_init()
{
	// Налаштування точки доступу WiFi
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(IPAddress(7, 7, 7, 7), IPAddress(7, 7, 7, 7), IPAddress(255, 255, 255, 0));
	WiFi.softAP(ssid);
	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	// Налаштування DNS
	dnsServer.start(53, "*", WiFi.softAPIP());

	// Налаштування хост-імені
	if (!MDNS.begin(hostname)) {
		Serial.println("Error starting MDNS");
	} else {
		Serial.println("mDNS responder started");
		MDNS.addService("http", "tcp", 80);
	}

	// Налаштування WebServer
	server.on("/", HTTP_GET, handleRoot);
	server.on("/update", HTTP_POST, send_ok, download_firmware);
	server.onNotFound(handleRoot);
	server.begin();

	// Виведення IP адреси
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());
}

void tbox_web_server_update()
{
	dnsServer.processNextRequest();
	server.handleClient();
}
