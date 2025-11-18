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
        "ts": datetime.now().isoformat() + "Z"
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



@app.route('/')
def index():
    return readings

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
