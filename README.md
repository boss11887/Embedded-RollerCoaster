#  Roller Coaster - a simulated roller coaster queue system project #
	Made by (Name - Student ID):
		1) Sukachathum Seawsiritaworn - 6310500066
		2) Tiwat Suprattanakul - 6310500295
		3) Pongpob Krairaveeroj	- 6310500325
		4) Witnapat Champaruang - 6310500350

	Computer Engineering (CPE) Faculty, Kasetsart University, Bangkok
	This project was made for 01204322 Embedded System.
	
------------------------------------------------------------------------

Libraries used:

	1) Adafruit_SSD1306: library for the SSD1306 128x64 OLED display
		by Limor Fried
		(https://github.com/adafruit/Adafruit_SSD1306)
	
	2) PubSubClient: allows the ESP32-S3 to communicate with the MQTT broker  
		by Nick O’Leary
		(https://pubsubclient.knolleary.net/)
	
	3) ESP32Servo: allows ESP32 boards to control servo
		by Kevin Harrington
		(https://www.arduinolibraries.info/libraries/esp32-servo)

------------------------------------------------------------------------

Components used:

	1) NodeMCU-32S microcontrollers
	2) SSD1306 128x64 I2C OLED display
	3) green LEDs
	4) 4.7kΩ ,10kΩ ,and 330Ω resistors
	5) MG90S Servo Motor
	6) IR Infrared photoelectric Sensor
	7) Metal Touch Sensor
	
------------------------------------------------------------------------	

Included in this directory:

	1) Source: a folder containing the source code written for the ESP32-S3 microcontrollers and the Node-RED dashboard project file
	
		1.1) Controller: a folder containing the source code for the controller board
			
			1.1.1) Controller.ino
				The main code in which contains essential operations (Wifi connection, MQTT broker connection, MQTT publishing and subscription, etc.) This code is responsible for creating and regulating the "pet."
			
			1.1.2) pet.cpp & pet.h
				The C++ source code and the header respectively. These two files allow the main code (1.1.1) to instantiate a new class called Pet, which contains attributes and methods related to the "pet."
				
			1.1.3) settings.cpp & settings.h
				These two files declare essential variables such as the Wifi SSID and password, frame update frequency and the certificate authority key which allows the ESP32-S3 to connect to the MQTT broker hosted on HiveMQ Cloud Broker (https://www.hivemq.com/mqtt-cloud-broker/).
				
			1.1.4) sprites.cpp & sprites.h	
				These two files declare arrays of constant character arrays used for the animation played on the SSD1306 128x64 OLED display. Moreover, an integer variable called frame used to keep track of the animation progress is also declared.
				
			1.1.5) TomThumb.h
				A header file containing the TomThumb font as seen on the SSD1306 128x64 OLED display.
				
		1.2) Node-RED: a folder containing the dashboard project file
		
			1.2.1) Dashboard.json
				A JSON file containing two Node-RED flows (Flow 1 and Flow 2.) Only Flow 2 is used to create the dashboard.
				
		1.3) Sensors: a folder containing the source code for the sensors board
				
			1.3.1) DHT20.cpp & DHT20.h
				These files contain variables and functions related to the DHT20 I2C temperature and humidity sensor. They are written by Rob Tillaart and the projecrt's repository can be accessed here: https://github.com/RobTillaart/DHT20.
		
			1.3.2) Sensors.ino 
				The main code in which contains essential operations (Wifi connection, MQTT broker connection, MQTT publishing.) This code is responsible for publishing data read from the light-dependent sensor (LDR) and the DHT20 I2C temperature and humidity sensor to the MQTT broker.
				
			1.3.3) settings.cpp & settings.h
				See 1.1.3 for information.				
				
	2) LICENSE.txt: a GNU General Public License file
	
	3) README.txt: a readme file describing the project and the files included in this directory
	
	4) Schematic.pdf: a schematic file detailing the construction of the two boards (sensors and controller boards.)
	
