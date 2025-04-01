//Required Libraries:

    //WiFi library (for the Raspberry Pi Pico W or Pico 2W WiFi connection).
    //NTPClient library (to get the current time via NTP).
    //ESPAsyncWebServer library (to serve the local webpage).

//Make sure you have these libraries installed in your Arduino IDE.
//Full Code for Raspberry Pi Pico W / Pico 2W - wifi, buttons and limit switch functionality.
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>

// WiFi credentials
const char* ssid = "your_SSID";  // Type your wifi name between the quotes
const char* password = "your_PASSWORD"; // Type your wifi password between the quotes

// Motor control pins - RPi Pico W/2W
#define MOTOR_PIN_1 2 //GP2
#define MOTOR_PIN_2 3 //GP3
#define MOTOR_PIN_3 4 //GP4
#define MOTOR_PIN_4 5 //GP5

// Button control pins
#define BUTTON_FORWARD_PIN 6 //GP6
#define BUTTON_REVERSE_PIN 7 //GP7

// Limit switch pin for stopping the motor
#define LIMIT_SWITCH_PIN 8 //GP8

// Feeding time variables - Hour and minute definition to set for feeding time.
int FEED_HOUR = 22;  // Hour in 24-hour format
int FEED_MINUTE = 33;  // Minute

// Timer variables for feeding
unsigned long last_feed_time = 0; // Store the time of last feed
const unsigned long FEED_DELAY = 1000 * 60 * 60 * 24; // 24 hours in milliseconds

// WiFi and NTP client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Time zone offset in seconds

// Web server
AsyncWebServer server(80);

// Stepper motor sequence (4 steps per full cycle)
#define STEP_COUNT 4
int steps[STEP_COUNT][4] = {
  {1, 0, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 1}
};

// Time interval for each step (in milliseconds)
#define STEP_PERIOD 20

void setup() {
  // Setup motor pins as output
  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  pinMode(MOTOR_PIN_3, OUTPUT);
  pinMode(MOTOR_PIN_4, OUTPUT);
  
  // Setup button pins as input with pull-up
  pinMode(BUTTON_FORWARD_PIN, INPUT_PULLUP);
  pinMode(BUTTON_REVERSE_PIN, INPUT_PULLUP);

  // Setup limit switch pin as input with pull-up
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

  // Initialize Serial for debugging
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  
  // Initialize NTP Client
  timeClient.begin();
  
  // Serve a simple web page to set the feeding time
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h1>Set Feeding Time</h1>";
    html += "<form action=\"/setTime\" method=\"GET\">";
    html += "Hour: <input type=\"number\" name=\"hour\" min=\"0\" max=\"23\" value=\"" + String(FEED_HOUR) + "\"><br>";
    html += "Minute: <input type=\"number\" name=\"minute\" min=\"0\" max=\"59\" value=\"" + String(FEED_MINUTE) + "\"><br>";
    html += "<input type=\"submit\" value=\"Set Time\">";
    html += "</form></body></html>";
    request->send(200, "text/html", html);
  });

  // Handle the form submission to update feeding time
  server.on("/setTime", HTTP_GET, [](AsyncWebServerRequest *request){
    String hour = request->getParam("hour")->value();
    String minute = request->getParam("minute")->value();
    FEED_HOUR = hour.toInt();
    FEED_MINUTE = minute.toInt();
    
    String message = "Feeding time set to: " + String(FEED_HOUR) + ":" + String(FEED_MINUTE);
    request->send(200, "text/html", "<html><body><h1>" + message + "</h1></body></html>");
  });

  // Start the server
  server.begin();
}

void loop() {
  // Get the current time from the NTP client
  timeClient.update();
  int current_hour = timeClient.getHours();
  int current_minute = timeClient.getMinutes();

  // Check if it's feeding time
  if (current_hour == FEED_HOUR && current_minute == FEED_MINUTE) {
    unsigned long current_time = millis();
    if (current_time - last_feed_time >= FEED_DELAY) {
      Serial.println("Feeding time reached! Moving motor forward.");
      step_motor(350, false);  // Move motor forward for feeding (350 steps)
      last_feed_time = current_time;
    }
  }

  // Check button presses to manually move the motor
  check_buttons();

  // Delay to avoid high CPU usage
  delay(1000);  // Check once every second
}

// Function to move motor for a given number of steps
void step_motor(int steps_to_move, bool reverse = false) {
  int direction = reverse ? -1 : 1;
  int step_idx = 0;
  int steps_left = abs(steps_to_move);

  while (steps_left > 0) {
    for (int pin = 0; pin < 4; pin++) {
      digitalWrite(MOTOR_PIN_1, steps[step_idx][0]);
      digitalWrite(MOTOR_PIN_2, steps[step_idx][1]);
      digitalWrite(MOTOR_PIN_3, steps[step_idx][2]);
      digitalWrite(MOTOR_PIN_4, steps[step_idx][3]);
    }
    step_idx = (step_idx + direction + STEP_COUNT) % STEP_COUNT;
    steps_left--;
    delay(STEP_PERIOD);

    // Check limit switch during reverse movement
    if (reverse && digitalRead(LIMIT_SWITCH_PIN) == HIGH) {  // Limit switch pressed
      Serial.println("Limit switch pressed during reverse: stopping motor");
      break;  // Stop motor if the limit switch is pressed
    }
  }

  // Stop the motor after movement
  stop_motor();
}

// Function to stop the motor
void stop_motor() {
  digitalWrite(MOTOR_PIN_1, LOW);
  digitalWrite(MOTOR_PIN_2, LOW);
  digitalWrite(MOTOR_PIN_3, LOW);
  digitalWrite(MOTOR_PIN_4, LOW);
}

// Function to check button presses
void check_buttons() {
  if (digitalRead(BUTTON_FORWARD_PIN) == LOW) {
    Serial.println("Moving motor forward");
    step_motor(350, false);  // Move motor forward (350 steps) *Tune this number to your build.
  }
  
  if (digitalRead(BUTTON_REVERSE_PIN) == LOW) {
    Serial.println("Moving motor reverse");
    step_motor(3200, true);  // Move motor reverse (3200 steps) *Tune this number to your build.

    // If limit switch was pressed, move forward 100 steps to disengage it *Tune this number to your build.
    if (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
      Serial.println("Limit switch engaged during reverse, moving forward 100 steps to disengage");
      step_motor(100, false);
    }
  }
}



//Key Functions:

    //WiFi Connection and NTP Time: The Pico W connects to WiFi and retrieves the current time via the NTP //protocol. It then checks if it's time to feed based on the scheduled time (FEED_HOUR and FEED_MINUTE).

    //Web Interface: A simple web page allows you to set the feeding time (hour and minute). The setTime //route captures the user input and updates the FEED_HOUR and FEED_MINUTE variables.

    //Forward/Reverse Button: The BUTTON_FORWARD_PIN and BUTTON_REVERSE_PIN are set up to manually move the //motor forward (350 steps) and reverse (3200 steps). The limit switch is checked during the reverse //movement, and if engaged, the motor moves forward 100 steps to disengage the switch.

    //Limit Switch: The limit switch is used to stop the motor when it's engaged during reverse movement. //The motor will automatically move forward 100 steps to disengage the limit switch.

    //Motor Control: The motor control is based on a stepper sequence. The step_motor function moves the //motor in the desired direction by controlling the motor pins.

//Testing:

    //Upload the code to your Raspberry Pi Pico W (or Pico 2W).
    //Monitor the Serial Output for connection and feeding time logs.
    //Access the Webpage: Open the IP address of the Pico in a browser (shown in the Serial Monitor) to set //the feeding time.
    //Manual Control: Press the forward or reverse buttons to control the motor manually.
    //Limit Switch: If the reverse movement is interrupted by the limit switch, the motor will move forward //100 steps to disengage it.

//Libraries:

//Make sure to install the following libraries:

    //WiFi (for WiFi functionality)
    //NTPClient (for NTP time synchronization)
    //ESPAsyncWebServer (for creating the web interface)

//This setup combines time scheduling with manual motor control, making it ideal for the Raspberry Pi Pico //W/2W with web interface functionality.

//Testing:

    //Connect the Pico W to WiFi using your credentials (ssid and password).
    //Open a browser and navigate to the IP address shown in the Serial Monitor to set the feeding time //(e.g., http://192.168.x.x).
    //Adjust the feeding time through the webpage, and the Pico will follow the updated schedule.
    //The motor will move at the scheduled time every day (or based on the interval you define).

//Requirements:

    //Arduino IDE (Make sure to select Raspberry Pi Pico W/2W as the board).
	When connecting 5V to the Raspberry Pi Pico W or Pico 2W models, here's what you need to know:

//Power Input:

    //The 5V input is typically provided through the VBUS pin (pin 40) on the Raspberry Pi Pico.
    //VBUS is directly connected to the 5V line from the USB input if you're powering the Pico via USB, or //you can directly supply 5V to this pin.

//3.3V Output:

    //The 3.3V pin (pin 36) provides 3.3V regulated voltage, which is used to power peripherals that require //3.3V.
    //This 3.3V voltage is generated from the 5V input via an onboard voltage regulator. When the board is //powered with 5V, the onboard regulator steps down the voltage to 3.3V to power the microcontroller and //other peripherals that require 3.3V.

//Summary:

    //5V input: VBUS pin (pin 40) or the 5V input from the USB.
    //3.3V output: 3.3V pin (pin 36) which can be used to power peripherals that require 3.3V.

//Be cautious when connecting peripherals to the 3.3V pin, as the Pico W/2W can only provide a limited //amount of current. If you need to power peripherals that draw significant current, consider using an //external power supply to avoid overloading the onboard regulator.
