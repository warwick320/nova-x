
![logo](img/LOGO_ONLY_SMALL.png)
# [项目网页 — 点击查看！](https://warwick320.github.io/NovaX-5G-RTL8720dn-Bw16-Deauther/)
## ⭐ 如果你喜欢这个项目，请给它一个星标！

---

## 🚀 最新更新 – 2025-06-07

- ✅ **添加 Logo 动画**
- ✅ **添加文字动画**


---

## 🛠 固件上传指南

### 🔹 上传固件（使用上传工具）

1. 首先，[下载工具](https://github.com/warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther/releases/tag/ImageTool_v2)。
2. 通过 USB **连接**你的 **BW16** 开发板。
3. 为你的 BW16 选择正确的 **COM 端口**。
4. 点击 **开始固件上传** 按钮。
5. ✅ 完成！

---

### 🔹 BW16 Arduino 设置

1. 打开 **Arduino IDE**。
2. 转到 **文件 > 首选项**。
3. 在 **附加开发板管理器网址** 中添加以下网址：`https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek_amebad_index.json`
4. 打开 **开发板管理器**，搜索 **BW16**。
5. 安装 **Realtek Ameba Boards**（版本 **3.1.8**）。
> 你需要更新 Realtek Ameba 开发板管理器
---

### 🔹 ESP32 Arduino 设置

1. 打开 **Arduino IDE**。
2. 转到 **文件 > 首选项**。
3. 在 **附加开发板管理器网址** 中添加以下网址：`https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   4. 打开 **开发板管理器**，搜索 **ESP32**。
5. 安装 **ESP32 by Espressif Systems**。

---

### 🔹 上传固件（使用 Arduino IDE）

#### 📦 对于 BW16

1. 使用 Arduino IDE 打开 **BW16** 文件夹中的 `BW16.ino` 文件。
2. 转到 **工具 > 开发板** 并选择 `Ai-Thinker BW16 (RTL8720DN)`。
3. 启用 **自动上传模式**，然后点击 **启用**。
4. 启用 **标准库** 并选择 `Arduino_STD_PRINTF`。
5. 点击 **上传** 以刷写固件。

#### 📦 对于 ESP32

1. 使用 Arduino IDE 打开 **ESP32** 文件夹中的 `.ino` 文件。
2. 转到 **工具 > 开发板** 并选择你的开发板（例如：`ESP32 Dev Module`）。
3. 将 **分区方案** 设置为 `No OTA (2MB APP/2MB SPIFFS)`。
4. 점击 **上传** 以刷写固件。

> ⚠️ **注意：** **Evil Twin** 功能在 **ESP32-S3** 上 **不支持**，  
> 但它可能有助于扩展 **Sour Apple** 攻击的范围。

---

## 📋 所需组件

- ✅ **SSD1306 OLED 显示屏**  
- ✅ **4 个触觉按钮**  
- ✅ **BW16 开发板**  
- ✅ **ESP32 开发板** *(可选)*

---

## 🔌 硬件连接

### 🎮 按钮（BW16）

| 功能 | 引脚 |
|----------|------|
| 上       | PA27 |
| 下     | PA12 |
| 选择   | PA13 |
| 返回     | PB3  |

> ⚠️ **在最新版本中，返回按钮现在是必需的！**

---

### 📺 SSD1306 显示屏（I2C）

| 信号 | BW16 引脚 |
|--------|----------|
| SDA    | PA26     |
| SCL    | PA25     |

---

## 🔁 ESP32 ↔ BW16 I2C 接线

| I2C 线 | ESP32 引脚 | BW16 引脚 |
|----------|------------|-----------|
| SDA      | GPIO 21    | PA26      |
| SCL      | GPIO 22    | PA25      |
| GND      | GND        | GND       |
| 3V3      | 3V3        | 3V3       |

> ⚠️ 如果你不使用 **Evil Twin** 或 **Sour Apple**，可以不连接 ESP32。

---

## 📂 源代码

[GitHub 仓库](https://github.com/tesa-klebeband/RTL8720dn-Deauther)

---

## ⚠️ 免责声明

此工具仅用于 **教育** 和 **测试** 目的。  
**严格禁止** 将此工具用于任何 **非法活动**。  
作者对使用此工具造成的任何后果 **不承担责任**。

---

## 💬 反馈与支持

如果你发现任何 **问题** 或有 **建议**，请通过 **Discord** 联系：  
`warwick.320`

## 星标历史

<a href="https://www.star-history.com/#warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&type=Date" />
 </picture>
</a>
