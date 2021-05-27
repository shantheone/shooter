#include <Arduboy2.h>

Arduboy2 arduboy;

// Constants
constexpr int16_t screenCenterX = 64;
constexpr int16_t screenCenterY = 32;
constexpr int8_t radius = 6; // Radius of the turrets body
constexpr int16_t gunSize = 3; // Size (length) of the gun
constexpr int8_t bullets = 3; // The number of bullets that can be on the screen at the same time

// Variables
float gunAngle = 4.8; // Full circle is 6.28 radian, let's put the gun at 4.8 so it's facing up

// Bullets
struct Bullet
{
    int16_t x, y, lifetime; // screen position
    float initialAngle; // which direction the bullet will go
    bool isOnScreen; // is the bullet exists
};

Bullet bullet[bullets];

// Drawing the turret
void drawGun(float angle) {
    // The body of the turret is a circle
    arduboy.drawCircle(screenCenterX, screenCenterY, radius, WHITE);

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

// Printing some info about the current angle of the turret __FIXME__ can be removed later
void printGunInfo() {
    arduboy.setCursor(1, 1);
    arduboy.print(bullet[0].initialAngle);
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
        }
    }
}

// Find the next unused bullet slot in the array and return the index
uint8_t findUnusedBullet() {
    uint8_t bulletNum;
    for (bulletNum = 0; bulletNum < bullets; bulletNum++) {
        // If .isOnScreen property is not set then set it to true and break the for loop
        // since we found the next empty slot in the array
        if (!(bullet[bulletNum].isOnScreen)) {
            bullet[bulletNum].isOnScreen = true;
            break;
        }
    }
    return bulletNum;
}

// Moving the bullets
void moveBullets() {
    for (uint8_t bulletNum = 0; bulletNum < bullets; bulletNum++) {
        if (bullet[bulletNum].isOnScreen) {
            // If the bullet is on the screen then start its lifetime counter so we can 
            // move it outward from the position it was originally fired
            bullet[bulletNum].lifetime++; 

            // If the bullet just came into existence fix the turrets angle as this is 
            // the angle the bullet should fly
            if (bullet[bulletNum].lifetime == 1) {
                bullet[bulletNum].initialAngle = gunAngle;
            }

            // Calculating the bullet's x and y position according the above
            bullet[bulletNum].x = screenCenterX + ((bullet[bulletNum].lifetime + radius + gunSize) * cos(bullet[bulletNum].initialAngle));
            bullet[bulletNum].y = screenCenterY + ((bullet[bulletNum].lifetime + radius + gunSize) * sin(bullet[bulletNum].initialAngle));
            
            // Check if the bullet is out of the screen. If yes, reset its .isOnScreen and 
            // .lifetime properties
            if (bullet[bulletNum].x < 0 || bullet[bulletNum].x > 128 || bullet[bulletNum].y < 0 || bullet[bulletNum].y > 64)  {
                bullet[bulletNum].isOnScreen = false;
                bullet[bulletNum].lifetime = 0;
            }
        }
    }
}

// Drawing the bullets
void drawBullets() {
    // Firing with the B button
    for (uint8_t bulletNum = 0; bulletNum < bullets; bulletNum++)
    {
        if (bullet[bulletNum].isOnScreen) {
                arduboy.drawPixel(bullet[bulletNum].x, bullet[bulletNum].y, WHITE);
            }
    }
}

// Collision detection __FIXME__
void checkBullets() {
    return;
}

void setup() {
    arduboy.begin();
    arduboy.initRandomSeed(); // For generating the enemies at random points

    // Init bullets
    for (uint8_t bulletNum = 0; bulletNum < bullets; ++bulletNum) {
        bullet[bulletNum].x = 0;
        bullet[bulletNum].y = 0;
        bullet[bulletNum].lifetime = 0;
        bullet[bulletNum].initialAngle = 0.0;
        bullet[bulletNum].isOnScreen = false;
    }
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

    // Game functions

    // Turret
    drawGun(gunAngle);
    rotateGun();

    // Bullets
    fireBullets();
    moveBullets();
    checkBullets();
    drawBullets();

    // Draw everything
    arduboy.display();
}