#pragma once

#include "entity.h"
#include "input.h"
#include "tail.h"

typedef struct Snake {
  Entity entity;
  enum Direction dir;
  float moveTimer;
  int points;
  Tail *tail;
} Snake;

void SnakeMove(Snake *self, enum Direction dir);

void SnakeReset(Snake *self);

void SnakeUpdate(Snake *self, float delta, int latency_ms, int bounds);

void SnakeGrow(Snake *self);

void SnakeDraw(Snake *self, int size);
