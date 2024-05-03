#include "input.h"
#include "raylib.h"


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