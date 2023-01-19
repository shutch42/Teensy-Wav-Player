# Teensy WAV File Player 🎵
This is a project I used to introduce myself to the Teensy Audio Library. 
The finished product is able to read .wav files loaded onto an microSD card, and play the audio over a DAC into headphones. 
In addition, the device has an LCD to show the currently playing file, as well as the volume level and play/pause status. 
The song playing can be controlled via play/pause, rewind, and fast-forward buttons. The volume level is controlled with an encoder.
## Hardware
![breadboard](https://github.com/shutch42/Teensy-Wav-Player/blob/main/img/breadboard.jpg)  
The project is controlled using a Teensy 4.1 microcontroller. 
The onboard microSD card reader is used to read .wav files, and the audio signal is sent via I2S to a PCM5102A DAC. 
The LCD is controlled using a Hitachi HD44780 LCD controller. A rotary encoder and 3 pushbuttons are used as input. 
Also pictured on the breadboard is a PCM1808 ADC, but this chip was not used in the project. 
It was just connected to be used on a later project.  
## Software
The most important piece of software here is the teensy audio library. 
The general structure of the device was designed using the Teensy's Audio System Design Tool, shown below.  
![design](https://github.com/shutch42/Teensy-Wav-Player/blob/main/img/teensy_audio_design_tool.png)  
As you can see, WAV files are pulled from the SD card and directed using I2S into 2 mixers, 1 for each channel. 
These mixers are used to control the volume of the track. The audio is then directed to the first I2S port to be sent to the DAC. 
The arduino SD library was used to traverse files on the SD card, and determine which file to send to the audio library. 
Finally, all of these features were controlled using input polling on the pushbuttons and rotary encoder. 
Depending on the values of each input, the audio library would pause/play or change volume, or the SD card would traverse to the next song.
## Future Plans
This was a very satifying project, and I am planning to continue working with audio on the Teensy in the future. 
In particular, I'd like to start recording audio over I2S, and possibly even look into recording multi-channel audio.
The goal with this platform is to eventually create an audio mixer, recorder, and midi controller to simplify the process of making music and recording with hardware synths. 
