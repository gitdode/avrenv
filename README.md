# avrenv

## About

Experimental project to log measurements of different environmental sensors 
such as temperature, humidity, barometric pressure and gas sensor resistance 
plus GPS position to an SD card and transmit them via radio to a receiver, 
displaying and publishing the measurements to a web site.  

The idea is to send the sensors up to the sky with a weather baloon and collect 
some hopefully useful and interesting measurements, and of course to recover the 
baloon afterwards.  

![IMG_20251107_223051](https://github.com/user-attachments/assets/9540ea7a-6358-4019-a66e-71e69bba621a)

The transmitter is reset by the watchdog should it get stuck somehow, while the SD card address is 
maintained so previously written records are not overwritten. If battery power goes below 3000 mV, 
the PA1616S GPS module is powered down, saving ~20 mA for the the rest to keep running. Below 2500 mV,
no more attempt is made to write to SD card, while the measurements of the BME688 sensor are still 
transmitted via radio. Below 2400 mV the whole thing pretty much stops doing anything.  

<img width="2096" height="1278" alt="Bildschirmfoto_20251119_004403" src="https://github.com/user-attachments/assets/54ed0d0b-42d3-49b2-9c6b-86ce40191ab2" />

GPX tracks created from the records written to SD card and sent via radio every 4 seconds 
look very good, so the PA1616S GPS module seems to do a good job.  

![IMG_20251113_201312](https://github.com/user-attachments/assets/777dec69-9fb7-4a42-8252-c54466862766)

The receiver currently has a display and logs the received measurements via USART to the PC.  
