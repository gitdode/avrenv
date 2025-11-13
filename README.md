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

The receiver currently has a display and logs the received measurements via USART to the PC.
Works all fine so far, but at least the transmitter will get a watchdog that resets it in case
something gets stuck.  

![IMG_20251113_201312](https://github.com/user-attachments/assets/777dec69-9fb7-4a42-8252-c54466862766)
