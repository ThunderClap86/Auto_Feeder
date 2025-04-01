# Run the feeder system
import esp_feeder

# boot.py

import network
import time
import machine
import esp
import gc

# Clear up memory (optional)
gc.collect()

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect("SSID", "PASSWORD") # Enter your wifi name (SSID) and password between the quotes

    while not wlan.isconnected():
        print("Connecting to Wi-Fi...")
        time.sleep(1)

    print("Connected to Wi-Fi:", wlan.ifconfig())

# Connect to Wi-Fi when the device boots
connect_wifi()


