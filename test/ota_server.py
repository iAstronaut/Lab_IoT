from flask import Flask, send_file, send_from_directory
import os

app = Flask(__name__)

# Directory containing firmware files
FIRMWARE_DIR = "firmware"
VERSION_FILE = os.path.join(FIRMWARE_DIR, "version.txt")
FIRMWARE_FILE = os.path.join(FIRMWARE_DIR, "firmware.bin")

@app.route('/version.txt')
def get_version():
    try:
        with open(VERSION_FILE, 'r') as f:
            return f.read().strip()
    except FileNotFoundError:
        return "1.0.0"

@app.route('/firmware.bin')
def get_firmware():
    try:
        return send_file(FIRMWARE_FILE, mimetype='application/octet-stream')
    except FileNotFoundError:
        return "Firmware file not found", 404

if __name__ == '__main__':
    # Create firmware directory if it doesn't exist
    os.makedirs(FIRMWARE_DIR, exist_ok=True)

    # Create test firmware file if it doesn't exist
    if not os.path.exists(FIRMWARE_FILE):
        with open(FIRMWARE_FILE, 'wb') as f:
            f.write(b'Test firmware content' * 1000)  # Create a 19KB test file

    # Create version file if it doesn't exist
    if not os.path.exists(VERSION_FILE):
        with open(VERSION_FILE, 'w') as f:
            f.write("1.0.1")

    app.run(host='0.0.0.0', port=5000)