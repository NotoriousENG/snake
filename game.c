#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define PIXEL_SIZE 64
#define GAME_DIMENSIONS 10
#define MOVE_LATENCY 0.25f

#define FOOTER_WIDTH 150
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
                   (Vector2){GAME_DIMENSIONS - 1, GAME_DIMENSIONS - 1});
}

bool BoundsCollisionCheck(Vector2 pos) {
  return pos.x < 0 || pos.y < 0 || pos.x > GAME_DIMENSIONS ||
         pos.y > GAME_DIMENSIONS;
}

void EntityDraw(Entity *self) {
  DrawRectangle(self->position.x * PIXEL_SIZE, self->position.y * PIXEL_SIZE,
                PIXEL_SIZE, PIXEL_SIZE, self->color);
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

void SnakeUpdate(Snake *self, float delta) {
  self->moveTimer -= delta;

  enum Direction lastDir = self->dir;
  self->dir = GetMoveDirection(self->dir);

  Entity *e = &self->entity;

  // only run every BoundsCollisionCheck seconds
  if (self->moveTimer <= 0) {
    self->moveTimer = MOVE_LATENCY;
    SnakeMove(self, self->dir);
  }

  EntityClamp(e);
  if (TailCollisionCheck(self->tail, e->position) ||
      (self->points > 0 && isInvertedDirection(self->dir, lastDir)) ||
      BoundsCollisionCheck(e->position)) {
    self->points = 0;
    self->entity.position = (Vector2){5, 5};
    TailFree(self->tail);
    self->tail = NULL; // the player stuct needs this to be NULL too
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

    int overlaps = 1;
    while (overlaps) {
      food->position = (Vector2){GetRandomValue(0, GAME_DIMENSIONS - 1),
                                 GetRandomValue(1, GAME_DIMENSIONS - 1)};
      // check if the food is not on the snake (or any tail part)
      const Vector2 pos = self->entity.position;
      if (!EntityCollisionCheck(food, pos) &&
          !TailCollisionCheck(self->tail, food->position)) {
        overlaps = 0;
        break;
      }
    }
  }
}

void SnakeDraw(Snake *self) {
  const Color c1 = {217, 177, 250, 255};
  const Color c2 = {225, 192, 252, 255};
  EntityDraw(&self->entity);
  Tail *tail = self->tail;
  int useDefaultColor = 1;
  Vector2 lastPos = self->entity.position;
  while (tail) {
    useDefaultColor = !useDefaultColor;
    if (tail->position.x == lastPos.x && tail->position.y == lastPos.y) {
      return;
    }
    lastPos = tail->position;
    Color c = useDefaultColor ? c1 : c2;
    DrawRectangle(tail->position.x * PIXEL_SIZE, tail->position.y * PIXEL_SIZE,
                  PIXEL_SIZE, PIXEL_SIZE, c);
    tail = tail->next;
  }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = fmax(PIXEL_SIZE * GAME_DIMENSIONS, FOOTER_WIDTH);
  const int screenHeight = PIXEL_SIZE * GAME_DIMENSIONS + FOOTER_HEIGHT;

  Rectangle inputRect = {10, screenHeight - FOOTER_HEIGHT + 40, 200, 25};

  const Color COLOR_GRASS = (Color){0, 255, 0, 255};

  InitWindow(screenWidth, screenHeight, "Snake");

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

    //----------------------------------------------------------------------------------
    // Update
    SnakeUpdate(&player, delta);
    SnakeEat(&player, &fruit);
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(COLOR_GRASS);

    EntityDraw(&fruit);

    SnakeDraw(&player);

    DrawRectangle(0, screenHeight - FOOTER_HEIGHT, screenWidth, FOOTER_HEIGHT,
                  BLACK);
    DrawRectangle(0, screenHeight - FOOTER_HEIGHT + FOOTER_MARGIN, screenWidth,
                  FOOTER_HEIGHT - FOOTER_MARGIN, RAYWHITE);

    // DrawText(TextFormat("Points: %d", player.points), 10,
    //          screenHeight - FOOTER_HEIGHT + 10, 20, BLACK);

    DrawRectangleRec(inputRect, LIGHTGRAY);

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