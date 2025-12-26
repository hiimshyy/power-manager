# 📋 TÀI LIỆU BÀN GIAO DỰ ÁN
## Hệ Thống Quản Lý Nguồn STM32 Power Management

---

## 📌 MỤC LỤC

1. [Tổng Quan Dự Án](#1-tổng-quan-dự-án)
2. [Kiến Trúc Hệ Thống](#2-kiến-trúc-hệ-thống)
3. [Yêu Cầu Phần Cứng](#3-yêu-cầu-phần-cứng)
4. [Các Module Chính](#4-các-module-chính)
5. [Cấu Hình Phần Cứng](#5-cấu-hình-phần-cứng)
6. [Giao Tiếp Modbus RTU](#6-giao-tiếp-modbus-rtu)
7. [Troubleshooting](#7-troubleshooting)
8. [Tài Liệu Tham Khảo](#8-tài-liệu-tham-khảo)

---

## 1. TỔNG QUAN DỰ ÁN

### 1.1. Mô Tả Dự Án

Dự án **STM32 Power Management** là một hệ thống quản lý nguồn thông minh sử dụng vi điều khiển STM32F103C8TX làm bộ điều khiển trung tâm. Hệ thống có khả năng:

- **Giám sát pin**: Đọc thông tin từ BMS Daly (điện áp, dòng điện, SoC, nhiệt độ, trạng thái cell...)
- **Điều khiển sạc**: Giao tiếp với bộ sạc SK60X để điều khiển quá trình sạc pin
- **Quản lý nguồn**: Điều khiển các relay cho các nguồn 12V, 5V, 3.3V
- **Giám sát nguồn**: Đọc cảm biến INA219 để giám sát điện áp/dòng điện của các nguồn
- **Giao tiếp**: Cung cấp giao diện Modbus RTU qua RS485 để giao tiếp với thiết bị master (ví dụ: Radxa)
- **Debug**: Hỗ trợ USB CDC để debug và giám sát

### 1.2. Thông Tin Kỹ Thuật

- **MCU**: STM32F103C8TX (ARM Cortex-M3, 72MHz, 64KB Flash, 20KB RAM)
- **RTOS**: FreeRTOS
- **Giao tiếp**:
  - UART1: Daly BMS (9600 baud)
  - UART2: Modbus RTU (115200 baud, có thể cấu hình)
  - UART3: SK60X Charger (115200 baud)
  - I2C1: INA219 sensors
  - USB: CDC (debug)
- **GPIO**: 
  - Relay điều khiển: 12V, 5V, 3.3V, Charge, Fault
  - LED: Status, BMS, SK60X, Modbus, Fault
  - Input: 4 kênh input (INP_1 đến INP_4)

---

## 2. KIẾN TRÚC HỆ THỐNG

### 2.1. Sơ Đồ Kiến Trúc

```
┌─────────────────────────────────────────────────────────────┐
│                    STM32F103C8TX                            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │ FreeRTOS │  │  BMS     │  │  SK60X   │  │ Modbus   │     │
│  │  Tasks   │  │  Module  │  │  Module  │  │  RTU     │     │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │
│       │             │              │              │         │
└───────┼─────────────┼──────────────┼──────────────┼─────────┘
        │             │              │              │
    ┌───▼───┐    ┌───▼───┐     ┌───▼───┐     ┌───▼───┐
    │ UART1 │    │ UART2 │     │ UART3 │     │ I2C1  │
    │ BMS   │    │Modbus │     │ SK60X │     │ INA219│
    └───┬───┘    └───┬───┘     └───┬───┘     └───┬───┘
        │            │              │             │
    ┌───▼───┐    ┌───▼───┐     ┌───▼───┐     ┌───▼───┐
    │ Daly  │    │ RS485 │     │ SK60X │     │ INA219│
    │  BMS  │    │Master │     │Charger│     │Sensors│
    └───────┘    └───────┘     └───────┘     └───────┘
```

### 2.2. FreeRTOS Tasks

Hệ thống sử dụng 4 tasks chính:

1. **defaultTask** (Priority: Normal, Stack: 128 bytes)
   - Khởi tạo USB CDC
   - Điều khiển relay nguồn dựa trên điện áp BMS
   - LED heartbeat (500ms)

2. **bmsTask** (Priority: Normal, Stack: 512 bytes)
   - Giao tiếp với Daly BMS
   - Đọc dữ liệu BMS theo chu kỳ (11 loại request)
   - Xử lý lỗi và phục hồi kết nối

3. **sk60xTask** (Priority: Normal, Stack: 128 bytes)
   - Xử lý logic điều khiển sạc
   - Kiểm tra điều kiện sạc
   - Điều khiển relay sạc

4. **modbusTask** (Priority: Normal, Stack: 128 bytes)
   - Xử lý giao tiếp Modbus RTU
   - Đọc/ghi register
   - Reset timeout sau 10 giây không hoạt động

---

## 3. YÊU CẦU PHẦN CỨNG

### 3.1. MCU và Ngoại Vi

- **STM32F103C8TX** (Blue Pill compatible)
- **Clock**: HSE 8MHz, PLL x9 = 72MHz
- **Flash**: 64KB
- **RAM**: 20KB

### 3.2. Kết Nối Phần Cứng

| Module | Interface | Pin | Baudrate | Notes |
|--------|-----------|-----|----------|-------|
| Daly BMS | UART1 | PA9/PA10 | 9600 | 8N1 |
| Modbus RTU | UART2 | PA2/PA3 | 115200 (configurable) | 8N1, RS485 |
| SK60X | UART3 | PB10/PB11 | 115200 | 8N1 |
| INA219 | I2C1 | PB6/PB7 | 100kHz | Multiple addresses |

### 3.3. GPIO Pins

**Output (Relay Control):**
- `RL_12V`: PA7 - Relay nguồn 12V
- `RL_5V`: PB0 - Relay nguồn 5V
- `RL_3V3`: PB1 - Relay nguồn 3.3V
- `RL_CHG`: PA6 - Relay sạc
- `FAULT_OUT`: PB5 - Relay fault

**Output (LED):**
- `LED`: PC13 - LED heartbeat
- `LED_BMS`: PA0 - LED trạng thái BMS
- `LED_SK`: PA1 - LED trạng thái SK60X
- `LED_MB`: PC14 - LED trạng thái Modbus
- `LED_FAULT`: PC15 - LED lỗi

**Input:**
- `INP_1`: PB4 - Input 1
- `INP_2`: PB3 - Input 2
- `INP_3`: PA15 - Input 3
- `INP_4`: PA8 - Input 4

---

## 4. CÁC MODULE CHÍNH

### 4.1. Daly BMS Module (`daly_bms.h/c`)

**Chức năng:**
- Giao tiếp với Daly BMS qua UART1
- Đọc thông tin pin: điện áp, dòng điện, SoC, nhiệt độ, cell voltages...
- Điều khiển MOS charge/discharge
- Xử lý lỗi và phục hồi kết nối

**Các hàm chính:**
- `DalyBMS_Get_Pack_Data()` - Đọc điện áp, dòng, SoC
- `DalyBMS_Get_Cell_Voltages()` - Đọc điện áp từng cell
- `DalyBMS_Get_Failure_Codes()` - Đọc mã lỗi
- `DalyBMS_Set_Charge_MOS()` - Bật/tắt MOS sạc
- `DalyBMS_Set_Discharge_MOS()` - Bật/tắt MOS xả

**Cấu trúc dữ liệu:**
```c
DalyBMS_Data bms_data;  // Chứa tất cả dữ liệu BMS
```

**Chu kỳ đọc dữ liệu:**
BMS task đọc 11 loại dữ liệu theo thứ tự:
1. Pack data (voltage, current, SoC)
2. Min/Max cell voltage
3. Temperature
4. Charge/Discharge MOS status
5. Status info
6. Cell voltages (tất cả cells)
7. Cell temperatures
8. Cell balance state
9. Failure codes
10. Voltage thresholds
11. Pack thresholds

### 4.2. SK60X Module (`sk60x.h/c`)

**Chức năng:**
- Giao tiếp với bộ sạc SK60X qua UART3
- Đọc thông tin sạc: điện áp, dòng điện, nhiệt độ...
- Điều khiển sạc: set voltage, current, on/off

**Các hàm chính:**
- `SK60X_Read_Data()` - Đọc dữ liệu từ SK60X
- `SK60X_Set_Voltage()` - Đặt điện áp sạc
- `SK60X_Set_Current()` - Đặt dòng sạc
- `SK60X_Set_On_Off()` - Bật/tắt output

**Cấu trúc dữ liệu:**
```c
SK60X_Data sk60x_data;  // Chứa dữ liệu SK60X
```

### 4.3. Charge Control Module (`charge_control.h/c`)

**Chức năng:**
- Logic điều khiển quá trình sạc
- Kiểm tra điều kiện sạc (điện áp input, set voltage...)
- Điều khiển relay sạc
- Quản lý trạng thái sạc (IDLE, WAITING, READY, CHARGING)

**Các hàm chính:**
- `ChargeControl_Init()` - Khởi tạo
- `ChargeControl_Process()` - Xử lý logic sạc (gọi trong sk60xTask)
- `ChargeControl_HandleRequest()` - Xử lý yêu cầu sạc từ Modbus
- `ChargeControl_SetChargeRelay()` - Điều khiển relay sạc

**Trạng thái sạc:**
- `CHARGE_STATE_IDLE` (0): Không được phép sạc
- `CHARGE_STATE_WAITING` (1): Đang chờ điều kiện sạc
- `CHARGE_STATE_READY` (2): Sẵn sàng sạc
- `CHARGE_STATE_CHARGING` (3): Đang sạc

**Điều kiện sạc:**
- SK60X input voltage > 24.0V
- SK60X set voltage > 16.8V
- Charge request = true (từ Modbus register 0x003F)

### 4.4. Modbus RTU Module (`modbus_rtu.h/c`)

**Chức năng:**
- Triển khai giao thức Modbus RTU
- Xử lý các function code: 0x03 (Read), 0x06 (Write Single), 0x10 (Write Multiple)
- Quản lý register map
- Tính toán và kiểm tra CRC

**Các hàm chính:**
- `ModbusRTU_Init()` - Khởi tạo
- `ModbusRTU_Process()` - Xử lý frame nhận được (gọi trong modbusTask)
- `ModbusRTU_ReadRegister()` - Đọc register
- `ModbusRTU_WriteRegister()` - Ghi register

**Cấu hình:**
- Slave ID: 0x02 (mặc định, có thể thay đổi qua register 0x0100)
- Baudrate: 115200 (mặc định, có thể cấu hình qua register 0x0101)
- Parity: None (có thể cấu hình)
- Stop bits: 1 (có thể cấu hình)

**Register Map:** Xem file `Docs/modbus_register_map.md`

### 4.5. INA219 Module (`ina219.h/c`)

**Chức năng:**
- Đọc cảm biến INA219 qua I2C
- Đo điện áp, dòng điện, công suất của các nguồn 12V, 5V, 3.3V

**Các hàm chính:**
- `INA219_Init()` - Khởi tạo sensor
- `INA219_ReadVoltage()` - Đọc điện áp
- `INA219_ReadCurrent()` - Đọc dòng điện
- `INA219_ReadPower()` - Đọc công suất

**Địa chỉ I2C:**
- INA219_12V: 0x40
- INA219_5V: 0x41
- INA219_3V3: 0x44

---

## 5. CẤU HÌNH PHẦN CỨNG

### 5.1. STM32CubeMX Configuration

File cấu hình: `power_management.ioc`

**Các bước cấu hình:**

1. **Clock Configuration:**
   - HSE: 8MHz
   - PLL: x9 = 72MHz
   - System Clock: 72MHz
   - APB1: 36MHz
   - APB2: 72MHz

2. **UART Configuration:**
   - **USART1**: 
     - Mode: Asynchronous
     - Baudrate: 9600
     - Word Length: 8 bits
     - Parity: None
     - Stop Bits: 1
     - RX: PA10, TX: PA9
   - **USART2**:
     - Mode: Asynchronous
     - Baudrate: 115200
     - Word Length: 8 bits
     - Parity: None
     - Stop Bits: 1
     - RX: PA3, TX: PA2
   - **USART3**:
     - Mode: Asynchronous
     - Baudrate: 115200
     - Word Length: 8 bits
     - Parity: None
     - Stop Bits: 1
     - RX: PB11, TX: PB10

3. **I2C Configuration:**
   - **I2C1**:
     - Mode: I2C
     - Speed: 100kHz
     - SCL: PB6
     - SDA: PB7

4. **GPIO Configuration:**
   - Cấu hình các pin output cho relay và LED
   - Cấu hình các pin input

5. **FreeRTOS Configuration:**
   - Enable FreeRTOS
   - Heap: heap_4.c
   - Tạo 4 tasks

6. **USB Configuration:**
   - Mode: Device Only
   - Class: Communication Device Class (Virtual Port Com)

### 5.2. Điều Khiển Relay Nguồn

Logic điều khiển relay nguồn (12V, 5V, 3.3V) trong `defaultTask`:

```c
// Bật relay khi điện áp BMS > 13.5V
if (!relay_power_enabled && bms_data.voltage > voltage_threshold) {
    HAL_GPIO_WritePin(RL_12V_GPIO_Port, RL_12V_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RL_5V_GPIO_Port, RL_5V_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RL_3V3_GPIO_Port, RL_3V3_Pin, GPIO_PIN_SET);
    relay_power_enabled = true;
}
// Tắt relay khi điện áp BMS < 13.5V
else if (relay_power_enabled && bms_data.voltage < voltage_threshold) {
    HAL_GPIO_WritePin(RL_12V_GPIO_Port, RL_12V_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RL_5V_GPIO_Port, RL_5V_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RL_3V3_GPIO_Port, RL_3V3_Pin, GPIO_PIN_RESET);
    relay_power_enabled = false;
}
```

**Ngưỡng điện áp:** 13.5V (có thể thay đổi qua Modbus register 0x004E)

---

## 6. GIAO TIẾP MODBUS RTU

### 6.1. Tổng Quan

Hệ thống cung cấp giao diện Modbus RTU qua UART2 (RS485) để giao tiếp với thiết bị master. Tất cả dữ liệu từ BMS, SK60X, INA219 được expose qua Modbus registers.

### 6.2. Register Map

Xem chi tiết trong file: `Docs/modbus_register_map.md`

**Các nhóm register chính:**

1. **0x0000 - 0x002B**: Daly BMS Status
   - Điện áp, dòng, SoC
   - Cell voltages, temperatures
   - Trạng thái MOS, fault flags
   - Thresholds

2. **0x0030 - 0x003F**: SK60X Data
   - Voltage/Current setpoints
   - Output voltage/current/power
   - Input voltage/current
   - Trạng thái sạc

3. **0x0040 - 0x0048**: INA219 Sensor Values
   - Điện áp, dòng, công suất của 12V, 5V, 3.3V

4. **0x0049 - 0x004E**: Relay Status
   - Trạng thái các relay
   - Voltage threshold

5. **0x0100 - 0x0109**: System Configuration
   - Slave ID, baudrate, parity
   - Firmware/hardware version
   - System status, errors

### 6.3. Function Codes Hỗ Trợ

- **0x03**: Read Holding Registers
- **0x06**: Write Single Register
- **0x10**: Write Multiple Registers

### 6.4. Ví Dụ Sử Dụng

**Đọc điện áp pin:**
```
Slave ID: 0x02
Function: 0x03 (Read Holding Registers)
Address: 0x0000 (BMS voltage)
Quantity: 1
```

**Bật SK60X output:**
```
Slave ID: 0x02
Function: 0x06 (Write Single Register)
Address: 0x003C (SK60X on_off)
Value: 0x0001
```

**Đọc nhiều register:**
```
Slave ID: 0x02
Function: 0x03 (Read Holding Registers)
Address: 0x0000
Quantity: 10 (đọc 10 register từ 0x0000)
```

### 6.5. Cấu Hình Modbus

Có thể thay đổi cấu hình Modbus qua các register:

- **0x0100**: Slave ID (1-247)
- **0x0101**: Baudrate code (1=9600, 2=19200, 3=38400, 4=57600, 5=115200)
- **0x0102**: Parity (0=None, 1=Even, 2=Odd)
- **0x0103**: Stop bits (1 hoặc 2)

**Lưu ý:** Sau khi thay đổi cấu hình, cần reset hoặc apply lại để có hiệu lực.

---

## 7. TROUBLESHOOTING

### 7.1. BMS Không Kết Nối

**Triệu chứng:**
- LED_BMS không sáng
- Dữ liệu BMS = 0 trong Modbus

**Nguyên nhân và giải pháp:**

1. **Kiểm tra kết nối UART1:**
   - Kiểm tra dây TX/RX
   - Kiểm tra baudrate (phải là 9600)
   - Kiểm tra địa chỉ BMS (mặc định 0x40)

2. **Kiểm tra BMS:**
   - BMS có được cấp nguồn không?
   - BMS có đang hoạt động không?

3. **Debug:**
   - Kiểm tra `bms_data.connection_status` trong code
   - Xem log UART1 nếu có

### 7.2. SK60X Không Phản Hồi

**Triệu chứng:**
- LED_SK không sáng
- Dữ liệu SK60X = 0

**Nguyên nhân và giải pháp:**

1. **Kiểm tra kết nối UART3:**
   - Kiểm tra dây TX/RX
   - Kiểm tra baudrate (115200)

2. **Kiểm tra SK60X:**
   - SK60X có được cấp nguồn không?
   - Địa chỉ Modbus của SK60X (mặc định 0x01)

### 7.3. Modbus Không Hoạt Động

**Triệu chứng:**
- Master không nhận được response
- LED_MB không nhấp nháy

**Nguyên nhân và giải pháp:**

1. **Kiểm tra cấu hình:**
   - Slave ID có đúng không? (mặc định 0x02)
   - Baudrate có khớp không? (mặc định 115200)
   - Parity và stop bits có đúng không?

2. **Kiểm tra kết nối RS485:**
   - Kiểm tra dây A/B
   - Kiểm tra termination resistor (120Ω)
   - Kiểm tra ground chung

3. **Debug:**
   - Sử dụng Modbus tool (ModScan, pymodbus) để test
   - Kiểm tra CRC có đúng không

### 7.4. Relay Không Bật

**Triệu chứng:**
- Relay không hoạt động khi điện áp > 13.5V

**Nguyên nhân và giải pháp:**

1. **Kiểm tra logic:**
   - Điện áp BMS có > 13.5V không?
   - `relay_power_enabled` có được set không?

2. **Kiểm tra GPIO:**
   - Kiểm tra pin relay có được cấu hình đúng không?
   - Kiểm tra mạch điều khiển relay

3. **Kiểm tra nguồn:**
   - Relay có được cấp nguồn không?

### 7.5. INA219 Không Đọc Được

**Triệu chứng:**
- Dữ liệu INA219 = 0 trong Modbus

**Nguyên nhân và giải pháp:**

1. **Kiểm tra I2C:**
   - Kiểm tra dây SCL/SDA
   - Kiểm tra pull-up resistors (4.7kΩ)
   - Kiểm tra địa chỉ I2C có đúng không

2. **Kiểm tra INA219:**
   - Sensor có được cấp nguồn không?
   - Shunt resistor có đúng không?

### 7.6. FreeRTOS Task Crash

**Triệu chứng:**
- Hệ thống treo hoặc reset

**Nguyên nhân và giải pháp:**

1. **Stack overflow:**
   - Tăng stack size của task
   - Kiểm tra bằng FreeRTOS stack watermark

2. **Deadlock:**
   - Kiểm tra mutex/semaphore
   - Tránh blocking trong ISR

3. **Watchdog:**
   - Kiểm tra xem có watchdog timer không
   - Reset watchdog trong task

---

## 8. TÀI LIỆU THAM KHẢO

### 8.1. Tài Liệu STM32

- **STM32F103C8 Datasheet**: [ST Website](https://www.st.com)
- **STM32F1 HAL User Manual**: UM1850
- **STM32CubeIDE User Guide**: UM2606

### 8.2. Tài Liệu FreeRTOS

- **FreeRTOS Reference Manual**: [FreeRTOS.org](https://www.freertos.org)
- **FreeRTOS API Documentation**

### 8.3. Tài Liệu Module

- **Daly BMS Protocol**: Xem trong code `daly_bms.h`
- **SK60X Protocol**: Xem trong code `sk60x.h`
- **INA219 Datasheet**: [TI Website](https://www.ti.com)
- **Modbus RTU Specification**: Modbus.org

### 8.4. Tài Liệu Dự Án

- `README.md` - Tổng quan dự án
- `Docs/modbus_register_map.md` - Chi tiết Modbus registers
- `Docs/power_manage_flow.png` - Sơ đồ luồng hệ thống
- `Docs/pin_used.png` - Sơ đồ chân

### 8.5. Tools

- **STM32CubeIDE**: [ST Website](https://www.st.com/en/development-tools/stm32cubeide.html)
- **STM32CubeMX**: [ST Website](https://www.st.com/en/development-tools/stm32cubemx.html)
- **ST-Link Utility**: [ST Website](https://www.st.com/en/development-tools/stsw-link004.html)
- **Modbus Tools**:
  - ModScan (Windows)
  - pymodbus (Python)
  - QModMaster (Cross-platform)

---

## KẾT LUẬN

Tài liệu này cung cấp thông tin tổng quan về dự án STM32 Power Management System. Nếu có bất kỳ câu hỏi nào, vui lòng tham khảo code comments hoặc liên hệ với người phát triển.

**Chúc bạn thành công với dự án!** 🚀

---

*Tài liệu được tạo tự động - Cập nhật lần cuối: [Ngày]*

