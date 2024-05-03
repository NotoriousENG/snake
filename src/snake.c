#include "snake.h"
#include "input.h"
#include <raymath.h>
#include <stdlib.h>

// PRIVATE --------------------
bool boundsContainsPosition(Vector2 pos, int bounds) {
  return pos.x < 0 || pos.y < 0 || pos.x > bounds || pos.y > bounds;
}
// ----------------------------

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

void SnakeUpdate(Snake *self, float delta, int latency_ms, int bounds) {
  self->moveTimer -= delta;
  self->dir = GetMoveDirection(self->lastDir);
  if (self->points > 0 && isInvertedDirection(self->dir, self->lastDir)) {
    self->dir = self->lastDir; // don't allow bw movement
  }

  Entity *e = &self->entity;

  // only run every BoundsCollisionCheck seconds
  if (self->moveTimer <= 0) {
    self->moveTimer = latency_ms / 1000.0f;
    SnakeMove(self, self->dir);
    self->lastDir = self->dir;
  }

  EntityClamp(e, bounds);
  if (TailCollisionCheck(self->tail, e->position) ||
      boundsContainsPosition(e->position, bounds)) {
    SnakeReset(self);
  }
}

void SnakeGrow(Snake *self) {
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
}

void SnakeDraw(Snake *self, int size) {
  const Color c1 = {217, 177, 250, 255};
  const Color c2 = {225, 192, 252, 255};
  EntityDraw(&self->entity, size);
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
    DrawRectangle(tail->position.x * size, tail->position.y * size, size, size,
                  c);
    tail = tail->next;
  }
}
