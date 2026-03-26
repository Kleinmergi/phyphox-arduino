#include <phyphoxBle.h>

// Beispiel fuer ESP32: Slider steuert eine PWM-Spannung, Button toggelt LED an/aus.
// Falls dein Board andere Pins braucht, diese beiden Werte anpassen:
const int voltagePin = 25; // Viele ESP32-Boards: GPIO25 ist gut messbar (PWM/DAC-faehig)
#ifdef LED_BUILTIN
const int ledPin = LED_BUILTIN;
#else
const int ledPin = 2; // Fallback fuer Boards ohne LED_BUILTIN-Definition
#endif

float buttonValue = 0.0;
float sliderVoltage = 0.0;
float lastButtonValue = 0.0;
bool ledEnabled = false;
uint32_t configUpdateCount = 0;
unsigned long lastDebugPrintMs = 0;

void onConfigUpdate()
{
  // Reihenfolge: CB1=Button, CB2=Slider
  PhyphoxBLE::read(buttonValue, sliderVoltage);
  configUpdateCount++;
}

void setup()
{
  Serial.begin(115200);
  pinMode(voltagePin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  PhyphoxBLE::start("voltage-led-control");

  PhyphoxBleExperiment experiment;
  experiment.setTitle("Voltage + LED Control");
  experiment.setCategory("Arduino Experiments");
  experiment.setDescription("Slider steuert PWM-Spannung, Button toggelt LED an/aus.");

  PhyphoxBleExperiment::View controlView;
  controlView.setLabel("Control");

  PhyphoxBleExperiment::Info info;
  info.setInfo("Button toggelt LED. Slider setzt Ausgangsspannung (0..3.3V, via PWM). ");

  PhyphoxBleExperiment::Button ledButton;
  ledButton.setLabel("LED toggle");
  ledButton.setOutputChannel(1);
  ledButton.setValue(1.0);

  PhyphoxBleExperiment::Slider voltageSlider;
  voltageSlider.setLabel("Output voltage");
  voltageSlider.setMin(0.0);
  voltageSlider.setMax(3.3);
  voltageSlider.setStep(0.05);
  voltageSlider.setValue(0.0);
  voltageSlider.setOutputChannel(2);

  PhyphoxBleExperiment::Value voltageValue;
  voltageValue.setLabel("Ist-Spannung");
  voltageValue.setUnit("V");
  voltageValue.setPrecision(2);
  voltageValue.setChannel(1);

  PhyphoxBleExperiment::Value sliderDebugValue;
  sliderDebugValue.setLabel("Slider roh");
  sliderDebugValue.setPrecision(2);
  sliderDebugValue.setChannel(2);

  controlView.addElement(info);
  controlView.addElement(ledButton);
  controlView.addElement(voltageSlider);
  controlView.addElement(voltageValue);
  controlView.addElement(sliderDebugValue);

  experiment.addView(controlView);
  PhyphoxBLE::addExperiment(experiment);
  //PhyphoxBLE::printXML(&Serial);

  PhyphoxBLE::configHandler = &onConfigUpdate;
}

void loop()
{
  // Rising edge am Button -> LED-Zustand toggeln
  if (buttonValue > 0.5 && lastButtonValue <= 0.5)
  {
    ledEnabled = !ledEnabled;
  }
  lastButtonValue = buttonValue;

  // Slider (0..3.3V) auf PWM (0..255) mappen
  float limitedVoltage = sliderVoltage;
  if (limitedVoltage < 0.0)
    limitedVoltage = 0.0;
  if (limitedVoltage > 3.3)
    limitedVoltage = 3.3;

  uint8_t pwm = (uint8_t)(limitedVoltage / 3.3f * 255.0f);
  analogWrite(voltagePin, pwm);
  digitalWrite(ledPin, ledEnabled ? HIGH : LOW);

  // Channel 1 -> Value in phyphox
  // Channel 2 -> Rohwert vom Slider fuer Debug-Anzeige
  PhyphoxBLE::write(limitedVoltage, sliderVoltage);

  unsigned long now = millis();
  if (now - lastDebugPrintMs >= 250)
  {
    lastDebugPrintMs = now;
    Serial.print("cfg#=");
    Serial.print(configUpdateCount);
    Serial.print("  btn=");
    Serial.print(buttonValue, 2);
    Serial.print("  sliderRaw=");
    Serial.print(sliderVoltage, 2);
    Serial.print("  limitedV=");
    Serial.print(limitedVoltage, 2);
    Serial.print("  pwm=");
    Serial.print((int)pwm);
    Serial.print("  led=");
    Serial.println(ledEnabled ? "ON" : "OFF");
  }

  PhyphoxBLE::poll();
  delay(50);
}
