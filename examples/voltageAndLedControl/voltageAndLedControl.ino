#include <phyphoxBle.h>

// Beispiel fuer ESP32: Slider steuert eine PWM-Spannung, Button toggelt LED an/aus.
const int voltagePin = 25; // PWM-/DAC-faehiger Pin beim ESP32
const int ledPin = LED_BUILTIN;

float buttonValue = 0.0;
float sliderVoltage = 0.0;
float lastButtonValue = 0.0;
bool ledEnabled = false;

void onConfigUpdate()
{
  // Reihenfolge: CB1=Button, CB2=Slider
  PhyphoxBLE::read(buttonValue, sliderVoltage);
}

void setup()
{
  Serial.begin(115200);
  pinMode(voltagePin, OUTPUT);
  pinMode(ledPin, OUTPUT);

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

  controlView.addElement(info);
  controlView.addElement(ledButton);
  controlView.addElement(voltageSlider);
  controlView.addElement(voltageValue);

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
  PhyphoxBLE::write(limitedVoltage);

  Serial.print("LED=");
  Serial.print(ledEnabled ? "ON" : "OFF");
  Serial.print("  Voltage=");
  Serial.println(limitedVoltage, 2);

  PhyphoxBLE::poll();
  delay(50);
}
