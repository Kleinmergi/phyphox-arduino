const state = {
  cb: {1:0,2:0,3:0,4:0,5:0},
  ch: {1:0,2:0,3:0,4:0,5:0},
  pulseCounter: 0,
  motorEnabled: false,
  lastCb1: 0,
  lastTickMs: performance.now()
};

const el = (id) => document.getElementById(id);
const logBox = el('log');
const channelTable = el('channelTable');

function log(msg) {
  const ts = new Date().toLocaleTimeString();
  logBox.textContent = `[${ts}] ${msg}\n` + logBox.textContent;
}

function num(id) {
  return parseFloat(el(id).value) || 0;
}

function syncInputsToState() {
  state.cb[1] = num('cb1');
  state.cb[2] = num('cb2');
  state.cb[3] = num('cb3');
  state.cb[4] = num('cb4');
  state.cb[5] = num('cb5');
}

function updateMotorLatchFromButton() {
  if (state.cb[1] > 0.5 && state.lastCb1 <= 0.5) {
    state.motorEnabled = !state.motorEnabled;
    log(`Motor toggle via CB1 -> ${state.motorEnabled ? 'ON' : 'OFF'}`);
  }
  state.lastCb1 = state.cb[1];
}

function updateChannels() {
  const now = performance.now();
  const dt = Math.max(0, (now - state.lastTickMs) / 1000);
  state.lastTickMs = now;

  const weight = num('weight');
  const pulseRate = num('pulseRate');
  const current = num('current');
  const voltage = num('voltage');

  state.pulseCounter += pulseRate * dt;

  const motorCmdOverride = num('motorCmd');
  const sliderCmd = state.cb[2];
  let motorCmd = motorCmdOverride > 0 ? motorCmdOverride : sliderCmd;
  motorCmd = Math.min(3.3, Math.max(0, motorCmd));
  if (!state.motorEnabled) motorCmd = 0;

  state.ch[1] = weight;
  state.ch[2] = state.pulseCounter;
  state.ch[3] = current;
  state.ch[4] = voltage;
  state.ch[5] = motorCmd;
}

function renderChannels() {
  const rows = [
    [1, 'HX711 Gewicht [g]', state.ch[1]],
    [2, 'Lichtschranke Pulse [count]', state.ch[2]],
    [3, 'INA219 Strom [mA]', state.ch[3]],
    [4, 'INA219 Spannung [V]', state.ch[4]],
    [5, 'Motor Sollspannung [V]', state.ch[5]],
  ].map(([ch, label, value]) => `<tr><td>CH${ch}</td><td>${label}</td><td>${value.toFixed(3)}</td></tr>`).join('');
  channelTable.innerHTML = rows;
}

function tick() {
  syncInputsToState();
  updateMotorLatchFromButton();
  updateChannels();
  renderChannels();
  requestAnimationFrame(tick);
}

function buildPhyphoxXml() {
  return `<?xml version="1.0" encoding="UTF-8"?>
<phyphox version="1.19">
  <title>Stage-1 Simulator Export</title>
  <category>Simulator</category>
  <description>Exportierte phyphox-Datei fuer Stage-1 Simulator (ohne BLE).</description>
  <data-containers>
    <container size="0" static="false">CH0</container>
    <container size="0" static="false">CH1</container>
    <container size="0" static="false">CH2</container>
    <container size="0" static="false">CH3</container>
    <container size="0" static="false">CH4</container>
    <container size="0" static="false">CH5</container>
    <container size="0" static="false">CB1</container>
    <container size="0" static="false">CB2</container>
    <container size="0" static="false">CB3</container>
    <container size="0" static="false">CB4</container>
    <container size="0" static="false">CB5</container>
  </data-containers>
  <analysis></analysis>
  <views>
    <view label="Sim Data">
      <value label="Gewicht" unit="g"><input>CH1</input></value>
      <value label="Pulse" unit="count"><input>CH2</input></value>
      <value label="Strom" unit="mA"><input>CH3</input></value>
      <value label="Spannung" unit="V"><input>CH4</input></value>
      <value label="Motor Soll" unit="V"><input>CH5</input></value>
      <graph label="Strom ueber Zeit" labelX="t" labelY="I" unitX="s" unitY="mA">
        <input axis="x">CH0</input>
        <input axis="y">CH3</input>
      </graph>
    </view>
    <view label="RX Buffers">
      <value label="CB1"><input>CB1</input></value>
      <value label="CB2"><input>CB2</input></value>
      <value label="CB3"><input>CB3</input></value>
      <value label="CB4"><input>CB4</input></value>
      <value label="CB5"><input>CB5</input></value>
    </view>
  </views>
  <export>
    <set name="simulator">
      <data name="weight">CH1</data>
      <data name="pulse">CH2</data>
      <data name="current">CH3</data>
      <data name="voltage">CH4</data>
      <data name="motor_cmd">CH5</data>
    </set>
  </export>
</phyphox>`;
}

function download(name, content) {
  const blob = new Blob([content], { type: 'application/xml;charset=utf-8' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = name;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

el('btnToggle').addEventListener('click', () => {
  el('cb1').value = '1';
  setTimeout(() => {
    el('cb1').value = '0';
  }, 120);
});

el('btnExport').addEventListener('click', () => {
  const xml = buildPhyphoxXml();
  download('stage1-simulator.phyphox', xml);
  log('Export: stage1-simulator.phyphox erzeugt');
});

el('btnSnapshot').addEventListener('click', () => {
  log(`SNAPSHOT cb=[${Object.values(state.cb).map(v => v.toFixed(2)).join(', ')}] ch=[${Object.values(state.ch).map(v => v.toFixed(2)).join(', ')}]`);
});

log('Simulator gestartet');
requestAnimationFrame(tick);
