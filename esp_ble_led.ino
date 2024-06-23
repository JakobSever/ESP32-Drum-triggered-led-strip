#include <FastLED.h>

#define PIEZO_PIN 34   // GPIO 34 as the piezo sensor pin
#define STATUS_LED 5   // GPIO 5 as the status LED pin

#define NUM_LEDS 59    // Number of LEDs in your strip
#define DATA_PIN 16    // GPIO 16 as the data pin
#define BUTTON_PIN 2   // GPIO 2 for the button
#define MAX_TRAILS 59  // Maximum number of active trails

#define DEBOUNCE_INTERVAL 200 // Debounce interval in milliseconds
#define BLINK_INTERVAL 500    // Blink interval in milliseconds

struct Trail {
  int position;         	// Current position of the trail
  int speed;            	// Speed of the trail in LEDs per second
  CRGB color1;				// Start color of the trail
  CRGB color2;		  		// End color of the trail	
  bool direction;			// Direction of the trail
  unsigned long lastUpdate; // Last update time for the trail
};

CRGB leds[NUM_LEDS];
Trail trails[MAX_TRAILS];  		// Store information about each trail
int numActiveTrails = 0;   		// Number of active trails
int nextTrailIndex = 0;    		// Index to add the next trail
bool trailDirection = false; 	// Used for program 2
bool colorChange = true; 		// Used for program 3
int previousProgram;			// Previous program selection

unsigned int transitionCurrentStep = 1; // Current step of the transition effect
const int totalTransitionSteps = 100; 	// Total number of steps for the transition effect
bool direction = true; 					// Transition direction

unsigned long lastHitTime = 0; // Last time a hit was registered

#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Davidove lulčke"
#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 57 bytes
  { 255,9,0,0,0,50,0,17,0,0,0,25,1,106,200,1,1,5,0,6,
  25,140,56,56,1,29,6,25,78,56,56,1,29,3,5,6,96,21,133,1,
  27,4,6,56,94,15,128,218,27,4,6,35,94,15,128,1,27 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t rgb_01_r; // =0..255 Red color value
  uint8_t rgb_01_g; // =0..255 Green color value
  uint8_t rgb_01_b; // =0..255 Blue color value
  uint8_t rgb_02_r; // =0..255 Red color value
  uint8_t rgb_02_g; // =0..255 Green color value
  uint8_t rgb_02_b; // =0..255 Blue color value
  uint8_t select_01; // from 0 to 5
  int8_t slider_01; // from 0 to 100
  int8_t slider_02; // from 0 to 100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)

unsigned long checkTime;
unsigned long elapsedTime;

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
    trails[i].lastUpdate = millis();
  }

  // Set the starting colors for the effects
  RemoteXY.rgb_01_r = 255;
  RemoteXY.rgb_02_b = 255;
  RemoteXY.slider_02 = 50;

  RemoteXY.select_01 = 3;
  previousProgram = RemoteXY.select_01;

  checkTime = millis();

}

void resetAllTrails() {
  for (int i = 0; i < MAX_TRAILS; i++) {
      trails[i].position = -1;
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  numActiveTrails = 0;
}

CRGB getRandomColor() {
  uint8_t components[3] = {random(0, 256), random(0, 256), random(0, 256)};
  int zeroIndex = random(0, 3);
  components[zeroIndex] = 0;
  return CRGB(components[0], components[1], components[2]);
}

void shootTrail(CRGB *color1, CRGB *color2, int *speed, bool useDirection = false) {
  if (numActiveTrails < MAX_TRAILS) {
    trails[nextTrailIndex].speed = *speed * 2; // Max speed 200
    Serial.println(*speed);
    trails[nextTrailIndex].color1 = *color1;
    trails[nextTrailIndex].color2 = *color2;
    if(useDirection) {
      trails[nextTrailIndex].direction = nextTrailIndex % 2 == 0;
    } else {
      trails[nextTrailIndex].direction = true;
    }

    if(trails[nextTrailIndex].direction) {
      trails[nextTrailIndex].position = 0;
    } else {
      trails[nextTrailIndex].position = NUM_LEDS - 1;
    }

    trails[nextTrailIndex].lastUpdate = millis();

    numActiveTrails++;

    // Increment the next trail index in a circular manner
    nextTrailIndex = (nextTrailIndex + 1) % MAX_TRAILS;
  }
}

void shootTrailUpdate() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < MAX_TRAILS; i++) {
    if (trails[i].position >= 0 && trails[i].position < NUM_LEDS) {
      unsigned long timeElapsed = currentTime - trails[i].lastUpdate;

      // Calculate the time needed to move one LED based on the speed
      unsigned long timePerLED = 1000 / trails[i].speed;

      // Update the position if enough time has passed
      if (timeElapsed >= timePerLED) {
        // Clear the previous position
        if (trails[i].direction && trails[i].position > 0) {
          leds[trails[i].position - 1] = CRGB::Black;
        } else if (!trails[i].direction && trails[i].position < NUM_LEDS - 1) {
          leds[trails[i].position + 1] = CRGB::Black;
        }

        trails[i].lastUpdate = currentTime;

        // Update the position based on the direction
        if (trails[i].direction) {
          trails[i].position += 1;
        } else {
          trails[i].position -= 1;
        }

        // Calculate the color of the trail position
        float progress = float(trails[i].position) / float(NUM_LEDS - 1);
        CRGB color = blend(trails[i].color1, trails[i].color2, progress * 255);

        if (trails[i].position >= 0 && trails[i].position < NUM_LEDS) {
          leds[trails[i].position] = color;
        }
      }
    }

    // Check if trails have gone beyond the strip and reset them
    if (trails[i].position >= NUM_LEDS || trails[i].position < 0) {
      if (trails[i].position >= NUM_LEDS) {
        leds[NUM_LEDS - 1] = CRGB::Black;
      } else if (trails[i].position < 0) {
        leds[0] = CRGB::Black;
      }
      trails[i].position = -1;
      numActiveTrails--;
    }
  }
}

void transitionStrip(CRGB *color1, CRGB *color2, int *effectValue) {
  elapsedTime = millis() - checkTime;

  int neweffectValue = *effectValue;
  if(neweffectValue > 50) {
    neweffectValue * 10;
  }
  
  if(elapsedTime > neweffectValue) {
    checkTime = millis();

    if((transitionCurrentStep > totalTransitionSteps - 1) || transitionCurrentStep < 1) {
      direction = !direction;
    }

    if(direction) {
      transitionCurrentStep += 1;
    } else {
      transitionCurrentStep -= 1;
    }

    float progress = (transitionCurrentStep * 255) / totalTransitionSteps;
    CRGB color = blend(*color1, *color2, progress);

    // Set all LEDs to the current color
    fill_solid(leds, NUM_LEDS, color);
  }
}

void blinkStrip(CRGB *color1, CRGB *color2, int *effectValue) {
	if(*effectValue > 66) {
    fill_solid(leds, NUM_LEDS, *color2);
	} else if(*effectValue > 33) {
    if(colorChange) {
			fill_solid(leds, NUM_LEDS, *color1);
		} else {
			fill_solid(leds, NUM_LEDS, *color2);
		}
		colorChange = !colorChange;
	} else {
		fill_solid(leds, NUM_LEDS, *color1);
	}
}

void switchBetweenColors(CRGB *color1, CRGB *color2) {
  if(colorChange) {
		fill_solid(leds, NUM_LEDS, *color1);
	} else {
		fill_solid(leds, NUM_LEDS, *color2);
	}
	colorChange = !colorChange;
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

  // Get threshold value
  int threshold = RemoteXY.slider_01 * 4095 / 100;

  // Get effect value
  int effectValue = RemoteXY.slider_02;
  if(effectValue == 0) {
    // Division by zero fix
    RemoteXY.slider_02 = 1;
  }

  // Get color values from app
  CRGB color1 = CRGB(RemoteXY.rgb_01_r, RemoteXY.rgb_01_g, RemoteXY.rgb_01_b);
  CRGB color2 = CRGB(RemoteXY.rgb_02_r, RemoteXY.rgb_02_g, RemoteXY.rgb_02_b);

  // Check if color1 is gray and set to random color if true
  if (color1.r > 35 && color1.r == color1.g && color1.g == color1.b) {
    color1 = getRandomColor();
  }

  // Check if color2 is gray and set to random color if true
  if (color2.r > 35 && color2.r == color2.g && color2.g == color2.b) {
    color2 = getRandomColor();
  }

  if(previousProgram != RemoteXY.select_01) {
    if(previousProgram == 1 || previousProgram == 3 || previousProgram == 4) {
      resetAllTrails();
    }

    if(RemoteXY.select_01 == 3) {
      fill_solid(leds, NUM_LEDS, color1);
    }

    previousProgram = RemoteXY.select_01;
  }

  // Debouncing logic: Check if the piezo value exceeds the threshold and enough time has passed since the last hit
  unsigned long currentTime = millis();
  if (piezoValue > threshold && currentTime - lastHitTime > DEBOUNCE_INTERVAL) {
    lastHitTime = currentTime;  // Update the last hit time

    // Execute the selected program
    switch (RemoteXY.select_01) {
      case 0:
        shootTrail(&color1, &color2, &effectValue);
        break;
      case 2:
        shootTrail(&color1, &color2, &effectValue, true);
        break;
      case 3:
        switchBetweenColors(&color1, &color2);
      	break;
	  case 4:
        blinkStrip(&color1, &color2, &effectValue);
        break;
    }
  }

  // Update trails or execute transition effect based on the selected program
  switch (RemoteXY.select_01) {
    case 0:
    case 2:
      shootTrailUpdate();
      break;
    case 1:
      transitionStrip(&color1, &color2, &effectValue);
      break;
    case 4:
      if (currentTime - lastHitTime > BLINK_INTERVAL) {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
      }
	    break;
  }

  // Update the LEDs
  FastLED.show();
}
 		// Debounce interval in m 		// Debounce interval in millisecondslliseconds
