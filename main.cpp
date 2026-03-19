#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>

// Defines the x and y of rays, distance, and angle offset for fisheye correction
struct ray {
    float x = 0.0;
    float y = 0.0;
    float dx = 1.0;
    float dy = 1.0;
    float dist = 0.0f;
    float angleOffset = 0.0f;
};

struct Map {
    int rows = 10;
    int cols = 10;
    std::vector<std::vector<int>> grid;
    int tileSize = 64;

    Map() {
        grid.push_back({1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 9, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        grid.push_back({1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
    }
};

int main(void) {
    // Sets up the player x, y, speed, angle, rotation speed, and FOV, and rayDistance
    float playerX = 400.0;
    float playerY = 300.0;
    int speed = 5;
    float playerAngle = 0.0f;
    float rotationSpeed = 2.0f;
    float playerFOV = 45;
    int rayDistance = 600;

    // Fixed ray count ensures consistent slices every frame, no float drift
    int rayCount = 90;

    // Sets up width and height of the window
    int WIDTH = 1280;
    int HEIGHT = 720;

    std::vector<ray> rays;

    // Stores the walls in the wall vector
    std::vector<Rectangle> walls;

    Map map1;

    for (int row = 0; row < map1.rows; row++) {
        for(int col = 0; col < map1.cols; col++) {
            if (map1.grid[row][col] == 1) {
                Rectangle rect = { col * map1.tileSize, row * map1.tileSize, map1.tileSize, map1.tileSize };
                walls.push_back(rect);
            }
            if (map1.grid[row][col] == 9) {
                playerX = col * map1.tileSize + map1.tileSize / 2;
                playerY = row * map1.tileSize / 2;
            }
        }
    }

    // Inits the window creation, set as 800x600 and named "Simple Raycaster"
    InitWindow(WIDTH, HEIGHT, "Simple Raycaster");
    SetTargetFPS(60);

    // Sets up the game loop and runs until the window closes
    while (!WindowShouldClose()) {
        // Draws FPS in the upper left corner
        DrawFPS(10, 10);

        rays.clear();

        // Rebuilds rays every frame based on player facing direction
        // Fixed ray count means exactly rayCount rays every frame, no drift
        for (int i = 0; i < rayCount; i++) {
            float angle = playerAngle - playerFOV + (i * (playerFOV * 2.0f / rayCount));
            float radians = angle * (3.14159f / 180.0f);
            ray r = {playerX, playerY, (float)cos(radians), (float)sin(radians)};
            // Stores the angle offset from center for fisheye correction later
            r.angleOffset = (angle - playerAngle) * (3.14159f / 180.0f);
            rays.push_back(r);
        }

        // Creates the playerLast x and y for collision to work properly
        float playerLastX = playerX;
        float playerLastY = playerY;

        // W - Move Forward along facing direction using cos and sin
        if (IsKeyDown(KEY_W)) {
            playerX += cos(playerAngle * (3.14159f / 180.0f)) * speed;
            playerY += sin(playerAngle * (3.14159f / 180.0f)) * speed;
        }

        // S - Move backward along facing direction
        if (IsKeyDown(KEY_S)) {
            playerX -= cos(playerAngle * (3.14159f / 180.0f)) * speed;
            playerY -= sin(playerAngle * (3.14159f / 180.0f)) * speed;
        }

        // A - Rotate left by subtracting from playerAngle
        if (IsKeyDown(KEY_A)) playerAngle -= rotationSpeed;
        // D - Rotate right by adding to playerAngle
        if (IsKeyDown(KEY_D)) playerAngle += rotationSpeed;

        // Boundary collisions clamped to account for centered 50x50 player
        if (playerX < 25) playerX = 25;
        if (playerY < 25) playerY = 25;
        if (playerX > WIDTH - 25) playerX = WIDTH - 25;
        if (playerY > HEIGHT - 25) playerY = HEIGHT - 25;

        // Defines the player rect centered on playerX/playerY
        Rectangle player = {playerX - 25, playerY - 25, 50, 50};

        // Loops through all walls and snaps player back if collision detected
        for (Rectangle& wall : walls) {
            if (CheckCollisionRecs(player, wall)) {
                playerX = playerLastX;
                playerY = playerLastY;
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            // Casts each ray and finds the closest wall hit using CheckCollisionLines
            for (ray& r : rays) {
                Vector2 start = {playerX, playerY};
                Vector2 end = {r.x + r.dx * rayDistance, r.y + r.dy * rayDistance};
                r.x = playerX;
                r.y = playerY;

                Vector2 hitPoint = {0, 0};
                Vector2 drawEnd = end;
                float closest = 1000.0f;

                // Checks all four edges of every wall for ray intersection
                for (Rectangle& wall : walls) {
                    Vector2 topStart    = {wall.x, wall.y};
                    Vector2 topEnd      = {wall.x + wall.width, wall.y};
                    Vector2 bottomStart = {wall.x, wall.y + wall.height};
                    Vector2 bottomEnd   = {wall.x + wall.width, wall.y + wall.height};
                    Vector2 leftStart   = {wall.x, wall.y};
                    Vector2 leftEnd     = {wall.x, wall.y + wall.height};
                    Vector2 rightStart  = {wall.x + wall.width, wall.y};
                    Vector2 rightEnd    = {wall.x + wall.width, wall.y + wall.height};

                    // Keeps only the closest hit point across all edges and walls
                    if (CheckCollisionLines(start, end, topStart, topEnd, &hitPoint)) {
                        float dist = Vector2Distance(start, hitPoint);
                        if (dist < closest) { closest = dist; drawEnd = hitPoint; r.dist = dist; }
                    }
                    if (CheckCollisionLines(start, end, bottomStart, bottomEnd, &hitPoint)) {
                        float dist = Vector2Distance(start, hitPoint);
                        if (dist < closest) { closest = dist; drawEnd = hitPoint; r.dist = dist; }
                    }
                    if (CheckCollisionLines(start, end, leftStart, leftEnd, &hitPoint)) {
                        float dist = Vector2Distance(start, hitPoint);
                        if (dist < closest) { closest = dist; drawEnd = hitPoint; r.dist = dist; }
                    }
                    if (CheckCollisionLines(start, end, rightStart, rightEnd, &hitPoint)) {
                        float dist = Vector2Distance(start, hitPoint);
                        if (dist < closest) { closest = dist; drawEnd = hitPoint; r.dist = dist; }
                    }
                }
            } // ray loop ends

            // Slice width calculated from ray count so it always fills the screen
            int sliceWidth = (int)ceil(WIDTH / rayCount);

            // Draws each wall slice based on corrected distance
            for (int i = 0; i < (int)rays.size(); i++) {
                if (rays[i].dist > 0) {
                    // correctedDist fixes fisheye by projecting diagonal distance onto forward axis
                    float correctedDist = rays[i].dist * cos(rays[i].angleOffset);
                    // Closer wall means smaller dist means taller slice
                    float wallHeight = 30000.0f / correctedDist;
                    // Centers the slice vertically on screen
                    int sliceY = HEIGHT / 2 - wallHeight / 2;
                    // Draws the slice at its correct screen position
                    DrawRectangle(i * sliceWidth, sliceY, sliceWidth, (int)wallHeight, GREEN);
                }
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}