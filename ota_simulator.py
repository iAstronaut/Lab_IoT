import requests
import json
import time
import os
from datetime import datetime
import uuid
import random

class ESP32Simulator:
    def __init__(self, api_key, product_id):
        self.api_key = api_key
        self.product_id = product_id
        self.base_url = "https://api.otadrive.com/deviceapi/v1"
        self.current_version = "1.0.0"

        # Generate ESP32-like MAC address (starting with 24:0A:C4 for ESP32)
        mac_bytes = [0x24, 0x0A, 0xC4]  # ESP32 OUI (Organizationally Unique Identifier)
        for _ in range(3):
            mac_bytes.append(random.randint(0x00, 0xFF))
        self.device_id = ':'.join([f"{b:02X}" for b in mac_bytes])

        self.headers = {
            "X-API-KEY": api_key,
            "Content-Type": "application/json",
            "User-Agent": "ESP32/1.0"  # Add ESP32 user agent
        }

        # ESP32 specific device info
        self.device_info = {
            "name": f"ESP32_{self.device_id.replace(':', '')}",
            "type": "ESP32-WROOM-32",
            "firmware_version": self.current_version,
            "mac": self.device_id.lower(),  # MAC addresses are typically lowercase
            "ip_address": "192.168.1.100",
            "sdk_version": "v4.4.5",  # ESP-IDF version
            "last_seen": datetime.now().isoformat()
        }

        print(f"\nInitializing ESP32 Simulator")
        print(f"--------------------------------")
        print(f"API Key: {api_key}")
        print(f"Product ID: {product_id}")
        print(f"MAC Address: {self.device_id}")
        print(f"Device Name: {self.device_info['name']}")
        print(f"ESP32 Type: {self.device_info['type']}")
        print(f"SDK Version: {self.device_info['sdk_version']}")
        print(f"--------------------------------\n")

    def register_device(self):
        """Register the device with OTADrive"""
        url = f"{self.base_url}/device"
        payload = {
            "mac": self.device_info["mac"],
            "firmware_version": self.current_version,
            "device_name": self.device_info["name"],
            "ip": self.device_info["ip_address"],
            "type": self.device_info["type"],
            "sdk": self.device_info["sdk_version"]
        }

        print("\nRegistering ESP32 device...")
        print(f"URL: {url}")
        print(f"Headers: {json.dumps(self.headers, indent=2)}")
        print(f"Payload: {json.dumps(payload, indent=2)}")

        try:
            response = requests.post(url, headers=self.headers, json=payload)
            print(f"\nResponse Status Code: {response.status_code}")
            print(f"Response Headers: {dict(response.headers)}")
            print(f"Response Body: {response.text}")

            if response.status_code in [200, 201]:
                print(f"ESP32 device registered successfully")
                return True
            else:
                print(f"Failed to register ESP32 device. Status code: {response.status_code}")
                return False
        except Exception as e:
            print(f"Error registering device: {e}")
            return False

    def update_device_status(self):
        """Update device status with OTADrive"""
        url = f"{self.base_url}/device/status"
        payload = {
            "mac": self.device_info["mac"],
            "firmware_version": self.current_version,
            "ip": self.device_info["ip_address"],
            "type": self.device_info["type"],
            "sdk": self.device_info["sdk_version"]
        }

        try:
            response = requests.post(url, headers=self.headers, json=payload)
            if response.status_code in [200, 201]:
                print("ESP32 status updated successfully")
                return True
            else:
                print(f"Failed to update ESP32 status. Status code: {response.status_code}")
                print(f"Response: {response.text}")
                return False
        except Exception as e:
            print(f"Error updating device status: {e}")
            return False

    def check_for_updates(self):
        url = f"{self.base_url}/device/update"
        payload = {
            "mac": self.device_info["mac"],
            "firmware_version": self.current_version,
            "type": self.device_info["type"],
            "sdk": self.device_info["sdk_version"]
        }

        try:
            response = requests.post(url, headers=self.headers, json=payload)
            if response.status_code == 200:
                data = response.json()
                if data.get("status") == "update_available":
                    latest_version = data.get("version")
                    firmware_url = data.get("url")
                    print(f"New firmware available: {latest_version}")
                    return True, firmware_url
            return False, None
        except Exception as e:
            print(f"Error checking for updates: {e}")
            return False, None

    def download_firmware(self, firmware_url):
        try:
            response = requests.get(firmware_url, headers={"User-Agent": "ESP32/1.0"})
            if response.status_code == 200:
                with open("firmware.bin", "wb") as f:
                    f.write(response.content)
                return True
            return False
        except Exception as e:
            print(f"Error downloading firmware: {e}")
            return False

    def simulate_update(self):
        print(f"Starting ESP32 simulation...")
        print(f"Current firmware version: {self.current_version}")

        # Register device first
        if not self.register_device():
            print("Failed to register ESP32 device. Exiting...")
            return

        while True:
            print("\nChecking for firmware updates...")
            update_available, firmware_url = self.check_for_updates()

            if update_available and firmware_url:
                print("New firmware available! Starting download...")
                if self.download_firmware(firmware_url):
                    print("Firmware downloaded successfully")
                    print("Simulating ESP32 update process...")
                    time.sleep(2)
                    print("Update completed!")
                    self.current_version = "1.0.1"
                    self.update_device_status()
                    break
                else:
                    print("Failed to download firmware")

            self.update_device_status()
            print("No updates available. Waiting 30 seconds...")
            time.sleep(30)

def main():
    # OTADrive credentials
    API_KEY = "85e7d3e5-05ab-46ad-bd6b-fbbe3492491f"
    PRODUCT_ID = "34000"

    # Create and run simulator
    simulator = ESP32Simulator(API_KEY, PRODUCT_ID)
    simulator.simulate_update()

if __name__ == "__main__":
    main()