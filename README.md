# ESPS8266_mqtt_gateway

Depending of the type of ESP8266 you are using, a combination of Onewire devices, I2C devices and RG433mHz devices can be presented to a MQTT gateway.


The program provides web-management of configuration on port 80.


## Requirements

- Arduino IDE
- an ESP8266
- an Onewire temperature censor (DS1820 or similar)
- and or an I2C device
- and or RF433 receiver and transmitter.

ESP8266-01 has 2 vacant GPIO pins (pin 0 and pn 2) This will fit for one I2C bus or one RF433mHz connection or one Onewire bus plus one I/O pin.

Larger ESP8266 using (ESP-12 or similar) have more GPIO and will alow for combinations of the abowe. Adjust port numbers accordingly

## Installation

- copy the ESP8266_mqt_gateway folder to your Arduino folder
- enable/install required libraries
- compile and upload to ESP8266 using the IDE monitor.
- Follow the monitor printout for configuring the WiFi

## Configuration

- on a browser, go to the IP-address shown on the IDE - monitor
Configure your devices.

