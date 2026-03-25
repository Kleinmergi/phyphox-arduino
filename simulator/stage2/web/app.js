const $ = (id) => document.getElementById(id);
const statusEl = $('status');
const outEl = $('out');
const logEl = $('log');

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
  ['cb1','cb2','cb3','cb4','cb5'].forEach((id, i) => { $(id).value = s.cb[i+1]; });
  Object.keys(s.sensors).forEach((k) => { if ($(k)) $(k).value = s.sensors[k]; });
  outEl.innerHTML = Object.entries(s.ch).map(([k,v]) => `<tr><td>CH${k}</td><td>${Number(v).toFixed(3)}</td></tr>`).join('');
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

bindInput('cb1', (v) => send('setCB', { index: 1, value: v }));
bindInput('cb2', (v) => send('setCB', { index: 2, value: v }));
bindInput('cb3', (v) => send('setCB', { index: 3, value: v }));
bindInput('cb4', (v) => send('setCB', { index: 4, value: v }));
bindInput('cb5', (v) => send('setCB', { index: 5, value: v }));

['weight','pulseRate','current','voltage','motorCmdOverride'].forEach((key) => {
  bindInput(key, (v) => send('setSensor', { key, value: v }));
});

$('impulse').addEventListener('click', () => send('buttonImpulse'));
$('export').addEventListener('click', () => send('exportPhyphox'));
