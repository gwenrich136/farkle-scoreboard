#include <Arduino.h>
#include "Game.h"

// Instantiate the global Game engine
Game farkleGame;

void setup() {
    // Basic Serial initialization for debugging
    Serial.begin(9600);
    delay(1000); // Give Serial monitor time to open
    Serial.println("BOOT: Starting setup...");
    
    // Delegate all initialization to the Game engine
    farkleGame.setup();
    Serial.println("BOOT: Setup complete. Entering loop.");
}

void loop() {
    // Run the main game state machine loop
    // Serial.println("LOOP: Tick"); // Too verbose, commented out
    farkleGame.loop();
}
