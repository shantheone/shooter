#include <Arduboy2.h>

Arduboy2 arduboy;

// Constants
constexpr int16_t screenCenterX = 64;
constexpr int16_t screenCenterY = 32;
constexpr int8_t radius = 6; // Radius of the turrets body
constexpr int16_t gunSize = 3; // Size (length) of the gun
constexpr int8_t bullets = 6; // The number of bullets that can be on the screen at the same time

// Variables
float gunAngle = 4.8; // Full circle is 6.28 radians, let's put the gun at 4.8 so it's facing up

// Bullets
struct Bullet
{
    int16_t x, y;
    bool isOnScreen;
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

// Printing some info about the current angle of the turret FIXME can be removed later
void printGunInfo() {
    arduboy.setCursor(1, 1);
    arduboy.print(gunAngle);
}

// Drawing the bullets
void drawBullet() {
    // Firing with the B button
    if (arduboy.pressed(B_BUTTON)) {
        /* code */
        for (uint8_t i = 0; i < bullets; i++)
        {
            if (bullet[i].isOnScreen) {
                // draw bullet
            }
        }
    }
}

void setup() {
    arduboy.begin();
    arduboy.initRandomSeed();
    arduboy.pollButtons();

    // Init bullets
    for (uint8_t bulletNum = 0; bulletNum < bullets; ++bulletNum) {
        bullet[bulletNum].x = 1;
        bullet[bulletNum].y = 1;
        bullet[bulletNum].isOnScreen = false;
    }
}

void loop() {
    // Wait for the next frame
    if (!(arduboy.nextFrame())) {
        return;
    }

    // Clear the screen
    arduboy.clear();

    // Game functions
    drawGun(gunAngle);
    rotateGun();
    printGunInfo();

    // Draw everything
    arduboy.display();
}