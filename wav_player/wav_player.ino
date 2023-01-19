#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <LiquidCrystal.h>
#include <stddef.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=136,333
AudioMixer4              mixer1;         //xy=328,298
AudioMixer4              mixer2;         //xy=330,368
AudioOutputI2S           i2s1;           //xy=476,332
AudioConnection          patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playSdWav1, 1, mixer2, 0);
AudioConnection          patchCord3(mixer1, 0, i2s1, 0);
AudioConnection          patchCord4(mixer2, 0, i2s1, 1);
// GUItool: end automatically generated code

struct Song{
  char song_name[256] = {'\n'};
  Song *next_song;
  Song *prev_song;
};

Song* curr_song;

byte play_char[8] = {
  0b00000,
  0b00000,
  0b01000,
  0b01100,
  0b01110,
  0b01100,
  0b01000,
  0b00000
};

byte pause_char[8] = {
  0b00000,
  0b00000,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b00000
};

byte volume_1[8] = {
  0b00000,
  0b00010,
  0b00110,
  0b11110,
  0b11110,
  0b00110,
  0b00010,
  0b00000
};

byte volume_2[8] = {
  0b01000,
  0b00100,
  0b00010,
  0b01010,
  0b01010,
  0b00010,
  0b00100,
  0b01000
};

byte fast_forward[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01010,
  0b01111,
  0b01010,
  0b00000,
  0b00000
};

byte rewind_symbol[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01010,
  0b11110,
  0b01010,
  0b00000,
  0b00000
};

#define SDCARD_CS_PIN BUILTIN_SDCARD
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13

#define button_rw 33
#define button_ff 34
#define button_play 35
#define encoder_a 36
#define encoder_b 37

bool button_state_rw = 0;
bool button_state_ff = 0;
bool button_state_play = 0;
bool curr_state_a;
bool curr_state_b;
int sound_volume = 50;

bool sound_playing = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(encoder_a, INPUT);
  pinMode(encoder_b, INPUT);
  digitalWrite(encoder_a, HIGH);
  digitalWrite(encoder_b, HIGH);
  curr_state_a = digitalRead(encoder_a);
  curr_state_b = digitalRead(encoder_b);
  
  AudioMemory(2);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }


  // Open first file on drive
  File root = SD.open("/");
  File curr_entry = root.openNextFile();

  // Load filename as starting song
  curr_song = (Song *)malloc(sizeof(Song));
  const char* song_name = curr_entry.name();
  strcpy(curr_song->song_name, song_name);
  
  curr_song->prev_song = NULL;
  curr_song->next_song = NULL;
  Song* start_song = curr_song;
  
  // Start setting up next song
  Song* prev_song = curr_song;
  curr_entry.close();
  curr_entry = root.openNextFile();

  // Loop through remaining files
  while(curr_entry) {
    curr_song = (Song *)malloc(sizeof(Song));
    strcpy(curr_song->song_name, curr_entry.name());
    curr_song->prev_song = prev_song;
    curr_song->next_song = NULL;
    
    if(prev_song != NULL) {
      prev_song->next_song = curr_song;
    }
    
    prev_song = curr_song;
    curr_entry.close();
    curr_entry = root.openNextFile();
  }
  curr_song = start_song;

  
  lcd.begin(16, 2);
  lcd.print(curr_song->song_name);
  lcd.createChar(0, play_char);
  lcd.createChar(1, pause_char);
  lcd.createChar(2, volume_1);
  lcd.createChar(3, volume_2);
  lcd.createChar(4, fast_forward);
  lcd.createChar(5, rewind_symbol);
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(10, 1);
  lcd.write(byte(2));
  lcd.write(byte(3));
  lcd.print(" 50%");
  
  mixer1.gain(0, .5);
  mixer2.gain(0, .5);
  playSdWav1.play(curr_song->song_name);
  delay(25);
}

int check_enc() {
  bool next_state_a = digitalRead(encoder_a);
  bool next_state_b = digitalRead(encoder_b);
  int change = 0;
  
  if(!next_state_a && !next_state_b) {
    if (!curr_state_a && curr_state_b && sound_volume > 0) {
      change = -1;
    }
    else if (curr_state_a && !curr_state_b && sound_volume < 100) {
      change = 1;
    }
  }

  curr_state_a = next_state_a;
  curr_state_b = next_state_b;
  return change;
}

void check_ff() {
  if(digitalRead(button_ff) && !button_state_ff) {
    button_state_ff = 1;
    if(curr_song->next_song != NULL) {
      sound_playing = 1;
      lcd.setCursor(0, 1);
      lcd.write(byte(4)); // Display fast-forward symbol
      
      curr_song = curr_song->next_song;
      playSdWav1.play(curr_song->song_name);
      lcd.setCursor(0, 0);
      lcd.print("                ");
      lcd.setCursor(0, 0);
      lcd.print(curr_song->song_name);
    }
  }
  else if(!digitalRead(button_ff) && button_state_ff) {
    button_state_ff = 0;
    lcd.setCursor(0, 1);
    lcd.write(byte(0)); // Display play symbol
  }
}

void check_rw() {
  if(digitalRead(button_rw) && !button_state_rw && curr_song->prev_song != NULL) {
    button_state_rw = 1;
    lcd.setCursor(0, 1);
    lcd.write(byte(5)); // Display rewind symbol
    
    sound_playing = 1;
    curr_song = curr_song->prev_song;
    playSdWav1.play(curr_song->song_name);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(curr_song->song_name);
  }
  else if(!digitalRead(button_rw) && button_state_rw) {
    button_state_rw = 0;
    lcd.setCursor(0, 1);
    lcd.write(byte(0)); // Display "play" symbol
  }
}

void check_play() {
  if(digitalRead(button_play) && !button_state_play) {
    if(playSdWav1.isPlaying()) {
      sound_playing = 0;
      lcd.setCursor(0, 1);
      lcd.write(byte(1)); // Display "pause" symbol
    }
    else {
      sound_playing = 1;
      lcd.setCursor(0, 1);
      lcd.write(byte(0)); // Display "play" symbol
    }
    button_state_play = 1;
    playSdWav1.togglePlayPause();
  }
  else if(!digitalRead(button_play) && button_state_play) {
    button_state_play = 0;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  check_play();
  check_ff();
  check_rw();
  
  int volume_change = check_enc();
  if(volume_change) {
    sound_volume += volume_change;
    
    lcd.setCursor(12, 1);
    lcd.print("    ");
    
    if(sound_volume == 100) {
      lcd.setCursor(12, 1);
      lcd.print("100%");
    }
    else if(sound_volume > 9) {
      lcd.setCursor(13, 1);
      lcd.print(sound_volume);
      lcd.print("%");
    }
    else {
      lcd.setCursor(14, 1);
      lcd.print(sound_volume);
      lcd.print("%");
    }
    
    mixer1.gain(0, sound_volume/100.0);
    mixer2.gain(0, sound_volume/100.0);
  }

  // When song ends, play next song
  if(sound_playing and !playSdWav1.isPlaying() and curr_song->next_song != NULL) {
    curr_song = curr_song->next_song;
    playSdWav1.play(curr_song->song_name);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(curr_song->song_name);
  }
}
