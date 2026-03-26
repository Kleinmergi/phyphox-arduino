const $ = (id) => document.getElementById(id);

const state = {
  inputs: [],  // [{name, channel, value}]
  outputs: [], // [{name, channel, value}]
};

function log(msg) {
  const ts = new Date().toLocaleTimeString();
  $('log').textContent = `[${ts}] ${msg}\n` + $('log').textContent;
}

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
  while ((m = readRegex.exec(code)) !== null) {
    readVars.push(...splitArgs(m[1]));
  }
  while ((m = writeRegex.exec(code)) !== null) {
    writeVars.push(...splitArgs(m[1]));
  }

  return {
    readVars: uniqOrdered(readVars),
    writeVars: uniqOrdered(writeVars),
  };
}

function render() {
  const rx = $('rxFields');
  rx.innerHTML = '';

  if (state.inputs.length === 0) {
    rx.innerHTML = '<p>Keine <code>read(...)</code>-Eingänge erkannt.</p>';
  } else {
    state.inputs.forEach((entry) => {
      const wrap = document.createElement('div');
      wrap.innerHTML = `
        <label>CB${entry.channel} -> ${entry.name}
          <input type="number" step="0.01" data-kind="input" data-channel="${entry.channel}" value="${entry.value}" />
        </label>
      `;
      rx.appendChild(wrap);
    });
  }

  const tx = $('txTable');
  tx.innerHTML = '';
  if (state.outputs.length === 0) {
    tx.innerHTML = '<tr><td colspan="3">Keine <code>write(...)</code>-Ausgänge erkannt.</td></tr>';
  } else {
    state.outputs.forEach((entry) => {
      const tr = document.createElement('tr');
      tr.innerHTML = `
        <td>CH${entry.channel}</td>
        <td>${entry.name}</td>
        <td><input type="number" step="0.01" data-kind="output" data-channel="${entry.channel}" value="${entry.value}" /></td>
      `;
      tx.appendChild(tr);
    });
  }

  const cbMap = Object.fromEntries(state.inputs.map((i) => [`CB${i.channel}`, i.value]));
  const chMap = Object.fromEntries(state.outputs.map((o) => [`CH${o.channel}`, o.value]));
  $('jsonOut').textContent = JSON.stringify({ inputs: cbMap, outputs: chMap }, null, 2);

  document.querySelectorAll('input[data-kind]').forEach((inputEl) => {
    inputEl.addEventListener('input', () => {
      const channel = Number(inputEl.dataset.channel);
      const value = Number(inputEl.value) || 0;
      if (inputEl.dataset.kind === 'input') {
        const item = state.inputs.find((x) => x.channel === channel);
        if (item) item.value = value;
      } else {
        const item = state.outputs.find((x) => x.channel === channel);
        if (item) item.value = value;
      }
      render();
    });
  });
}

function buildPhyphoxXml() {
  const chCount = state.outputs.length;
  const cbCount = state.inputs.length;

  const containers = [
    '<container size="0" static="false">CH0</container>',
    ...state.outputs.map((o) => `<container size="0" static="false">CH${o.channel}</container>`),
    ...state.inputs.map((i) => `<container size="0" static="false">CB${i.channel}</container>`)
  ].join('\n    ');

  const valuesOut = state.outputs
    .map((o) => `<value label="${o.name}"><input>CH${o.channel}</input></value>`)
    .join('\n      ');

  const valuesIn = state.inputs
    .map((i) => `<value label="${i.name}"><input>CB${i.channel}</input></value>`)
    .join('\n      ');

  return `<?xml version="1.0" encoding="UTF-8"?>
<phyphox version="1.19">
  <title>INO Parsed Export</title>
  <category>Simulator</category>
  <description>Automatisch aus INO read/write Signaturen generiert.</description>
  <data-containers>
    ${containers}
  </data-containers>
  <analysis></analysis>
  <views>
    <view label="Write Outputs">
      ${valuesOut || '<info label="Keine write()-Ausgaenge erkannt." />'}
    </view>
    <view label="Read Inputs">
      ${valuesIn || '<info label="Keine read()-Eingaenge erkannt." />'}
    </view>
  </views>
  <export>
    <set name="parsed">
      ${state.outputs.map((o) => `<data name="${o.name}">CH${o.channel}</data>`).join('\n      ')}
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

$('btnAnalyze').addEventListener('click', () => {
  const code = $('inoCode').value;
  const parsed = parseIno(code);

  state.inputs = parsed.readVars.map((name, idx) => ({ name, channel: idx + 1, value: 0 }));
  state.outputs = parsed.writeVars.map((name, idx) => ({ name, channel: idx + 1, value: 0 }));

  log(`Analyse fertig: ${state.inputs.length} read-Variablen, ${state.outputs.length} write-Variablen.`);
  render();
});

$('btnExport').addEventListener('click', () => {
  const xml = buildPhyphoxXml();
  download('ino-parsed-simulator.phyphox', xml);
  log('Export erzeugt: ino-parsed-simulator.phyphox');
});

// initial render with default code
$('btnAnalyze').click();
