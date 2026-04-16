#include <Arduino.h>
#include <Wire.h>
#include <AS5600.h>
#include <phyphoxBle.h>

/* =========================================================
 * (A) DEFINES / VARIABLEN
 * ========================================================= */

// -------- AS5600 --------
#define SDA_PIN 21
#define SCL_PIN 20
#define AS5600_ADDR 0x36
#define RAW_TO_DEG 0.087890625f

AS5600 as5600;
bool as5600Available = false;

// -------- Motor --------
#define FAN_PIN 9
#define PWM_FREQ 25000
#define PWM_RESOLUTION 8

int currentPercent = 0;

// -------- phyphox --------
float pwmInput = 0;
float startInput = 0;

// -------- Zeit --------
unsigned long lastSend = 0;


/* =========================================================
 * MOTOR FUNKTIONEN
 * ========================================================= */

void setupFan() {
  if (!ledcAttach(FAN_PIN, PWM_FREQ, PWM_RESOLUTION)) {
    Serial.println("PWM init fehlgeschlagen!");
  } else {
    Serial.println("PWM bereit");
  }
}

void setFanPercent(int percent) {
  percent = constrain(percent, 0, 100);
  currentPercent = percent;

  int duty = map(percent, 0, 100, 0, 255);
  ledcWrite(FAN_PIN, duty);
}


/* =========================================================
 * SENSOR
 * ========================================================= */

bool checkI2C(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

float readAngle() {
  if (as5600Available) {
    uint16_t raw = as5600.readAngle();
    return raw * RAW_TO_DEG;
  }
  return NAN;
}


/* =========================================================
 * phyphox CALLBACK
 * ========================================================= */

void receivedData() {
  // Reihenfolge = Channels!
  PhyphoxBLE::read(startInput, pwmInput);

  Serial.print("Start: ");
  Serial.print(startInput);
  Serial.print(" | PWM: ");
  Serial.println(pwmInput);

  if (startInput > 0.5) {
    setFanPercent((int)pwmInput);
  } else {
    setFanPercent(0);
  }
}


/* =========================================================
 * (B) SETUP
 * ========================================================= */

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Start...");

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  if (checkI2C(AS5600_ADDR) && as5600.begin()) {
    as5600Available = true;
    Serial.println("AS5600 gefunden");
  } else {
    Serial.println("AS5600 NICHT gefunden");
  }

  // Motor
  setupFan();

  // phyphox starten
  PhyphoxBLE::start();
  PhyphoxBLE::configHandler = &receivedData;

  // -------- Experiment --------
  PhyphoxBleExperiment experiment;
  experiment.setTitle("Motor + Encoder");
  experiment.setCategory("ESP32");
  experiment.setDescription("Steuerung + Messung");

  // ===== View 1: Steuerung =====
  PhyphoxBleExperiment::View controlView;
  controlView.setLabel("Steuerung");

  // Start (als Schieberegler 0..1)
  PhyphoxBleExperiment::Slider startSlider;
  startSlider.setLabel("Start");
  startSlider.setMin(0);
  startSlider.setMax(1);
  startSlider.setStep(1);
  startSlider.setValue(0);
  startSlider.setChannel(1);

  // PWM (als Schieberegler 0..100)
  PhyphoxBleExperiment::Slider pwmSlider;
  pwmSlider.setLabel("PWM (%)");
  pwmSlider.setUnit("%");
  pwmSlider.setMin(0);
  pwmSlider.setMax(100);
  pwmSlider.setStep(1);
  pwmSlider.setValue(0);
  pwmSlider.setChannel(2);

  controlView.addElement(startSlider);
  controlView.addElement(pwmSlider);

  experiment.addView(controlView);

  // ===== View 2: Live =====
  PhyphoxBleExperiment::View liveView;
  liveView.setLabel("Live");

  PhyphoxBleExperiment::Value angleVal;
  angleVal.setLabel("Winkel");
  angleVal.setUnit("deg");
  angleVal.setChannel(1);

  PhyphoxBleExperiment::Value pwmVal;
  pwmVal.setLabel("PWM Ist");
  pwmVal.setUnit("%");
  pwmVal.setChannel(2);

  liveView.addElement(angleVal);
  liveView.addElement(pwmVal);

  experiment.addView(liveView);

  // ===== View 3: Graph =====
  PhyphoxBleExperiment::View graphView;
  graphView.setLabel("Graph");

  PhyphoxBleExperiment::Graph graph;
  graph.setLabel("PWM vs Winkel");
  graph.setLabelX("Winkel");
  graph.setLabelY("PWM");
  graph.setUnitX("deg");
  graph.setUnitY("%");
  graph.setChannel(1, 2);

  graphView.addElement(graph);
  experiment.addView(graphView);

  // Experiment starten
  PhyphoxBLE::addExperiment(experiment);

  Serial.println("phyphox bereit");
}


/* =========================================================
 * (C) LOOP
 * ========================================================= */

void loop() {
  PhyphoxBLE::poll();

  if (millis() - lastSend > 100) {
    lastSend = millis();

    float angle = readAngle();
    float pwm = (float)currentPercent;

    // Daten an phyphox senden
    PhyphoxBLE::write(angle, pwm);

    Serial.print("Winkel: ");
    Serial.print(angle);
    Serial.print(" | PWM: ");
    Serial.println(pwm);
  }
}
