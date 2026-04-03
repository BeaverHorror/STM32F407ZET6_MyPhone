# STM32F407ZET6_MyPhone
Всё что за годы программирования STM32 собрал пытаюсь запихнуть в одну кучу.

![image](https://github.com/BeaverHorror/STM32F407ZET6_MyPhone/blob/main/Media/IMG_20260402_200945.jpg)

Давно хотел проявить себя в разработке печатных плат и это мой первый, во многом неудачный, опыт.

Фото и видео работы смотри в папке Media. 

Работа с модулями SIM800, LoRa, W5200 и многими другими там не показана, но я там всего лишь демонстрирую идею.

Плату разводил в EasyEDA Лежит в папке Schematic.

Разное и не нужное лежит в папке Other. Напрмер там можно найти UDP сервер для отладки W5500.

Код писал в Keil и конфигурировал в CubeMX. Код лежит в архиве Ethernet.7z (Изначально начинал с W5500 поэтому и Ethernet, а остальной код по ходу дела появился).

На данный момент проверены и корректно работают:

	Питание, Релле, ESP8266, SIM800C, W5500, SD карта, FSMC дисплей, кнопки, BMP280, W25Q, USB-SD-W25Q
	LoRa см примечания LoRa

Примечания:

	LoRa (код отлажен но на плате разводка под RFM без вывода под антену и электростатической защиты)
	Примечание: добился работы на одном SPI c W25Q. Теперь можно впаять либо RFM модуль либо оставить как есть или разработать плату переходник под SX-1278

На плате обнаружены следующие ошибки:

	1. разъёмы j7 и j9 неправильно разведены RX TX (связь STM32 с SIM800C ESP8266) косяк неприятный но костылём поправимый
	2. несоответствие отверстий под крепления (не критично но не приятно)
	3. размеры транзисторов все разные SOT-23, SOT-323, SOT523, SOT723 (не критично, аналоги нашёл, но по факту все эти транзисторы можно было унифицировать одним)
	4. схема переключения реле работает а вот схема перезагрузки STM32 с помощью ESP8266 через транзистор не работает и рушит всю систему (по факту можно кинуть прямой провод на NRST STM32 от ESP8266 так что критичной проблема не стала но могла)
	5. от PWRKEY SIM800C нужно сделать подтяжку к питанию (не критично, поставил резистор навесным монтажом)
	6. Критическая ошибка LoRa модуля разная распиновка у SX-1278 и RF модулей (на плате под RF) возможно из-за этого не работает W25Q
	7. Не работают BMP280 и MPU6050 (причина не ясна)

Идеи по улучшению:

	8. PF2 рестарт ESP8266 - Надо подробнее изучить. Также добавил в схему PF2 ESP8266_BOOT
	9. Необходимо перевести LoRa на ногодрыг

Необходимо проверить работу:
	SD карты на ESP8266
	работу W25Q на ESP8266
	самое главное камеру

Распиновка:

SD-Card - SDIO (SD 4 bits Wide bus)

	PC8  - SDIO_D0
	PC9  - SDIO_D1
	PC10 - SDIO_D2
	PC11 - SDIO_D3
	PC12 - SDIO_CK
	PD2  - SDIO_CMD

W25Q Flash - SPI1

	PB3  - SPI1_SCK
	PB4  - SPI1_MISO
	PB5  - SPI1_MOSI
	PB14 - F_CS         (Output)

LoRa - SPI Software или SPI1 (см. W25Q)

	PF7  - SPI1_SCK
	PF8  - SPI1_MISO
	PF6  - SPI1_MOSI
	PG13 - LoRa_DIO0    (EXTI13)
	PD3  - LoRa_DIO1    (EXTI3)
	PG11 - LoRa_CS      (Output)
	PG10 - LoRa_RST     (Output)

Тачскрин - SPI программный

	PC13 - T_CS         (Output)
	PB0  - T_SCK        (Output)
	PB2  - T_MISO       (Input)
	PF11 - T_MOSI       (Output)
	PB1  - T_PEN        (EXTI1)

LCD Дисплей - FSMC

	PD14  FSMC_D0
	PD15  FSMC_D1
	PD0   FSMC_D2
	PD1   FSMC_D3
	PE7   FSMC_D4
	PE8   FSMC_D5
	PE9   FSMC_D6
	PE10  FSMC_D7
	PE11  FSMC_D8
	PE12  FSMC_D9
	PE13  FSMC_D10
	PE14  FSMC_D11
	PE15  FSMC_D12
	PD8   FSMC_D13
	PD9   FSMC_D14
	PD10  FSMC_D15
	PD4   FSMC_NOE
	PD5   FSMC_NWE
	PG12  FSMC_NE4
	PF12  FSMC_A6
	PB15  LCD_BL        (Output)

Камера OV7670 - DCMI

	PA4  - DCMI_HSYNC
	PA6  - DCMI_PIXCLK
	PB6  - DCMI_D5
	PB7  - DCMI_VSYNC
	PC6  - DCMI_D0
	PC7  - DCMI_D1
	PE0  - DCMI_D2
	PE1  - DCMI_D3
	PE4  - DCMI_D4
	PE5  - DCMI_D6
	PE6  - DCMI_D7
	PB8  - I2C1_SCL
	PB9  - I2C1_SDA
	PD12 - XLK          (TIM4_CH1 PWM)
	PD6  - OV7670_RET
	PD7  - OV7670_PWDN

USART1 ESP8266

	PA9  - USART1_TX
	PA10 - USART1_RX
	PF2  - ESP_RST
	PF3  - ESP_BOOT

USART2 SIM800

	PA2  - USART2_TX
	PA3  - USART2_RX
	PG15 - SIM800C_START   (OutPut)
	PA15 - SIM800C_STATUS  (Input)

USART3 Дебагер

	PB10 - USART3_TX
	PB11 - USART3_RX

Encoder 

	PA0  - TIM5_CH1
	PA1  - TIM5_CH2
	PE2  - Encoder_BTN

W5500 SPI2

	PB13  - SPI2_SCK
	PC2   - SPI2_MISO
	PC3   - SPI2_MOSI
	PG2   - W5500_INT    (EXTI2)
	PG3   - W5500_CS     (Output)
	PG4   - W5500_RST    (Output)

MPU-6050 I2C2

	PF1   - I2C2_SCL
	PF0   - I2C2_SDA
	PG5   - MPU6050_INT (EXTI5)

BME280

	PB8  - I2C1_SCL
	PB9  - I2C1_SDA

NEO-6M-0-001 USART6

	PG14 - TX6
	PG9  - RX6

	PG7  - BTN1 (BTN_UP)      (EXTI7)
	PG8  - BTN2 (BTN_DOWN)    (EXTI8)
	PG6  - BTN5 (BTN_ENTER)   (EXTI6)
	PB12 - BTN6 (BTN_BACK)    (EXTI12)
	PD11 - BTN7 (BTN_DOP)     (EXTI11)
	PE3  - BTN  (BTN_KEY1)    (Input)
	PE2  - BTN  (BTN_ENCODER) (Input)

USART6

	PG14 - TX6
	PG9  - RX6

I2C1

	PB8  - SCL1
	PB9  - SDA1

I2C2

	PF1  - SCL2
	PF0  - SDA2

SPI2

	PB13  - SPI2_SCK
	PC2   - SPI2_MISO
	PC3   - SPI2_MOSI

Serial Wire

	PA13 - SYS_JTMS_SWDIO
	PA14 - SYS_JTMS_SWCLK
