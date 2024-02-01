# Bike-Tour-Assistant
The next iteration of my previous project: Cyclocomputer.

It now uses Raspberry Pi Zero 2 W with LCD display and camera for hardware.

The codebase of the Raspberry Pi project is written in C++.

Separate branch contains Android project for phone control and GPS signal source.

---

### Prerequisites: 
```
sudo apt install build-essential
sudo apt install cmake
sudo apt install libbluetooth-dev
```

### Compilation:
```
mkdir build
cd ./build
cmake ..
make
```

### Executing
Run the `BikeTourAssistant` executable that generates in build directory