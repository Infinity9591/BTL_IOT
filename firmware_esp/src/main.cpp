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

unsigned char modeCharacter = '1';
unsigned char openCharacter = '0';
String pass;
String temp;
String temp1;
String temp2;

void getDataModeFromServer(const String &url)
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
            if (temp.startsWith("MODE_"))
            {
                command = temp.substring(5)[0];
                if (command[0] - 48 != modeCharacter)
                {
                    modeCharacter = (command[0] - 48);
                    mySerial.print(("MODE_" + String(modeCharacter)));
                    Serial.println(("MODE_" + String(modeCharacter)));
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
            Serial.println(jsonData);
        }
    }
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
            temp1 = doc["command"].as<String>();
            temp1.trim();
            if (temp1.startsWith("OPEN_"))
            {
                command = temp1.substring(5);
                command.trim();
                if (command[0] - 48 != openCharacter)
                {
                    openCharacter = (command[0] - 48);
                    mySerial.print(("OPEN_" + String(openCharacter)));
                    Serial.println(("OPEN_" + String(openCharacter)));
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
    temp1 = commandFromUno;
    command = temp1.substring(5);
    command.trim();
    if (command[0] - 48 != openCharacter)
    {
        openCharacter = command[0] - 48;
        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, url);
            http.addHeader("Content-Type", "application/json");
            String jsonData = "{\"open\":\"";
            jsonData += String(openCharacter);
            jsonData += "\"}";
            http.POST(jsonData);
            Serial.println(jsonData);
        }
    }
}

void getPasswordFromServer(const String &url)
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
            temp2 = doc["command"].as<String>();
            if (temp2.startsWith("PASS_"))
            {
                command = temp2.substring(5);
                command.trim();
                if (command != pass)
                {
                    pass = command;
                    mySerial.print(("PASS_" + command));
                    Serial.println(("PASS_" + command));
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

void setup()
{
    Serial.begin(9600);
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
    pinMode(LED_BUILTIN, OUTPUT);
    mySerial.begin(9600);
}
void loop()
{
    getPasswordFromServer(url + "/pass");
    getDataModeFromServer(url + "/changemode");
    openDoorFromServer(url + "/open");
    if (mySerial.available())
    {
        String command = mySerial.readString();
        command.trim();
        if (command.startsWith("MODE_"))
        {
            sendDataModeToServer(url + "/changemode", command);
        }
        if (command.startsWith("OPEN_"))
        {
            setDoorClosingToServer(url + "/open", command);
        }
    }
    if (Serial.available()){
        String command = Serial.readString();
        command.trim();
        if (command.startsWith("OPEN")){
            command = command.substring(4);
            mySerial.print("REGCONIZE_" + command);

        }
    }

}
