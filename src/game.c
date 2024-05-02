#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_CHARS 4
typedef struct {
  int pixelSize;
  int dimensions;
  int latency_ms;
} GameSettings;

static GameSettings Settings = {
    .pixelSize = 64,
    .dimensions = 10,
    .latency_ms = 250,
};

typedef struct {
  char buf[MAX_INPUT_CHARS + 1]; // Allow for null-terminator
  int len;
} FreeText;

static FreeText freeText = {.buf = "\0", .len = 0};

#define FOOTER_WIDTH 300
#define FOOTER_HEIGHT 100
#define FOOTER_MARGIN 10

// Check if any key is pressed
// NOTE: We limit keys check to keys between 32 (KEY_SPACE) and 126
bool IsAnyKeyPressed() {
  bool keyPressed = false;
  int key = GetKeyPressed();

  if ((key >= 32) && (key <= 126))
    keyPressed = true;

  return keyPressed;
}

typedef struct {
  Vector2 position;
  Color color;
} Entity;

void EntityClamp(Entity *self) {
  self->position =
      Vector2Clamp(self->position, (Vector2){0, 0},
                   (Vector2){Settings.dimensions - 1, Settings.dimensions - 1});
}

bool BoundsCollisionCheck(Vector2 pos) {
  return pos.x < 0 || pos.y < 0 || pos.x > Settings.dimensions ||
         pos.y > Settings.dimensions;
}

void EntityDraw(Entity *self) {
  DrawRectangle(self->position.x * Settings.pixelSize,
                self->position.y * Settings.pixelSize, Settings.pixelSize,
                Settings.pixelSize, self->color);
}

bool EntityCollisionCheck(Entity *a, Vector2 pos) {
  return a->position.x == pos.x && a->position.y == pos.y;
}

enum Direction { DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 };

enum Direction GetMoveDirection(enum Direction fallback) {
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
    return DIR_UP;
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
    return DIR_DOWN;
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    return DIR_LEFT;
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    return DIR_RIGHT;

  return fallback;
}

bool isInvertedDirection(enum Direction a, enum Direction b) {
  return (a == DIR_UP && b == DIR_DOWN) || (a == DIR_DOWN && b == DIR_UP) ||
         (a == DIR_LEFT && b == DIR_RIGHT) || (a == DIR_RIGHT && b == DIR_LEFT);
}

typedef struct Tail {
  Vector2 position;
  struct Tail *next;
} Tail;

bool TailCollisionCheck(Tail *tail, Vector2 pos) {
  while (tail) {
    if (tail->position.x == pos.x && tail->position.y == pos.y) {
      return true;
    }
    tail = tail->next;
  }
  return false;
}

void TailFree(Tail *tail) {
  while (tail) {
    Tail *next = tail->next;
    free(tail);
    tail = next;
  }
  tail = NULL;
}

typedef struct {
  Entity entity;
  enum Direction dir;
  float moveTimer;
  int points;
  Tail *tail;
} Snake;

void SnakeMove(Snake *self, enum Direction dir) {
  Vector2 pos = self->entity.position;
  switch (dir) {
  case DIR_UP:
    pos.y -= 1;
    break;
  case DIR_DOWN:
    pos.y += 1;
    break;
  case DIR_LEFT:
    pos.x -= 1;
    break;
  case DIR_RIGHT:
    pos.x += 1;
    break;
  }
  Tail *tail = self->tail;
  Vector2 lastPos = self->entity.position;
  while (tail) {
    Vector2 temp = tail->position;
    tail->position = lastPos;
    lastPos = temp;
    tail = tail->next;
  }
  self->entity.position = pos;
}

void SnakeReset(Snake *self) {
  self->entity.position = (Vector2){5, 5};
  self->dir = DIR_RIGHT;
  self->points = 0;
  TailFree(self->tail);
  self->tail = NULL;
}

void FoodReset(Entity *food, Snake *snake) {
  bool overlaps = true;
  while (overlaps) {
    food->position = (Vector2){GetRandomValue(0, Settings.dimensions - 1),
                               GetRandomValue(1, Settings.dimensions - 1)};
    // check if the food is not on the snake (or any tail part)
    const Vector2 pos = snake->entity.position;
    if (!EntityCollisionCheck(food, pos) &&
        !TailCollisionCheck(snake->tail, food->position)) {
      overlaps = false;
      break;
    }
  }
}

void ResetEntities(Entity *food, Snake *snake) {
  FoodReset(food, snake);
  SnakeReset(snake);
}

void SnakeUpdate(Snake *self, float delta) {
  self->moveTimer -= delta;

  enum Direction lastDir = self->dir;
  self->dir = GetMoveDirection(self->dir);

  Entity *e = &self->entity;

  // only run every BoundsCollisionCheck seconds
  if (self->moveTimer <= 0) {
    self->moveTimer = Settings.latency_ms / 1000.0f;
    SnakeMove(self, self->dir);
  }

  EntityClamp(e);
  if (TailCollisionCheck(self->tail, e->position) ||
      (self->points > 0 && isInvertedDirection(self->dir, lastDir)) ||
      BoundsCollisionCheck(e->position)) {
    SnakeReset(self);
  }
}

void SnakeEat(Snake *self, Entity *food) {
  if (self->entity.position.x == food->position.x &&
      self->entity.position.y == food->position.y) {
    self->points++;
    if (!self->tail) {
      self->tail = malloc(sizeof(Tail));
      self->tail->position = (Vector2){-1000, -1000};
      self->tail->next = NULL;
    } else {
      Tail *tail = self->tail;
      while (tail->next) {
        tail = tail->next;
      }
      tail->next = malloc(sizeof(Tail));
      tail->next->position = tail->position;
      tail->next->next = NULL;
    }

    FoodReset(food, self);
  }
}

void SnakeDraw(Snake *self) {
  const Color c1 = {217, 177, 250, 255};
  const Color c2 = {225, 192, 252, 255};
  EntityDraw(&self->entity);
  Tail *tail = self->tail;
  bool useDefaultColor = true;
  Vector2 lastPos = self->entity.position;
  while (tail) {
    useDefaultColor = !useDefaultColor;
    if (tail->position.x == lastPos.x && tail->position.y == lastPos.y) {
      return;
    }
    lastPos = tail->position;
    Color c = useDefaultColor ? c1 : c2;
    DrawRectangle(tail->position.x * Settings.pixelSize,
                  tail->position.y * Settings.pixelSize, Settings.pixelSize,
                  Settings.pixelSize, c);
    tail = tail->next;
  }
}

void SetScreenSize(int dimensions, int pixelSize) {
  const int width = fmax(pixelSize * dimensions, FOOTER_WIDTH);
  const int height = pixelSize * dimensions + FOOTER_HEIGHT;
  SetWindowSize(width, height);
}

bool InputFieldInt(const char *title, int *value, Rectangle rect) {
  DrawText(title, rect.x, GetScreenHeight() - FOOTER_HEIGHT + 25, 10, BLACK);
  int v = *value;

  DrawRectangleRec(rect, LIGHTGRAY);
  bool f = CheckCollisionPointRec(GetMousePosition(), rect);
  if (f) {
    DrawRectangleLinesEx(rect, 2, RED);
    int num = (int)fmax(atoi(freeText.buf), 1);
    v = num;
    DrawText(TextFormat("%s", freeText.buf), rect.x + 10, rect.y + 5, 20,
             BLACK);
  } else {
    DrawRectangleLinesEx(rect, 2, BLACK);
    DrawText(TextFormat("%i", v), rect.x + 10, rect.y + 5, 20, BLACK);
  }

  *value = v;
  return f;
}

bool SettingsPending(GameSettings *a, GameSettings *b) {
  return a->dimensions != b->dimensions || a->pixelSize != b->pixelSize ||
         a->latency_ms != b->latency_ms;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {

  GameSettings tempSettings = Settings;

  GameSettings defaultSettings = {
      .pixelSize = 64,
      .dimensions = 10,
      .latency_ms = 250,
  };

  bool mouseOnText = false;

  const Color COLOR_GRASS = (Color){0, 255, 0, 255};

  InitWindow(fmax(Settings.pixelSize * Settings.dimensions, FOOTER_WIDTH),
             Settings.pixelSize * Settings.dimensions + FOOTER_HEIGHT, "Snake");

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  Snake player = {
      .entity = (Entity){.position = (Vector2){5, 5}, .color = PURPLE},
      .dir = DIR_RIGHT,
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
    // get delta time
    float delta = GetFrameTime();

    Rectangle inputRectPixelSize = {10, GetScreenHeight() - FOOTER_HEIGHT + 40,
                                    70, 25};

    Rectangle inputRectDimensions = {
        100, GetScreenHeight() - FOOTER_HEIGHT + 40, 70, 25};

    Rectangle inputRectLatency = {200, GetScreenHeight() - FOOTER_HEIGHT + 40,
                                  70, 25};

    //----------------------------------------------------------------------------------
    // Update
    SnakeUpdate(&player, delta);
    SnakeEat(&player, &fruit);

    if (mouseOnText) {
      // Set the window's cursor to the I-Beam
      SetMouseCursor(MOUSE_CURSOR_IBEAM);

      // Get char pressed (unicode character) on the queue
      int key = GetCharPressed();

      // Check if more characters have been pressed on the same frame
      while (key > 0) {
        // NOTE: Only allow number keys
        if ((key >= '0') && (key <= '9') && (freeText.len < MAX_INPUT_CHARS)) {
          freeText.buf[freeText.len] = (char)key;
          freeText.buf[freeText.len + 1] =
              '\0'; // Add null terminator at the end of the string.
          freeText.len++;
        }

        key = GetCharPressed(); // Check next character in the queue
      }

      if (IsKeyPressed(KEY_BACKSPACE)) {
        freeText.len--;
        if (freeText.len < 0)
          freeText.len = 0;
        freeText.buf[freeText.len] = '\0';
      }
    } else
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(BLACK);

    DrawRectangle(0, 0, Settings.pixelSize * Settings.dimensions,
                  Settings.pixelSize * Settings.dimensions, COLOR_GRASS);

    EntityDraw(&fruit);

    SnakeDraw(&player);

    DrawRectangle(0, GetScreenHeight() - FOOTER_HEIGHT, GetScreenWidth(),
                  FOOTER_HEIGHT, BLACK);
    DrawRectangle(0, GetScreenHeight() - FOOTER_HEIGHT + FOOTER_MARGIN,
                  GetScreenWidth(), FOOTER_HEIGHT - FOOTER_MARGIN, RAYWHITE);

    bool f1 = InputFieldInt("Pixel Size", &tempSettings.pixelSize,
                            inputRectPixelSize);

    bool f2 = InputFieldInt("Level Size", &tempSettings.dimensions,
                            inputRectDimensions);

    bool f3 = InputFieldInt("Move Latency (ms)", &tempSettings.latency_ms,
                            inputRectLatency);

    mouseOnText = f1 || f2 || f3;

    if (!mouseOnText) {
      freeText.buf[0] = '\0';
      freeText.len = 0;
    }

    if (SettingsPending(&Settings, &tempSettings)) {
      DrawText("Press [ENTER] to apply, [C] to clear, [R] to Reset", 10,
               GetScreenHeight() - 20, 20, BLACK);

      if (IsKeyPressed(KEY_ENTER)) {
        Settings = tempSettings;
        SetScreenSize(Settings.dimensions, Settings.pixelSize);
        ResetEntities(&fruit, &player);
      } else if (IsKeyPressed(KEY_C)) {
        tempSettings = Settings;
      }
    }
    if (IsKeyPressed(KEY_R)) {
      Settings = defaultSettings;
      SetScreenSize(Settings.dimensions, Settings.pixelSize);
      ResetEntities(&fruit, &player);
      tempSettings = Settings;
      printf("Reset\n");
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();         // Close window and OpenGL context
  TailFree(player.tail); // free memory
  player.tail = NULL;
  //--------------------------------------------------------------------------------------

  return 0;
}