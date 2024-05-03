#pragma once

typedef struct {
  int pixelSize;
  int dimensions;
  int latency_ms;
} GameSettings;

void SettingsReset(GameSettings *settings) {
  settings->dimensions = 10;
  settings->pixelSize = 64;
  settings->latency_ms = 250;
}