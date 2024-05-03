#include "entity.h"
#include "raymath.h"

void EntityDraw(Entity *self, int size) {
  DrawRectangle(self->position.x * size, self->position.y * size, size, size,
                self->color);
}

void EntityClamp(Entity *self, int boundary) {
  self->position = Vector2Clamp(self->position, (Vector2){0.0f, 0.0f},
                                (Vector2){boundary - 1.0f, boundary - 1.0f});
}