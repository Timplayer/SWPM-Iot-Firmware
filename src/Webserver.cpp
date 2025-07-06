#include "Webserver.hpp"
#include <Arduino.h>
#include <WiFi.h>

WebServerManager::WebServerManager(uint16_t port)
: server_(port) {}

void WebServerManager::begin()
{
    if (running_) return;               // don’t re-add handlers
    running_ = true;

    /* root: plain-text ping */
    server_.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/plain", "ESP32 OK");
    });

    /* /sensor: JSON – uses the callback if provided */
    server_.on("/sensor", HTTP_GET, [this](AsyncWebServerRequest *req){
        String payload = sensorCb_ ? sensorCb_() : "{}";
        req->send(200, "application/json", payload);
    });

    server_.on("/reset", HTTP_GET, [this](AsyncWebServerRequest *req){
        req->send(200, "text/plain", "WiFi credentials will be reset and ESP32 will restart now.");
        //delay(5000); // Give time for the response to be sent
        reset = true;
    });

    server_.on("/mqtt", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (request->hasParam("mqttServer", true) && request->hasParam("mqttPort", true) && request->hasParam("mqttTopic", true)) {
            String mqttServer = request->getParam("mqttServer", true)->value();
            int mqttPort = request->getParam("mqttPort", true)->value().toInt();
            String mqttTopic = request->getParam("mqttTopic", true)->value();
            String mqttUser = "tim";
            String mqttPass = "tim";
            if (request->hasParam("mqttUser", true)) {
                mqttUser = request->getParam("mqttUser", true)->value();
            }
            if (request->hasParam("mqttPass", true)) {
                mqttPass = request->getParam("mqttPass", true)->value();
            }
            if (mqttConfigCb_) {
                mqttConfigCb_(mqttServer, mqttPort, mqttTopic, mqttUser, mqttPass);
                request->send(200, "text/plain", "MQTT configuration updated.");
            } else {
                request->send(500, "text/plain", "MQTT configuration callback not set.");
            }
        } else {
            request->send(400, "text/plain", "Missing MQTT parameters (mqttServer, mqttPort, mqttTopic).");
        }
    });

    server_.begin();
}

void WebServerManager::stop(){
    server_.end();
}