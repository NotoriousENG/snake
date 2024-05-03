#include "raylib.h"

#define MAX_INPUT_CHARS 4

#define FOOTER_WIDTH 300
#define FOOTER_HEIGHT 100
#define FOOTER_MARGIN 10

bool IsAnyKeyPressed();

typedef struct {
  char buf[MAX_INPUT_CHARS + 1]; // Allow for null-terminator
  int len;
} FreeText;

typedef struct TextInput {
  const char *title;
  Rectangle rect;
  int *value;
} TextInput;

void FreeTextUpdate(FreeText *self, bool mouseOver);

bool TextInputUpdate(TextInput *self, FreeText *freeText);
