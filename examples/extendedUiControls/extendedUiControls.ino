#include <phyphoxBle.h>

float startStopValue = 0.0;
float pwmValue = 0.0;
float motorEnabled = 0.0;
float modeValue = 0.0;

void receivedUiValues()
{
  // Reihenfolge entspricht den gemappten CB1..CB4 Ausgabekanaelen der UI-Elemente.
  PhyphoxBLE::read(startStopValue, pwmValue, motorEnabled, modeValue);
}

void setup()
{
  Serial.begin(115200);
  PhyphoxBLE::start("extended-ui");

  PhyphoxBleExperiment experiment;
  experiment.setTitle("Extended UI Controls");
  experiment.setCategory("Arduino Experiments");
  experiment.setDescription("Demonstriert button, slider, toggle, dropdown und info inkl. read().");

  PhyphoxBleExperiment::View controlsView;
  controlsView.setLabel("Controls");

  PhyphoxBleExperiment::Button startButton;
  startButton.setLabel("Start/Stop");
  startButton.setOutputChannel(1);
  startButton.setValue(1.0);

  PhyphoxBleExperiment::Slider pwmSlider;
  pwmSlider.setLabel("PWM");
  pwmSlider.setMin(0);
  pwmSlider.setMax(255);
  pwmSlider.setStep(1);
  pwmSlider.setValue(120);
  pwmSlider.setOutputChannel(2);

  PhyphoxBleExperiment::Toggle motorToggle;
  motorToggle.setLabel("Motor aktiv");
  motorToggle.setDefault(false);
  motorToggle.setOutputChannel(3);

  PhyphoxBleExperiment::Dropdown modeDropdown;
  modeDropdown.setLabel("Modus");
  modeDropdown.setOutputChannel(4);
  modeDropdown.addOption("Manuell", 0);
  modeDropdown.addOption("Rampe", 1);
  modeDropdown.addOption("Sinus", 2);

  PhyphoxBleExperiment::Info infoText;
  infoText.setInfo("Button startet, Slider setzt PWM, Toggle aktiviert, Dropdown waehlt Modus.");

  PhyphoxBleExperiment::Value pwmEcho;
  pwmEcho.setLabel("PWM aktuell");
  pwmEcho.setPrecision(0);
  pwmEcho.setChannel(1);

  PhyphoxBleExperiment::Graph graph;
  graph.setLabel("PWM Verlauf");
  graph.setLabelX("t");
  graph.setLabelY("PWM");
  graph.setUnitX("s");
  graph.setUnitY("");
  graph.setChannel(0, 1);

  controlsView.addElement(infoText);
  controlsView.addElement(startButton);
  controlsView.addElement(pwmSlider);
  controlsView.addElement(motorToggle);
  controlsView.addElement(modeDropdown);
  controlsView.addElement(pwmEcho);
  controlsView.addElement(graph);

  experiment.addView(controlsView);
  PhyphoxBLE::addExperiment(experiment);

  PhyphoxBLE::configHandler = &receivedUiValues;
}

void loop()
{
  // PWM-Wert zur Anzeige in Channel 1 (Graph + Value).
  PhyphoxBLE::write(pwmValue);

  // Optionales Monitoring auf der seriellen Konsole.
  Serial.print("button=");
  Serial.print(startStopValue);
  Serial.print(", pwm=");
  Serial.print(pwmValue);
  Serial.print(", toggle=");
  Serial.print(motorEnabled);
  Serial.print(", mode=");
  Serial.println(modeValue);

  PhyphoxBLE::poll();
  delay(100);
}
