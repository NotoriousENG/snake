#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define BLOCK_SIZE 32

typedef struct {
  Vector2 position;
  Color color;
} Entity;

void EntityClamp(Entity *self, const Rectangle bounds) {
  self->position = Vector2Clamp(
      self->position, (Vector2){bounds.x, bounds.y},
      (Vector2){bounds.width - BLOCK_SIZE, bounds.height - BLOCK_SIZE});
}

bool BoundsCollisionCheck(Vector2 pos, const Rectangle bounds) {
  return pos.x < bounds.x || pos.y < bounds.y ||
         pos.x + BLOCK_SIZE > bounds.width ||
         pos.y + BLOCK_SIZE > bounds.height;
}

void EntityDraw(Entity *self) {
  DrawRectangle(self->position.x, self->position.y, BLOCK_SIZE, BLOCK_SIZE,
                self->color);
}

bool EntityCollisionCheck(Entity *a, Vector2 pos) {
  return CheckCollisionRecs(
      (Rectangle){a->position.x, a->position.y, BLOCK_SIZE, BLOCK_SIZE},
      (Rectangle){pos.x, pos.y, BLOCK_SIZE, BLOCK_SIZE});
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
    pos.y -= BLOCK_SIZE;
    break;
  case DIR_DOWN:
    pos.y += BLOCK_SIZE;
    break;
  case DIR_LEFT:
    pos.x -= BLOCK_SIZE;
    break;
  case DIR_RIGHT:
    pos.x += BLOCK_SIZE;
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

  const Rectangle screenBounds = {
      .x = 0, .y = 0, .width = GetScreenWidth(), .height = GetScreenHeight()};

  enum Direction lastDir = self->dir;
  self->dir = GetMoveDirection(self->dir);

  Entity *e = &self->entity;

  // only run every 0.5 seconds
  if (self->moveTimer <= 0) {
    self->moveTimer = 0.25f;
    SnakeMove(self, self->dir);
  }

  EntityClamp(e, screenBounds);
  if (TailCollisionCheck(self->tail, e->position) ||
      (self->points > 0 && isInvertedDirection(self->dir, lastDir)) ||
      BoundsCollisionCheck(e->position, screenBounds)) {
    self->points = 0;
    self->entity.position =
        (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2};
    Tail *tail = self->tail;
    while (tail) {
      Tail *next = tail->next;
      free(tail);
      tail = next;
    }
    self->tail = NULL;
  }
}

void SnakeEat(Snake *self, Entity *food) {
  if (CheckCollisionRecs((Rectangle){self->entity.position.x,
                                     self->entity.position.y, BLOCK_SIZE,
                                     BLOCK_SIZE},
                         (Rectangle){food->position.x, food->position.y,
                                     BLOCK_SIZE, BLOCK_SIZE})) {
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

    Vector2 MaxCoords = {(GetScreenWidth() - BLOCK_SIZE) / BLOCK_SIZE,
                         (GetScreenHeight() - BLOCK_SIZE) / BLOCK_SIZE};

    int overlaps = 1;
    while (overlaps) {
      food->position = (Vector2){GetRandomValue(1, MaxCoords.x) * BLOCK_SIZE,
                                 GetRandomValue(1, MaxCoords.y) * BLOCK_SIZE};
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
    DrawRectangle(tail->position.x, tail->position.y, BLOCK_SIZE, BLOCK_SIZE,
                  c);
    tail = tail->next;
  }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = BLOCK_SIZE * 10;
  const int screenHeight = BLOCK_SIZE * 10;

  const Color COLOR_GRASS = (Color){0, 255, 0, 255};

  InitWindow(screenWidth, screenHeight, "Snake");

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  const Vector2 center = {screenWidth / 2.0f, screenHeight / 2.0f};

  Snake player = {
      .entity = (Entity){.position = center, .color = PURPLE},
      .dir = DIR_RIGHT,
      .points = 0,
      .tail = NULL,
  };

  Entity fruit = {
      .position = (Vector2){BLOCK_SIZE * 4, BLOCK_SIZE * 4},
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