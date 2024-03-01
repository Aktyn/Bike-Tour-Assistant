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
```
sudo apt install build-essential
sudo apt install cmake
sudo apt install libbluetooth-dev
sudo apt install libjpeg-dev
sudo apt install exiftool
```
- ##### LCD Display
```
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