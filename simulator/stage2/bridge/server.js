const fs = require('fs');
const path = require('path');
const http = require('http');
const { WebSocketServer } = require('ws');

const PORT = process.env.PORT ? Number(process.env.PORT) : 8090;
const MOCK = process.env.STAGE2_MOCK === '1';

const state = {
  cb: { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 },
  ch: { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 },
  pulseCounter: 0,
  motorEnabled: false,
  lastCb1: 0,
  sensors: {
    weight: 250,
    pulseRate: 5,
    current: 120,
    voltage: 12,
    motorCmdOverride: 0
  },
  lastTickMs: Date.now()
};

function updateModel() {
  const now = Date.now();
  const dt = Math.max(0, (now - state.lastTickMs) / 1000);
  state.lastTickMs = now;

  if (state.cb[1] > 0.5 && state.lastCb1 <= 0.5) {
    state.motorEnabled = !state.motorEnabled;
  }
  state.lastCb1 = state.cb[1];

  state.pulseCounter += state.sensors.pulseRate * dt;

  let motorCmd = state.sensors.motorCmdOverride > 0 ? state.sensors.motorCmdOverride : state.cb[2];
  motorCmd = Math.min(3.3, Math.max(0, motorCmd));
  if (!state.motorEnabled) motorCmd = 0;

  state.ch[1] = state.sensors.weight;
  state.ch[2] = state.pulseCounter;
  state.ch[3] = state.sensors.current;
  state.ch[4] = state.sensors.voltage;
  state.ch[5] = motorCmd;
}

function buildPhyphoxXml() {
  return `<?xml version="1.0" encoding="UTF-8"?>
<phyphox version="1.19">
  <title>Stage-2 Bridge Export</title>
  <category>Simulator</category>
  <description>Bridge export (Stage-2).</description>
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
    <view label="Bridge Data">
      <value label="Gewicht" unit="g"><input>CH1</input></value>
      <value label="Pulse" unit="count"><input>CH2</input></value>
      <value label="Strom" unit="mA"><input>CH3</input></value>
      <value label="Spannung" unit="V"><input>CH4</input></value>
      <value label="Motor Soll" unit="V"><input>CH5</input></value>
    </view>
  </views>
</phyphox>`;
}

function toPayload() {
  return {
    type: 'state',
    mock: MOCK,
    cb: state.cb,
    ch: state.ch,
    sensors: state.sensors,
    motorEnabled: state.motorEnabled
  };
}

function send(ws, obj) {
  ws.send(JSON.stringify(obj));
}

const webRoot = path.join(__dirname, '..', 'web');
const server = http.createServer((req, res) => {
  let filePath = req.url === '/' ? '/index.html' : req.url;
  filePath = path.normalize(filePath).replace(/^\/+/, '');
  const abs = path.join(webRoot, filePath);

  if (!abs.startsWith(webRoot)) {
    res.writeHead(403);
    res.end('forbidden');
    return;
  }

  fs.readFile(abs, (err, data) => {
    if (err) {
      res.writeHead(404);
      res.end('not found');
      return;
    }
    const type = abs.endsWith('.html') ? 'text/html'
      : abs.endsWith('.js') ? 'application/javascript'
      : abs.endsWith('.json') ? 'application/json'
      : 'text/plain';
    res.writeHead(200, { 'Content-Type': type });
    res.end(data);
  });
});

const wss = new WebSocketServer({ server });

wss.on('connection', (ws) => {
  send(ws, toPayload());

  ws.on('message', (msg) => {
    let data;
    try {
      data = JSON.parse(msg.toString());
    } catch {
      send(ws, { type: 'error', message: 'invalid json' });
      return;
    }

    if (data.type === 'setCB' && data.index >= 1 && data.index <= 5) {
      state.cb[data.index] = Number(data.value) || 0;
    } else if (data.type === 'setSensor' && data.key in state.sensors) {
      state.sensors[data.key] = Number(data.value) || 0;
    } else if (data.type === 'buttonImpulse') {
      state.cb[1] = 1;
      setTimeout(() => { state.cb[1] = 0; }, 120);
    } else if (data.type === 'exportPhyphox') {
      send(ws, { type: 'phyphoxFile', filename: 'stage2-bridge.phyphox', content: buildPhyphoxXml() });
      return;
    }

    updateModel();
    send(ws, toPayload());
  });
});

setInterval(() => {
  updateModel();
  const payload = JSON.stringify(toPayload());
  wss.clients.forEach((client) => {
    if (client.readyState === 1) client.send(payload);
  });
}, 200);

server.listen(PORT, () => {
  console.log(`Stage-2 bridge listening on http://localhost:${PORT}`);
  console.log(`Mode: ${MOCK ? 'MOCK' : 'BRIDGE-SKELETON'} (BLE integration to be plugged in next).`);
});
