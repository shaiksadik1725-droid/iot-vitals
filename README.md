# IoT Patient Vital Monitoring System

An ESP32-based health-monitoring prototype that combines multiple sensors with Blynk for remote vital-sign monitoring and abnormal-condition alerts.

## Monitored Parameters
- Heart-rate-related optical readings
- SpO2-oriented monitoring
- Body temperature
- Respiratory / flex sensor activity
- Motion and impact sensing for fall detection

## Hardware / Software
ESP32, MAX30105, MLX90614, MPU-based motion sensing, flex sensor, LEDs, Blynk, Arduino/C++.

## Structure
```text
iot-vitals/
├── code/PATIENT.ino
├── PATIENTBLOCK.drawio
├── PATIENT_MONITORING.drawio
└── patient.fzz
```

## System Flow
Sensors → ESP32 → local condition checks → LED indicators / alerts → Blynk dashboard.

## Run
Open `code/PATIENT.ino`, install the required libraries, configure your own Wi-Fi and Blynk credentials, then upload to the ESP32.

## Important Note
This is an academic engineering prototype and not a certified medical device. Sensor values and thresholds require proper calibration and validation before any real clinical use.

## Future Improvements
- Replace simulated / heuristic values with validated signal-processing algorithms
- Add secure patient data storage
- Add battery monitoring
- Add caregiver notification workflows
- Add calibration and validation tests

## Author
**Sadik Shaik**
