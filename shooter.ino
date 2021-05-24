#include <Arduboy2.h>

Arduboy2 arduboy;

// Constants
constexpr int16_t screenCenterX = 64;
constexpr int16_t screenCenterY = 32;
constexpr int8_t radius = 6;
constexpr int16_t gunSize = 3;

// Variables
float gunAngle = 4.8;

void drawGun(float angle) {
    arduboy.drawCircle(screenCenterX, screenCenterY, radius, WHITE);

    float x0 = screenCenterX + (radius * cos(angle));
    float y0 = screenCenterY + (radius * sin(angle));

    float x1 = screenCenterX + ((radius + gunSize) * cos(angle));
    float y1 = screenCenterY + ((radius + gunSize) * sin(angle));

    arduboy.drawLine(x0, y0, x1, y1, WHITE);
}

void rotateGun() {
    if (arduboy.pressed(RIGHT_BUTTON)) {
        if (gunAngle <= 6.28) {
            gunAngle = gunAngle + 0.05;
        }
        else {
            gunAngle = 0;
        }
    }

    if (arduboy.pressed(LEFT_BUTTON)) {
        if (gunAngle >= 0.01) {
            gunAngle = gunAngle - 0.05;
        }
        else {
            gunAngle = 6.28;
        }
    }
}

void printGunInfo() {
    arduboy.setCursor(1, 1);
    arduboy.print(gunAngle);
}

void setup() {
    arduboy.begin();
    arduboy.initRandomSeed();
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