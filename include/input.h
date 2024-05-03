#pragma once
#include <stdbool.h>

enum Direction { DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 };

enum Direction GetMoveDirection(enum Direction fallback);

bool isInvertedDirection(enum Direction a, enum Direction b);