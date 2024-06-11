
#include <Wire.h>
#include "DHT20.h"
#include "Arduino.h"
#include "Si115X.h"

#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <HttpClient.h>
#include <WiFi.h>
#include <string>
// Define pins for the second I2C bus (for Grove Sunlight Sensor)
#define SDA2 32
#define SCL2 33

int soilPin = 36;
int soilPower = 27;
int val = 0;

// Initialize default I2C bus for DHT20 (uses SDA: 21, SCL: 22)

// Initialize the second I2C bus for the Grove Sunlight Sensor
TwoWire I2C_2 = TwoWire(1);
Si115X SI1151 = Si115X();
DHT20 temp_humid = DHT20(&I2C_2);

void connectWifi()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    // Write
    Serial.printf("Updating ssid/pass in NVS ... ");

    // char ssid[] = "BCC-GUEST";
    // char pass[] = "acts1711!";
    char ssid[50] = "TMO";       // your network SSID (name)
    char pass[50] = "xdtimmyxd"; // your network password (use for WPA, or use
    err = nvs_set_str(my_handle, "ssid", ssid);
    err |= nvs_set_str(my_handle, "pass", pass);
    Serial.printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes
    // are written to flash storage. Implementations may write to storage at
    // other times, but this is not guaranteed.
    Serial.printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    Serial.printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    // Close
    nvs_close(my_handle);
  }
}

// This example downloads the URL "http://arduino.cc/"
// char ssid[50] = "BCC-GUEST"; // your network SSID (name)
// char pass[50] = "acts1711!"; // your network password (use for WPA, or use
char ssid[50] = "TMO";       // your network SSID (name)
char pass[50] = "xdtimmyxd"; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "worldtimeapi.org";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err)
    {
    case ESP_OK:
      Serial.printf("Done\n");
      // Serial.printf("SSID = %s\n", ssid);
      // Serial.printf("PASSWD = %s\n", pass);
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }

  // Close
  nvs_close(my_handle);
}

void setup()
{

  // Initialize Serial Monitor
  Serial.begin(9600);
  delay(2000);

  // CONNECT TO WIFI HERE///
  connectWifi();
  delay(1000);
  /////////////////////////

  nvs_access();
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println(pass);
  WiFi.begin(ssid, pass);
  // WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Initialize the default I2C bus (Light Sensor)
  Wire.begin(); // Default pins 21 (SDA) and 22 (SCL)
  Serial.println("Default I2C bus initialized (Light Sensor)");
  while (!SI1151.Begin())
  {
    Serial.println("SI1151 is not ready!");
    delay(1000);
  }

  // Initialize the second I2C bus (DHT20 Sensor)
  I2C_2.begin(SDA2, SCL2);
  Serial.println("I2C_2 initialized (DHT20 Sensor)");

  temp_humid.begin();
  Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (C)");

  pinMode(soilPower, OUTPUT);   // Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW); // Set to LOW so no power is flowing through the sensor
}

// void loop()
// {
//   // Read data from the Grove Sunlight Sensor
//   Serial.print("//--------------------------------------//\r\n");
//   Serial.print("Vis: ");
//   Serial.println(SI1151.ReadVisible());
//   Serial.print("IR: ");
//   Serial.println(SI1151.ReadIR()); // DO NOT NEED TO SEND
//   // the real UV value must be div 100 from the reg value , datasheet for more information.

//   // Read data from the DHT20 Sensor
//   temp_humid.read();
//   float temperature = temp_humid.getTemperature();
//   float humidity = temp_humid.getHumidity();
//   float moisture = 10;
//   Serial.print("Temperature: ");
//   Serial.print(temperature);
//   Serial.println(" °C");
//   Serial.print("Humidity: ");
//   Serial.print(humidity);
//   Serial.println(" %");
//   Serial.println();
//   delay(2000);

//   // sending to server
//   int err = 0;
//   WiFiClient c;
//   HttpClient http(c);

//   String url = "/?var=";
//   url += temperature;
//   url += ",";
//   url += humidity;
//   url += ",";
//   url += moisture;

//   const char *urlCharArray = url.c_str();
//   err = http.get("18.191.49.137", 5000, urlCharArray, NULL);

//   if (err == 0)
//   {
//     Serial.println("startedRequest ok");
//     err = http.responseStatusCode();
//     if (err >= 0)
//     {
//       Serial.print("Got status code: ");
//       Serial.println(err);
//       // Usually you'd check that the response code is 200 or a
//       // similar "success" code (200-299) before carrying on,
//       // but we'll print out whatever response we get
//       err = http.skipResponseHeaders();
//       if (err >= 0)
//       {
//         int bodyLen = http.contentLength();
//         Serial.print("Content length is: ");
//         Serial.println(bodyLen);
//         Serial.println();
//         Serial.println("Body returned follows:");
//         // Now we've got to the body, so we can print it out
//         unsigned long timeoutStart = millis();
//         char c;
//         // Whilst we haven't timed out & haven't reached the end of the body
//         while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout))
//         {
//           if (http.available())
//           {
//             c = http.read();
//             // Print out this character
//             Serial.print(c);
//             bodyLen--;
//             // We read something, reset the timeout counter
//             timeoutStart = millis();
//           }
//           else
//           {
//             // We haven't got any data, so let's pause to allow some to
//             // arrive
//             delay(kNetworkDelay);
//           }
//         }
//       }
//       else
//       {
//         Serial.print("Failed to skip response headers: ");
//         Serial.println(err);
//       }
//     }
//     else
//     {
//       Serial.print("Getting response failed: ");
//       Serial.println(err);
//     }
//   }
//   else
//   {
//     Serial.print("Connect failed: ");
//     Serial.println(err);
//   }
//   http.stop();

//   // Delay before the next reading
//   delay(2000);
// }

int readSoil()
{

  digitalWrite(soilPower, HIGH); // turn D7 "On"
  delay(10);                     // wait 10 milliseconds
  val = analogRead(soilPin);     // Read the SIG value form sensor
  digitalWrite(soilPower, LOW);  // turn D7 "Off"
  return val;                    // send current moisture value
}

// TESTING ONLY READING VALUES
//  void setup()
//  {

//   // Initialize Serial Monitor
//   Serial.begin(9600);
//   delay(2000);

//   // Initialize the default I2C bus (Light Sensor)
//   Wire.begin(); // Default pins 21 (SDA) and 22 (SCL)
//   Serial.println("Default I2C bus initialized (Light Sensor)");
//   while (!SI1151.Begin())
//   {
//     Serial.println("SI1151 is not ready!");
//     delay(1000);
//   }

//   // Initialize the second I2C bus (DHT20 Sensor)
//   I2C_2.begin(SDA2, SCL2);
//   Serial.println("I2C_2 initialized (DHT20 Sensor)");

//   temp_humid.begin();
//   Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (C)");

//   pinMode(soilPower, OUTPUT);   // Set D7 as an OUTPUT
//   digitalWrite(soilPower, LOW); // Set to LOW so no power is flowing through the sensor
// }

int counts = 0;
int light_luxs = 0;
int humidities = 0;
int moistures = 0;
int tempers = 0;

void loop()
{

  if (counts < 30)
  {
    // Read data from the Grove Sunlight Sensor
    Serial.print("//--------------------------------------//\r\n");
    Serial.print("Vis: ");

    int visibs = (int)SI1151.ReadVisible();
    Serial.println(visibs);
    // Serial.print("IR: ");
    // Serial.println(SI1151.ReadIR()); // DO NOT NEED TO SEND

    // Read data from the DHT20 Sensor
    temp_humid.read();
    float temperature = temp_humid.getTemperature();
    float humidity = temp_humid.getHumidity();
    float moisture = readSoil();
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.println("Moisture:");
    Serial.println(moisture);

    light_luxs += visibs;
    tempers += (int)temperature;
    moistures += (int)moisture;
    humidities += (int)humidity;
    delay(500);
    counts += 1;
    Serial.println(counts);
  }
  else if (counts == 30)
  {
    // SEND DATA
    light_luxs = (int)(light_luxs / 30);
    tempers = (int)(tempers / 30);
    moistures = (int)(moistures / 30);
    humidities = (int)(humidities / 30);
    Serial.println(light_luxs);
    Serial.println(tempers);
    Serial.println(moistures);
    Serial.println(humidities);
    // sending to server
    int err = 0;
    WiFiClient c;
    HttpClient http(c);

    // String url = "/greet?var=";
    // url += String(tempers, 1);
    // url += ",";
    // url += String(humidities, 1);
    // url += ",";
    // url += String(moistures, 1);
    // url += ",";
    // url += String(light_luxs, 1);

    std::string url = "/greet?var=";
    url += std::to_string(tempers);
    url += ",";
    url += std::to_string(humidities);
    url += ",";
    url += std::to_string(moistures);
    url += ",";
    url += std::to_string(light_luxs);
    Serial.println("PRINTING URL!!!");
    Serial.println(url.c_str());

    const char *urlCharArray = url.c_str();
    err = http.get("18.191.193.162", 5000, urlCharArray, NULL);
    // err = http.post("18.191.193.162", 5000, urlCharArray, NULL);

    if (err == 0)
    {
      Serial.println("startedRequest ok");
      err = http.responseStatusCode();
      if (err >= 0)
      {
        Serial.print("Got status code: ");
        Serial.println(err);
        // Usually you'd check that the response code is 200 or a
        // similar "success" code (200-299) before carrying on,
        // but we'll print out whatever response we get
        err = http.skipResponseHeaders();
        if (err >= 0)
        {
          int bodyLen = http.contentLength();
          Serial.print("Content length is: ");
          Serial.println(bodyLen);
          Serial.println();
          Serial.println("Body returned follows:");
          // Now we've got to the body, so we can print it out
          unsigned long timeoutStart = millis();
          char c;
          // Whilst we haven't timed out & haven't reached the end of the body
          while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout))
          {
            if (http.available())
            {
              c = http.read();
              // Print out this character
              Serial.print(c);
              bodyLen--;
              // We read something, reset the timeout counter
              timeoutStart = millis();
            }
            else
            {
              // We haven't got any data, so let's pause to allow some to
              // arrive
              delay(kNetworkDelay);
            }
          }
        }
        else
        {
          Serial.print("Failed to skip response headers: ");
          Serial.println(err);
        }
      }
      else
      {
        Serial.print("Getting response failed: ");
        Serial.println(err);
      }
    }
    else
    {
      Serial.print("Connect failed: ");
      Serial.println(err);
    }
    http.stop();

    // Delay before the next reading
    delay(10);
    counts += 1;
  }
}
