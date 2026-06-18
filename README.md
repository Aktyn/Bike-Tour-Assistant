# Bike-Tour-Assistant
The next iteration of my previous project: Cyclocomputer.

It now uses Raspberry Pi Zero 2 W with LCD Module and camera for hardware.

The codebase of the Raspberry Pi project is written in C++.

Separate branch contains Android project for phone control and GPS signal source.

---

### Hardware
- ##### Raspberry Pi Zero 2 W
- ##### [2.4inch LCD Module](https://www.waveshare.com/wiki/2.4inch_LCD_Module) ([Hardware connection table](https://www.waveshare.com/wiki/2.4inch_LCD_Module#Raspberry_Pi_hardware_connection))
- ##### Any compatible raspberry pi camera (e.g.: RPi Zero V1.3 Camera)
- Any power source for raspberry pi

### Prerequisites: 
- ##### Dependencies
    `sudo apt install git build-essential cmake libbluetooth-dev libjpeg-dev exiftool python3-smbus python3-setuptools`  
    Enable **SPI** interface in raspi-config

- ##### LCD Display
    ```bash
    wget https://github.com/joan2937/lg/archive/master.zip
    unzip master.zip
    cd lg-master
    sudo make install
    ```
    More at [waveshare.com/wiki/2.4inch_LCD_Module](https://www.waveshare.com/wiki/2.4inch_LCD_Module)

### Compilation:
```
mkdir build
cd ./build
cmake ..
make
```

### Executing
Run the `BikeTourAssistant` executable that generates in build directory (sudo is required)  
Sometimes it is required to unblock bluetooth with `sudo rfkill unblock bluetooth`

#### Optional systemd service to set up autostart:
*/etc/systemd/system/bike-tour-assistant.service*
```ini
[Unit]
Description=Bike Tour Assistant autostart service
After=bluetooth.service
Requires=bluetooth.service

[Service]
Type=simple
User=root
WorkingDirectory=/home/pi/Bike-Tour-Assistant
ExecStartPre=/usr/sbin/rfkill unblock bluetooth
ExecStart=/home/pi/Bike-Tour-Assistant/build/BikeTourAssistant
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Adjust the paths according to where you cloned the Bike-Tour-Assistant repository.
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now bike-tour-assistant
sudo systemctl start bike-tour-assistant
```
