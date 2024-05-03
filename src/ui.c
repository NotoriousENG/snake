#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void FreeTextUpdate(FreeText *self, bool mouseOver) {
  if (!mouseOver) {
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    self->buf[0] = '\0';
    self->len = 0;
    return;
  }
  // Set the window's cursor to the I-Beam
  SetMouseCursor(MOUSE_CURSOR_IBEAM);

  // Get char pressed (unicode character) on the queue
  int key = GetCharPressed();

  // Check if more characters have been pressed on the same frame
  while (key > 0) {
    // NOTE: Only allow number keys
    if ((key >= '0') && (key <= '9') && (self->len < MAX_INPUT_CHARS)) {
      self->buf[self->len] = (char)key;
      self->buf[self->len + 1] =
          '\0'; // Add null terminator at the end of the string.
      self->len++;
    }

    key = GetCharPressed(); // Check next character in the queue
  }

  if (IsKeyPressed(KEY_BACKSPACE)) {
    self->len--;
    if (self->len < 0)
      self->len = 0;
    self->buf[self->len] = '\0';
  }
}

bool TextInputUpdate(TextInput *self, FreeText *freeText) {
  DrawText(self->title, self->rect.x, GetScreenHeight() - FOOTER_HEIGHT + 25,
           10, BLACK);

  DrawRectangleRec(self->rect, LIGHTGRAY);
  bool f = CheckCollisionPointRec(GetMousePosition(), self->rect);
  int v = *self->value;
  if (f) {
    DrawRectangleLinesEx(self->rect, 2, RED);
    int num = (int)fmax(atoi(freeText->buf), 1);
    v = num;
    DrawText(TextFormat("%s", freeText->buf), self->rect.x + 10,
             self->rect.y + 5, 20, BLACK);
    *self->value = v;
  } else {
    DrawRectangleLinesEx(self->rect, 2, BLACK);
    DrawText(TextFormat("%i", v), self->rect.x + 10, self->rect.y + 5, 20,
             BLACK);
  }
  return f;
}