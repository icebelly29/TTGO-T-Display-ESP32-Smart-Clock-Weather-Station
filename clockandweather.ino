// This Code file includes Weather + Clock

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <time.h>

const char* ssid = "YourWiFiName";  // <-- Replace with your SSID (WiFi Username)
const char* password = "YourWiFiPassword";  // <-- Replace with your WiFi Password
const char* apiKey = "YourOpenWeatherMapAPIKey";  // <-- Replace with your API key
const char* city = "YourCity";  // <-- Replace with your City eg. "Kochi"
const char* country = "YourCountryCode";  // <-- Replace with your Country Code eg. "IN"
const char* units = "metric";

TFT_eSPI tft = TFT_eSPI();

String weatherDescription = "", cityName = "", windDirectionStr = "";
float temperature = 0.0, humidity = 0.0, pressure = 0.0, windSpeed = 0.0;
int windDeg = 0;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 300000;

String windDegToCompass(int deg) {
  const char* directions[] = {"N","NE","E","SE","S","SW","W","NW"};
  return directions[(int)((deg + 22.5) / 45.0) % 8];
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void fetchWeather() {
  if ((millis() - lastWeatherUpdate > weatherUpdateInterval) || lastWeatherUpdate == 0) {
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(country) + "&units=" + String(units) + "&appid=" + String(apiKey);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, payload);

      cityName = doc["name"].as<String>();
      temperature = doc["main"]["temp"];
      humidity = doc["main"]["humidity"];
      pressure = doc["main"]["pressure"];
      weatherDescription = doc["weather"][0]["main"].as<String>();
      windSpeed = doc["wind"]["speed"];
      windDeg = doc["wind"]["deg"];
      windDirectionStr = windDegToCompass(windDeg);
    }
    http.end();
    lastWeatherUpdate = millis();
  }
}

void setupTimeIST() {
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");  // UTC+5:30   // <-- Replace with your time zone  +5:30 GMT = 19800 seconds (1 Hour = 3600 seconds)
  while (time(nullptr) < 100000) delay(100);
}

void drawScreen() {
  tft.fillScreen(TFT_BLACK);
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char timeStr[6], dateStr[12], tempStr[10];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  strftime(dateStr, sizeof(dateStr), "%d %b", &timeinfo);
  sprintf(tempStr, "%.1f °C", temperature);

  // Date (top-left)
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(dateStr, 5, 5, 2);
  uint16_t tempColor;

  if (temperature < 24.0) {
    tempColor = TFT_BLUE;
  } else if (temperature <= 31.0) {
    tempColor = TFT_GREEN;
  } else {
    tempColor = TFT_ORANGE;
  }

  tft.setTextColor(tempColor);
  tft.drawString(String(temperature, 1) + " °C ", 240, 5, 2);  // Top-right

  // Temperature (top-right)
  tft.setTextDatum(TR_DATUM);
  tft.drawString(tempStr, 240, 5, 2);

  // Time (centered)
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString(timeStr, 120, 50, 6);

  // City • Description (combined)
  tft.setTextColor(TFT_CYAN);
  tft.drawString(cityName + " - " + weatherDescription, 120, 80, 2);

  // Humidity (bottom-left)
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Humidity: " + String(humidity, 0) + "%", 5, 95, 2);

  // Pressure (bottom-right)
  tft.setTextDatum(TR_DATUM);
  tft.drawString("Pressure: " + String(pressure, 0) + " hPa", 235, 95, 2);

  // Wind Speed (bottom-left row 2)
  tft.setTextDatum(TL_DATUM);
  float windKmph = windSpeed * 3.6;
  tft.drawString("Wind: " + String(windKmph, 1) + " km/h", 5, 115, 2);

  // Wind Direction (bottom-right row 2)
  tft.setTextDatum(TR_DATUM);
  tft.drawString("Direction: " + windDirectionStr, 235, 115, 2);
}

void setup() {
  tft.init();
  tft.setRotation(1);  // Landscape
  tft.setTextDatum(MC_DATUM);
  tft.fillScreen(TFT_BLACK);

  connectWiFi();
  setupTimeIST();
  fetchWeather();
  drawScreen();
}

void loop() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10000) {
    fetchWeather();
    drawScreen();
    lastUpdate = millis();
  }
}
