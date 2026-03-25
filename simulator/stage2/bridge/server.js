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
  lastTickMs: Date.now(),
  mappings: {
    inputs: [{ name: 'motorButton', channel: 1 }, { name: 'motorSlider', channel: 2 }],
    outputs: [{ name: 'weight', channel: 1 }, { name: 'pulseCounter', channel: 2 }, { name: 'current', channel: 3 }, { name: 'voltage', channel: 4 }, { name: 'motorCmd', channel: 5 }]
  }
};

function uniqOrdered(arr) {
  const seen = new Set();
  const out = [];
  for (const a of arr) {
    if (!seen.has(a)) {
      seen.add(a);
      out.push(a);
    }
  }
  return out;
}

function splitArgs(argString) {
  return argString
    .split(',')
    .map((s) => s.trim())
    .filter(Boolean)
    .map((s) => s.replace(/^&/, ''));
}

function parseIno(code) {
  const readVars = [];
  const writeVars = [];
  const readRegex = /PhyphoxBLE::read\s*\(([^)]*)\)\s*;/g;
  const writeRegex = /PhyphoxBLE::write\s*\(([^)]*)\)\s*;/g;

  let m;
  while ((m = readRegex.exec(code)) !== null) readVars.push(...splitArgs(m[1]));
  while ((m = writeRegex.exec(code)) !== null) writeVars.push(...splitArgs(m[1]));

  return {
    inputs: uniqOrdered(readVars).slice(0, 5).map((name, i) => ({ name, channel: i + 1 })),
    outputs: uniqOrdered(writeVars).slice(0, 5).map((name, i) => ({ name, channel: i + 1 }))
  };
}

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
    ${state.mappings.outputs.map((o) => `<container size="0" static="false">CH${o.channel}</container>`).join('\n    ')}
    ${state.mappings.inputs.map((i) => `<container size="0" static="false">CB${i.channel}</container>`).join('\n    ')}
  </data-containers>
  <analysis></analysis>
  <views>
    <view label="Bridge Data">
      ${state.mappings.outputs.map((o) => `<value label="${o.name}"><input>CH${o.channel}</input></value>`).join('\n      ')}
    </view>
    <view label="Read Inputs">
      ${state.mappings.inputs.map((i) => `<value label="${i.name}"><input>CB${i.channel}</input></value>`).join('\n      ')}
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
    motorEnabled: state.motorEnabled,
    mappings: state.mappings
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
    } else if (data.type === 'setInoCode') {
      const parsed = parseIno(String(data.code || ''));
      state.mappings = parsed;
      for (let i = 1; i <= 5; i++) {
        state.cb[i] = 0;
        state.ch[i] = 0;
      }
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
