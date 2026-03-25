# Stage-1 phyphox Simulator (ohne BLE)

Dieser Simulator ist die erste Ausbaustufe:

- Einstellen von simulierten Sensorwerten auf dem sendenden Geraet
- Anzeige der empfangenen phyphox-Werte (CB1..CB5)
- Anzeige der ausgehenden Messkanaele (CH1..CH5)
- Export einer `.phyphox`-Datei zur Nutzung auf einem anderen Geraet

## Start lokal

```bash
cd simulator/stage1
python3 -m http.server 8080
```

Dann im Browser aufrufen:

`http://localhost:8080`

## Mapping

- CB1: Motor-Button Trigger
- CB2: Motor-Slider (Spannung)
- CH1: Gewicht (HX711)
- CH2: Pulszaehler (Lichtschranke)
- CH3: Strom (INA219)
- CH4: Spannung (INA219)
- CH5: Motor-Sollspannung

## Hinweis

Dies ist bewusst **ohne BLE** implementiert. In Stufe 2 folgt eine BLE-Bridge/PWA-Integration.
