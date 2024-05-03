#pragma once

#include "raylib.h"

typedef struct Entity {
  Vector2 position;
  Color color;
} Entity;

void EntityClamp(Entity *self, int boundary);

void EntityDraw(Entity *self, int size);