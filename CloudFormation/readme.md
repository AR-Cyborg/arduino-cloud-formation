# ☁️ Cloud Formation Control System (Arduino)

This project is a personal hobby experiment to simulate cloud formation conditions using humidity, temperature, pressure, and gas sensors. It features an interactive TFT display, sensor data visualization, and automatic relay control for fan, humidifier, Peltier cooler, and water pump.

> ⚠️ Built for learning purposes by a student exploring embedded systems, sensors, and automation.

---

## 🧠 Project Context

This system aims to mimic the stages of cloud formation:
1. **Humidification** to reach a target humidity.
2. **Cooling** the air until dew point or low temperatures are achieved.
3. **Monitoring** for gas safety (butane detection).

All sensors and actuators are controlled by an Arduino Mega 2560 with a visual interface on an ST7735 TFT screen.

---

## 🔧 Hardware Components

| Component         | Purpose                                 |
|------------------|-----------------------------------------|
| **Arduino Mega 2560** | Main microcontroller             |
| **DHT22**         | Measures relative humidity              |
| **BMP280**        | Measures temperature & atmospheric pressure |
| **MQ-2**          | Detects butane gas levels               |
| **ST7735 TFT**    | 1.8" color display for real-time feedback |
| **Relay Module (x2)** | Controls fan, humidifier, Peltier, and pump |
| **Peltier Module**| Provides cooling                        |
| **Air Pump**      | Simulates airflow/water vapor           |
| **Humidifier**    | Increases humidity                      |
| **Fan**           | Circulates air                          |
| **LED**           | Gas alert indicator                     |

---

## 📌 Arduino Pin Connections

| Device              | Arduino Pin |
|---------------------|-------------|
| TFT CS              | 53          |
| TFT RST             | 48          |
| TFT DC              | 49          |
| TFT SCLK            | 52          |
| TFT MOSI            | 51          |
| TFT Backlight       | 12 (PWM)    |
| DHT22               | 2           |
| MQ2 (Analog)        | A0          |
| LED Alert           | 10          |
| Fan (Relay IN1)     | 5           |
| Pump (Relay IN2)    | 6           |
| Peltier (Relay IN3) | 3           |
| Humidifier (Relay IN4) | 4        |

---

## 🔄 System Behavior

- **IDLE:** Waiting for environment to meet humidification start conditions.
- **HUMIDIFYING:** Activates humidifier + fan for `20s` to increase humidity.
- **COOLING:** Starts Peltier and pump to reduce temperature to dew point.
- **ABORT:** If over-saturation or temperature drops too low, system resets to IDLE.

---

## 📺 Display Interface

The TFT shows:
- Humidity (%)
- Temperature (°C)
- Pressure (hPa)
- Butane Level (Analog Value)
- Current System Phase (IDLE / HUMIDIFY / COOLING)
- Device Status (Fan, Peltier, Pump, Humidifier — ON/OFF)

---

## ⚠️ Safety Feature

- If the MQ2 sensor reads a butane level above `300`, an LED alert turns ON.
- System avoids dangerous conditions such as:
  - Pressure < 800 hPa
  - Temperature < 12°C
  - Humidity > 99%

---

## ⚙️ Configuration Parameters

| Name                  | Default | Description                              |
|-----------------------|---------|------------------------------------------|
| `HUMIDIFY_TARGET`     | `85.0`  | Target humidity before cooling begins    |
| `COOLING_TEMP`        | `16.0`  | Minimum temp before stopping cooling     |
| `HUMIDIFY_DURATION`   | `20000` | Duration of humidifying phase (ms)       |
| `BUTANE_WARNING_LEVEL`| `300`   | MQ2 analog threshold for alert           |
| `MIN_PRESSURE`        | `800.0` | Min safe pressure to operate             |

---

## 🛠️ How to Edit

If you'd like to modify the behavior:
- **Relay pins** → Edit the `#define` section at the top.
- **Sensor thresholds** → Change values under `// Cloud Formation Parameters`.
- **Interface design** → Modify `drawInterface()` or `updateDisplay()`.

---

## 📦 File Structure

