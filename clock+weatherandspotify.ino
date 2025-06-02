// This Code file includes Weather + Clock and Spotify Status

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <time.h>
#include <Button2.h>
#include <WiFiClientSecure.h>
#include <mbedtls/base64.h>

#define BUTTON_1 35
#define BUTTON_2 0
#define ADC_EN   14

const char* ssid = "YourWiFiName";  // <-- Replace with your SSID (WiFi Username)
const char* password = "YourWiFiPassword";  // <-- Replace with your WiFi Password
const char* apiKey = "YourOpenWeatherMapAPIKey";  // <-- Replace with your API key
const char* city = "YourCity";  // <-- Replace with your City eg. "Kochi"
const char* country = "YourCountryCode";  // <-- Replace with your Country Code eg. "IN"
const char* units = "metric";

String spotifyAccessToken = "";
String spotifyRefreshToken = "YourSpotifyRefreshToken";  // <-- Replace with your Refresh Token (Use the guide in the README)
String spotifyClientId = "YourSpotifyClientID";   // <-- Replace with your Spotiy Client ID (Use the guide in the README)
String spotifyClientSecret = "YourSpotifyClientSecret";  // <-- Replace with your Spotiy Client Secret (Use the guide in the README)

TFT_eSPI tft = TFT_eSPI(135, 240);
Button2 btn1(BUTTON_1);

enum Screen { WEATHER_SCREEN, SPOTIFY_SCREEN };
Screen currentScreen = WEATHER_SCREEN;
bool redraw = true;

String weatherDescription = "", cityName = "", windDirectionStr = "";
float temperature = 0.0, humidity = 0.0, pressure = 0.0, windSpeed = 0.0;
int windDeg = 0;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 300000;
time_t sunrise = 0, sunset = 0;

bool isPlaying = false;
String trackTitle = "Unknown Title";
String artistName = "Unknown Artist";
String albumName = "Unknown Album";

String windDegToCompass(int deg) {
  const char* directions[] = {"N","NE","E","SE","S","SW","W","NW"};
  return directions[(int)((deg + 22.5) / 45.0) % 8];
}

String capitalizeWords(String text) {
  bool newWord = true;
  for (int i = 0; i < text.length(); i++) {
    if (isalpha(text[i])) {
      text[i] = newWord ? toupper(text[i]) : tolower(text[i]);
      newWord = false;
    } else {
      newWord = true;
    }
  }
  return text;
}

uint16_t getWeatherColor(String description) {
  description.toLowerCase();
  if (description.indexOf("clear") >= 0) return TFT_YELLOW;
  if (description.indexOf("cloud") >= 0) return TFT_LIGHTGREY;
  if (description.indexOf("rain") >= 0) return TFT_BLUE;
  if (description.indexOf("drizzle") >= 0) return TFT_CYAN;
  if (description.indexOf("thunder") >= 0) return TFT_PURPLE;
  if (description.indexOf("snow") >= 0) return TFT_WHITE;
  if (description.indexOf("mist") >= 0 || description.indexOf("fog") >= 0) return TFT_DARKGREY;
  return TFT_GREEN;
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
      weatherDescription = doc["weather"][0]["description"].as<String>();
      windSpeed = doc["wind"]["speed"];
      windDeg = doc["wind"]["deg"];
      windDirectionStr = windDegToCompass(windDeg);
      sunrise = doc["sys"]["sunrise"];
      sunset = doc["sys"]["sunset"];
    }
    http.end();
    lastWeatherUpdate = millis();
  }
}

void refreshSpotifyToken() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, "https://accounts.spotify.com/api/token")) {
    String creds = spotifyClientId + ":" + spotifyClientSecret;
    unsigned char output[128];
    size_t outLen;
    mbedtls_base64_encode(output, sizeof(output), &outLen, (const unsigned char*)creds.c_str(), creds.length());
    String auth = String((char*)output);

    https.addHeader("Authorization", "Basic " + auth);
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String body = "grant_type=refresh_token&refresh_token=" + spotifyRefreshToken;

    int code = https.POST(body);
    if (code == HTTP_CODE_OK) {
      String response = https.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      spotifyAccessToken = doc["access_token"].as<String>();
    }
    https.end();
  }
}

void fetchSpotifyTrack() {
  if (spotifyAccessToken == "") refreshSpotifyToken();

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, "https://api.spotify.com/v1/me/player/currently-playing")) {
    https.addHeader("Authorization", "Bearer " + spotifyAccessToken);
    int code = https.GET();
    if (code == HTTP_CODE_UNAUTHORIZED) {
      refreshSpotifyToken();
      https.end();
      return fetchSpotifyTrack();
    }
    if (code == HTTP_CODE_OK) {
      String body = https.getString();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, body);

      isPlaying = doc["is_playing"] | false;
      trackTitle = doc["item"]["name"].as<String>();
      artistName = doc["item"]["artists"][0]["name"].as<String>();
      albumName = doc["item"]["album"]["name"].as<String>();
    }
    https.end();
  }
}

String formatUnixTime(time_t rawTime) {
  struct tm* timeInfo = localtime(&rawTime);
  char buffer[6];
  strftime(buffer, sizeof(buffer), "%H:%M", timeInfo);
  return String(buffer);
}

void drawWeatherScreen() {
  tft.fillScreen(TFT_BLACK);
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char timeStr[6], dateStr[12];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  strftime(dateStr, sizeof(dateStr), "%d %b", &timeinfo);

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(dateStr, 5, 5, 2);

  uint16_t tempColor = temperature < 24 ? TFT_BLUE : (temperature <= 31 ? TFT_GREEN : TFT_ORANGE);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(tempColor);
  tft.drawString(String(temperature, 1) + " °C", 235, 5, 2);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString(timeStr, 120, 50, 6);

  String desc = capitalizeWords(weatherDescription);
  tft.setTextColor(getWeatherColor(desc));
  tft.drawString(cityName + " - " + desc, 120, 80, 2);

  float windKmph = windSpeed * 3.6;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Humidity: " + String(humidity, 0) + "%", 5, 100, 2);
  tft.drawString("Wind: " + String(windKmph, 1) + " km/h", 5, 120, 2);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("Direction: " + windDirectionStr, 235, 120, 2);

  time_t now = time(nullptr);
  String label = (now < sunset) ? "Sunset: " + formatUnixTime(sunset) : "Sunrise: " + formatUnixTime(sunrise);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_ORANGE);
  tft.drawString(label, 190, 105, 2);
}

void drawSpotifyScreen() {
  fetchSpotifyTrack();
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("Spotify", 120, 10, 2);

  tft.setTextColor(TFT_WHITE);
  tft.drawString(isPlaying ? "Now Playing:" : "Last Played:", 120, 35, 2);
  tft.setTextColor(TFT_CYAN);
  tft.drawString(trackTitle, 120, 60, 2);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString(artistName, 120, 85, 2);
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString(albumName, 120, 110, 2);
}

void buttonHandler(Button2& btn) {
  if (btn.wasPressed()) {
    currentScreen = (currentScreen == WEATHER_SCREEN) ? SPOTIFY_SCREEN : WEATHER_SCREEN;
    redraw = true;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.setTextDatum(MC_DATUM);

  btn1.begin(BUTTON_1);
  btn1.setPressedHandler(buttonHandler);

  connectWiFi();
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  fetchWeather();
  fetchSpotifyTrack();
}

void loop() {
  btn1.loop();

  if (millis() - lastWeatherUpdate > 10000) {
    fetchWeather();
  }

  if (redraw) {
    if (currentScreen == WEATHER_SCREEN) drawWeatherScreen();
    else drawSpotifyScreen();
    redraw = false;
  }

  delay(100);
}
