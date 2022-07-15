#include <Arduboy2.h>
#include "bitmaps.h"
#include <Tinyfont.h>

Arduboy2 arduboy;
BeepPin1 beep;
Sprites sprites;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

// Constants
constexpr int16_t screenCenterX = 64; // Center of the screen for gun placement
constexpr int16_t screenCenterY = 48; // Center of the screen for gun placement
constexpr uint8_t radius = 5; // Radius of the turret's body
constexpr uint8_t gunSize = 3; // Size (length) of the gun
constexpr uint8_t bullets = 3; // The number of bullets that can be on the screen at the same time
constexpr uint8_t enemies = 3; // Max active enemies
constexpr uint8_t explosions = enemies; // There should be only as many explosions as there are enemies
constexpr uint8_t menuOptions = 3; // The number of available menu options
constexpr uint8_t minIndex = 0; // The index of the first menu item
constexpr uint8_t maxIndex = (menuOptions - 1); // The index of the last menu item

// Game states
enum class GameState : uint8_t {
    Menu,
    Game,
    Paused,
    Credits,
    GameOver
};

// Variables
float gunAngle { 4.8 }; // Full circle is 6.28 radian, let's put the gun at 4.8 so it's facing up
uint8_t frame { 0 }; // Used for counting frames for the sprite animations
uint8_t score { 0 }; // For keeping score
uint8_t selectedIndex = 0; // The index of the currently selected menu item

// Bullets
struct Bullet {
    int16_t x, y, lifetime { 0 }; // screen position, and lifetime
    float initialAngle { 0.0 }; // which direction the bullet will go
    bool isOnScreen { false }; // does the bullet exist
};

// Enemies
struct Enemy {
    int16_t x, y { 0 };
    float dx, dy { 0.0 };
    uint8_t width { 8 };
    uint8_t height { 8 };
    bool isOnScreen { false };
};

// Explosions
struct Explosion {
    int16_t x, y { 0 }; // screen position
    bool isOnScreen { false }; // does the bullet exist
};

Bullet bullet[bullets];
Enemy enemy[enemies];
Explosion explosion[explosions];
GameState gameState = GameState::Menu;

// Change game states
void changeGameState (GameState newGameState) {
    gameState = newGameState;
}

// Drawing the turret
void drawGun(float angle) {
    // The body of the turret is a circle
    arduboy.fillCircle(screenCenterX, screenCenterY, radius, WHITE);

    // Drawing the gun
    // The gun is a line with its starting point fixed on the circle. To achive this there is
    // a need for some math
    float x0 = screenCenterX + (radius * cos(angle));
    float y0 = screenCenterY + (radius * sin(angle));

    // The gun's endpoint is calculated in the same manner as the starting point, but we'll
    // add the length of the gun (gunSize) to the equation
    float x1 = screenCenterX + ((radius + gunSize) * cos(angle));
    float y1 = screenCenterY + ((radius + gunSize) * sin(angle));

    // After the starting and ending point is calculated, let's draw the gun
    arduboy.drawLine(x0, y0, x1, y1, WHITE);
}

// Rotating the turret
void rotateGun() {
    // Rotating the turret clockwise, if we reach the full radian then start from 0 so the
    // turret can be rotated in an endless circle
    if (arduboy.pressed(RIGHT_BUTTON)) {
        if (gunAngle <= 6.28) {
            gunAngle = gunAngle + 0.05;
        }
        else {
            gunAngle = 0;
        }
    }

    // Rotating the turret counterclockwise, if we reach 0 radian then start over from 6.28
    // so the turret can be rotated in an endless circle in this direction as well
    if (arduboy.pressed(LEFT_BUTTON)) {
        if (gunAngle >= 0.01) {
            gunAngle = gunAngle - 0.05;
        }
        else {
            gunAngle = 6.28;
        }
    }
}

// Printing some info __FIXME__ can be removed later
void printInfo(uint16_t text) {
    arduboy.setCursor(1, 1);
    arduboy.print(text);
}

// Fire bullets by pressing the B_BUTTON
void fireBullets() {
    if (arduboy.justPressed(B_BUTTON)) {
        // Check if we have unused bullets
        uint8_t bulletNum = findUnusedBullet();
        // If yes, set its starting coordinates
        if (bulletNum != bullets) {
            bullet[bulletNum].x = screenCenterX + ((1 + radius + gunSize) * cos(gunAngle));
            bullet[bulletNum].y = screenCenterY + ((1 + radius + gunSize) * sin(gunAngle));
            beep.tone(beep.freq(240), 10); // Make a sound when bullet is fired
        }
    }
}

// Find the next unused bullet slot in the array and return the index
uint8_t findUnusedBullet() {
    uint8_t bulletNum;
    for (bulletNum = 0; bulletNum < bullets; bulletNum++) {
        // If .isOnScreen property is not set then set it to true and break the for loop
        // since we found the next usable slot in the array
        if (!(bullet[bulletNum].isOnScreen)) {
            bullet[bulletNum].isOnScreen = true;
            break;
        }
    }
    return bulletNum;
}

// Find the next unused explosion slot in the array and return the index
uint8_t findUnusedExplosion() {
    uint8_t explosionNum;
    for (explosionNum = 0; explosionNum < explosions; explosionNum++) {
        // If .isOnScreen property is not set then set it to true and break the for loop
        // since we found the next usable slot in the array
        if (!(explosion[explosionNum].isOnScreen)) {
            explosion[explosionNum].isOnScreen = true;
            break;
        }
    }
    return explosionNum;
}

// Moving the bullets
void moveBullets() {
    for (uint8_t bulletNum = 0; bulletNum < bullets; bulletNum++) {
        if (bullet[bulletNum].isOnScreen) {
            // If the bullet is on the screen then start its lifetime counter so we can 
            // move it outward from the position it was originally fired
            bullet[bulletNum].lifetime++; 

            // If the bullet just came into existence get the turrets angle as this is 
            // the angle the bullet should fly
            if (bullet[bulletNum].lifetime == 1) {
                bullet[bulletNum].initialAngle = gunAngle;
            }

            // Calculating the bullet's x and y position according the above
            bullet[bulletNum].x = screenCenterX + ((bullet[bulletNum].lifetime + radius + gunSize) * cos(bullet[bulletNum].initialAngle));
            bullet[bulletNum].y = screenCenterY + ((bullet[bulletNum].lifetime + radius + gunSize) * sin(bullet[bulletNum].initialAngle));
            
            // Check if the bullet is out of the screen. If yes, reset its .isOnScreen and .lifetime properties
            if (bullet[bulletNum].x < 0 || bullet[bulletNum].x > 128 || bullet[bulletNum].y < 8 || bullet[bulletNum].y > 64)  {
                bullet[bulletNum].isOnScreen = false;
                bullet[bulletNum].lifetime = 0;
            }
        }
    }
}

// Drawing the bullets
void drawBullets() {
    for (uint8_t bulletNum = 0; bulletNum < bullets; bulletNum++) {
        if (bullet[bulletNum].isOnScreen) {
            arduboy.drawPixel(bullet[bulletNum].x, bullet[bulletNum].y, WHITE);
        }
    }
}

// Reset game when game is over
void resetGame() {
    // Remove bullets
    for (uint8_t bulletNum = 0; bulletNum < bullets; bulletNum++) {
        if (bullet[bulletNum].isOnScreen) {
            bullet[bulletNum].isOnScreen = false;
            bullet[bulletNum].lifetime = 0;
        }
    }
   
   // Remove enemies
    for (uint8_t enemyNum = 0; enemyNum < enemies; enemyNum++) {
        if (enemy[enemyNum].isOnScreen) {
            enemy[enemyNum].isOnScreen = false;
        }
    }
   
    // Remove explosions
    for (uint8_t explosionNum = 0; explosionNum < explosions; explosionNum++) {
        if (explosion[explosionNum].isOnScreen) {
            explosion[explosionNum].isOnScreen = false;
        }
    }

    // Reset score
    score = 0;

    // Reset gunAngle
    gunAngle = 4.8;
}

// Collision detection
void bulletHit_or_turretHit() {
    // Create hitbox for the turret
    Rect turretBox { screenCenterX-radius, screenCenterY-radius, radius * 2, radius * 2 };
    // Iterate through all enemies
    for (uint8_t enemyNum = 0; enemyNum < enemies; enemyNum++) {
        // If any of them is on the screen...
        if (enemy[enemyNum].isOnScreen) {
            // ...then create a hitBox for them...
            Rect hitBox { enemy[enemyNum].x, enemy[enemyNum].y, enemy[enemyNum].width, enemy[enemyNum].height };
            // Check if the hitbox collides with the turret
            if (arduboy.collide(hitBox, turretBox)) {
                // And change GameState to GameOver if it does
                changeGameState(GameState::GameOver);
            }
            // ...and iterate through all the bullets to see if we have a collision...
            for (uint8_t bulletNum = 0; bulletNum < bullets; bulletNum++) {
                if (bullet[bulletNum].isOnScreen) {
                    // Create a hitbox for each of the bullets
                    Rect bulletBox { bullet[bulletNum].x, bullet[bulletNum].y, 1, 1 };
                    // Check if there is a collision
                    if (arduboy.collide(hitBox, bulletBox)) {
                        // If there is, remove the enemy
                        enemy[enemyNum].isOnScreen = false;
                        // Make a sound when enemy is hit
                        beep.tone(beep.freq(440), 10);
                        // Create the explosion
                        summonExplosion(enemy[enemyNum].x, enemy[enemyNum].y);
                        // Remove the bullet and reset its lifetime
                        bullet[bulletNum].isOnScreen = false;
                        bullet[bulletNum].lifetime = 0;
                        // Increase the score
                        ++score;
                    }
                }
            }
        }
    }
}

// Drawing the enemies
void drawEnemy() {
    for (uint8_t enemyNum = 0; enemyNum < enemies; enemyNum++) {
        if (enemy[enemyNum].isOnScreen) {
            sprites.drawOverwrite(enemy[enemyNum].x, enemy[enemyNum].y, enemy_01, frame);
        }
    }
}

// Moving the enemies randomly
void moveEnemy() {
    for (uint8_t enemyNum = 0; enemyNum < enemies; enemyNum++) {
        if (enemy[enemyNum].isOnScreen) {
            // Move the enemy every 4 frames
            if (arduboy.everyXFrames(4)) {
                // Direction at random at every 40 frames
                if (arduboy.everyXFrames(50)) {
                    enemy[enemyNum].dx = random(-2.0, 2.0);
                    enemy[enemyNum].dy = random(-2.0, 2.0);
                }
                // Change enemy movement direction if enemy is on the edge of screen (X axis)
                if (enemy[enemyNum].x < 1 || enemy[enemyNum].x > 120) {
                    enemy[enemyNum].dx *= -1;
                    enemy[enemyNum].x = enemy[enemyNum].x + enemy[enemyNum].dx;
                }
                
                // Change enemy movement direction if enemy is on the edge of screen (Y axis)
                if (enemy[enemyNum].y < 10 || enemy[enemyNum].y > 52) {
                    enemy[enemyNum].dy *= -1;
                    enemy[enemyNum].y = enemy[enemyNum].y + enemy[enemyNum].dy;
                }

            // Move the enemies
            enemy[enemyNum].x += enemy[enemyNum].dx;
            enemy[enemyNum].y += enemy[enemyNum].dy;
            }
        }
    }    
}

// If there is no enemy, summon new ones at random intervals
void summonEnemy() {
    for (uint8_t enemyNum = 0; enemyNum < enemies; enemyNum++) {
        if (!(enemy[enemyNum].isOnScreen) && (arduboy.everyXFrames(random(240)))) {
            enemy[enemyNum].x = random(0, 120);
            enemy[enemyNum].y = random(8, 56);
            // Avoid putting enemies right next to or on top of the turret
            if (enemy[enemyNum].x < 56 || enemy[enemyNum].x > 72) {
                if (enemy[enemyNum].y < 24 || enemy[enemyNum].y > 40) {
                    enemy[enemyNum].isOnScreen = true;
                }
            }
        }
    }
}

// Check for on screen explosions and display a new one
void summonExplosion(int16_t x, int16_t y) {
    for (uint8_t explosionNum = 0; explosionNum < explosions; explosionNum++) {
        if (!(explosion[explosionNum].isOnScreen)) {
            explosion[explosionNum].x = x;
            explosion[explosionNum].y = y;
            explosion[explosionNum].isOnScreen = true;
        }
    }
}

// Draw the explosion
void drawExplosion() {
    uint8_t explosion_internal { 0 }; // temp variable for te explosion index to use later for removal
    for (uint8_t explosionNum = 0; explosionNum < explosions; explosionNum++) {
        if (explosion[explosionNum].isOnScreen) {
            explosion_internal = explosionNum;
            // Draw the explosion sprite
            sprites.drawOverwrite(explosion[explosionNum].x, explosion[explosionNum].y, explosion_bitmap, frame);
        }
    }

    // If second frame is reached, remove the explosion sprite
    if (frame == 2) {
        explosion[explosion_internal].isOnScreen = false;
    }
}

void displayScore() {
    tinyfont.setCursor (4, 2);
    tinyfont.print("Score: ");
    tinyfont.setCursor (32, 2);
    tinyfont.print(score);
    // Draw separator for the game area and the score
    arduboy.drawFastHLine(4, 8, 120, WHITE);
}

void gamePlay() {
    // Score
    displayScore();

    // Turret
    drawGun(gunAngle);
    rotateGun();

    // Bullets
    fireBullets();
    moveBullets();
    bulletHit_or_turretHit();
    drawBullets();

    // Enemies
    summonEnemy();
    drawEnemy();
    moveEnemy();
    
    // printInfo(frame);

    // Explosions
    drawExplosion();
}

// Display intro image + menu
void displayMenu() {
    // Intro image
    arduboy.drawSlowXYBitmap(0, 0, intro_image, 128, 64, 1);
    
    // Draw the menu sprite
    if (arduboy.audio.enabled()) {
        sprites.drawOverwrite(92, 40, menuSpriteWithSoundOn, 0);

        // Store potential coordinates in arrays
        static const uint8_t arrowCoordsX[menuOptions] PROGMEM { 99, 88, 93 };
        static const uint8_t arrowCoordsY[menuOptions] PROGMEM { 41, 49, 57 };

        // Read the arrow coordinates from the progmem arrays
        // (This is usually cheaper than using lots of if statements)
        const uint8_t arrowX = pgm_read_byte(&arrowCoordsX[selectedIndex]);
        const uint8_t arrowY = pgm_read_byte(&arrowCoordsY[selectedIndex]);

        // Draw the arrow
        sprites.drawSelfMasked(arrowX, arrowY, arrowSprite, 0);
    }
    else {
        sprites.drawOverwrite(91, 40, menuSpriteWithSoundOff, 0);

        // Store potential coordinates in arrays
        static const uint8_t arrowCoordsX[menuOptions] PROGMEM { 99, 84, 93 };
        static const uint8_t arrowCoordsY[menuOptions] PROGMEM { 41, 49, 57 };

        // Read the arrow coordinates from the progmem arrays
        // (This is usually cheaper than using lots of if statements)
        const uint8_t arrowX = pgm_read_byte(&arrowCoordsX[selectedIndex]);
        const uint8_t arrowY = pgm_read_byte(&arrowCoordsY[selectedIndex]);

        // Draw the arrow
        sprites.drawSelfMasked(arrowX, arrowY, arrowSprite, 0);
    }

    if (arduboy.justPressed(A_BUTTON)){
        switch (selectedIndex)
        {
        case 0:
            changeGameState(GameState::Game);
            break;
        
        case 1:
            arduboy.audio.toggle();
            break;
            
        case 2:
            changeGameState(GameState::Credits);

        default:
            break;
        }
    }
}

void moveInMenu() {
    // Move in the menu
    if (arduboy.justPressed(UP_BUTTON))
    {
        // Decrease the selectedIndex
        // (Making sure it stays in range)
        if(selectedIndex > minIndex)
            --selectedIndex;
    }
    if (arduboy.justPressed(DOWN_BUTTON))
    {
        // Increase the selectedIndex
        // (Making sure it stays in range)
        if(selectedIndex < maxIndex)
            ++selectedIndex;
    }
}

void displayPauseScreen() {
    arduboy.drawBitmap(0, 0, pause, 128, 64, WHITE);
    tinyfont.setCursor (18, 36);
    tinyfont.print("Press A to continue");
    tinyfont.setCursor (36, 42);
    tinyfont.print("or B to quit");
}

void setup() {
    arduboy.begin();
    beep.begin();
    arduboy.initRandomSeed(); // For generating the enemies at random points
}

void loop() {
    // Wait for the next frame
    if (!(arduboy.nextFrame())) {
        return;
    }

    // So arduboy.justPressed can work
    arduboy.pollButtons();

    // Clear the screen
    arduboy.clear();

    // Beep timer
    beep.timer();

    // Check GameState
    switch (gameState)
    {
        case GameState::Menu:
            moveInMenu();
            displayMenu();
            break;

        case GameState::Game:
            arduboy.clear();
            gamePlay();
            if (arduboy.justPressed(A_BUTTON)) {
                changeGameState(GameState::Paused);
            }
            break;

        case GameState::Paused:
            displayPauseScreen();
            if (arduboy.justPressed(A_BUTTON)) {
                changeGameState(GameState::Game);
            }
            if (arduboy.justPressed(B_BUTTON)) {
                changeGameState(GameState::GameOver);
            }
            break;

        case GameState::Credits:
            arduboy.clear();
            arduboy.print("Credits");
            if (arduboy.justPressed(A_BUTTON)) {
                changeGameState(GameState::Menu);
            }
            break;

        case GameState::GameOver:
            arduboy.clear();
            arduboy.print("GameOver");
            resetGame();
            if (arduboy.justPressed(A_BUTTON)) {
                changeGameState(GameState::Menu);
            }
        
        default: GameState::Menu; break;
    }
    
    // Draw everything
    arduboy.display();

    if (arduboy.everyXFrames(10)) frame++; // Frame counting for sprite animation
    if (frame > 2) frame = 0; // Reset every third frame, since we are using sprites with three frames
}