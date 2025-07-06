#pragma once
#include <ESPAsyncWebServer.h>

/**
 *  WebServerManager – wraps a tiny REST/JSON server
 *  End-points
 *      GET /          → "ESP32 OK"
 *      GET /sensor    → {"temp":23.4,"hum":48.7}
 *      GET /reset     → restarts the ESP32
 *      GET /reset-wifi → resets WiFi credentials and restarts the ESP32
 */
class WebServerManager {
public:
    explicit WebServerManager(uint16_t port = 80);

    /** Start the HTTP server (safe to call again – runs once). */
    void begin();

    void stop();

    /** Inject a lambda that returns live sensor JSON. */
    void setSensorCallback(std::function<String(void)> fn)
        { sensorCb_ = std::move(fn); }

    void setMqttConfigCallback(std::function<void(String, int, String, String, String)> fn)
        { mqttConfigCb_ = std::move(fn); }

    bool reset = false;
private:
    AsyncWebServer           server_;
    std::function<String()>  sensorCb_;
    std::function<void(String, int, String, String, String)> mqttConfigCb_;
    bool                     running_ = false;

};
