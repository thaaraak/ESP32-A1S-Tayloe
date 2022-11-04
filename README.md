# ESP32-A1S-Tayloe

Unfortunately the build is a bit of a pain to set up and I don't have a build script. These are the following build steps to take

1) I would first get the arduino audio tools built and running with some of the examples (i2s i2s for example). Until you get that set up nothing else will compile. Running the examples also confirms your I2S setup is correct. Instructions are here:
- https://github.com/pschatzmann/arduino-audio-tools

2) Then you need to make sure the general library for Button processing
- https://www.arduino.cc/reference/en/libraries/bounce2/

3) Then finally install the libraries from the forks I have created
- https://github.com/thaaraak/firduino
- https://github.com/thaaraak/LiquidCrystal_I2C
- https://github.com/thaaraak/es8388
- https://github.com/thaaraak/Si5351Arduino
