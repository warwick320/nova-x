![logo](img/LOGO_ONLY_SMALL.png)
# [프로젝트 웹페이지 — 확인해보세요!](https://warwick320.github.io/NovaX-5G-RTL8720dn-Bw16-Deauther/)
## ⭐ 이 프로젝트가 마음에 드시면 별을 눌러주세요!

---

## 🚀 새로운 업데이트 – 2025-06-07

- ✅ **로고 애니메이션 추가**
- ✅ **텍스트 애니메이션 추가**


---

## 🛠 펌웨어 업로드 가이드

### 🔹 펌웨어 업로드 (업로드 도구 사용)

1. 먼저 [도구를 다운로드](https://github.com/warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther/releases/tag/ImageTool_v2)하세요.
2. USB를 통해 **BW16** 보드를 **연결**하세요.
3. BW16의 올바른 **COM 포트**를 선택하세요.
4. **펌웨어 업로드 시작** 버튼을 클릭하세요.
5. ✅ 완료!

---

### 🔹 BW16 Arduino 설정

1. **Arduino IDE**를 여세요.
2. **파일 > 환경설정**으로 이동하세요.
3. **추가 보드 매니저 URL**에 다음 URL을 추가하세요: `https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek_amebad_index.json`
4. **보드 매니저**를 열고 **BW16**을 검색하세요.
5. **Realtek Ameba Boards** (버전 **3.1.8**)를 설치하세요.
> Realtek Ameba 보드 매니저를 업데이트해야 합니다
---

### 🔹 ESP32 Arduino 설정

1. **Arduino IDE**를 여세요.
2. **파일 > 환경설정**으로 이동하세요.
3. **추가 보드 매니저 URL**에 다음 URL을 추가하세요: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
4. **보드 매니저**를 열고 **ESP32**를 검색하세요.
5. **ESP32 by Espressif Systems**를 설치하세요.

---

### 🔹 펌웨어 업로드 (Arduino IDE 사용시)

#### 📦 BW16

1. Arduino IDE를 사용하여 **BW16** 폴더의 `BW16.ino` 파일을 여세요.
2. **도구 > 보드**로 이동하여 `Ai-Thinker BW16 (RTL8720DN)`을 선택하세요.
3. **자동 업로드 모드**를 활성화한 후 **활성화**를 클릭하세요.
4. **표준 라이브러리**를 활성화하고 `Arduino_STD_PRINTF`를 선택하세요.
5. **업로드**를 클릭하여 펌웨어를 플래시하세요.

#### 📦 ESP32

1. Arduino IDE를 사용하여 **ESP32** 폴더의 `.ino` 파일을 여세요.
2. **도구 > 보드**로 이동하여 보드를 선택하세요 (예: `ESP32 Dev Module`).
3. **파티션 스키마**를 `No OTA (2MB APP/2MB SPIFFS)`로 설정하세요.
4. **업로드**를 클릭하여 펌웨어를 플래시하세요.

> ⚠️ **참고:** **Evil Twin** 기능은 **ESP32-S3**에서 **지원되지 않지만**,  
> **Sour Apple** 공격의 범위를 확장하는 데 도움이 될 수 있습니다.

---

## 📋 필요 구성품

- ✅ **SSD1306 OLED 디스플레이**  
- ✅ **4개의 택트 버튼**  
- ✅ **BW16 보드**  
- ✅ **ESP32 보드** *(선택사항)*

---

## 🔌 하드웨어 연결

### 🎮 버튼 (BW16)

| 기능 | 핀  |
|----------|------|
| 위       | PA27 |
| 아래     | PA12 |
| 선택   | PA13 |
| 뒤로     | PB3  |

> ⚠️ **최신 버전에서는 뒤로 버튼이 필수입니다!**

---

### 📺 SSD1306 디스플레이 (I2C)

| 신호 | BW16 핀 |
|--------|----------|
| SDA    | PA26     |
| SCL    | PA25     |

---

## 🔁 ESP32 ↔ BW16 I2C 배선

| I2C 라인 | ESP32 핀 | BW16 핀 |
|----------|------------|-----------|
| SDA      | GPIO 21    | PA26      |
| SCL      | GPIO 22    | PA25      |
| GND      | GND        | GND       |
| 3V3      | 3V3        | 3V3       |

> ⚠️ **Evil Twin**이나 **Sour Apple**을 사용하지 않는다면 ESP32 연결을 생략할 수 있습니다.

---

## 📂 소스 코드

[GitHub 저장소](https://github.com/tesa-klebeband/RTL8720dn-Deauther)

---

## ⚠️ 면책조항

이 도구는 **교육** 및 **테스트** 목적으로만 제작되었습니다.  
불법적인 활동을 위한 이 도구의 **오용**은 **엄격히 금지**됩니다.  
작성자는 이 도구 사용으로 인한 어떠한 결과에 대해서도 **책임을 지지 않습니다**.

---

## 💬 피드백 및 지원

**문제점**을 발견하거나 **제안사항**이 있으시면 **디스코드**를 통해 연락해 주세요:  
`warwick.320`

## 별표 히스토리

<a href="https://www.star-history.com/#warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther&type=Date" />
 </picture>
</a>
