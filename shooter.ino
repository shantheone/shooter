#include <Arduboy2.h>

Arduboy2 arduboy;

// Constants
constexpr int16_t screenCenterX = 64;
constexpr int16_t screenCenterY = 32;
constexpr int8_t distanceFromCenter = 6;
constexpr int16_t gunSize = 3;

void drawGun(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    arduboy.drawLine(x0, y0, x1, y1, WHITE);
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

    arduboy.drawCircle(screenCenterX, screenCenterY, distanceFromCenter, WHITE);
    drawGun(screenCenterX + distanceFromCenter, screenCenterY + distanceFromCenter, screenCenterX + distanceFromCenter + gunSize, screenCenterY + distanceFromCenter + gunSize);

    arduboy.display();
}