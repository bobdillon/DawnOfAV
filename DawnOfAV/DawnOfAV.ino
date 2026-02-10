// CRT Video/Audio Tester - based on bitluni's DawnOfAV
// NTSC color composite output on GPIO 25 (DAC1)
// Audio test tone on GPIO 27 via LEDC PWM (avoids DAC conflict)
// Pattern button on GPIO 14 (cycle test patterns)
// Audio button on GPIO 13 (toggle continuous tone / momentary beep)
//
// Wire changes from original:
//   - Move audio wire from D26 to GPIO 27
//   - Add momentary buttons: GPIO 14 to GND, GPIO 13 to GND
//  Must use esp32 by Espressif Systems V1.0.4 in boards manager
//  Must use "No OTA (2MB APP/2MB SPIFFS)" or "Huge APP (3MB No OTA/1MB SPIFFS)"
#include "esp_pm.h"
#include "Graphics.h"
#include "Image.h"
#include "SimpleNTSCOutput.h"
#include "Font.h"

// Font
namespace font88 {
  #include "gfx/font.h"
}
Font font(8, 8, font88::pixels);

////////////////////////////
// Pin configuration
const int AUDIO_PIN = 27;       // LEDC PWM audio output
const int PATTERN_BTN = 14;     // Button to cycle patterns (connect to GND)
const int AUDIO_BTN = 13;       // Button to toggle audio (connect to GND)

////////////////////////////
// Audio configuration via LEDC
const int LEDC_CHANNEL = 0;
const int LEDC_RESOLUTION = 8;  // 8-bit resolution
bool audioOn = false;
bool lastAudioBtn = true;
bool lastPatternBtn = true;
unsigned long lastPatternPress = 0;
unsigned long lastAudioPress = 0;
const unsigned long DEBOUNCE_MS = 250;

////////////////////////////
// Video configuration
const int XRES = 320;
const int YRES = 240;
Graphics graphics(XRES, YRES);
SimpleNTSCOutput composite;

// Test pattern enum
enum TestPattern {
  PATTERN_COLOR_BARS,
  PATTERN_SMPTE_BARS,
  PATTERN_CROSSHATCH,
  PATTERN_DOTS,
  PATTERN_WHITE,
  PATTERN_RED,
  PATTERN_GREEN,
  PATTERN_BLUE,
  PATTERN_GRAY_RAMP,
  PATTERN_CIRCLE,
  PATTERN_CONVERGENCE,
  PATTERN_COUNT
};

const char* patternNames[] = {
  "COLOR BARS",
  "SMPTE BARS",
  "CROSSHATCH",
  "DOT GRID",
  "WHITE FIELD",
  "RED FIELD",
  "GREEN FIELD",
  "BLUE FIELD",
  "GRAY RAMP",
  "CIRCLE",
  "CONVERGENCE"
};

int currentPattern = 0;

void compositeCore(void *data) {
  while (true) {
    composite.sendFrame(&graphics.frame);
  }
}

////////////////////////////
// Audio functions using LEDC
void audioSetup() {
  ledcSetup(LEDC_CHANNEL, 1000, LEDC_RESOLUTION);  // 1kHz default
  ledcAttachPin(AUDIO_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, 0);  // Start silent
}

void audioToneOn(int freqHz) {
  ledcWriteTone(LEDC_CHANNEL, freqHz);
}

void audioToneOff() {
  ledcWriteTone(LEDC_CHANNEL, 0);
  ledcWrite(LEDC_CHANNEL, 0);
}

////////////////////////////
// Drawing helper functions

void drawColorBars(Graphics &g) {
  // Standard 8-bar color bars: White, Yellow, Cyan, Green, Magenta, Red, Blue, Black
  const int barWidth = XRES / 8;
  
  // Colors in r4g4b4 format
  unsigned int colors[] = {
    g.rgb(255, 255, 255),  // White
    g.rgb(255, 255, 0),    // Yellow
    g.rgb(0, 255, 255),    // Cyan
    g.rgb(0, 255, 0),      // Green
    g.rgb(255, 0, 255),    // Magenta
    g.rgb(255, 0, 0),      // Red
    g.rgb(0, 0, 255),      // Blue
    g.rgb(0, 0, 0),        // Black
  };
  
  for (int i = 0; i < 8; i++) {
    g.fillRect(i * barWidth, 0, barWidth, YRES, colors[i]);
  }
}

void drawSMPTEBars(Graphics &g) {
  // Top 2/3: standard color bars
  const int barWidth = XRES / 7;
  const int topHeight = YRES * 2 / 3;
  const int midHeight = YRES / 12;
  const int botHeight = YRES - topHeight - midHeight;

  unsigned int topColors[] = {
    g.rgb(192, 192, 192),  // Gray
    g.rgb(192, 192, 0),    // Yellow
    g.rgb(0, 192, 192),    // Cyan
    g.rgb(0, 192, 0),      // Green
    g.rgb(192, 0, 192),    // Magenta
    g.rgb(192, 0, 0),      // Red
    g.rgb(0, 0, 192),      // Blue
  };

  for (int i = 0; i < 7; i++) {
    g.fillRect(i * barWidth, 0, barWidth, topHeight, topColors[i]);
  }

  // Middle strip: reverse bars (blue, black, magenta, black, cyan, black, gray)
  unsigned int midColors[] = {
    g.rgb(0, 0, 192),
    g.rgb(16, 16, 16),
    g.rgb(192, 0, 192),
    g.rgb(16, 16, 16),
    g.rgb(0, 192, 192),
    g.rgb(16, 16, 16),
    g.rgb(192, 192, 192),
  };
  for (int i = 0; i < 7; i++) {
    g.fillRect(i * barWidth, topHeight, barWidth, midHeight, midColors[i]);
  }

  // Bottom strip: grayscale ramp
  int botSections = 4;
  int secWidth = XRES / botSections;
  for (int i = 0; i < botSections; i++) {
    int v = (i * 255) / (botSections - 1);
    g.fillRect(i * secWidth, topHeight + midHeight, secWidth, botHeight, g.rgb(v, v, v));
  }
}

void drawCrosshatch(Graphics &g) {
  // Black background
  g.fillRect(0, 0, XRES, YRES, g.rgb(0, 0, 0));
  
  unsigned int white = g.rgb(255, 255, 255);
  int spacingX = 32;
  int spacingY = 30;
  
  // Vertical lines
  for (int x = 0; x < XRES; x += spacingX) {
    g.line(x, 0, x, YRES - 1, white);
  }
  // Horizontal lines
  for (int y = 0; y < YRES; y += spacingY) {
    g.line(0, y, XRES - 1, y, white);
  }
  // Border
  g.line(0, 0, XRES - 1, 0, white);
  g.line(0, YRES - 1, XRES - 1, YRES - 1, white);
  g.line(0, 0, 0, YRES - 1, white);
  g.line(XRES - 1, 0, XRES - 1, YRES - 1, white);
}

void drawDotGrid(Graphics &g) {
  g.fillRect(0, 0, XRES, YRES, g.rgb(0, 0, 0));
  unsigned int white = g.rgb(255, 255, 255);
  
  for (int y = 15; y < YRES; y += 30) {
    for (int x = 16; x < XRES; x += 32) {
      g.dot(x, y, white);
      g.dot(x + 1, y, white);
      g.dot(x, y + 1, white);
      g.dot(x + 1, y + 1, white);
    }
  }
}

void drawSolidField(Graphics &g, int r, int gr, int b) {
  g.fillRect(0, 0, XRES, YRES, g.rgb(r, gr, b));
}

void drawGrayRamp(Graphics &g) {
  for (int x = 0; x < XRES; x++) {
    int v = (x * 255) / XRES;
    unsigned int c = g.rgb(v, v, v);
    for (int y = 0; y < YRES; y++) {
      g.dot(x, y, c);
    }
  }
}

void drawCircle(Graphics &g) {
  g.fillRect(0, 0, XRES, YRES, g.rgb(0, 0, 0));
  unsigned int white = g.rgb(255, 255, 255);
  
  int cx = XRES / 2;
  int cy = YRES / 2;
  int radius = YRES / 2 - 10;
  
  // Draw circle using Bresenham-ish approach
  for (int angle = 0; angle < 3600; angle++) {
    float rad = angle * M_PI / 1800.0;
    int x = cx + (int)(radius * cos(rad));
    int y = cy + (int)(radius * sin(rad));
    g.dot(x, y, white);
  }
  
  // Crosshair
  g.line(cx - radius, cy, cx + radius, cy, white);
  g.line(cx, cy - radius, cx, cy + radius, white);
  
  // Inner circle at half radius
  int r2 = radius / 2;
  for (int angle = 0; angle < 3600; angle++) {
    float rad = angle * M_PI / 1800.0;
    int x = cx + (int)(r2 * cos(rad));
    int y = cy + (int)(r2 * sin(rad));
    g.dot(x, y, white);
  }
}

void drawConvergence(Graphics &g) {
  // RGB convergence pattern - colored crosshatch
  g.fillRect(0, 0, XRES, YRES, g.rgb(0, 0, 0));

  unsigned int red = g.rgb(255, 0, 0);
  unsigned int green = g.rgb(0, 255, 0);
  unsigned int blue = g.rgb(0, 0, 255);
  unsigned int white = g.rgb(255, 255, 255);
  
  int spacingX = 32;
  int spacingY = 30;
  
  // Red vertical lines
  for (int x = 0; x < XRES; x += spacingX) {
    g.line(x, 0, x, YRES - 1, red);
  }
  // Green horizontal lines
  for (int y = 0; y < YRES; y += spacingY) {
    g.line(0, y, XRES - 1, y, green);
  }
  // Blue diagonals
  g.line(0, 0, XRES - 1, YRES - 1, blue);
  g.line(XRES - 1, 0, 0, YRES - 1, blue);
  
  // White center cross
  g.line(XRES / 2 - 20, YRES / 2, XRES / 2 + 20, YRES / 2, white);
  g.line(XRES / 2, YRES / 2 - 20, XRES / 2, YRES / 2 + 20, white);
}

void drawPattern(int pattern) {
  graphics.begin(0);

  switch (pattern) {
    case PATTERN_COLOR_BARS:   drawColorBars(graphics); break;
    case PATTERN_SMPTE_BARS:   drawSMPTEBars(graphics); break;
    case PATTERN_CROSSHATCH:   drawCrosshatch(graphics); break;
    case PATTERN_DOTS:         drawDotGrid(graphics); break;
    case PATTERN_WHITE:        drawSolidField(graphics, 255, 255, 255); break;
    case PATTERN_RED:          drawSolidField(graphics, 255, 0, 0); break;
    case PATTERN_GREEN:        drawSolidField(graphics, 0, 255, 0); break;
    case PATTERN_BLUE:         drawSolidField(graphics, 0, 0, 255); break;
    case PATTERN_GRAY_RAMP:    drawGrayRamp(graphics); break;
    case PATTERN_CIRCLE:       drawCircle(graphics); break;
    case PATTERN_CONVERGENCE:  drawConvergence(graphics); break;
  }

  // Draw pattern name overlay at top
  graphics.setTextColor(graphics.rgb(255, 255, 255));
  int nameLen = strlen(patternNames[pattern]);
  graphics.setCursor(XRES / 2 - nameLen * 4, 4);
  graphics.print(patternNames[pattern]);

  // Draw pattern number
  char numStr[16];
  sprintf(numStr, "%d/%d", pattern + 1, PATTERN_COUNT);
  int numLen = strlen(numStr);
  graphics.setCursor(XRES / 2 - numLen * 4, 14);
  graphics.print(numStr);

  // Audio indicator
  if (audioOn) {
    graphics.setCursor(XRES - 32, 4);
    graphics.print("1kHz");
  }

  graphics.end();
}

////////////////////////////
// Setup and main loop

void setup() {
  // Max CPU frequency for stable video
  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);

  // Initialize composite video
  composite.init();
  graphics.init();
  graphics.setFont(font);

  // Start video output on core 0
  xTaskCreatePinnedToCore(compositeCore, "compositeCoreTask", 1024, NULL, 1, NULL, 0);

  // Initialize audio via LEDC
  audioSetup();

  // Initialize buttons with internal pullups
  pinMode(PATTERN_BTN, INPUT_PULLUP);
  pinMode(AUDIO_BTN, INPUT_PULLUP);

  // Draw initial pattern
  drawPattern(currentPattern);
}

void loop() {
  unsigned long now = millis();
  
  // Read pattern button (active low)
  bool patternBtn = digitalRead(PATTERN_BTN);
  if (!patternBtn && lastPatternBtn && (now - lastPatternPress > DEBOUNCE_MS)) {
    currentPattern = (currentPattern + 1) % PATTERN_COUNT;
    drawPattern(currentPattern);
    lastPatternPress = now;
  }
  lastPatternBtn = patternBtn;

  // Read audio button (active low) - toggle mode
  bool audioBtn = digitalRead(AUDIO_BTN);
  if (!audioBtn && lastAudioBtn && (now - lastAudioPress > DEBOUNCE_MS)) {
    audioOn = !audioOn;
    if (audioOn) {
      audioToneOn(1000);  // 1kHz test tone
    } else {
      audioToneOff();
    }
    drawPattern(currentPattern);  // Redraw to update audio indicator
    lastAudioPress = now;
  }
  lastAudioBtn = audioBtn;

  delay(10);
}
