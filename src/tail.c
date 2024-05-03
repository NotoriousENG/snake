#include "tail.h"
#include <stdlib.h>

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