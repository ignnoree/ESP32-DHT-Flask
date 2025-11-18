# app.py
from flask import Flask, request, jsonify, render_template_string
from datetime import datetime

app = Flask(__name__)
readings = []  # store last 100 readings

@app.route('/data', methods=['POST'])
def data():
    payload = request.get_json(force=True)
    if not payload or 'temperature' not in payload or 'humidity' not in payload:
        return jsonify({"error":"bad payload"}), 400

    entry = {
        "device": payload.get("device","esp32"),
        "temperature": float(payload["temperature"]),
        "humidity": float(payload["humidity"]),
        "ts": datetime.utcnow().isoformat() + "Z"
    }
    readings.append(entry)
    if len(readings) > 100:
        readings.pop(0)
    return jsonify({"status":"ok", "received": entry}), 200

@app.route('/latest')
def latest():
    if not readings:
        return jsonify({"message":"no data"}), 200
    return jsonify(readings[-1]), 200

INDEX_HTML = """
<!doctype html>
<title>ESP32 DHT Dashboard</title>
<h1>ESP32 DHT Dashboard</h1>
<div id="last">Loading...</div>
<script>
async function fetchLatest(){
  try {
    let r = await fetch('/latest');
    let j = await r.json();
    if (j.temperature !== undefined) {
      document.getElementById('last').innerHTML =
        '<b>Device:</b> '+j.device+' <br>' +
        '<b>Temp:</b> '+j.temperature.toFixed(1)+' °C <br>' +
        '<b>Humidity:</b> '+j.humidity.toFixed(1)+' % <br>' +
        '<b>Time (UTC):</b> '+j.ts;
    } else {
      document.getElementById('last').innerText = JSON.stringify(j);
    }
  } catch(e){
    document.getElementById('last').innerText = 'Error: '+e;
  }
}
setInterval(fetchLatest, 3000);
fetchLatest();
</script>
"""

@app.route('/')
def index():
    return render_template_string(INDEX_HTML)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
