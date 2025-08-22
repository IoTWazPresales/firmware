// Mux.h
#pragma once
#include <Arduino.h>

// these must match what you have in DeviceScanner.cpp
static constexpr uint8_t MUX_S0  = 12;
static constexpr uint8_t MUX_S1  = 13;
static constexpr uint8_t MUX_S2  = 14;
static constexpr uint8_t MUX_S3  = 15;
static constexpr uint8_t MUX_SIG = 35;

static inline void selectMuxChannel(uint8_t ch) {
  digitalWrite(MUX_S0,  ch & 1);
  digitalWrite(MUX_S1, (ch >> 1) & 1);
  digitalWrite(MUX_S2, (ch >> 2) & 1);
  digitalWrite(MUX_S3, (ch >> 3) & 1);
}
