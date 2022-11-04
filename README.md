# ESP32-A1S-Tayloe

Unfortunately the build is a bit of a pain to set up and I don't have a build script. These are the following build steps to take

1) I would first get the arduino audio tools built and running with some of the examples (i2s i2s for example). Until you get that set up nothing else will compile. Running the examples also confirms your I2S setup is correct. Instructions are here:
- https://github.com/pschatzmann/arduino-audio-tools


2) Then you need to make sure the general library for Button processing is installed
- https://www.arduino.cc/reference/en/libraries/bounce2/


3) Then finally install the libraries from the repos/forks I have created
- https://github.com/thaaraak/firduino
- https://github.com/thaaraak/LiquidCrystal_I2C
- https://github.com/thaaraak/es8388
- https://github.com/thaaraak/Si5351Arduino

4) Once you have done all that the main Radio code is here:
- https://github.com/thaaraak/ESP32-A1S-Tayloe/tree/master/Radio

There are a selection of different coefficient files in there so you can play around with different taps/samplling frequencies. Make sure the sample rate in the Radio code matches the sample rate in the header file. You can generate new coefficients using this code:
- https://github.com/thaaraak/ESP32-Arduino-Audio/blob/main/hilbert_generator.py

I have a video on that here
- https://www.youtube.com/watch?v=BUGQaTJ6aWI&t=71s

You will need the python programming environment and the scipy libraries. I use Anaconda and Spyder

The kicad files are from kicad 5.x and the libraries I use are here:
- https://github.com/thaaraak/kicad-libraries
