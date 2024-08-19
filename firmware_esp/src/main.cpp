#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "SoftwareSerial.h"
#include "ArduinoJson.h"

#define RX_PIN D2 // D2
#define TX_PIN D1 // D1

SoftwareSerial mySerial(RX_PIN, TX_PIN);

const char *ssid = "I_want_to_end_my_life_ver_laptop";
const char *password = "lifeissosuck.";
String url = "http://192.168.1.4:4001";

unsigned char modeCharacter = '0';
unsigned char openCharacter = '0';
String temp;

void getDataModeFromServer(const String &url)
{
    String command;
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, url);
        int httpCode = http.GET();
        // Serial.println(httpCode);
        if (httpCode > 0)
        {
            JsonDocument doc;
            deserializeJson(doc, http.getString());
            temp = doc["command"].as<String>();
            if (temp.startsWith("MODE_"))
            {
                command = temp.substring(5)[0];
                // Serial.println(command);
                if (command[0] - 48 != modeCharacter)
                {
                    modeCharacter = (command[0] - 48);
                    mySerial.print(("MODE_" + String(modeCharacter)));
                    // Serial.println(("MODE_" + String(modeCharacter)));
                }
            }
        }
        else
        {
            Serial.println("Error on HTTP request");
        }

        http.end();
    }
    else
    {
        Serial.println("WiFi not connected");
    }
}

void sendDataModeToServer(const String &url, String commandFromUno)
{
    String command;
    temp = commandFromUno;
    command = temp.substring(5);
    if (command[0] - 48 != modeCharacter)
    {
        modeCharacter = command[0] - 48;
        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, url);
            http.addHeader("Content-Type", "application/json");
            String jsonData = "{\"mode\":\"";
            jsonData += (modeCharacter);
            jsonData += "\"}";
            http.POST(jsonData);
            // Serial.println(jsonData);
            Serial.println(modeCharacter);
        }
    }
    // Serial.println(command);
}

void openDoorFromServer(const String &url)
{
    String command;
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, url);
        int httpCode = http.GET();

        if (httpCode > 0)
        {

            JsonDocument doc;
            deserializeJson(doc, http.getString());
            temp = doc["command"].as<String>();
            // Serial.println(temp);
            if (temp.startsWith("OPEN_"))
            {
                command = temp.substring(5);
                if (command[0] - 48 != '0')
                {
                    // openCharacter = (command[0] - 48);
                    mySerial.print(("OPEN_1"));
                    Serial.println(("OPEN_1"));
                }
            }
        }
        else
        {
            Serial.println("Error on HTTP request");
        }

        http.end();
    }
    else
    {
        Serial.println("WiFi not connected");
    }
}

void setDoorClosingToServer(const String &url, String commandFromUno)
{
    String command;
    temp = commandFromUno;
    command = temp.substring(5);
    // Serial.println(temp);
    if (command[0] != openCharacter)
    {
        openCharacter = command[0];
        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, url);
            http.addHeader("Content-Type", "application/json");
            String jsonData = "{\"open\":\"";
            jsonData += (openCharacter - 48);
            jsonData += "\"}";
            http.POST(jsonData);
            // Serial.println(modeCharacter);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    mySerial.begin(9600);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
void loop()
{
    getDataModeFromServer(url + "/changemode");
    openDoorFromServer(url + "/open");
    if (mySerial.available())
    {
        String command = mySerial.readString();
        if (command.startsWith("MODE_"))
        {
            sendDataModeToServer(url + "/changemode", command);
        }
        if (command.startsWith("OPEN_"))
        {
            setDoorClosingToServer(url + "/open", command);
        }
    }
}