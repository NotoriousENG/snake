#pragma once

#include "raylib.h"

typedef struct Tail {
  Vector2 position;
  struct Tail *next;
} Tail;

bool TailCollisionCheck(Tail *tail, Vector2 pos);

void TailFree(Tail *tail);