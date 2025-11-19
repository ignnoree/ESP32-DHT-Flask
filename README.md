
# ESP32 DHT22 Monitor + Flask Backend ![Python](https://img.shields.io/badge/Python-3.11-blue) ![ESP32](https://img.shields.io/badge/ESP32-OK-green)

**Minimal, reliable ESP32 + DHT22 setup to log temperature & humidity to a Flask backend.**  

Data is displayed on a 16x2 LCD and accessible via your Flask server.

---

## Features

- Real-time temperature & humidity logging from **DHT22**
- **16x2 LCD** display with status messages
- **JSON POST** to Flask backend
- Auto-retry & WiFi reconnection
- Lightweight, professional, and easy to integrate

---

## Hardware

| Component  | ESP32 Pin      |
|------------|----------------|
| DHT22 Data | 14             |
| DHT22 VCC  | 3.3V           |
| DHT22 GND  | GND            |
| LCD SDA    | 21 (default)   |
| LCD SCL    | 22 (default)   |
| LCD VCC    | 3.3V           |
| LCD GND    | GND            |

> Adjust SDA/SCL pins for your setup.  
> [Wokwi Demo](https://wokwi.com/projects/448046375787581441)

---

## Quick Setup

Edit WiFi credentials and server URL in `esp32_dht_lcd.ino`:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "https://your-flask-backend.vercel.app/data";
````

1. Upload the sketch to your ESP32.
2. Install required libraries: `DHTesp`, `LiquidCrystal_I2C`.
3. Open **Serial Monitor** for debug logs.

---

## Flask Backend Example

```python
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/data", methods=["POST"])
def receive_data():
    data = request.get_json()
    print(data)
    return jsonify({"success": True})
```

---

## Data Flow

* ESP32 reads sensor → updates LCD → sends JSON → Flask server
* Retry up to 3 times if network fails
* 60s interval between readings

---

## LCD Example

```
Temp: 24.5°C
Hum: 55.2%
```

or sending:

```
Sending...
```

