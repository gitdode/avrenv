# avrenv

[![make-and-test](https://github.com/gitdode/avrenv/actions/workflows/make-and-test.yml/badge.svg)](https://github.com/gitdode/avrenv/actions/workflows/make-and-test.yml)

## About

Experimental project to log measurements of different environmental sensors 
such as temperature, humidity, barometric pressure and gas sensor resistance 
plus GPS position to an SD card and transmit them via radio to a receiver 
publishing the measurements to a web page.  

The idea is to send the sensors up to the sky with a weather baloon and collect 
some hopefully useful and interesting measurements, and of course to recover the 
baloon afterwards.  

### Transmitter

The transmitter using a Microchip `AVR64EA` MCU is reset by the watchdog should it get stuck somehow, 
while the SD card address is maintained so previously written records are not overwritten. 
If battery voltage goes below 3000 mV, the `PA1616S` GPS module is powered down, saving ~20 mA for 
the rest to keep running. Below 2500 mV, no more attempt is made to write to SD card, while the 
measurements of the `BME688` sensor are still transmitted via an `RFM95` radio module. 
Below 2400 mV the whole thing pretty much stops doing anything.  

![IMG_20251122_214512](https://github.com/user-attachments/assets/3c698c72-fcee-4ab1-9523-372c5a5f4ef3)

GPX tracks created from the records written to SD card and sent via radio every 4 seconds 
look very good, so the `PA1616S` GPS module seems to do a good job.  

<img width="2096" height="1278" alt="IMG_20251119_004403" src="https://github.com/user-attachments/assets/297a95a5-d6ed-4c48-ab64-34f81f6b8821" />

### Receiver

The receiver is also using an `AVR64EA` and an `RFM95`, and currently has a display and logs the received 
measurements via USART to the PC. Very simple.  

![IMG_20251113_201312](https://github.com/user-attachments/assets/777dec69-9fb7-4a42-8252-c54466862766)

### Client

The client program running on a Debian PC reads the data from the receiver via the serial port and converts 
it to Json using `json-c`. Then it gets an access token from the Keycloak server and POSTs the Json data 
along with the token using `libcurl` to the [Quarkus web app](https://baloon.luniks.net).  

### Web Application

Besides the REST endpoint, the Quarkus web app provides a simple but responsive web page 
written in plain HTML, CSS and JavaScript, using Quarkus Web Bundler including `jQuery`, 
and `Leaflet` to show the current position on an OpenStreetMap map.

### Tests

Also a small experimental project should have tests, right? The client program is using 
`AceUnit` for unit tests, while those making HTTP requests are actually integration tests, 
simply using the Quarkus web app running in dev mode with Dev Services for Keycloak, which is 
also very convenient for development. The Quarkus web app is simply tested by building the project.
