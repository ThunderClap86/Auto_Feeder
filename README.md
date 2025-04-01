# Auto_Feeder
 Automatic pet feeder driven by Arduino Nano, ESP32 or RPi Pico2W

An Arduino, Raspberry Pi Pico 2W, or ESP32 controlling a device that releases pet food at regular intervals.
# Hardware

    1 28BYJ-48 Stepper motor and ULN2003 motor driver (here)
    2 momentary push buttons (any kind will do, but you'll have to modify the file for anything other than what I used)
    Omron D2F limit switch (here)
    Arduino Nano (here) OR ESP32-WROOM-32 module (here) OR Raspberry Pi Pico W/2W
    5V power supply OR (Buck converter) capable of 5V. (2.5A rated or better preferred.)
    10x - 2ml Autosampler Vials (here)
    WIRE! Your choice.
    Dupont connectors and crimps to make your own jumpers/cables.
    Screws:
      2-M3x8 SHCS w/nuts (mount stepper motor)
      2-M2.5x6 coarse thread screws (mount limit switch)
 

# Key Functions:

    //WiFi Connection and NTP Time: The Pico W connects to WiFi and retrieves the current time via the NTP //protocol. It then checks if it's time to feed based on the scheduled time (FEED_HOUR and FEED_MINUTE).

    //Web Interface: A simple web page allows you to set the feeding time (hour and minute). The setTime //route captures the user input and updates the FEED_HOUR and FEED_MINUTE variables.

    //Forward/Reverse Button: The BUTTON_FORWARD_PIN and BUTTON_REVERSE_PIN are set up to manually move the //motor forward (350 steps) and reverse (3200 steps). The limit switch is checked during the reverse //movement, and if engaged, the motor moves forward 100 steps to disengage the switch.

    //Limit Switch: The limit switch is used to stop the motor when it's engaged during reverse movement. //The motor will automatically move forward 100 steps to disengage the limit switch.

    //Motor Control: The motor control is based on a stepper sequence. The step_motor function moves the //motor in the desired direction by controlling the motor pins.

# Testing:

    //Upload the code to your Raspberry Pi Pico W (or Pico 2W).
    //Monitor the Serial Output for connection and feeding time logs.
    //Access the Webpage: Open the IP address of the Pico in a browser (shown in the Serial Monitor) to set //the feeding time.
    //Manual Control: Press the forward or reverse buttons to control the motor manually.
    //Limit Switch: If the reverse movement is interrupted by the limit switch, the motor will move forward //100 steps to disengage it.

# Libraries:

Make sure to install the following libraries for WiFi devices (RPi Pico2W / ESP32):

    WiFi (for WiFi functionality)
    NTPClient (for NTP time synchronization)
    ESPAsyncWebServer (for creating the web interface)**Work In Pogress**

This setup combines time scheduling with manual motor control, making it ideal for the Raspberry Pi PicoW/2W and ESP32 with web interface functionality.

# Testing:

    Connect the Pico W/ESP32 to WiFi using your credentials (ssid and password).
    Open a browser and navigate to the IP address shown in the Serial Monitor to set the feeding time //(e.g., http://192.168.x.x).
    Adjust the feeding time through the webpage, and the Pico will follow the updated schedule.
    The motor will move at the scheduled time every day (or based on the interval you define).

# Requirements:

    Arduino IDE (Make sure to select Raspberry Pi Pico W/2W as the board).
	When connecting 5V to the Pico 2W, Arduino Nano, or ESP32, here's what you need to know:

# Power Input:

    The 5V input is typically provided IN through the VBUS pin (pin 40) on the Raspberry Pi Pico.
    VBUS is directly connected to the 5V line from the USB input if you're powering the Pico via USB, or you can directly supply 5V to this pin.

    Some Arduino clones CANNOT handle 5V directly, make sure you know before you fry your device. 

    ESP32 5V input is provided IN through the VIN pin. 

# 3.3V Output:

    The 3.3V pin (pin 36 Pico 2W) provides 3.3V regulated voltage, which is used to power peripherals that require 3.3V (The limit switch w/LED in my case).
    This 3.3V voltage is generated from the 5V input via an onboard voltage regulator. When the board is powered with 5V, the onboard regulator steps down the voltage to 3.3V to power the microcontroller and other peripherals that require 3.3V.

# Summary:

    5V input: VBUS or VIN pin or the 5V input from the USB - USB current will NOT be sufficient at powering the ULN2003 Stepper Driver AND Motor. 
    3.3V output: 3.3V pin (3V3) which can be used to power peripherals at 3.3V.

//Be cautious when connecting peripherals to the 3.3V pin, as the ESP32, Arduino and Pico W/2W can only provide a limited amount of current. If you need to power peripherals that draw significant current, consider using an external power supply to avoid overloading the onboard regulator. If you use separate power supplies (or buck converters) ensure all of them share a common ground. 
