// Author: M. S. S. Weerasinghe
// Title: Smart Medicine Box

// Include the required libraries
#include <Wire.h>  // For I2C communication
#include <Adafruit_GFX.h> // For the OLED display
#include <Adafruit_SSD1306.h> // For the OLED display
#include <DHTesp.h> // For the DHT sensor
#include <WiFi.h> // For the WiFi communication
#include <PubSubClient.h> // For the MQTT communication
#include <ESP32Servo.h> // For the servo motor

// Define constants for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // shares reset pin of ESP32
#define SCREEN_ADDRESS 0x3C // I2C address

// Define constants for the hardware pins
#define DHT_PIN 12
#define LED 18
#define BUZZER 19
#define UP_BUTTON 26
#define DOWN_BUTTON 25
#define OK_BUTTON 27
#define CANCEL_BUTTON 33
#define SERVO_PIN 32
#define LDR_RIGHT 34
#define LDR_LEFT 35

// Define MQTT topics
#define TEMPERATURE_TOPIC "medibox-210690B-temperature"
#define HUMIDITY_TOPIC "medibox-210690B-humidity"
#define ALARM_ON_TOPIC "medibox-210690B-alarm-on"
#define ALARM_ON_GET_TOPIC "medibox-210690B-alarm-on-get"
#define ALARM_1_TIME_TOPIC "medibox-210690B-alarm-1-time"
#define ALARM_2_TIME_TOPIC "medibox-210690B-alarm-2-time"
#define ALARM_3_TIME_TOPIC "medibox-210690B-alarm-3-time"
#define ALARM_1_TIME_GET_TOPIC "medibox-210690B-alarm-1-time-get"
#define ALARM_2_TIME_GET_TOPIC "medibox-210690B-alarm-2-time-get"
#define ALARM_3_TIME_GET_TOPIC "medibox-210690B-alarm-3-time-get"
#define LIGHT_LEFT_TOPIC "medibox-210690B-light-left"
#define LIGHT_RIGHT_TOPIC "medibox-210690B-light-right"
#define MOTOR_ANGLE_TOPIC "medibox-210690B-motor-angle"

// Create an instances of the required libraries
DHTesp dht; // DHT sensor
Servo servo;  // Servo motor
WiFiClient espClient;  // WiFi client
PubSubClient mqttClient(espClient); // MQTT client
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED display

// Define global variables
int utcOffset = 0; // UTC offset
int dstOffset = 0; // Daylight saving time offset
int milliTimenow = 0; // Milliseconds since the program started
int initmilliseconds = 0; // Time, the program started in milliseconds
int timenow[3] = {0, 0, 0}; // Current time in hours, minutes, and seconds
const int buzzerToneCount = 6;  // Number of buzzer tones
int buzzerTones[buzzerToneCount] = {262, 294, 330, 349, 392, 440}; // Buzzer tones
float humidity, newHumidity, temperature, newTemperature; // For the DHT sensor
int leftLight, rightLight, newLeftLight, newRightLight; // For the LDR sensors
// Strings to store the values for the MQTT communication
char temperatureStr[6], humidityStr[6], timeInMillisStr[10], lightLeftStr[6], lightRightStr[6];
// Variables to configure the alarms
bool alarmsEnabled = true;
const int alarmCount = 3;  // Number of alarms that can be set
int alarmTimes[alarmCount][2] = {{0, 1}, {1, 0}, {2, 0}}; // Alarm times in hours and minutes
int currentAlarmOption = 0; // Current selected alarm option
bool alarmRingingFinished[alarmCount] = {false, false, false}; // Alarm ringing status
// Variables to configure the menu
const int menuOptionCount = 3;  // Number of menu options
String menuOptions[menuOptionCount] = {"Set Time Zone", "Set Alarm", "Enable/Disable Alarms"}; // Menu options
int currentMenuOption = 0;  // Current selected menu option


void setup() {
  // Start the serial communication
  Serial.begin(115200);
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    // If the display cannot be initialized, print an error message
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Loop indefinitely
  }

  // Initialize the hardware pins
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  pinMode(OK_BUTTON, INPUT);
  pinMode(CANCEL_BUTTON, INPUT);
  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);
  pinMode(SERVO_PIN, OUTPUT);

  servo.attach(SERVO_PIN);  // Initialize the servo motor
  dht.setup(DHT_PIN, DHTesp::DHT22);  // Initialize the DHT sensor

  display.display();
  delay(2000);

  // Connect to the WiFi network
  WiFi.begin("Wokwi-GUEST", "", 6); // Change the SSID and password to your WiFi network
  // Loop and display waiting message until the WiFi network is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    display.println("Connecting to WiFi...");
    display.display();
  }
  // Successfully connected to the WiFi network
  display.clearDisplay();
  display.println("Connected to the WiFi network");

  setupMqtt();  // Setup the MQTT communication
  updateTime();  // Update initial time (program start time) from the NTP server

  display.display();
  delay(2000);

  display.clearDisplay();

  // Display the welcome message
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Welcome to Medibox");

  display.display();
  delay(2000);

}


// Main loop
void loop() {
  if (!mqttClient.connected()) connectToBroker();

  updateTimeAndTemp();
  updateLightIntensity();

  // Check if the alarm time has been reached and ring the alarm
  if (alarmsEnabled) {
    checkAlarmReached();
  }
  // Check if the user wants to go to the menu
  if (digitalRead(OK_BUTTON) == LOW) {
    goToMenu();
  }

  mqttClient.loop();
  delay(500);
}


// Function to setup the MQTT communication
void setupMqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883); // Set the MQTT broker (host, port)
  mqttClient.setCallback(receiveCallback);  // Set the callback function for receiving messages
}


// Function to connect to the MQTT broker
void connectToBroker() {
  // Loop until connected to the MQTT broker
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection....");

    if (mqttClient.connect("ESP-6783-876345267")) { // "ESP-6783-876345267" is the client ID
      // Successfully connected to the MQTT broker
      Serial.println("connected to MQTT broker");
      // Subscribe to the MQTT topics
      mqttClient.subscribe(ALARM_ON_GET_TOPIC);
      mqttClient.subscribe(ALARM_1_TIME_GET_TOPIC);
      mqttClient.subscribe(ALARM_2_TIME_GET_TOPIC);
      mqttClient.subscribe(ALARM_3_TIME_GET_TOPIC);
      mqttClient.subscribe(MOTOR_ANGLE_TOPIC);

      // Publish initial configuration to the MQTT broker
      // Alarms enabled/disabled
      if (alarmsEnabled)
        mqttClient.publish(ALARM_ON_TOPIC, "true");
      else
        mqttClient.publish(ALARM_ON_TOPIC, "false");

      // Alarm times - convert hours and minutes to a milliseconds string before publishing
      sprintf(timeInMillisStr, "%d", alarmTimeToMillis(alarmTimes[0][0], alarmTimes[0][1]));
      mqttClient.publish(ALARM_1_TIME_TOPIC, timeInMillisStr);
      sprintf(timeInMillisStr, "%d", alarmTimeToMillis(alarmTimes[1][0], alarmTimes[1][1]));
      mqttClient.publish(ALARM_2_TIME_TOPIC, timeInMillisStr);
      sprintf(timeInMillisStr, "%d", alarmTimeToMillis(alarmTimes[2][0], alarmTimes[2][1]));
      mqttClient.publish(ALARM_3_TIME_TOPIC, timeInMillisStr);
    }
    else {
      // Failed to connect to the MQTT broker
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


// Function to convert hours and minutes to milliseconds
int alarmTimeToMillis(int hour, int minute) {
  return hour * 60 * 60 * 1000 + minute * 60 * 1000;
}


// Function to convert milliseconds to hours and minutes
void millisToAlarmTime(int millis, int alarmIndex) {
  // updates global alarm times array
  alarmTimes[alarmIndex][0] = (millis / 1000 / 60 / 60) % 24; // hours
  alarmTimes[alarmIndex][1] = (millis / 1000 / 60) % 60;  // minutes
}


// Function to receive and handle messages from the MQTT broker
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  // Print the received message on the serial monitor
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
    Serial.print((char)payload[i]);
  Serial.println();

  // Check if the received message is to enable/disable the alarms
  if (strcmp(topic, ALARM_ON_GET_TOPIC) == 0) {
    // Update the alarm status based on the received message
    if (strcmp((char*)payload, "true") == 0)
      alarmsEnabled = true;
    else
      alarmsEnabled = false;
  }
  // Check if the received message is to update the alarm times
  else if (strcmp(topic, ALARM_1_TIME_GET_TOPIC) == 0)  // If the message is to update alarm 1 time
    millisToAlarmTime(atoi((char*)payload), 0);  // Update the alarm 1 time
  else if (strcmp(topic, ALARM_2_TIME_GET_TOPIC) == 0)  // If the message is to update alarm 2 time
    millisToAlarmTime(atoi((char*)payload), 1);  // Update the alarm 2 time
  else if (strcmp(topic, ALARM_3_TIME_GET_TOPIC) == 0)  // If the message is to update alarm 3 time
    millisToAlarmTime(atoi((char*)payload), 2);  // Update the alarm 3 time
  // Check if the received message is to update the motor angle
  else if (strcmp(topic, MOTOR_ANGLE_TOPIC) == 0)
    servo.write(atoi((char*)payload));  // Update the motor angle
}


// Function to print a given text on the OLED display
void printTextLine(String text, int row = 0, int column = 0, int textSize = 1) {
  display.setTextSize(textSize);
  display.setCursor(column, row); // Set the cursor position
  display.println(text);
  display.display();
}


// Function to display the menu
void showMenu() {
  display.clearDisplay();
  printTextLine("Menu", 0, 0, 2);
  for (int i = 0; i < menuOptionCount; i++) {
    if (i == currentMenuOption)
      printTextLine("-> " + menuOptions[i], 20 + i * 10, 0); // Highlight the current menu option
    else
      printTextLine(menuOptions[i], 20 + i * 10, 0);
  }
}


// Function to change the time using the NTP server
void updateTime() {
  // Connect to the WiFi network
  configTime(utcOffset * 3600, dstOffset * 3600, "pool.ntp.org"); // (UTC offset, daylight saving time offset, NTP host)
  struct tm timeinfo; // struct to store the time information
  // Get the current time from the NTP server
  if (!getLocalTime(&timeinfo)) {
    // If the time cannot be obtained, print an error message
    Serial.println("Failed to obtain time from NTP server");
    return;
  }
  // Get the current time in hours, minutes, and seconds from the timeinfo struct
  char timeSecond[3], timeMinute[3], timeHour[3];
  strftime(timeSecond, 3, "%S", &timeinfo);
  strftime(timeMinute, 3, "%M", &timeinfo);
  strftime(timeHour, 3, "%H", &timeinfo);
  // Calculate started time in milliseconds, corresponding to the current timezone
  initmilliseconds = atoi(timeSecond) * 1000 + atoi(timeMinute) * 60 * 1000 + atoi(timeHour) * 60 * 60 * 1000 - millis();
  display.clearDisplay();
}


// Function to display the current time zone
void displayTimeZone(int utcOffset) {
  display.clearDisplay();
  if (utcOffset >= 0)
    printTextLine("Current Time Zone: \nUTC+" + String(utcOffset), 0, 0);  // Add + sign for positive UTC offset
  else
    printTextLine("Current Time Zone: \nUTC" + String(utcOffset), 0, 0);
}


// Function to set the time zone
void setTimeZone() {
  // Display the current time zone at the start
  int tempUtcOffset = utcOffset;
  displayTimeZone(tempUtcOffset);
  delay(200);
  // Loop until the user sets the time zone
  while (true) {
    if (digitalRead(UP_BUTTON) == LOW) {
      // Increase the UTC offset
      tempUtcOffset++;
      if (tempUtcOffset > 14) // Maximum UTC offset is 14; set offset to -12
        tempUtcOffset = -12;
      displayTimeZone(tempUtcOffset);
    }
    else if (digitalRead(DOWN_BUTTON) == LOW) {
      // Decrease the UTC offset
      tempUtcOffset--;
      if (tempUtcOffset < -12)  // Minimum UTC offset is -12; set offset to 14
        tempUtcOffset = 14;
      displayTimeZone(tempUtcOffset);
    }
    else if (digitalRead(OK_BUTTON) == LOW) {
      // Set the selected UTC offset
      utcOffset = tempUtcOffset;
      display.clearDisplay();
      if (utcOffset >= 0)  // Add + sign for positive UTC offset
        printTextLine("Time Zone set to \nUTC+" + String(utcOffset), 0, 0);
      else
        printTextLine("Time Zone set to \nUTC" + String(utcOffset), 0, 0);
      delay(2000);
      updateTime();  // Update the time based on the new time zone
      break;
    }
    else if (digitalRead(CANCEL_BUTTON) == LOW) {
      break;  // Cancel the time zone setting
    }
    delay(200);
  }
  display.clearDisplay();
  showMenu();  // Display the main menu after setting/not setting the time zone
  delay(200);
}


// Function to display the alarm menu
void displayAlarmMenu() {
  display.clearDisplay();
  printTextLine("Select alarm to set", 0, 0);
  // Display the alarm options
  for (int i = 0; i < alarmCount; i++) {
    if (i == currentAlarmOption)
      printTextLine("-> Alarm " + String(i + 1) + ": " + String(alarmTimes[i][0]) + ":" + String(alarmTimes[i][1]), 10 + i * 10, 0);  // Highlight the current alarm option
    else
      printTextLine("Alarm " + String(i + 1) + ": " + String(alarmTimes[i][0]) + ":" + String(alarmTimes[i][1]), 10 + i * 10, 0);
  }
}


// Function to display selected alarm time in hours or minutes
void displayAlarmUnit(int timeIndex, int value) {
  display.clearDisplay();
  if (timeIndex == 0) {
    printTextLine("Set hour ", 0, 0);
    printTextLine("-> " + String(value), 10, 0);  // Display the selected hour
  } else if (timeIndex == 1) {
    printTextLine("Set minute ", 0, 0);
    printTextLine("-> " + String(value), 10, 0);  // Display the selected minute
  }
}


// Function to set the alarm time in hours or minutes
bool setAlarmTimeUnit(int alarmIndex, int timeIndex) {
  int value = alarmTimes[alarmIndex][timeIndex]; // Get the current alarm time (hour or minute) of the selected alarm
  display.clearDisplay();
  // Set the alarm time in hours
  if (timeIndex == 0) {
    displayAlarmUnit(timeIndex, value);  // Display the selected hour
    while (true) {
      if (digitalRead(UP_BUTTON) == LOW) {  // Increase the hour
        value = (value + 1) % 24;
        displayAlarmUnit(timeIndex, value);
      } else if (digitalRead(DOWN_BUTTON) == LOW) { // Decrease the hour
        value = (value - 1) % 24;
        if (value == -1) { value = 23; }
        displayAlarmUnit(timeIndex, value);
      } else if (digitalRead(OK_BUTTON) == LOW) { // Set the selected hour
        alarmTimes[alarmIndex][timeIndex] = value;
        return true;
      } else if (digitalRead(CANCEL_BUTTON) == LOW) { // Cancel the alarm time setting
        return false;
      }
      delay(200);
    }
  }
  // Set the alarm time in minutes
  else if (timeIndex == 1) {
    displayAlarmUnit(timeIndex, value);  // Display the selected minute
    while (true) {
      if (digitalRead(UP_BUTTON) == LOW) {  // Increase the minute
        value = (value + 1) % 60;
        displayAlarmUnit(timeIndex, value);
      } else if (digitalRead(DOWN_BUTTON) == LOW) { // Decrease the minute
        value = (value - 1) % 60;
        if (value == -1) { value = 59; }
        displayAlarmUnit(timeIndex, value);
      } else if (digitalRead(OK_BUTTON) == LOW) { // Set the selected minute
        alarmTimes[alarmIndex][timeIndex] = value;
        return true;
      } else if (digitalRead(CANCEL_BUTTON) == LOW) { // Cancel the alarm time setting
        return false;
      }
      delay(200);
    }
  }
  return false;
}


// Function to set the alarm time
void setAlarmTime(int alarmIndex) {
  display.clearDisplay();
  printTextLine("Set alarm " + String(alarmIndex + 1) + " time", 0, 0);  // Display the selected alarm
  int prev_hour = alarmTimes[alarmIndex][0];
  // Update alarm time only if setting both hours and minutes is successful
  // Set the alarm time in hours
  if (setAlarmTimeUnit(alarmIndex, 0)) {
    // Set the alarm time in minutes
    if (setAlarmTimeUnit(alarmIndex, 1)) {
      // Alarm time set successfully
      display.clearDisplay();
      printTextLine("Alarm " + String(alarmIndex + 1) + " time set to " + String(alarmTimes[alarmIndex][0]) + ":" + String(alarmTimes[alarmIndex][1]), 0, 0);
      // publish the new alarm time to the MQTT broker
      if (!mqttClient.connected())  // Connect to the MQTT broker if not connected
        connectToBroker();
      // Convert the alarm time to milliseconds string before publishing
      sprintf(timeInMillisStr, "%d", alarmTimeToMillis(alarmTimes[alarmIndex][0], alarmTimes[alarmIndex][1]));
      // Check the alarm index and publish the alarm time to the relevant topic
      if (alarmIndex == 0)
        mqttClient.publish(ALARM_1_TIME_TOPIC, timeInMillisStr);
      else if (alarmIndex == 1)
        mqttClient.publish(ALARM_2_TIME_TOPIC, timeInMillisStr);
      else if (alarmIndex == 2)
        mqttClient.publish(ALARM_3_TIME_TOPIC, timeInMillisStr);
      delay(2000);
    }
    // If setting the alarm time in minutes is not successful, set the alarm time in hours to the previous value
    else {
      alarmTimes[alarmIndex][0] = prev_hour;
    }
  }
  delay(1000);
}


// Function to select alarm option for updating the alarm time
void setAlarm() {
  while (true) {
    if (digitalRead(UP_BUTTON) == LOW) {
      // Decrement the alarm option
      currentAlarmOption = (currentAlarmOption - 1) % alarmCount;
      if (currentAlarmOption == -1) { currentAlarmOption = alarmCount - 1; }
      displayAlarmMenu(); // Display the alarm menu
    } else if (digitalRead(DOWN_BUTTON) == LOW) {
      // Increment the alarm option
      currentAlarmOption = (currentAlarmOption + 1) % alarmCount;
      displayAlarmMenu(); // Display the alarm menu
    } else if (digitalRead(OK_BUTTON) == LOW) {
      // Set the alarm time for the selected alarm option
      setAlarmTime(currentAlarmOption);
      displayAlarmMenu(); // Display the alarm menu after setting the alarm time
    } else if (digitalRead(CANCEL_BUTTON) == LOW) {
      // Return to the main screen
      display.clearDisplay();
      break;
    }
    delay(200);
  } 
}


// Function to navigate to main menu
void goToMenu() {
  showMenu();  // Display the main menu
  while (true) {
    // Check if the user wants to navigate the menu
    if (digitalRead(UP_BUTTON) == LOW) {
      currentMenuOption = (currentMenuOption - 1) % menuOptionCount;  // Decrement the menu option
      if (currentMenuOption == -1) { currentMenuOption = menuOptionCount - 1; }
      showMenu();  // Display updated menu
    }
    else if (digitalRead(DOWN_BUTTON) == LOW) {
      currentMenuOption = (currentMenuOption + 1) % menuOptionCount;  // Increment the menu option
      showMenu();  // Display updated menu
    }
    // Navigate to relevant menu option based on user input
    else if (digitalRead(OK_BUTTON) == LOW) {
      if (currentMenuOption == 0) {
        setTimeZone();  // Navigate to the set time zone menu
      } else if (currentMenuOption == 1) {
        setAlarm();  // Navigate to the set alarms menu
      } else if (currentMenuOption == 2) {
        // Enable/disable the alarms
        display.clearDisplay();
        if (!mqttClient.connected())  // Connect to the MQTT broker if not connected
          connectToBroker();
        // If the alarms are disabled, enable them
        if (!alarmsEnabled) {
          // Publish new alarm status to the MQTT broker
          mqttClient.publish(ALARM_ON_TOPIC, "true");
          printTextLine("Alarms enabled", 0, 0);
          alarmsEnabled = true;  // Enable the alarms
        }
        // If the alarms are enabled, disable them
        else {
          // Publish new alarm status to the MQTT broker
          mqttClient.publish(ALARM_ON_TOPIC, "false");
          printTextLine("Alarms disabled", 0, 0);
          alarmsEnabled = false; // Disable the alarms
        }
        delay(2000);
        showMenu();  // Display the main menu after enabling/disabling the alarms
      }
    }
    else if (digitalRead(CANCEL_BUTTON) == LOW) {
      // Return to the main screen
      display.clearDisplay();
      return;
    }
  }
}


// Function to check if the temperature has changed
bool temperatureChanged() {
  if (newTemperature != temperature) {
    temperature = newTemperature; // Update the temperature
    return true;
  }
  return false;
}

// Function to check if the humidity has changed
bool humidityChanged() {
  if (newHumidity != humidity) {
    humidity = newHumidity; // Update the humidity
    return true;
  }
  return false;
}

// Function to check if the intensityof left LDR sensor has changed
bool leftLightChanged() {
  if (newLeftLight != leftLight) {
    leftLight = newLeftLight; // Update the left light intensity
    return true;
  }
  return false;
}

// Function to check if the intensity of right LDR sensor has changed
bool rightLightChanged() {
  if (newRightLight != rightLight) {
    rightLight = newRightLight; // Update the right light intensity
    return true;
  }
  return false;
}

// Function to publish updated light intensity
void updateLightIntensity() {
  // Read the light intensity from the LDR sensors
  newLeftLight = analogRead(LDR_LEFT);
  newRightLight = analogRead(LDR_RIGHT);
  // Check if the light intensity has changed
  if (leftLightChanged()) {
    // Publish the light intensity to the MQTT broker
    String(leftLight).toCharArray(lightLeftStr, 6);
    mqttClient.publish(LIGHT_LEFT_TOPIC, lightLeftStr);
  }
  if (rightLightChanged()) {
    // Publish the light intensity to the MQTT broker
    String(rightLight).toCharArray(lightRightStr, 6);
    mqttClient.publish(LIGHT_RIGHT_TOPIC, lightRightStr);
  }
}

// Function to update temperature, humidity, and time on the OLED display
void updateTimeAndTemp() {
  // Read the temperature and humidity from the DHT sensor
  newHumidity = dht.getHumidity();
  newTemperature = dht.getTemperature();

  // Check if the humidity has changed
  if (humidityChanged()) {
    String(humidity).toCharArray(humidityStr, 6);
    // Publish the humidity to the MQTT broker
    mqttClient.publish(HUMIDITY_TOPIC, humidityStr);
  }
  // Check if the temperature has changed
  if (temperatureChanged()) {
    String(temperature).toCharArray(temperatureStr, 6);
    // Publish the temperature to the MQTT broker
    mqttClient.publish(TEMPERATURE_TOPIC, temperatureStr);
  }

  // Calculate the current time (= program start time + milliseconds since the program started)
  milliTimenow = initmilliseconds + millis();

  // Calculate the current time in hours, minutes, and seconds
  timenow[0] = (milliTimenow / 1000 / 60 / 60) % 24;    // Hours
  timenow[1] = (milliTimenow / 1000 / 60) % 60;   // Minutes
  timenow[2] = (milliTimenow / 1000) % 60;    // Seconds

  display.clearDisplay();
  // Display updated time, temperature, and humidity on the OLED display
  printTextLine("Time: " + String(timenow[0]) + ":" + String(timenow[1]) + ":" + String(timenow[2]), 0, 0);
  printTextLine("Temperature: " + String(temperature) + "C", 10, 0);
  printTextLine("Humidity: " + String(humidity) + "%", 20, 0);
  // Check if the temperature and humidity are within the acceptable range
  // If not, display a warning message and turn on the LED
  if ((temperature > 32 || temperature < 26) && (humidity > 80 || humidity < 60)) {
    printTextLine("Temperature and humidity out of range!", 30, 0);
    display.display();
    digitalWrite(LED, HIGH);
  } else if (temperature > 32 || temperature < 26) {
    printTextLine("Temperature out of range!", 30, 0);
    display.display();
    digitalWrite(LED, HIGH);
  } else if (humidity > 80 || humidity < 60) {
    printTextLine("Humidity out of range!", 30, 0);
    display.display();
    digitalWrite(LED, HIGH);
  } else {  // Temperature and humidity are within the acceptable range
    digitalWrite(LED, LOW);
    display.display();
    delay(500);
  }
}

// Function to ring the alarm
void ringAlarm(int alarmIndex) {
  digitalWrite(LED, HIGH);
  // Keep ringing the alarm until the user stops it
  while (!alarmRingingFinished[alarmIndex]) {
    for (int j = 0; j < buzzerToneCount; j++) {
      // Check if the user has stopped the alarm
      if (digitalRead(CANCEL_BUTTON) == LOW) {
        alarmRingingFinished[alarmIndex] = true; // Update ringing status to exit the loop
        digitalWrite(LED, LOW);
        break;
      }
      // Ring the buzzer with different tones
      tone(BUZZER, buzzerTones[j]); delay(400);
      noTone(BUZZER); delay(2);
    }
  }
}

// Function to check if the alarm time has been reached
void checkAlarmReached() {
  // For each alarm, check if the alarm time has reached (hours and minutes match the current time)
  for (int i = 0; i < alarmCount; i++) {
    if (alarmTimes[i][0] == timenow[0] && alarmTimes[i][1] == timenow[1]) {
      if (!alarmRingingFinished[i]) {
        display.clearDisplay();
        printTextLine("Medicine time!", 0, 0, 2);
        ringAlarm(i);
      }
    } else {
      // After Exit from the loop in ringAlarm function, reset the alarm ringing status
      alarmRingingFinished[i] = false;
    }
  }
}

