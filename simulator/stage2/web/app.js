const $ = (id) => document.getElementById(id);
const statusEl = $('status');
const outEl = $('out');
const logEl = $('log');
const cbFieldsEl = $('cbFields');

function log(msg) {
  logEl.textContent = `[${new Date().toLocaleTimeString()}] ${msg}\n` + logEl.textContent;
}

function send(type, payload = {}) {
  if (!window.ws || ws.readyState !== 1) return;
  ws.send(JSON.stringify({ type, ...payload }));
}

function bindInput(id, fn) {
  $(id).addEventListener('input', () => fn($(id).value));
}

function renderState(s) {
  cbFieldsEl.innerHTML = (s.mappings?.inputs || []).map((inp) => `
    <label>CB${inp.channel} ${inp.name}
      <input id="cb${inp.channel}" type="number" step="0.01" value="${s.cb[inp.channel] ?? 0}" />
    </label>
  `).join('');

  (s.mappings?.inputs || []).forEach((inp) => {
    $(`cb${inp.channel}`).addEventListener('input', (e) => send('setCB', { index: inp.channel, value: e.target.value }));
  });

  Object.keys(s.sensors).forEach((k) => { if ($(k)) $(k).value = s.sensors[k]; });
  const outputNames = new Map((s.mappings?.outputs || []).map((o) => [String(o.channel), o.name]));
  outEl.innerHTML = Object.entries(s.ch).map(([k,v]) => `<tr><td>CH${k}</td><td>${outputNames.get(String(k)) || '-'}</td><td>${Number(v).toFixed(3)}</td></tr>`).join('');
  statusEl.textContent = `verbunden (${s.mock ? 'mock' : 'bridge'}) motor=${s.motorEnabled ? 'ON' : 'OFF'}`;
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

window.ws = new WebSocket(`ws://${location.host}`);
ws.onopen = () => log('websocket connected');
ws.onclose = () => { statusEl.textContent = 'disconnected'; log('websocket disconnected'); };
ws.onmessage = (event) => {
  const msg = JSON.parse(event.data);
  if (msg.type === 'state') renderState(msg);
  if (msg.type === 'phyphoxFile') {
    download(msg.filename, msg.content);
    log(`downloaded ${msg.filename}`);
  }
};

['weight','pulseRate','current','voltage','motorCmdOverride'].forEach((key) => {
  bindInput(key, (v) => send('setSensor', { key, value: v }));
});

$('analyze').addEventListener('click', () => {
  send('setInoCode', { code: $('inoCode').value });
});

$('impulse').addEventListener('click', () => send('buttonImpulse'));
$('export').addEventListener('click', () => send('exportPhyphox'));
