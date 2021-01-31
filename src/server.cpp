#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>

// Wireless declarations
const char *ssid;
const char *pass;
AsyncWebServer server(80);

const int capacity = JSON_OBJECT_SIZE(2);
StaticJsonDocument<256> config_doc;

void start_access_point(const char *, const char *);
void start_station(const char *, const char *);
void export_config(File, const char *, const char *);
String processor(const String &);
String handle_not_found();

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println();

    if (!SPIFFS.begin(true))
    {
        Serial.println("ERROR - Mounting SPIFFS failed");
        return;
    }

    // Open the file in append+read mode. The file is created if it doesn't exist.
    File config_file = SPIFFS.open("/config.json", "a+");
    if (config_file.size() <= 0)
    {
        Serial.println("WARNING - Could not find a configuration file.");
        Serial.println("Starting server in AP mode, connect to network device directly\n");

        start_access_point("PRISM Web Setup", "defaultpassword1234");
        IPAddress ip = WiFi.softAPIP();
        Serial.println("IP Address: " + ip.toString());

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
            req->send(SPIFFS, "/config.html", "text/html");
        });
        server.on("/set", HTTP_POST, [config_file](AsyncWebServerRequest *req) {
            if (req->hasParam("ssid") && req->hasParam("pass"))
            {
                ssid = req->getParam("ssid")->value().c_str();
                pass = req->getParam("pass")->value().c_str();

                //        config_doc["ssid"] = "FBI Surveillance Van";
                //        config_doc["pass"] = "Hockey11";
                //        serializeJson(config_doc, config_file);
                export_config(config_file, ssid, pass);

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
            req->send(404, "text/plain", "Not Found..");
        });

        server.begin();
    }

    else
    {
        DeserializationError d_err = deserializeJson(config_doc, config_file);
        if (d_err)
        {
            Serial.println("Failed to deserialize the config file. Aborting...");
            Serial.println(d_err.c_str());
            //return;
        }

        ssid = "Apt_WiFi_EXT";
        pass = "basketjudge382";

        Serial.println("Found config file,  connecting to " + String(ssid));
        Serial.println("Network settings will be available to change if WiFi fails to connect.");

        start_station(ssid, pass);

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
            req->send(SPIFFS, "./index.html", "text/html");
        });
        server.on("/set", HTTP_POST, [](AsyncWebServerRequest *req) {
            if (req->hasParam("mode") && req->hasParam("args"))
            {
 //               const char *mode = req->getParam("mode")->value().c_str();
//                const char *mode_args = req->getParam("args")->value().c_str();

                // TODO: Call fastled modes and write json
            }
            else
            {
                req->send(400, "text/plain", "Invalid arguments provided in /set");
            }
        });
        server.onNotFound([](AsyncWebServerRequest *req) {
            req->send(404, "text/plain", "Not found..");
        });

        server.begin();
    }
}

void loop() {}

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

void export_config(File config_file, const char *ssid, const char *pass)
{
    config_file.println("ssid: \"FBI\"");
}

/*
  A function which dynamically replaces placeholders inside the html
*/
String processor(const String &var)
{
    // TODO: Add replacements for SVG circles in correct color
    if (var == "PLACEHOLDER")
    {
        String title = "Index page served on " + String(ssid) + "...";

        return title;
    }

    return String();
}