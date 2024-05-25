<h1 align="center">💊 Smart Medicine Box 💊</h1>

<h2 align="center">🏢 System Architecture 🛠️</h2>


![sysArchi](images/System_Architecture.jpg)

<h2 align="center">📚 Project Description 📚</h2>

The Smart Medicine Box is a system that helps the user to keep track of their medicine intake. System consists of multiple components, coorperating with each other to provide a comprehensive set of functionalities. Here's a detailed list of features:

Menu Functionalities:

1) Set Time Zone:
    Accepts the offset from UTC as input to set the time zone.
2) Set Alarms:
    Allows setting of up to 3 alarms.
3) Disable Alarms:
    Provides an option to disable all alarms.

Either medibox menu or Node-RED dashboard can be used for the above functionalities.

Time Synchronization:

4) Fetch and Display Time:
    Fetches the current time from the NTP server based on the selected time zone and displays the current time on the OLED screen.

Alarm Management:

5) Alarm Indication:
    Rings the alarm with proper indication when the set alarm times are reached. Further provides suitable indications such as turned on LED and an message on OLED. If user does not responds to the alarm within the predefined time, he will be notified with a message via Twilio SMS Gateway.
6) Stop Alarm:
    Allows to use a push button to stop the ringing alarm.
7) Disable All Alarms:
    Provides an option to disable all alarms.

Both medibox and Node-RED dashboard provide the above functionalities.

Environmental Monitoring:

8) Monitor Temperature and Humidity:
    Continuously monitors temperature and humidity levels, providing warnings using proper indications when either or both exceed the healthy limits (Temperature: 26°C to 32°C, Humidity: 60% to 80%). Uses gauges to display real-time temperature and humidity values, and a plot to visualize past variations on the Node-RED dashboard. If the temperature or humidity exceeds the healthy limits, the user will be notified with a message via Twilio SMS Gateway.

Light Intensity Monitoring:

9) Measure Light Intensity:
    Uses two LDRs to measure light intensity on either side of the Medibox. Then displays the highest light intensity on the Node-RED dashboard. Indicates from which LDR (left or right) the highest light intensity is obtained. Uses a gauge to display real-time highest intensity and a plot to visualize past variations. Intensity values are normalized to a range of 0 to 1 (0 for analog reading 0 and 1 for analog reading 1023).

Light Control Mechanism:

10) Shaded Sliding Window:
    Uses a servo motor to adjust the angle of the shaded sliding window to regulate light intensity entering the Medibox. Implements dynamic adjustment of the window based on the light intensity using a given controlling factor.

User Interface Enhancements:

11) Adjust System Parameters:
    Allows user to adjust the minimum angle and controlling factor using sliders on the Node-RED dashboard. Provides slider controls for the minimum angle (range 0 to 120) and the controlling factor (range 0 to 1).

12) Medicine-Specific Settings:
    Includes a dropdown menu with options for commonly used medicines and a custom option. Automatically applies predefined values for the minimum angle and controlling factor based on the selected medicine. User can add custom medicines to the list. Allows manual adjustment of values when the custom option is selected.

By implementing these functionalities, the Medibox effectively assists users in managing their medication schedules, while also ensuring the safe storage of medicines through environmental monitoring and dynamic light control.

<h2 align="center"> 🏗️ Implementation 🏗️ </h2>

### Hardware Components:
1) ESP32 Microcontroller with WiFi
2) OLED Display
3) Servo Motor
4) LDRs
5) Push Buttons
6) Buzzer
7) DHT22 Sensor
8) LEDs

### Software Components:
1) Arduino IDE
2) Node-RED
3) Docker
4) Twilio SMS Gateway
5) HiveMQ MQTT Broker
6) Azure Cloud Platform

### Implementation Details:

* Node-RED was used to develop a dashboard for the user to interact with the system. The dashboard provides a user-friendly interface to set alarms, stop alarms, disable alarms, adjust system parameters, and monitor environmental conditions. The development process utilized Docker and GitHub Actions for continuous integration and deployment (CI/CD) pipelines, with deployment to an Azure Web App.

* The system uses MQTTS protocol for communication between the ESP32 and Node-RED. HiveMQ MQTT broker was used to facilitate communication between the ESP32 and Azure Web App (Node-RED App).

* The system also uses Twilio SMS Gateway to send SMS notifications to the user when alarms are not acknowledged and temperature/humidity levels exceed the healthy limits.

* Details (controlling factor, minimum angle) of predefined medicines are persistently stored in an Azure Table Storage, allowing to recover the details even after a system restart.

* Azure Monitor and Azure Application Insights were configured to monitor the web app's functioning and performance.

### Security

* Communication between the ESP32 and the web application via MQTT was secured with TLS/SSL encryption, ensuring a secure transmission channel.

* Basic Authentication was implemented on the Node-RED dashboard to ensure that only authorized users can access the dashboard.

* Credentials to access Azure Table Storage and HiveMQ MQTT broker were encrypted and stored in the Node-RED configuration. Credentials to access the Twilio SMS Gateway were stored as environment variables in the Azure Web App.

<h2 align="center"> 📸 Project Snapshots 📸 </h2>

### Node-RED Dashboard:

Refer `dev` branch for deployment guidelines.
