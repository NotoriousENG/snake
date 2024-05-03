#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "input.h"
#include "settings.h"
#include "snake.h"
#include "tail.h"
#include "ui.h"

// Check if any key is pressed
// NOTE: We limit keys check to keys between 32 (KEY_SPACE) and 126
bool IsAnyKeyPressed() {
  bool keyPressed = false;
  int key = GetKeyPressed();

  if ((key >= 32) && (key <= 126))
    keyPressed = true;

  return keyPressed;
}

void FoodReset(Entity *food, Snake *snake, int bounds) {
  bool overlaps = true;
  while (overlaps) {
    food->position =
        (Vector2){GetRandomValue(0, bounds - 1), GetRandomValue(1, bounds - 1)};
    // check if the food is not on the snake (or any tail part)
    const Vector2 pos = snake->entity.position;
    if (!Vector2Equals(food->position, pos) &&
        !TailCollisionCheck(snake->tail, food->position)) {
      overlaps = false;
      break;
    }
  }
}

void ResetEntities(Entity *food, Snake *snake, int dimensions) {
  FoodReset(food, snake, dimensions);
  SnakeReset(snake);
}

void SetScreenSize(int dimensions, int pixelSize) {
  const int width = fmax(pixelSize * dimensions, FOOTER_WIDTH);
  const int height = pixelSize * dimensions + FOOTER_HEIGHT;
  SetWindowSize(width, height);
}

bool SettingsPending(GameSettings *a, GameSettings *b) {
  return a->dimensions != b->dimensions || a->pixelSize != b->pixelSize ||
         a->latency_ms != b->latency_ms;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {

  GameSettings settings;
  SettingsReset(&settings);
  GameSettings tempSettings = settings;

  FreeText freeText = {.buf = "\0", .len = 0};

  bool mouseOnText = false;

  const Color COLOR_GRASS = (Color){0, 255, 0, 255};

  InitWindow(fmax(settings.pixelSize * settings.dimensions, FOOTER_WIDTH),
             settings.pixelSize * settings.dimensions + FOOTER_HEIGHT, "Snake");

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  Snake player = {
      .entity = (Entity){.position = (Vector2){5, 5}, .color = PURPLE},
      .dir = DIR_RIGHT,
      .lastDir = DIR_RIGHT,
      .points = 0,
      .tail = NULL,
  };

  Entity fruit = {
      .position = (Vector2){4, 4},
      .color = RED,
  };

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    //----------------------------------------------------------------------------------
    // Update

    float delta = GetFrameTime();

    int textInputsLen = 3;
    TextInput textInputs[3] = {
        (TextInput){
            .title = "Pixel Size",
            .rect =
                (Rectangle){10, GetScreenHeight() - FOOTER_HEIGHT + 40, 70, 25},
            .value = &tempSettings.pixelSize},
        (TextInput){.title = "Game Size",
                    .rect =
                        (Rectangle){100, GetScreenHeight() - FOOTER_HEIGHT + 40,
                                    70, 25},
                    .value = &tempSettings.dimensions},
        (TextInput){.title = "Latency(ms)",
                    .rect =
                        (Rectangle){200, GetScreenHeight() - FOOTER_HEIGHT + 40,
                                    70, 25},
                    .value = &tempSettings.latency_ms}};

    FreeTextUpdate(&freeText, mouseOnText);
    SnakeUpdate(&player, delta, settings.latency_ms, settings.dimensions);
    if (Vector2Equals(player.entity.position, fruit.position)) {
      SnakeGrow(&player);
      FoodReset(&fruit, &player, settings.dimensions);
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(BLACK);

    DrawRectangle(0, 0, settings.pixelSize * settings.dimensions,
                  settings.pixelSize * settings.dimensions, COLOR_GRASS);

    EntityDraw(&fruit, settings.pixelSize);

    SnakeDraw(&player, settings.pixelSize);

    DrawRectangle(0, GetScreenHeight() - FOOTER_HEIGHT, GetScreenWidth(),
                  FOOTER_HEIGHT, BLACK);
    DrawRectangle(0, GetScreenHeight() - FOOTER_HEIGHT + FOOTER_MARGIN,
                  GetScreenWidth(), FOOTER_HEIGHT - FOOTER_MARGIN, RAYWHITE);

    // shouldn't be here too lazy to move
    mouseOnText = false;
    for (int i = 0; i < 3; i++) {
      bool f = TextInputUpdate(&textInputs[i], &freeText);
      mouseOnText = mouseOnText || f;
    }

    if (SettingsPending(&settings, &tempSettings)) {
      DrawText("Press [ENTER] to apply, [C] to clear, [R] to Reset", 10,
               GetScreenHeight() - 20, 20, BLACK);

      if (IsKeyPressed(KEY_ENTER)) {
        settings = tempSettings;
        SetScreenSize(settings.dimensions, settings.pixelSize);
        ResetEntities(&fruit, &player, settings.dimensions);
      } else if (IsKeyPressed(KEY_C)) {
        tempSettings = settings;
      }
    }
    if (IsKeyPressed(KEY_R)) {
      SettingsReset(&settings);
      SetScreenSize(settings.dimensions, settings.pixelSize);
      ResetEntities(&fruit, &player, settings.dimensions);
      tempSettings = settings;
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();         // Close window and OpenGL context
  TailFree(player.tail); // free memory
  //--------------------------------------------------------------------------------------

  return 0;
}