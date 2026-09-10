import urllib.request
import time
import os

ESP32_IP = "192.168.0.94"
CLASS_NAME = "left"
INTERVAL_SEC = 0.1

if not os.path.exists(CLASS_NAME):
    os.makedirs(CLASS_NAME)

img_count = 0
print(f"Collecting images for class '{CLASS_NAME}'...")
print(f"Ensure your ESP32 is running and accessible at http://{ESP32_IP}")

while True:
    try:
        url = f"http://{ESP32_IP}/roi"
        filename = f"{CLASS_NAME}/{CLASS_NAME}_{int(time.time()*1000)}.jpg"
        urllib.request.urlretrieve(url, filename)
        img_count += 1
        print(f"Saved {filename} (Total: {img_count})")
    except Exception as e:
        print(f"Error fetching image: {e}")
    
    time.sleep(INTERVAL_SEC)
