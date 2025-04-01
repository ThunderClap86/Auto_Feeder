import machine
import time
import network
import ntptime

# Motor Control - Pins for 28BYJ-48 Stepper Motor and ULN2003 Driver
MOTOR_PIN_1 = 33
MOTOR_PIN_2 = 25
MOTOR_PIN_3 = 27
MOTOR_PIN_4 = 14

# Button Control Pins
BUTTON_FORWARD_PIN = 22
BUTTON_REVERSE_PIN = 23

# Limit Switch Pin for stopping the motor
LIMIT_SWITCH_PIN = 26

# Setup pins
motor_pins = [machine.Pin(MOTOR_PIN_1, machine.Pin.OUT),
              machine.Pin(MOTOR_PIN_2, machine.Pin.OUT),
              machine.Pin(MOTOR_PIN_3, machine.Pin.OUT),
              machine.Pin(MOTOR_PIN_4, machine.Pin.OUT)]

button_forward = machine.Pin(BUTTON_FORWARD_PIN, machine.Pin.IN, machine.Pin.PULL_UP)
button_reverse = machine.Pin(BUTTON_REVERSE_PIN, machine.Pin.IN, machine.Pin.PULL_UP)
limit_switch = machine.Pin(LIMIT_SWITCH_PIN, machine.Pin.IN, machine.Pin.PULL_UP)

# Define stepper motor sequence (4 steps per full cycle)
STEP_COUNT = 4
STEPS = [
    [1, 0, 0, 0],
    [0, 1, 0, 0],
    [0, 0, 1, 0],
    [0, 0, 0, 1]
]

# Control speed of stepping (time in ms between steps)
STEP_PERIOD = 20  # Time in ms

# Time zone offsets for CST and CDT
CST_OFFSET = -6 * 3600  # UTC -6 for Standard Time (CST)
CDT_OFFSET = -5 * 3600  # UTC -5 for Daylight Time (CDT)

# SET YOUR DESIRED FEEDING TIME HERE
FEED_HOUR = 22  # Hour in 24-hour format
FEED_MINUTE = 47  # Minute

# Feeding limit - only allow a max of 10 movements per day
feeding_count = 0
MAX_FEEDS_PER_DAY = 10

# Track the last feeding date (stored as a tuple of (year, month, day))
last_feeding_date = None

# Function to move motor for a given number of steps
def step_motor(steps, reverse=False):
    step_idx = 0
    steps_left = abs(steps)
    direction = -1 if reverse else 1

    while steps_left > 0:
        for pin in range(4):
            motor_pins[pin].value(STEPS[step_idx][pin])

        step_idx = (step_idx + direction) % STEP_COUNT
        steps_left -= 1
        time.sleep_ms(STEP_PERIOD)

        # Check the limit switch during reverse movement
        if reverse and limit_switch.value() == 1:  # Limit switch pressed
            print("Limit switch pressed during reverse: stopping motor")
            break  # Stop motor immediately if the limit switch is engaged

    # Stop the motor after moving
    for pin in motor_pins:
        pin.value(0)

# Function to check button presses
def check_buttons():
    if button_forward.value() == 0:
        print("Moving motor forward")
        step_motor(350, reverse=False)  # Move motor forward (350 steps)
    
    if button_reverse.value() == 0:
        print("Moving motor reverse")
        step_motor(3200, reverse=True)  # Move motor reverse (3200 steps)

        # If limit switch was pressed, move forward 100 steps to disengage it
        if limit_switch.value() == 1:
            print("Limit switch engaged during reverse, moving forward 100 steps to disengage")
            step_motor(100, reverse=False)

# Function to connect to Wi-Fi
def connect_wifi():
    ssid = 'Fire Dept Wifi'
    password = 'hazmat14'
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(ssid, password)

    while not wlan.isconnected():
        time.sleep(1)

    print("Connected to Wi-Fi:", wlan.ifconfig())

# Function to sync time with NTP
def sync_time():
    try:
        ntptime.settime()
        print("Time synchronized with NTP")
    except Exception as e:
        print("Failed to synchronize time:", e)

# Function to check if it's feeding time (any time within the specified minute)
def check_feeding_time():
    global feeding_count, last_feeding_date

    # Get current UTC time from NTP
    current_time = time.localtime()

    # Convert UTC to local time (CST/CDT)
    local_time = convert_utc_to_local(current_time)

    # Extract hour, minute, and second from the local time
    current_hour = local_time[3]
    current_minute = local_time[4]
    current_second = local_time[5]
    current_year = local_time[0]
    current_month = local_time[1]
    current_day = local_time[2]

    # Print the local time for debugging
    print(f"Local time: {current_hour}:{current_minute}:{current_second}")

    # Check if the current date is different from the last feeding date
    if last_feeding_date != (current_year, current_month, current_day):
        if current_hour == FEED_HOUR and current_minute == FEED_MINUTE:
            print(f"Feeding time reached at {current_hour}:{current_minute}:{current_second}, moving motor forward")
            step_motor(350, reverse=False)
            feeding_count += 1
            last_feeding_date = (current_year, current_month, current_day)
            print(f"Feeding count: {feeding_count}/{MAX_FEEDS_PER_DAY}")
            return True

    else:
        print(f"Feeding already performed today: {current_hour}:{current_minute}:{current_second}")

    return False

# Function to convert UTC to CST/CDT
def convert_utc_to_local(current_time):
    # Check if it's daylight saving time (March-November)
    is_dst = 3 <= current_time[1] <= 11  # Month in range 3 to 11
    timezone_offset = CDT_OFFSET if is_dst else CST_OFFSET
    
    # Convert UTC to local time
    local_time = time.mktime(current_time) + timezone_offset
    return time.localtime(local_time)

# Debug function to print current time (UTC, CST/CDT)
def print_current_time():
    # Get current time from NTP (in UTC)
    current_time = time.localtime()

    # Convert UTC to local CST/CDT
    local_time = convert_utc_to_local(current_time)

    print(f"Current UTC time: {current_time[3]}:{current_time[4]} UTC")
    print(f"Converted CST/CDT time: {local_time[3]}:{local_time[4]}")

# Main function
def main():
    global feeding_count
    print("Starting ESP32 Feeder")
    connect_wifi()
    sync_time()

    while True:
        # First check buttons to allow for immediate motor action
        check_buttons()

        # Print the current time for debugging
        print_current_time()

        # Check for scheduled feeding
        if not check_feeding_time():
            # Wait for a short period to avoid CPU overload
            time.sleep(10)  # Check once every 10 seconds

# Run main function when the script is executed
main()

