# Intelligent lock

At the beginning user enters SSID and password of WiFi.
Then esp32 enters a client mode and connects to the Internet.
Also it makes requests to server to get commands to smart lock.
If there is new command, it is send to smart lock through Bluetooth low energy.