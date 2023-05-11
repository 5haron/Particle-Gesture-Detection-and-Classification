#include <SparkFunMMA8452Q.h>
#include <math.h>
#include <Grove_ChainableLED.h>
#include "Particle.h"
#define NUM_LEDS  5

MMA8452Q accel;
ChainableLED leds(D2, D3, NUM_LEDS);

String selectedGesture = "all";

const float alpha = 0.2;
float emaX = 0;
float emaY = 0;
float emaZ = 0;

// Gesture detection states: only one gesture will be detected at a time
enum GestureState {NONE, LINE, TRIANGLE, SQUARE, CIRCLE, L, N, Z};
GestureState currentGesture = NONE;

// Gesture buffer parameters: have to wait two seconds after each gesture to start new gesture
unsigned long lastGestureDetectionTime = 0;
const unsigned long gestureBufferTime = 2000; // 2 seconds buffer time

// Line detection parameters
const unsigned long lineInterval = 1000; 
const unsigned long bufferTime = 2000; // 2 seconds buffer time
unsigned long lastLineDetectionTime = 0;
unsigned long lineStartTime = 0;
enum LineState {LNONE, LRIGHT_LEFT};
LineState lineState = LNONE;

// Triangle detection parameters
const unsigned long triangleInterval = 3000; // 3 seconds
enum TriangleState {TNONE, TLEFT_RIGHT, TRIGHT_TOP, TTOP_LEFT};
TriangleState triangleState = TNONE;
unsigned long triangleStartTime = 0;

// Square detection parameters
const unsigned long squareInterval = 4000; // 4 seconds
enum SquareState {SNONE, SBOTTOM_TOP, SLEFT_RIGHT, STOP_BOTTOM, SRIGHT_LEFT};
SquareState squareState = SNONE;
unsigned long squareStartTime = 0;

// Circle detection parameters
const int NCircle = 30; // Number of data points
float magnitudes[NCircle] = {0};
float threshold = 0.1; 

bool circleDetected = false;
bool circleDetectionPrinted = false;

// L detection parameters
const unsigned long lInterval = 2000; 
enum LState {LLNONE, LLTOP_BOTTOM, LLLEFT_RIGHT};
LState lState = LLNONE;
unsigned long lStartTime = 0;

// N detection parameters
const unsigned long nInterval = 3000; // 3 seconds
enum NState {NNONE, NBOTTOM_TOP, NTOP_RIGHT, NSBOTTOM_TOP};
NState nState = NNONE;
unsigned long nStartTime = 0;

// Z detection parameters
const unsigned long zInterval = 3000; // 3 seconds
enum ZState {ZNONE, ZLEFT_RIGHT, ZTOP_LEFT, ZSLEFT_RIGHT};
ZState zState = ZNONE;
unsigned long zStartTime = 0;

// EMA Filter applied to accelerometer data
void updateEMA(float x, float y, float z) {
  emaX = (alpha * x) + ((1 - alpha) * emaX);
  emaY = (alpha * y) + ((1 - alpha) * emaY);
  emaZ = (alpha * z) + ((1 - alpha) * emaZ);
}

// Calculates the magnitude from three axes
float magnitude(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

// Function to set LED color based on gesture
void setLEDColorForGesture(const char* gesture) {
  if (strcmp(gesture, "Line") == 0) {
    leds.setColorRGB(0, 255, 0, 0); // Red
  } else if (strcmp(gesture, "Circle") == 0) {
    leds.setColorRGB(0, 0, 255, 0); // Green
  } else if (strcmp(gesture, "Square") == 0) {
    leds.setColorRGB(0, 0, 0, 255); // Blue
  } else if (strcmp(gesture, "Triangle") == 0) {
    leds.setColorRGB(0, 255, 0, 255); // Magenta
  } else if (strcmp(gesture, "Rest") == 0) {
    leds.setColorRGB(0, 0, 0, 0); // Off
  }
}

// ParSub Function: Depending on the string you enter on the Particle Events console, it will only detect that specific gesture
int setGesture(String command) {
  selectedGesture = command;
  return 1;
}

// Line detection function
void detectLineMotion() {
     // Check if the current gesture is not SQUARE before detecting a line motion
  if (currentGesture == SQUARE) {
    return;
  }
  
  if (currentGesture != NONE && currentGesture != LINE) {
    return;
  }
  
  unsigned long currentTime = millis();
  if ((currentTime - lineStartTime) > lineInterval) {
    lineState = LNONE;
    
    if(currentGesture == LINE){
        currentGesture = NONE;
        Serial.println("ready for next gesture:");
    }
  }
  
  if (currentTime - lastGestureDetectionTime < gestureBufferTime) {
      return;
  }

  // Check if the buffer time has passed since the last line detection
  if (currentTime - lastLineDetectionTime > bufferTime) {
    if (lineState == LNONE && emaX < -0.38) {
      lineState = LRIGHT_LEFT;
      lineStartTime = currentTime;
      Serial.println("LINE detected!!");
      currentGesture = LINE;
      setLEDColorForGesture("Line");
      delay(1000);

      if (selectedGesture == "all" || selectedGesture == "Line") {
        Particle.publish("GestureDetected", "Line", PRIVATE);
      }
    }
  }

  if (lineState == LRIGHT_LEFT) {
    lineState = LNONE;
    currentGesture = NONE;
    setLEDColorForGesture("Rest");
    lastGestureDetectionTime = currentTime;
    Serial.println("ready for next gesture:");
  }
}

// Triangle detection function
void detectTriangleMotions() {
  if (currentGesture != NONE && currentGesture != TRIANGLE) {
    return;
  }
  
  unsigned long currentTime = millis();
  if ((currentTime - triangleStartTime) > triangleInterval) {
    triangleState = TNONE;
    
    if(currentGesture == TRIANGLE){
        currentGesture = NONE;
        Serial.println("ready for next gesture:");
    }
  }
  
  if (currentTime - lastGestureDetectionTime < gestureBufferTime) {
     return;
  }

  if (triangleState == TNONE && emaX > 0.26 && emaZ < 1) {
    triangleState = TLEFT_RIGHT;
    triangleStartTime = currentTime;
    Serial.println("TRI: FIRST LINE!!");
    currentGesture = TRIANGLE;
  } else if (triangleState == TLEFT_RIGHT && emaX < -0.2 && emaZ > 1.28) {
    triangleState = TRIGHT_TOP;
    Serial.println("TRI: SECOND LINE!");
  } else if (triangleState == TRIGHT_TOP && emaX < -0.2 && emaZ < 0.65) {
    triangleState = TTOP_LEFT;
    Serial.println("TRI: THIRDD!");
    setLEDColorForGesture("Triangle");

    if (selectedGesture == "all" || selectedGesture == "Triangle") {
        Particle.publish("GestureDetected", "Triangle", PRIVATE);
    }
    
    delay(1000);
  }

  if (triangleState == TTOP_LEFT) {
    Serial.println("Triangle detected!");
    triangleState = TNONE;
    currentGesture = NONE;
    setLEDColorForGesture("Rest");
    Serial.println("ready for next gesture:");
    
     lastGestureDetectionTime = currentTime;
  }
}

// Square detection function
void detectSquareMotions() {
  if (currentGesture != NONE && currentGesture != SQUARE) {
    return;
  }
  
  unsigned long currentTime = millis();
  if ((currentTime - squareStartTime) > squareInterval) {
    squareState = SNONE;
    
    if (currentGesture == SQUARE){
        currentGesture = NONE;
        Serial.println("ready for next gesture:");
    }
  }
  
  if (currentTime - lastGestureDetectionTime < gestureBufferTime) {
     return;
  }

  if (squareState == SNONE && emaZ > 1.2 && emaY < -0.36) {
    squareState = SBOTTOM_TOP;
    squareStartTime = currentTime;
    Serial.println("SQ: FIRST LINE!!");
    currentGesture = SQUARE;
  } else if (currentGesture == SQUARE && squareState == SBOTTOM_TOP && emaX > 0.26) {
    squareState = SLEFT_RIGHT;
    Serial.println("SQ: SECOND LINE!");
  } else if (currentGesture == SQUARE && squareState == SLEFT_RIGHT && emaZ < 0.39 && emaY < 0.14) {
    squareState = STOP_BOTTOM;
    Serial.println("SQ: THIRDD!");
  } else if (currentGesture == SQUARE && squareState == STOP_BOTTOM && emaX < -0.38) {
    squareState = SRIGHT_LEFT;
    Serial.println("SQ: LAST!");
    setLEDColorForGesture("Square");

    if (selectedGesture == "all" || selectedGesture == "Square") {
        Particle.publish("GestureDetected", "Square", PRIVATE);
    }
    
    delay(1000);
  }

  if (squareState == SRIGHT_LEFT) {
    Serial.println("Square detected!");
    squareState = SNONE;
    currentGesture = NONE;
    setLEDColorForGesture("Rest");
    Serial.println("ready for next gesture:");
    lastGestureDetectionTime = currentTime;
  }
}

// Circle detection function
void detectCircleMotions() {
    // Collect magnitudes
    if (accel.available()) {
        accel.read();
        updateEMA(accel.cx, accel.cy, accel.cz);

        for (int i = 0; i < NCircle - 1; i++) {
            magnitudes[i] = magnitudes[i + 1];
        }
        magnitudes[NCircle - 1] = magnitude(emaX, emaY, emaZ);
    }

    // Calculate the change in magnitude
    float deltaMagnitudes[NCircle - 1];
    for (int i = 0; i < NCircle - 1; i++) {
        deltaMagnitudes[i] = magnitudes[i + 1] - magnitudes[i];
    }

    // Identify peaks and valleys
    int peaks = 0;
    int valleys = 0;
    for (int i = 1; i < NCircle - 2; i++) {
        if (deltaMagnitudes[i] > threshold && deltaMagnitudes[i] > deltaMagnitudes[i - 1] && deltaMagnitudes[i] > deltaMagnitudes[i + 1]) {
            peaks++;
        } else if (deltaMagnitudes[i] < -threshold && deltaMagnitudes[i] < deltaMagnitudes[i - 1] && deltaMagnitudes[i] < deltaMagnitudes[i + 1]) {
            valleys++;
        }
    }
    
    if (currentGesture == LINE || currentGesture == TRIANGLE || currentGesture == SQUARE || currentGesture == L || currentGesture == N || currentGesture == Z){
        peaks = 0;
        valleys = 0;
    }
    
    circleDetected = (peaks >= 2 && valleys >= 2);
    
    if (circleDetected && !circleDetectionPrinted && currentGesture == NONE) {
        peaks = 0;
        valleys = 0;
        Serial.println("Circular detected");
        setLEDColorForGesture("Circle");
        circleDetectionPrinted = true;
        Serial.println("ready for next gesture:");
        
        if (selectedGesture == "all" || selectedGesture == "Circle") {
            Particle.publish("GestureDetected", "Circle", PRIVATE);
        }
    
        delay(1000);
        setLEDColorForGesture("Rest");
    } else if (!circleDetected) {
        circleDetectionPrinted = false;
    }
}

// Letter L detection function
void detectLMotions() {
  if (currentGesture != NONE && currentGesture != L) {
    return;
  }
  unsigned long currentTime = millis();
  if ((currentTime - lStartTime) > lInterval) {
    lState = LLNONE;
    
    if(currentGesture == L){
        currentGesture = NONE;
        Serial.println("ready for next gesture:");
    }
  }
  
  if (currentTime - lastGestureDetectionTime < gestureBufferTime) {
     return;
  }

  if (lState == LLNONE && emaZ < 0.39 && emaY < 0.14) {
    lState = LLTOP_BOTTOM;
    lStartTime = currentTime;
    currentGesture = L;
    Serial.println("L: FIRST LINE!!");
  } else if (lState == LLTOP_BOTTOM && emaX > 0.26) {
    lState = LLLEFT_RIGHT;
    Serial.println("L: SECOND LINE!");
  }

  if (lState == LLLEFT_RIGHT) {
    Serial.println("L detected!");
    lState = LLNONE;
    currentGesture = NONE;
    if (selectedGesture == "all" || selectedGesture == "Letter: L") {
            Particle.publish("GestureDetected", "Circle", PRIVATE);
        }
    Serial.println("ready for next gesture:");
    lastGestureDetectionTime = currentTime;
  }
}

// Letter N detection function
void detectNMotions() {
  if (currentGesture != NONE && currentGesture != N && currentGesture != SQUARE) {
    return;
  }
  
  unsigned long currentTime = millis();
  if ((currentTime - nStartTime) > nInterval) {
    nState = NNONE;
    
    if(currentGesture == N){
        currentGesture = NONE;
        Serial.println("ready for next gesture:");
    }
  }
  
  if (currentTime - lastGestureDetectionTime < gestureBufferTime) {
     return;
  }

  if (nState == NNONE && emaZ > 1.2 && emaY < -0.36) {
    nState = NBOTTOM_TOP;
    nStartTime = currentTime;
    Serial.println("N: FIRST LINE!!");
  } else if (nState == NBOTTOM_TOP && squareState != SLEFT_RIGHT &&emaZ < 0.6 && emaX > 0.3 && emaY < -0.25) {
    currentGesture = N;
    nState = NTOP_RIGHT;
    Serial.println("N: SECOND LINE!");
  } else if (currentGesture == N && nState == NTOP_RIGHT && emaZ > 1.25 && emaY < -0.36){
    nState = NSBOTTOM_TOP;
    Serial.println("N: THIRDD!");
  }

  if (nState == NSBOTTOM_TOP) {
    Serial.println("N detected!");
    nState = NNONE;
    currentGesture = NONE;
    if (selectedGesture == "all" || selectedGesture == "Letter: N") {
            Particle.publish("GestureDetected", "Circle", PRIVATE);
        }
    Serial.println("ready for next gesture:");
    lastGestureDetectionTime = currentTime;
  }
}

// Letter Z detection function
void detectZMotions() {
  if (currentGesture != NONE && currentGesture != Z && currentGesture != TRIANGLE) {
    return;
  }
  
  unsigned long currentTime = millis();
  if ((currentTime - zStartTime) > zInterval) {
    zState = ZNONE;
    
    if(currentGesture == Z){
        currentGesture = NONE;
        Serial.println("ready for next gesture:");
    }
  }
  
  if (currentTime - lastGestureDetectionTime < gestureBufferTime) {
     return;
  }

  if (zState == ZNONE && emaX > 0.26 && emaZ < 1) {
    zState = ZLEFT_RIGHT;
    zStartTime = currentTime;
    Serial.println("Z: FIRST LINE!!");
  } else if (zState == ZLEFT_RIGHT && emaX < -0.2 && emaZ < 0.65) {
    zState = ZTOP_LEFT;
    currentGesture = Z;
    Serial.println("Z: SECOND LINE!");
  } else if (zState == ZTOP_LEFT && emaX > 0.26) {
    zState = ZSLEFT_RIGHT;
    Serial.println("Z: THIRDD!");
  }

  if (zState == ZSLEFT_RIGHT) {
    Serial.println("Z detected!");
    zState = ZNONE;
    currentGesture = NONE;
    if (selectedGesture == "all" || selectedGesture == "Letter: Z") {
            Particle.publish("GestureDetected", "Circle", PRIVATE);
        }
    Serial.println("ready for next gesture:");
    lastGestureDetectionTime = currentTime;
  }
}

void setup() {
  Serial.begin(9600);
  leds.init();
  
  Particle.connect();
  Particle.function("setGesture", setGesture);


  if (!accel.begin(SCALE_2G, ODR_12)) {
    Serial.println("Failed to find IMU");
  }

  while (!accel.available()) {}
  accel.read();
  emaX = accel.cx;
  emaY = accel.cy;
  emaZ = accel.cz;
  lineStartTime = millis();
  triangleStartTime = millis();
  squareStartTime = millis();
  lStartTime = millis();
  nStartTime = millis();
  zStartTime = millis();
}

void loop() {
    detectCircleMotions();
  if (accel.available()) {
    accel.read();
    updateEMA(accel.cx, accel.cy, accel.cz);
    detectLineMotion();
    detectTriangleMotions();
    detectSquareMotions();
    detectLMotions();
    detectNMotions();
    detectZMotions();
    
    // Serial.println("X: " + String(emaX) + ", Y: " + String(emaY) + ", Z: " + String(emaZ));
    // Serial.println(currentGesture);
  }
}

