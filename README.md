# 🌡️ AHT Sensor & OLED Monitor (Bare-Metal I2C)

Este firmware es una implementación de bajo nivel para la monitorización de temperatura y humedad utilizando un sensor **AHT10/AHT20** y una pantalla **OLED SSD1306**. El proyecto destaca por prescindir totalmente de librerías gráficas externas, optimizando el uso de memoria y CPU.

---

## 🚀 Características Principales

* **Zero Dependencies:** No utiliza librerías como `Adafruit_GFX` o `U8g2`. Todo el protocolo de video está escrito desde cero.
* **Gestión de Framebuffer:** Control total de los 8,192 píxeles de la pantalla mediante un array de memoria de 1024 bytes.
* **Motor de Texto Custom:** * Incluye una fuente tipográfica de 5x7 píxeles integrada.
    * Soporte para **escalado x2** (Texto de gran tamaño para visibilidad a distancia).
* **Robustez I2C:** * Escaneo y comprobación de direcciones (`0x3C` para OLED y `0x38` para AHT).
    * Transmisión de datos por bloques (*chunking*) para evitar desbordamientos.
* **Sistema de Alertas:** Pantallas de error visuales en caso de fallo de hardware o desconexión de cables.

---

## 🛠️ Requisitos de Hardware

| Componente | Dirección I2C | Pin (Default) |
| :--- | :--- | :--- |
| **Microcontrolador** | N/A | Compatible con ESP32 / Arduino |
| **OLED SSD1306** | `0x3C` | SDA: 8 / SCL: 9 |
| **Sensor AHT10/20** | `0x38` | SDA: 8 / SCL: 9 |

> **Nota:** La configuración de pines está definida para arquitecturas modernas como el ESP32-C3 o similares que usan los GPIO 8 y 9. Puedes cambiarlos en la sección de `CONFIGURACION I2C`.

---

## 📐 Lógica de Funcionamiento

### 1. Inicialización OLED
El código envía una secuencia de 26 comandos hexadecimales (`initSeq`) que configuran el hardware de la pantalla:
* Activa la bomba de carga (*charge pump*).
* Configura el mapeo de segmentos y el escaneo de salida COM.
* Establece el modo de direccionamiento horizontal.

### 2. Procesamiento de Datos AHT
El sensor entrega 6 bytes de datos crudos. El firmware realiza el desplazamiento de bits manual para reconstruir la precisión de 20 bits:
* **Humedad:** $RH = (\frac{S_{rh}}{2^{20}}) \times 100$
* **Temperatura:** $T = (\frac{S_{t}}{2^{20}} \times 200) - 50$

---

## 💻 Configuración de PlatformIO

Para una correcta visualización en el terminal, añade esto a tu `platformio.ini`:

```ini
[env:tu_placa]
platform = espressif32
board = tu_board
framework = arduino
monitor_speed = 115200
📝 Ejemplo de Salida en Pantalla
El diseño de la interfaz se divide en tres secciones:

Encabezado: "AHT SENSOR" en texto estándar.

Valor Térmico: Temperatura en escala 2x (ej: 24.5 C).

Valor Humedad: Humedad relativa en escala 2x (ej: 48.2 %).

En caso de fallo, verás:

Plaintext
  ERROR
  NO SENSOR 0x38
📜 Licencia
Este software se distribuye bajo la licencia MIT. Es ideal para aprendizaje de protocolos industriales I2C y optimización de recursos en sistemas embebidos.
