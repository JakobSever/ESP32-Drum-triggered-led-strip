#define PIEZO_PIN 34

#define STATUS_LED 5

#include <FastLED.h>

#define NUM_LEDS 59    // Number of LEDs in your strip
#define DATA_PIN 16    // GPIO 16 as the data pin
#define BUTTON_PIN 2   // GPIO 2 for the button
#define MAX_TRAILS 59  // Maximum number of active trails

struct Trail {
  int position;  // Current position of the trail
  int speed;     // Speed of the trail
  CRGB color1;	 // Effect color 1
  CRGB color2; 	 // Effect color 2
};

CRGB leds[NUM_LEDS];
Trail trails[MAX_TRAILS];  // Store information about each trail
int numActiveTrails = 0;   // Number of active trails
int nextTrailIndex = 0;    // Index to add the next trail

#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Davidove lulčke"
#include <RemoteXY.h>

// RemoteXY GUI configuration
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =  // 57 bytes
  { 255, 8, 0, 1, 0, 50, 0, 17, 0, 0, 0, 8, 1, 106, 200, 1, 1, 5, 0, 3,
    6, 5, 93, 26, 132, 12, 31, 4, 5, 60, 94, 13, 128, 12, 31, 6, 23, 137, 58, 58,
    134, 26, 6, 23, 75, 58, 58, 134, 26, 66, 11, 37, 82, 17, 129, 106, 31 };

struct {
  // input variables
  uint8_t select_01;  // from 0 to 4
  int8_t slider_01;   // from 0 to 100
  uint8_t rgb_01_r;   // =0..255 Red color value
  uint8_t rgb_01_g;   // =0..255 Green color value
  uint8_t rgb_01_b;   // =0..255 Blue color value
  uint8_t rgb_02_r;   // =0..255 Red color value
  uint8_t rgb_02_g;   // =0..255 Green color value
  uint8_t rgb_02_b;   // =0..255 Blue color value

  // output variables
  int8_t level_01;  // from 0 to 100

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)


void setup() {
  RemoteXY_Init();

  Serial.begin(115200);

  // Piezo init
  pinMode(PIEZO_PIN, INPUT);

  // Connection status init
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  // Led init
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  // Initialize the trails
  for (int i = 0; i < MAX_TRAILS; i++) {
    trails[i].position = -1; // -1 indicates inactive trail
  }

  RemoteXY.rgb_01_r = 255;
  RemoteXY.rgb_02_b = 255;
}

void loop() {
  RemoteXY_Handler();

  if (RemoteXY.connect_flag) {
    digitalWrite(STATUS_LED, HIGH);
  } else {
    digitalWrite(STATUS_LED, LOW);
  }

  // Get piezo value
  const int piezoValue = analogRead(PIEZO_PIN);

  // Send value to app
  RemoteXY.level_01 = piezoValue * 100 / 4095;

  // Get threshold value
  const int threshold = RemoteXY.slider_01 * 4095 / 100;

  // Get color values from app
  CRGB color1 = CRGB(RemoteXY.rgb_01_r, RemoteXY.rgb_01_g, RemoteXY.rgb_01_b);
  CRGB color2 = CRGB(RemoteXY.rgb_02_r, RemoteXY.rgb_02_g, RemoteXY.rgb_02_b);

  // Check selected program
  switch (RemoteXY.select_01) {
    case 0:
      if (piezoValue > threshold) {
        Serial.print(threshold);
        Serial.print(" - ");
        Serial.println(piezoValue);
        shootTrail(color1, color2);
      }
      shootTrailUpdate();
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
  }

  FastLED.show();
}

void shootTrail(CRGB color1, CRGB color2) {
  if (numActiveTrails < MAX_TRAILS) {
    trails[nextTrailIndex].position = 0;
    trails[nextTrailIndex].speed = 1;
    trails[nextTrailIndex].color1 = color1;
    trails[nextTrailIndex].color2 = color2;
    numActiveTrails++;

    // Increment the next trail index in a circular manner
    nextTrailIndex = (nextTrailIndex + 1) % MAX_TRAILS;
  }
}

void shootTrailUpdate() {
  // Turn off all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (int i = 0; i < MAX_TRAILS; i++) {
    if (trails[i].position >= 0 && trails[i].position < NUM_LEDS) {
      // Calculate the color of the trail position
      float progress = float(trails[i].position) / float(NUM_LEDS - 1);
      CRGB color = blend(trails[i].color1, trails[i].color2, progress * 255);

      leds[trails[i].position] = color;
      trails[i].position += trails[i].speed;
    }

    // Check if trails have gone beyond the strip and reset them
    if (trails[i].position >= NUM_LEDS) {
      trails[i].position = -1;
      numActiveTrails--;
    }
  }
}

