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

	1) Source: a folder containing the source code written for the NodeMCU-32S microcontrollers and the Node-RED dashboard project file
		1.1) b6310500066_board1/src: a folder containing the source code for the controller board
			1.1.1) main.cpp
				The main code in which containing operations relate to detect a tain wether a train will come in or go. There is a led that will illuminate when a train will come in
		1.2) b6310500295_board2/src: a folder containing the source code for the controller board
			1.2.1) main.cpp
				The main code in which containing operations relate to detect a people which are in the fast entrance. OLED is shown to the player
		1.3) b6310500350_board3/src: a folder containing the source code for the controller board
			1.3.1) main.cpp
				The main code in which containing operations relate to detect a people which are in the normal entrance. OLED is shown to the player
		1.4) b6310500325_board4/src: a folder containing the source code for the controller board
			1.4.1) main.cpp
				The main code in which containing operations relate to detect a people. If people carry object which contains a metal meterial, people will not pass entrance

		1.5) Node-RED: a folder containing the dashboard project file
			1.2.1) Dashboard.json
				A JSON file containing two Node-RED flows (Flow 1 and Flow 2.) Only Flow 2 is used to create the dashboard.

	2) LICENSE.txt: a GNU General Public License file
	
	3) README.md: a readme file describing the project and the files included in this directory
	
	4) Schematic.pdf: a schematic file detailing the construction of the two boards (sensors and controller boards.)

	4) Picture: a folder containing picture of ths project

	
