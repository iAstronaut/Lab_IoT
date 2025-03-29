#include "ota_server.h"

#define FIRMWARE_DIR "/firmware"
#define VERSION_FILE "/firmware/version.txt"
#define FIRMWARE_FILE "/firmware/firmware.bin"
#define SERVER_PORT 80
#define CURRENT_FIRMWARE_VERSION "1.0.0"

OTAServer::OTAServer() : server(SERVER_PORT) {}

String OTAServer::getChipId() {
    String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
    ChipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
    return ChipIdHex;
}

void OTAServer::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Create firmware directory if it doesn't exist
    if (!SPIFFS.exists(FIRMWARE_DIR)) {
        SPIFFS.mkdir(FIRMWARE_DIR);
    }

    // Create version file if it doesn't exist
    if (!SPIFFS.exists(VERSION_FILE)) {
        File f = SPIFFS.open(VERSION_FILE, "w");
        if (f) {
            f.println(CURRENT_FIRMWARE_VERSION);
            f.close();
        }
    }

    // Create firmware file if it doesn't exist
    if (!SPIFFS.exists(FIRMWARE_FILE)) {
        File f = SPIFFS.open(FIRMWARE_FILE, "w");
        if (f) {
            // Create a dummy firmware file
            for (int i = 0; i < 1000; i++) {
                f.write("Test firmware content");
            }
            f.close();
        }
    }

    // Setup routes
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleRoot(request);
    });

    server.on("/version.txt", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleVersion(request);
    });

    server.on("/firmware.bin", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleFirmware(request);
    });

    server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleUpdate(request);
    });

    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleStatus(request);
    });

    server.on("/upload", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleUpload(request);
    });

    server.on("/device", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleDeviceInfo(request);
    });

    server.begin();
    Serial.println("OTA Server started");
}

void OTAServer::handleDeviceInfo(AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    doc["chip_id"] = getChipId();
    doc["current_version"] = CURRENT_FIRMWARE_VERSION;
    doc["firmware_exists"] = SPIFFS.exists(FIRMWARE_FILE);

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void OTAServer::handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", getUploadPage());
}

void OTAServer::handleUpload(AsyncWebServerRequest *request) {
    if (request->hasParam("firmware", true)) {
        AsyncWebParameter* p = request->getParam("firmware", true);
        if (p->isFile()) {
            String filename = p->value();
            String version = request->getParam("version", true)->value();

            // Save version
            File vFile = SPIFFS.open(VERSION_FILE, "w");
            if (vFile) {
                vFile.println(version);
                vFile.close();
            }

            // Save firmware
            File fFile = SPIFFS.open(FIRMWARE_FILE, "w");
            if (fFile) {
                fFile.write((uint8_t*)p->value().c_str(), p->size());
                fFile.close();
            }

            request->send(200, "text/plain", "Firmware uploaded successfully");
        } else {
            request->send(400, "text/plain", "No file uploaded");
        }
    } else {
        request->send(400, "text/plain", "No firmware parameter");
    }
}

const char* OTAServer::getUploadPage() {
    return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Firmware Upload</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
        }
        input[type="file"], input[type="text"] {
            width: 100%;
            padding: 8px;
            margin-bottom: 10px;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        button:hover {
            background-color: #45a049;
        }
        .status {
            margin-top: 20px;
            padding: 10px;
            border-radius: 4px;
        }
        .success {
            background-color: #dff0d8;
            color: #3c763d;
        }
        .error {
            background-color: #f2dede;
            color: #a94442;
        }
        .device-info {
            background-color: #f8f9fa;
            padding: 15px;
            border-radius: 4px;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
    <h1>Firmware Upload</h1>
    <div class="device-info">
        <h3>Device Information</h3>
        <p>Current Version: <span id="current-version">Loading...</span></p>
        <p>Chip ID: <span id="chip-id">Loading...</span></p>
    </div>
    <form action="/upload" method="post" enctype="multipart/form-data">
        <div class="form-group">
            <label for="firmware">Select Firmware File (.bin):</label>
            <input type="file" id="firmware" name="firmware" accept=".bin" required>
        </div>
        <div class="form-group">
            <label for="version">Firmware Version:</label>
            <input type="text" id="version" name="version" required placeholder="e.g., 1.0.0">
        </div>
        <button type="submit">Upload Firmware</button>
    </form>
    <div id="status" class="status" style="display: none;"></div>

    <script>
        // Load device information
        fetch('/device')
            .then(response => response.json())
            .then(data => {
                document.getElementById('current-version').textContent = data.current_version;
                document.getElementById('chip-id').textContent = data.chip_id;
            })
            .catch(error => {
                console.error('Error loading device info:', error);
            });

        document.querySelector('form').onsubmit = function(e) {
            e.preventDefault();
            const formData = new FormData(this);
            const status = document.getElementById('status');

            fetch('/upload', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(result => {
                status.textContent = result;
                status.style.display = 'block';
                status.className = 'status ' + (result.includes('successfully') ? 'success' : 'error');
            })
            .catch(error => {
                status.textContent = 'Error uploading firmware: ' + error;
                status.style.display = 'block';
                status.className = 'status error';
            });
        };
    </script>
</body>
</html>
)rawliteral";
}

void OTAServer::handleVersion(AsyncWebServerRequest *request) {
    if (SPIFFS.exists(VERSION_FILE)) {
        File f = SPIFFS.open(VERSION_FILE, "r");
        if (f) {
            String version = f.readStringUntil('\n');
            f.close();
            request->send(200, "text/plain", version);
        } else {
            request->send(500, "text/plain", "Error reading version file");
        }
    } else {
        request->send(404, "text/plain", "Version file not found");
    }
}

void OTAServer::handleFirmware(AsyncWebServerRequest *request) {
    if (SPIFFS.exists(FIRMWARE_FILE)) {
        request->send(SPIFFS, FIRMWARE_FILE, "application/octet-stream");
    } else {
        request->send(404, "text/plain", "Firmware file not found");
    }
}

void OTAServer::handleUpdate(AsyncWebServerRequest *request) {
    if (request->hasParam("v") && request->hasParam("s")) {
        String version = request->getParam("v")->value();
        String chipId = request->getParam("s")->value();

        // Verify chip ID
        if (chipId != getChipId()) {
            request->send(400, "text/plain", "Invalid device ID");
            return;
        }

        if (checkFirmwareVersion(version.c_str())) {
            request->send(200, "text/plain", "Update available");
        } else {
            request->send(200, "text/plain", "No update available");
        }
    } else {
        request->send(400, "text/plain", "Version and device ID parameters required");
    }
}

void OTAServer::handleStatus(AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    doc["status"] = "running";
    doc["firmware_exists"] = SPIFFS.exists(FIRMWARE_FILE);
    doc["version_exists"] = SPIFFS.exists(VERSION_FILE);
    doc["chip_id"] = getChipId();
    doc["current_version"] = CURRENT_FIRMWARE_VERSION;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

bool OTAServer::checkFirmwareVersion(const char* version) {
    if (SPIFFS.exists(VERSION_FILE)) {
        File f = SPIFFS.open(VERSION_FILE, "r");
        if (f) {
            String serverVersion = f.readStringUntil('\n');
            f.close();
            return strcmp(version, serverVersion.c_str()) != 0;
        }
    }
    return false;
}

void OTAServer::handleLoop() {
    // Add any periodic tasks here if needed
}