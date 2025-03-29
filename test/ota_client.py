import requests
import time
import os
import sys

class OTAUpdater:
    def __init__(self, server_url, current_version="1.0.0"):
        self.server_url = server_url
        self.current_version = current_version
        self.firmware_url = f"{server_url}/firmware.bin"
        self.version_url = f"{server_url}/version.txt"

    def check_update(self):
        try:
            response = requests.get(self.version_url)
            if response.status_code == 200:
                new_version = response.text.strip()
                print(f"Current version: {self.current_version}")
                print(f"Server version: {new_version}")
                return new_version != self.current_version
            return False
        except Exception as e:
            print(f"Error checking version: {e}")
            return False

    def download_firmware(self):
        try:
            response = requests.get(self.firmware_url, stream=True)
            if response.status_code == 200:
                content_length = int(response.headers.get('content-length', 0))
                print(f"Downloading firmware ({content_length} bytes)...")

                # Simulate writing to flash memory
                downloaded = 0
                for chunk in response.iter_content(chunk_size=8192):
                    if chunk:
                        downloaded += len(chunk)
                        # Simulate flash writing delay
                        time.sleep(0.01)
                        print(f"Downloaded: {downloaded}/{content_length} bytes")

                print("Firmware downloaded successfully!")
                return True
            return False
        except Exception as e:
            print(f"Error downloading firmware: {e}")
            return False

    def update(self):
        print("Starting OTA update process...")

        if self.check_update():
            print("New firmware available!")
            if self.download_firmware():
                print("Update completed successfully!")
                return True
            else:
                print("Update failed!")
                return False
        else:
            print("No update available.")
            return False

def main():
    # Get server URL from command line or use default
    server_url = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:5000"

    updater = OTAUpdater(server_url)

    while True:
        updater.update()
        print("Waiting 5 minutes before next check...")
        time.sleep(300)  # Wait 5 minutes

if __name__ == "__main__":
    main()