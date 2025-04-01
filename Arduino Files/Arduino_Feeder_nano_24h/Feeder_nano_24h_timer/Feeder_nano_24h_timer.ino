// Motor Control - Pins for 28BYJ-48 Stepper Motor and ULN2003 Driver
#define MOTOR_PIN_1 2
#define MOTOR_PIN_2 3
#define MOTOR_PIN_3 4
#define MOTOR_PIN_4 5

// Button Control Pins
#define BUTTON_FORWARD_PIN 6
#define BUTTON_REVERSE_PIN 7

// Limit Switch Pin for stopping the motor
#define LIMIT_SWITCH_PIN 8

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

// Timer variables for feeding
unsigned long last_feed_time;  // Store the time of last feed

// Regular 24-hour feeding delay (uncomment for real-world use)
// const unsigned long FEED_DELAY = 1000UL * 60 * 60 * 24; // 24 hours

// Test delay of 1 hour for faster testing
const unsigned long FEED_DELAY = 1000UL * 60 * 60; // 1 hour

// Button presses
int feeding_count = 0;
#define MAX_FEEDS_PER_DAY 10

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
  Serial.begin(9600);

  // Initialize last_feed_time properly
  last_feed_time = millis();
}

void loop() {
  // Check if it's time to feed (every 1 hour for testing)
  check_feeding_time();

  // Check button presses to manually move the motor
  check_buttons();

  // Check every 60 seconds instead of every second
  delay(60000);  // 1-minute delay
}

// Function to move motor for a given number of steps
void step_motor(int steps_to_move, bool reverse = false) {
  int direction = reverse ? -1 : 1;
  int step_idx = 0;
  int steps_left = abs(steps_to_move);

  while (steps_left > 0) {
    // Set step sequence based on direction
    for (int pin = 0; pin < 4; pin++) {
      digitalWrite(MOTOR_PIN_1, steps[step_idx][0]);
      digitalWrite(MOTOR_PIN_2, steps[step_idx][1]);
      digitalWrite(MOTOR_PIN_3, steps[step_idx][2]);
      digitalWrite(MOTOR_PIN_4, steps[step_idx][3]);
    }
    step_idx = (step_idx + direction + STEP_COUNT) % STEP_COUNT;  // Handling reverse properly
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
    step_motor(350, false);  // Move motor forward (350 steps)
  }
  
  if (digitalRead(BUTTON_REVERSE_PIN) == LOW) {
    Serial.println("Moving motor reverse");
    step_motor(3200, true);  // Move motor reverse (3200 steps)

    // Switch was pressed, move +100 steps to disengage it ** MAY NEED TO HAVE LOW DEPENDING ON LIMIT SWITCH **
    if (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
      Serial.println("Limit switch engaged during reverse, moving forward 100 steps to disengage");
      step_motor(100, false);
    }
  }
}

// Function to check if it's feeding time (every 1 hour for testing)
void check_feeding_time() {
  unsigned long current_time = millis();
  unsigned long elapsed_time = current_time - last_feed_time;
  unsigned long remaining_time = (FEED_DELAY > elapsed_time) ? (FEED_DELAY - elapsed_time) : 0;

  // Convert remaining time to hours and minutes
  unsigned long remaining_hours = remaining_time / (1000UL * 60 * 60);
  unsigned long remaining_minutes = (remaining_time % (1000UL * 60 * 60)) / (1000UL * 60);

  Serial.print("Next feed in: ");
  Serial.print(remaining_hours);
  Serial.print(" hours, ");
  Serial.print(remaining_minutes);
  Serial.println(" minutes.");

  // Only feed when the time delay has fully elapsed
  if (elapsed_time >= FEED_DELAY) {
    Serial.println("Feeding time reached! Moving motor forward.");
    step_motor(350, false);  // Move motor forward for feeding

    // Reset last_feed_time correctly after feeding
    last_feed_time = millis();
    feeding_count++;

    // Debugging feed count
    Serial.print("Feeding count: ");
    Serial.print(feeding_count);
    Serial.print("/");
    Serial.println(MAX_FEEDS_PER_DAY);
  }
}
