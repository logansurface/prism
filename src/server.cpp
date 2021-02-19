#include <Arduino.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "LEDStrip.h"

AsyncWebServer server(80);

LEDStrip<23> bar;
LEDStrip<22> u_left;
LEDStrip<21> u_right;

// Capacity determined by a json file holding WiFi and lighting information
const int capacity = JSON_OBJECT_SIZE(9);

// This is the configuration structure, it ensures that when we
// access data from the config there is minimal overhead.
struct Config  
{
    bool ap_enabled;
    char ssid[32];
    char password[32];

    int bar_mode;
    int u_left_mode;
    int u_right_mode;
}; 

Config config;

void load_config(const char *);
void write_config(const char *);
void start_access_point(const char *, const char *);
void start_station(const char *, const char *);
void set_mode(String, uint8_t, uint8_t, uint8_t);
String processor(const String &);

void setup()
{
    Serial.begin(115200);
    Serial.println();

    // Attempt to mount the filesystem
    if (!SPIFFS.begin(true))
    {
        Serial.println("ERROR - Mounting SPIFFS failed");
        return;
    }

    load_config("./config.json");
    if (config.ap_enabled)
    {
        Serial.println("WARNING - Could not find a configuration file.");
        Serial.println("Starting server in AP mode, connect to network device directly\n");

        start_access_point("PRISM Web Setup", "defaultpassword1234");
        IPAddress ip = WiFi.softAPIP();
        Serial.println("IP Address: " + ip.toString());

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
            req->send(SPIFFS, "/setup.html", "text/html");
        });
        server.on("/set", HTTP_POST, [](AsyncWebServerRequest *req) {
            if (req->hasParam("ssid", true) && req->hasParam("pass", true))
            {
                const char * ssid = req->getParam("ssid", true)->value().c_str();
                const char * pass = req->getParam("pass", true)->value().c_str();

                // Copy the wireless information provided into the configuration object
                // and write the configuration to the filesystem.
                strlcpy(config.ssid, ssid, sizeof(config.ssid));
                strlcpy(config.password, pass, sizeof(config.password));
                write_config("./config.json");

                req->send(200, "text/plain", "Configuration succeeded. Restarting device...");

                delay(2000);
                ESP.restart();
            }
            else
            {
                req->send(400, "text/plain", "Invalid parameters provided.");
                return;
            }
        });
        server.onNotFound([](AsyncWebServerRequest *req) {
            req->send(404, "text/plain", "Not found..");
        });

        server.begin();
    }

    else
    {
        Serial.println("Found config file,  connecting to " + String(config.ssid));
        Serial.println("Network settings will be available to change if WiFi fails to connect.");

        start_station(config.ssid, config.password);

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
            req->send(SPIFFS, "./index.html", "text/html", processor);
        });
        server.on("/set", HTTP_GET, [](AsyncWebServerRequest *req) {
            int num_params = req->params();
            String req_params[] = {};

            // Min parameters requried for updating the strip
            if(num_params < 5)
                req->send(400, "text/plain", "Too few arguments provided to /set");
            
            for(int i = 0; i < num_params; i++)
            {
                AsyncWebParameter *p = req->getParam(i);
                req_params[i] = p->value();
            }

            set_mode(req_params);
            
            // Pass the index back through the processor to update relevant page content
            req->send(SPIFFS, "./index.html", "text/html", processor);
        });
        server.onNotFound([](AsyncWebServerRequest *req) {
            req->send(404, "text/plain", "Not found..");
        });

        server.begin();
    }
}

void loop() {}

/*
    Loads a .json configuration file, from the path specified,
    into the Config struct.
*/
void load_config(const char * path)
{
    File config_file = SPIFFS.open(path, FILE_READ);
    StaticJsonDocument<capacity> doc;

    // If the file doesn't exist then set the wireless mode to be an access point
    DeserializationError d_err = deserializeJson(doc, config_file);
    if(d_err)
        config.ap_enabled = true;
    else
    {
        config.ap_enabled = false;
        strlcpy(config.ssid, doc["ssid"], sizeof(config.ssid));
        strlcpy(config.password, doc["pass"], sizeof(config.password));
    }

    // Close the recource
    config_file.close();
}

/*
    Writes to a .json file, at the specified path,
    the contents of the Config struct.
*/
void write_config(const char * path)
{
    File config_file = SPIFFS.open(path, FILE_WRITE);
    StaticJsonDocument<capacity> doc;

    // Generate the wireless credentials
    doc["ap_enabled"] = config.ap_enabled;
    doc["ssid"] = config.ssid;
    doc["pass"] = config.password;

    // Generate the lighting information
    doc["bar_mode"] = config.bar_mode;
    doc["u_left_mode"] = config.u_left_mode;
    doc["u_right_mode"] = config.u_right_mode;

    // Write to the file and close the resource
    serializeJson(doc, config_file);
    config_file.close();
}

/*
  Initializes a network to broadcast from the esp32 with the given network name and password
  The password state will default to unprotected if left empty
*/
void start_access_point(const char *broadcast_name, const char *pass = NULL)
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(broadcast_name, pass);

    Serial.println("SSID: " + String(broadcast_name));
    Serial.println("Password: " + String(pass));
}

/*
    Attemts to make a connection to a given network using the built in esp-wifi library
*/
void start_station(const char *ssid, const char *pass)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    Serial.println("Connecting to " + String(ssid));
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n\nIP Address: " + WiFi.localIP());
}

/*
    Takes a mode identification number for three strips along with a HSL value. 
    The intended function is for the proper mode for each to be set,
    along with the coresponding color.

    Only the HSL is updated at the moment for testing purposes.
*/
void set_mode(String mode_settings[])
{
    String strip_id = mode_settings[0];
    int strip_mode = mode_settings[1].toInt();
    int hue = mode_settings[2].toInt();
    int sat = mode_settings[3].toInt();
    int lum = mode_settings[4].toInt();

    if(strip_id == "bar")
    {
        config.bar_mode = strip_mode;
        bar.solid_fill(hue, sat, lum);
    }
    else if(strip_id == "uleft")
    {
        config.u_left_mode = strip_mode;
        u_left.solid_fill(hue, sat, lum);
    }
    else if(strip_id == "uright")
    {
        config.u_right_mode = strip_mode;
        u_right.solid_fill(hue, sat, lum);
    }
}

/*
    Dynamically replaces placeholder values within the HTML
*/
String processor(const String &var)
{
    // TODO: Add replacements for SVG circles in correct color
    if (var == "PLACEHOLDER")
    {
        String title = "Index page served on " + String(config.ssid) + "...";

        return title;
    }

    return String();
}