# Stage-2 Bridge (PWA + Host-Bridge Skeleton)

Stage-2 baut auf Stage-1 auf und trennt:

- **Web/PWA UI** (im Browser)
- **Host-Bridge Prozess** (Node.js), spaeter mit BLE-Peripheral-Anbindung

## Enthalten

- `bridge/server.js`: HTTP + WebSocket + Zustandsmodell + `.phyphox`-Export
- `web/`: PWA-faehiges Frontend mit Live-Controls fuer CB/Sensorwerte und CH-Ausgabe
- `package.json`: Startskripte

## Start (Mock-Modus)

```bash
cd simulator/stage2
npm install
npm run start:mock
```

Dann im Browser aufrufen:

`http://localhost:8090`

## Aktueller Stand

- Voll funktionsfaehiger Stage-2 Datenfluss **ohne echte BLE-Kopplung**
- BLE-Anbindung ist als naechster Plug-in-Schritt vorgesehen (Bridge-Skeleton bleibt stabil)
