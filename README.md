
# TTGO T-Display ESP32 – Clock + Weather + Spotify + F1 Dashboard

This project transforms your TTGO T-Display ESP32 into a vibrant real-time dashboard with multiple views including weather, clock, Spotify track info, and F1 driver standings. It utilizes OpenWeatherMap, Spotify Web API, and Ergast F1 API to create an informative and interactive 135x240 TFT widget.

<img src="demo.gif" alt="Live demo of widget display" width="400" height="320">

---

## 🚀 Features

- **Live Weather Display** (via OpenWeatherMap API):
  - 🌡️ Temperature (color-coded)
  - 💧 Humidity
  - 🌬️ Wind speed + direction
  - 📈 Pressure
  - 🌅 Sunrise / 🌇 Sunset
- **NTP Clock** synced to Indian Standard Time (IST)
- **Spotify Now Playing / Last Played** (via Spotify Web API):
  - 🎶 Song Title
  - 🧑‍🎤 Artist
  - 💼 Album
  - ♻️ Refresh Token Authentication
- **Live F1 Driver Standings** (via Ergast API):
  - 🏎️ Top 10 drivers
  - 🎨 Color-coded by constructor
- **Button Navigation**:
  - Button 1: Toggle Weather ↔ Spotify
  - Button 2: Show F1 Standings

---

## 🛠️ Requirements

### Hardware
- TTGO T-Display ESP32
- USB-C cable
- Wi-Fi access

### Libraries
Install these libraries via the Arduino Library Manager:
- `WiFi.h`
- `HTTPClient.h`
- `ArduinoJson`
- `TFT_eSPI`
- `SPI.h`
- `time.h`
- `Button2`
- `WiFiClientSecure`
- `mbedtls/base64.h`

---

## 📦 Setup Instructions

1. Clone or download this repo.
2. Open the desired `.ino` file:
   - `clockandweather.ino` → Weather + Clock only
   - `clock+weatherandspotify.ino` → Weather + Clock + Spotify
   - `clock+weather+spotify+F1.ino` → Full Version (Weather + Clock + Spotify + F1)
3. Edit credentials in the code:

```cpp
const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";
const char* apiKey = "YourOpenWeatherMapAPIKey";
const char* city = "YourCity";
const char* country = "YourCountryCode"; // e.g., "IN"
```

### For Spotify API:

1. Visit https://developer.spotify.com/dashboard
2. Create a new app and note your **Client ID** and **Client Secret**.
3. Use a tool like [Spotify Auth Code Generator](https://developer.spotify.com/console/post-refresh-token/) or [this guide](https://github.com/BjoernKW/spotify-refresh-token-generator) to get your **refresh token**.
4. Set the redirect URI to something like `http://localhost:8888/callback`
5. Paste these into the code:

```cpp
String spotifyRefreshToken = "YourRefreshToken";
String spotifyClientId = "YourClientId";
String spotifyClientSecret = "YourClientSecret";
```

---

## 💽 Screen Layout

### Weather View
```
┌──────────────────────────────┐
│ DATE                TEMP     │
│                              │
│          HH:MM               │
│   City - WeatherType         │
│                              │
│ Humidity  Sunrise/Sunset Info│
│ Wind Speed   Direction       │
└──────────────────────────────┘
```

### Spotify View
```
┌──────────────────────────────┐
│         Spotify              │
│  Now Playing / Last Played   │
│      Song Title              │
│      Artist                  │
│        Album                 │
└──────────────────────────────┘
```

### F1 Standings View
```
┌──────────────────────────────┐
│  Top 10 F1 Drivers - 2025    │
│  1. PIA - 186 pts            │
│  2. NOR - 176 pts            │
│  3. VER - 137 pts            │
│  ...                         │
└──────────────────────────────┘
```

---

## ⏱️ Refresh Rates

- **Weather Data**: Every 5 minutes (300,000 ms)
- **Clock Update**: Every 1 minute (60,000 ms)
- **Spotify Info**: Refreshed on entering the view or switching
- **F1 Standings**: Refreshed each time the view is loaded

---

## ✅ To-Do List

- [x] Weather + Clock Display
- [x] Button toggle between views
- [x] Spotify integration (OAuth2 + Now Playing)
- [x] F1 Standings via Ergast API
- [ ] Smooth UI transitions
- [ ] Offline handling

---

## 👤 Author

**Nikhil Nair**  
🌐 [nikhilnair.works](https://nikhilnair.works)  
🔗 [LinkedIn](https://linkedin.com/in/nikhilnair29)  
💻 [GitHub](https://github.com/icebelly29)

---

Made with ❤️ and a trusty TTGO ESP32 that bravely survives drop testing.
