# Stage-1 INO Simulator (ohne BLE)

Diese Version ist auf deinen Workflow ausgerichtet:

1. INO-Code einfügen
2. `PhyphoxBLE::read(...)` / `PhyphoxBLE::write(...)` automatisch erkennen
3. erkannte Eingänge als Eingabefelder darstellen
4. erkannte Ausgänge als Text-/Wertausgabe darstellen
5. `.phyphox` exportieren

## Start lokal

```bash
cd simulator/stage1
python3 -m http.server 8080
```

Dann im Browser aufrufen: `http://localhost:8080`

## Parsing-Regeln

- read-Erkennung: `PhyphoxBLE::read(a, b, c);`
- write-Erkennung: `PhyphoxBLE::write(x, y, z);`
- Kanäle werden in Reihenfolge zugewiesen:
  - `read` -> `CB1..CBn`
  - `write` -> `CH1..CHn`

## Einschränkung

Es wird **kein C++ ausgeführt**. Der Simulator erkennt Signaturen und stellt Felder/Ausgaben bereit.
Die echte BLE-Kommunikation folgt in Stage-2.
