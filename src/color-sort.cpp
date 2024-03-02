/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Adafruit_TCS34725.h"
#include "Stepper.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// The "LoadingWheel" is a continuous-rotation servo.
Servo LoadingWheel;

// The Chute is a conventional servo (position-control)
Servo Chute;

// PIN definitions
const pin_t CHUTE_PIN = D2;
const pin_t COLOR_LED = D3;

const int RED_CUP = 1;
const int GREEN_CUP = 2;
const int BLUE_CUP = 3;
// const int RED_CUP = 1;
// const int RED_CUP = 1;

// Gain options:
// TCS34725_GAIN_1X
// TCS34725_GAIN_4X
// TCS34725_GAIN_16X
// TCS34725_GAIN_60X

// Exposure Time options
// TCS34725_INTEGRATIONTIME_24MS
// TCS34725_INTEGRATIONTIME_50MS
// TCS34725_INTEGRATIONTIME_101MS
// TCS34725_INTEGRATIONTIME_154MS
// TCS34725_INTEGRATIONTIME_700MS

Adafruit_TCS34725 colorSensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_1X);

Stepper stepper = Stepper(200, D4, D5, D6, D7);

// Keep track of the cup that the chute is currently pointed at
int currentChuteCup = 0;
int servoDegrees = 0;

bool isActive = TRUE;

int servoControl(String command)
{

    if (command == "a") {
        Chute.attach(CHUTE_PIN);
        return 1;
    }
    if (command == "d") {
        Chute.detach();
        return 1;
    }

    // Convert
   int newPos = command.toInt();
   servoDegrees = newPos;

   // Make sure it is in the right range
   servoDegrees = constrain( newPos, 15, 180);

    // auto select appropriate value
    if(newPos >= 500)
    {
      Log.info("Writing Microseconds: " + command);
      Chute.writeMicroseconds(newPos);
    }
    else
    {   
      Log.info("Writing degrees: " + command);
      Chute.write(newPos); 
    }

   // done
   return 1;
}

int toggleActive(String command) {
    if (command == "1" || command == "true") {
        isActive = TRUE;
    } else {
        isActive = FALSE;
    }

    return 1;
}

// Convert to a 0 - 255 scale
float calibratedRange(float inputVal, float low, float high) {
    float range = high - low;
    return ((inputVal - low) / range * 256.0);
}

float calibratedR(float r) {
    return calibratedRange(r, 50, 180); 
}
float calibratedG(float g) {
    return calibratedRange(g, 40, 150);
}
float calibratedB(float b) {
    return calibratedRange(b, 40, 150);
}

bool isRed(float r, float g, float b) {
    if ((r > g) && (r > b) && (r > 100)) {
        return true;
    }
    return false;
}
bool isGreen(float r, float g, float b) {
    if ((g > r) && (g > b) && (g > 100)) {
        return true;
    }
    return false;
}
bool isBlue(float r, float g, float b) {
    if ((b > r) && (b > g) && (b > 100)) {
        return true;
    }
    return false;
}

String readColor() {
    uint16_t clear, red, green, blue;

    // Turn on the LED
    digitalWrite(COLOR_LED, HIGH);

    delay(160);  // Allow time to take a reading
    colorSensor.getRawData(&red, &green, &blue, &clear);

    // Turn off the LED
    digitalWrite(COLOR_LED, LOW);

        // Figure out some basic hex code for visualization
    uint32_t sum = clear;
    float r, g, b;     // Raw values
    float rr, gg, bb;  // Calibrated values
    
    r = red; r /= sum;
    g = green; g /= sum;
    b = blue; b /= sum;
    r *= 256; g *= 256; b *= 256;
    rr = calibratedR(r);
    gg = calibratedG(g);
    bb = calibratedB(b);

    Log.info("R:" + String(r) + " G:" + String(g) + " B:" + String(b));
    Log.info("calR:" + String(rr) + " calG:" + String(gg) + " calB:" + String(bb));
    Log.info("Color Temp: " + String(colorSensor.calculateColorTemperature(r,g,b)));
    Log.info("Lux: " + String(colorSensor.calculateLux(r,g,b)));

    if (isRed(rr,gg,bb)) {
        return "red";
    }
    if (isGreen(rr,gg,bb)) {
        return "green";
    }
    if (isBlue(rr,gg,bb)) {
        return "blue";
    }
    return "unknown";
}


void advanceLoadingWheel() {
    stepper.step(66);
}

void moveChute(int cup) {
    Log.info("Moving chute from cup " + String(currentChuteCup) + " to cup " + cup);

    if (cup == 1) {
        Log.info("Moving chute to cup 1");
        // Chute.writeMicroseconds(1000);
        Chute.write(15);

    } else if (cup == 2) {
        Log.info("Moving chute to cup 2");
        // Chute.writeMicroseconds(1250);
        Chute.write(50);

    } else if (cup == 3) {
        Log.info("Moving chute to cup 3");
        // Chute.writeMicroseconds(1500);
        Chute.write(95);

    } else if (cup == 4) {
        Log.info("Moving chute to cup 4");
        // Chute.writeMicroseconds(1750);
        // Chute.write(135);
        Chute.write(135);

    } else if (cup == 5) {
        Log.info("Moving chute to cup 5");
        // Chute.writeMicroseconds(2000);
        Chute.write(180);
    }

    if (currentChuteCup == 0) {
    //     // We don't know where the chute was pointed, so just wait the max amount of time for it to swing 
        delay(1500);
    } else {
        delay(50 + abs(cup - currentChuteCup) * 250);
    }

    // delay(500);
    currentChuteCup = cup;

    // digitalWrite(CHUTE_PIN, LOW);

    return;
}

// setup() runs once, when the device is first turned on
void setup() {
    // Tell the program that the "Chute" servo is connected to the PIN named "CHUTE_PIN"
    // Chute.attach(CHUTE_PIN, 600, 2500, 0, 180); // Used with the Photon
    Chute.attach(CHUTE_PIN); // Used with the Argon

    // Register a cloud function that can be invoked remotely
    Particle.function("servo", servoControl);
    Particle.function("toggleActive", toggleActive);

    // Keep a cloud variable for the current position
    Particle.variable("servoDegrees" , &servoDegrees , INT );
    Particle.variable("isActive" , &isActive, BOOLEAN );

    colorSensor.begin();
    pinMode(COLOR_LED, OUTPUT);

    // Stepper motor pins
    pinMode(D4, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);

    digitalWrite(COLOR_LED, LOW);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
    if (isActive) {
        String currentColor = readColor();

        if (currentColor == "red") {
            moveChute(RED_CUP);
        } else if (currentColor == "green") {
            moveChute(GREEN_CUP);
        } else if (currentColor == "blue") {
            moveChute(BLUE_CUP);
        }


        // Optional delay while the chute finishes moving (may not be necessary)...
        // delay(250);

        // Advance to the next Skittle
        // advanceLoadingWheel();

        delay(1500);
        advanceLoadingWheel();
    }

    // moveChute(1);
    // moveChute(2);
    // moveChute(3);
    // moveChute(4);
    // moveChute(5);
  
  // Example: Publish event to cloud
  // Particle.publish("Hello world!");
}

