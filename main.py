from flask import Flask, request, jsonify, render_template
from datetime import datetime
from flask_cors import CORS

app = Flask(__name__)
readings = []  # store last 100 readings
CORS(app)

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

@app.route('/')
def latest():
    if not readings:
        return render_template('index.html', 
                             temperature=None, 
                             humidity=None, 
                             device=None,
                             timestamp=None,
                             history=[])
    
    latest_reading = readings[-1]
    # Get last 10 readings for history
    recent_history = readings[-10:] if len(readings) >= 10 else readings
    
    return render_template('index.html',
                         temperature=latest_reading['temperature'],
                         humidity=latest_reading['humidity'],
                         device=latest_reading['device'],
                         timestamp=latest_reading['ts'],
                         history=recent_history[::-1])  # Reverse to show newest first

@app.route('/history')
def history():
    return jsonify(readings)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)