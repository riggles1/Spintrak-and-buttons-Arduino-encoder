#include <Encoder.h>
#include <Mouse.h>
#include <Joystick.h>

#define DEBOUNCE_MS 2

bool spinnerStickMode = false;
int16_t stickX = 0;
int16_t lastStickX = 0;
int16_t lastStickY = 0;

bool lastMouseLeft = false;
bool lastMouseRight = false;
bool lastMouseMiddle = false;

bool mouseClickMode = false;
bool debouncedButtons[8] = {0};
bool lastPhysicalButtons[8] = {0};
unsigned long lastChangeTime[8] = {0};

bool debouncedDpad[4] = {0};
bool lastPhysicalDpad[4] = {0};
unsigned long lastDpadChange[4] = {0};


bool lastButtons[14] = {0};
int lastHat = -1;


// Spinner 
#define ENC_X1 2
#define ENC_X2 3

Encoder spintrak(ENC_X1, ENC_X2);

long lastPosition = 0;
long accumulator = 0;

unsigned long lastSend = 0;
#define SEND_INTERVAL 1000  


// Gamepad Setup
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_GAMEPAD,
  14,    // buttons
  1,     // hat
  true,  // X axis enabled for spinner as stick mode
  true,  // Y enabled for Y centering
  false, // Z
  false, // Rx
  false, // Ry
  false, // Rz
  false, // rudder
  false, // throttle
  false, // accelerator
  false, // brake
  false  // steering
);

// Button pins 1–8
const int buttonPins[8] = {4,5,6,7,8,9,20,19};

// Shifter button
#define SHIFT_PIN 18   // D18 (A0)

// D-pad pins
#define DPAD_RIGHT 15
#define DPAD_LEFT  14
#define DPAD_UP    16
#define DPAD_DOWN  10

void setup() {

  pinMode(ENC_X1, INPUT_PULLUP);
  pinMode(ENC_X2, INPUT_PULLUP);

  for(int i=0; i<8; i++){
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  pinMode(SHIFT_PIN, INPUT_PULLUP);

  pinMode(DPAD_RIGHT, INPUT_PULLUP);
  pinMode(DPAD_LEFT,  INPUT_PULLUP);
  pinMode(DPAD_UP,    INPUT_PULLUP);
  pinMode(DPAD_DOWN,  INPUT_PULLUP);

  Mouse.begin();
  Joystick.begin(false);  
  Joystick.setXAxisRange(-32767, 32767);
  Joystick.setXAxis(0);
  Joystick.setYAxisRange(-32767, 32767);
  Joystick.setYAxis(0);
}

void loop() {


// Spinner
long newPosition = spintrak.read();

if (newPosition != lastPosition) {

    long delta = newPosition - lastPosition;

    accumulator -= (delta * 6) / 5;
    lastPosition = newPosition;

}

  unsigned long nowMicros = micros();

  if ((long)(nowMicros - lastSend) >= SEND_INTERVAL) {
    lastSend += SEND_INTERVAL;  

    int move = 0;
    if (accumulator > 127) move = 127;
    else if (accumulator < -127) move = -127;
    else move = accumulator;
    if (move != 0) {
    
        if (!spinnerStickMode) {    
            Mouse.move(move, 0, 0);    
        } else {          
          // Stick mode
          int32_t temp = (int32_t)stickX + ((int32_t)move * 80);         
          // Range clamp
          if (temp > 32767) temp = 32767;
          if (temp < -32767) temp = -32767;          
          stickX = (int16_t)temp;    
            Joystick.setXAxis(stickX);
        }
    
        accumulator -= move;
    }
  }


// Shift State
unsigned long now = millis();

bool shiftHeld = !digitalRead(SHIFT_PIN);


// Button Debounce
bool b[8];

for (int i = 0; i < 8; i++) {

  bool raw = !digitalRead(buttonPins[i]);

  if (raw != lastPhysicalButtons[i]) {
    lastChangeTime[i] = now;
    lastPhysicalButtons[i] = raw;
  }

  if ((now - lastChangeTime[i]) >= DEBOUNCE_MS) {
    debouncedButtons[i] = raw;
  }

  b[i] = debouncedButtons[i];
}

// Logical button state
bool currentButtons[14] = {0};

if (!shiftHeld) {

  // Normal mapping
  currentButtons[0] = b[0];
  currentButtons[1] = b[1];
  currentButtons[2] = b[2];
  currentButtons[3] = b[3];
  currentButtons[4] = b[4];
  currentButtons[5] = b[5];
  currentButtons[6] = b[6];
  currentButtons[7] = b[7];

} else {

  // Shifted mapping
  currentButtons[8]  = b[0];  // D4
  currentButtons[9]  = b[1];  // D5
  currentButtons[10] = b[2];  // D6
  currentButtons[11] = b[3];  // D7
  currentButtons[12] = b[4];  // D8
  currentButtons[13] = b[5];  // D9

  // A2 becomes Button 8
  currentButtons[7] = b[6];
}

// POV Hat Switch
int dpadPins[4] = {DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT};

for (int i = 0; i < 4; i++) {

  bool raw = !digitalRead(dpadPins[i]);

  if (raw != lastPhysicalDpad[i]) {
    lastDpadChange[i] = now;
    lastPhysicalDpad[i] = raw;
  }

  if ((now - lastDpadChange[i]) >= DEBOUNCE_MS) {
    debouncedDpad[i] = raw;
  }
}

bool up    = debouncedDpad[0];
bool down  = debouncedDpad[1];
bool left  = debouncedDpad[2];
bool right = debouncedDpad[3];

// Toggle Mouse Click Mode (Shift + Down)
static bool lastDownCombo = false;
static unsigned long lastDownToggleTime = 0;

bool downCombo = shiftHeld && down;

if (downCombo && !lastDownCombo && (millis() - lastDownToggleTime > 150)) {
    mouseClickMode = !mouseClickMode;
    lastDownToggleTime = millis();
}

lastDownCombo = downCombo;

// Toggle Stick Mode (Shift + Left)
static bool lastLeftCombo = false;
static unsigned long lastLeftToggleTime = 0;

bool leftCombo = shiftHeld && left;

if (leftCombo && !lastLeftCombo && (millis() - lastLeftToggleTime > 150)) {
    spinnerStickMode = !spinnerStickMode;
    stickX = 0;
    lastLeftToggleTime = millis();
}

lastLeftCombo = leftCombo;

if (spinnerStickMode && up) {
    stickX = 0;
    Joystick.setXAxis(stickX);
}

int hat = -1;

if (up && !down) {
  if (right && !left)      hat = 45;
  else if (left && !right) hat = 315;
  else                     hat = 0;
}
else if (down && !up) {
  if (right && !left)      hat = 135;
  else if (left && !right) hat = 225;
  else                     hat = 180;
}
else if (right && !left) {
  hat = 90;
}
else if (left && !right) {
  hat = 270;
}

// Change Detection
bool changed = false;

// Check buttons
for (int i = 0; i < 14; i++) {
  if (currentButtons[i] != lastButtons[i]) {
    changed = true;
    lastButtons[i] = currentButtons[i];
    Joystick.setButton(i, currentButtons[i]);
  }
}

// Check hat
if (hat != lastHat) {
  changed = true;
  lastHat = hat;
  Joystick.setHatSwitch(0, hat);
}

// X-axis
int16_t targetStick = spinnerStickMode ? stickX : 0;

if (targetStick != lastStickX) {
    Joystick.setXAxis(targetStick);
    lastStickX = targetStick;
    changed = true;
}

// Y-axis

int16_t targetY = 0;  // Y centered

if (targetY != lastStickY) {
    Joystick.setYAxis(targetY);
    lastStickY = targetY;
    changed = true;
}

// Spinner Mouse Button Mode
bool mouseLeft   = false;
bool mouseRight  = false;
bool mouseMiddle = false;

if (mouseClickMode) {
    mouseLeft   = b[3];
    mouseRight  = b[4];
    mouseMiddle = b[5];
} else {
    mouseLeft = false;
    mouseRight = false;
    mouseMiddle = false;
}

// Left Click
if (mouseLeft != lastMouseLeft) {
    if (mouseLeft)
        Mouse.press(MOUSE_LEFT);
    else
        Mouse.release(MOUSE_LEFT);
    lastMouseLeft = mouseLeft;
}

// Right Click
if (mouseRight != lastMouseRight) {
    if (mouseRight)
        Mouse.press(MOUSE_RIGHT);
    else
        Mouse.release(MOUSE_RIGHT);
    lastMouseRight = mouseRight;
}

// Middle Click
if (mouseMiddle != lastMouseMiddle) {
    if (mouseMiddle)
        Mouse.press(MOUSE_MIDDLE);
    else
        Mouse.release(MOUSE_MIDDLE);
    lastMouseMiddle = mouseMiddle;
}

if (changed) {
  Joystick.sendState();
}
}
