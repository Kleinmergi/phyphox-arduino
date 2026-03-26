#include <phyphoxBle.h>
#include <Wire.h>
#include <AS5600.h>
#include <Adafruit_BMP280.h>

// -------------------- Hardware --------------------
const int LED_PIN = 3; // gewuenschter Port 3
AS5600 encoder;
Adafruit_BMP280 bmp; // I2C

// -------------------- Control / State --------------------
float buttonValue = 0.0f;      // CB1: Oszillation toggeln
float sliderBrightness = 128;  // CB2: Helligkeit 0..255
float lastButtonValue = 0.0f;
bool oscillateEnabled = false;

// -------------------- Measurements --------------------
float temperatureC = 0.0f;   // CH1
float pressurehPa = 0.0f;    // CH2
float angleDeg = 0.0f;       // CH3
float ledBrightness = 0.0f;  // CH4

unsigned long lastPrintMs = 0;

void onConfigUpdate()
{
  // Reihenfolge entspricht den gemappten UI-Ausgaengen
  PhyphoxBLE::read(buttonValue, sliderBrightness);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);

  // AS5600 initialisieren
  if (!encoder.begin())
  {
    Serial.println("WARN: AS5600 nicht gefunden / init fehlgeschlagen.");
  }

  // BMP280 initialisieren (Standard-I2C-Adresse 0x76 oder 0x77)
  bool bmpOk = bmp.begin(0x76);
  if (!bmpOk)
  {
    bmpOk = bmp.begin(0x77);
  }
  if (!bmpOk)
  {
    Serial.println("WARN: BMP280 nicht gefunden.");
  }

  // -------------------- phyphox Experiment --------------------
  PhyphoxBLE::start("as5600-bmp280-led");

  PhyphoxBleExperiment exp;
  exp.setTitle("AS5600 + BMP280 + LED");
  exp.setCategory("Arduino Experiments");
  exp.setDescription("Druck/Temperatur, Winkel und LED-Helligkeit mit UI-Controls.");

  // View 1: Druck auf Temperatur
  PhyphoxBleExperiment::View viewPressureTemp;
  viewPressureTemp.setLabel("Druck-Temp");
  PhyphoxBleExperiment::Graph graphPressureTemp;
  graphPressureTemp.setLabel("Druck ueber Temperatur");
  graphPressureTemp.setLabelX("Temperatur");
  graphPressureTemp.setUnitX("degC");
  graphPressureTemp.setLabelY("Druck");
  graphPressureTemp.setUnitY("hPa");
  graphPressureTemp.setChannel(1, 2); // x=CH1 temp, y=CH2 pressure
  viewPressureTemp.addElement(graphPressureTemp);

  // View 2: Absoluter Winkel
  PhyphoxBleExperiment::View viewAbsAngle;
  viewAbsAngle.setLabel("Winkel abs");
  PhyphoxBleExperiment::Value valueAngle;
  valueAngle.setLabel("Winkel");
  valueAngle.setUnit("deg");
  valueAngle.setPrecision(2);
  valueAngle.setChannel(3); // CH3 angle
  viewAbsAngle.addElement(valueAngle);

  // View 3: Winkel auf Zeit
  PhyphoxBleExperiment::View viewAngleTime;
  viewAngleTime.setLabel("Winkel-Zeit");
  PhyphoxBleExperiment::Graph graphAngleTime;
  graphAngleTime.setLabel("Winkel ueber Zeit");
  graphAngleTime.setLabelX("Zeit");
  graphAngleTime.setUnitX("s");
  graphAngleTime.setLabelY("Winkel");
  graphAngleTime.setUnitY("deg");
  graphAngleTime.setChannel(0, 3); // x=time, y=angle
  viewAngleTime.addElement(graphAngleTime);

  // View 4: Helligkeit + Controls
  PhyphoxBleExperiment::View viewLed;
  viewLed.setLabel("LED Control");

  PhyphoxBleExperiment::Button buttonOsc;
  buttonOsc.setLabel("Oszillation EIN/AUS");
  buttonOsc.setOutputChannel(1); // CB1
  buttonOsc.setValue(1.0);

  PhyphoxBleExperiment::Slider sliderBright;
  sliderBright.setLabel("Helligkeit (0..255)");
  sliderBright.setMin(0);
  sliderBright.setMax(255);
  sliderBright.setStep(1);
  sliderBright.setValue(128);
  sliderBright.setOutputChannel(2); // CB2

  PhyphoxBleExperiment::Value valueLed;
  valueLed.setLabel("LED Ist");
  valueLed.setPrecision(0);
  valueLed.setChannel(4); // CH4 brightness

  viewLed.addElement(buttonOsc);
  viewLed.addElement(sliderBright);
  viewLed.addElement(valueLed);

  // Views an Experiment haengen
  exp.addView(viewPressureTemp);
  exp.addView(viewAbsAngle);
  exp.addView(viewAngleTime);
  exp.addView(viewLed);

  PhyphoxBLE::addExperiment(exp);
  // PhyphoxBLE::printXML(&Serial);

  PhyphoxBLE::configHandler = &onConfigUpdate;
}

void loop()
{
  // Button-Flankenerkennung -> Oszillation toggeln
  if (buttonValue > 0.5f && lastButtonValue <= 0.5f)
  {
    oscillateEnabled = !oscillateEnabled;
  }
  lastButtonValue = buttonValue;

  // Sensordaten lesen
  float rawAngle = encoder.rawAngle(); // 0..4095
  angleDeg = rawAngle * 360.0f / 4096.0f;

  temperatureC = bmp.readTemperature();
  pressurehPa = bmp.readPressure() / 100.0f;

  // LED-Helligkeit
  float target = sliderBrightness;
  if (target < 0) target = 0;
  if (target > 255) target = 255;

  if (oscillateEnabled)
  {
    float t = millis() / 1000.0f;
    float osc = 0.5f * (1.0f + sinf(t * 2.0f * PI * 0.5f)); // 0..1, 0.5Hz
    ledBrightness = target * osc;
  }
  else
  {
    ledBrightness = target;
  }

  analogWrite(LED_PIN, (int)ledBrightness);

  // CH1..CH4 senden: Temp, Druck, Winkel, LED
  PhyphoxBLE::write(temperatureC, pressurehPa, angleDeg, ledBrightness);

  if (millis() - lastPrintMs > 250)
  {
    lastPrintMs = millis();
    Serial.print("temp=");
    Serial.print(temperatureC, 2);
    Serial.print("C pressure=");
    Serial.print(pressurehPa, 2);
    Serial.print("hPa angle=");
    Serial.print(angleDeg, 2);
    Serial.print("deg led=");
    Serial.print(ledBrightness, 0);
    Serial.print(" osc=");
    Serial.println(oscillateEnabled ? "ON" : "OFF");
  }

  PhyphoxBLE::poll();
  delay(40);
}
