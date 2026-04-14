# GD32F350 USB & UART MIDI Controller

Full-featured MIDI controller firmware for GD32F350 (ARM Cortex-M4, 48MHz, 64KB Flash, 16KB SRAM).

## Features

| Feature | Description |
|---|---|
| **USB-MIDI** | USB Full-Speed device, Audio Class MIDI Streaming, 64B Bulk EP |
| **UART-MIDI** | 31250 baud DIN-5, DMA TX + DMA circular RX + IDLE detect |
| **MIDI Router** | Bidirectional USB↔UART bridge with per-channel/per-message-type filtering |
| **16-Key Matrix** | 4×4 GPIO scan, debounce, long-press, auto-repeat |
| **8 Knobs** | ADC DMA continuous scan, IIR filter, deadzone, linear/log/S-curve mapping |
| **2 Encoders** | Hardware quadrature decode (TIM encoder mode), velocity acceleration |
| **Audio Jack** | ADC timer-trigger 16kHz capture, RMS/peak/dBFS level, clip detect |
| **Jack Click** | EXTI insert/remove + ADC Analog Watchdog click, single/double/triple/long-press FSM |
| **Button LEDs** | HT16K33 I2C LED driver, 2×8 matrix, per-key blink/pulse, 16-step PWM dimming |
| **RGB Strip** | WS2812B via TIM PWM + DMA, solid/blink/breathe/pulse/rainbow/VU-meter modes |
| **LCD Display** | ST7789 240×240 RGB565 SPI DMA, dirty-rect strip-buffer partial refresh (7.5KB RAM) |
| **EEPROM** | AT24C64 I2C, wear-leveling, CRC16 protected, RAM cache with deferred flush |
| **16 Presets** | Full device config serialization (key map, CC assignment, routing rules, LED scheme) |
| **DFU Upgrade** | Application-mode SysEx firmware transfer, CRC32 verify, FMC flash programming |
| **Bootloader** | 8KB standalone, DFU flag check, app validity verify, vector table relocation |

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│  Common/          Shared types, flash map, product info  │
├──────────────────────────────────────────────────────────┤
│  Utilities/       Cross-cutting: debug, error, math,     │
│                   MIDI utils, color, string, event queue │
├──────────────────────────────────────────────────────────┤
│  App/             Business logic, UI pages, LED control, │
│                   preset management, input mapping       │
├──────────────────────────────────────────────────────────┤
│  Middleware/       MIDI parser/router/codec, input        │
│                   engines, audio/jack/LED engines, DFU,  │
│                   EEPROM, GUI, ring buffer, soft timer   │
├──────────────────────────────────────────────────────────┤
│  Driver/          BSP: GPIO, SPI, I2C, UART, ADC, TIM,  │
│                   USB, DMA, FMC — register-level access  │
├──────────────────────────────────────────────────────────┤
│  GD32Lib/         Official firmware library + USB library │
├──────────────────────────────────────────────────────────┤
│  Bootloader/      Standalone 8KB DFU bootloader project  │
└──────────────────────────────────────────────────────────┘
```

## Project Structure

```
project/
├── Common/
│   └── common_types.h
├── Utilities/
│   ├── debug_rtt.h / .c          RTT 5-level log, MIDI msg printer
│   ├── error_handler.h / .c      32-entry circular error log
│   ├── assert_handler.h / .c     Compile-out debug guard
│   ├── math_utils.h / .c         Clamp, map, EMA, dB, velocity curve
│   ├── midi_utils.h / .c         Note name, CC name, 7-bit encode, SysEx
│   ├── color_utils.h / .c        HSV→RGB, RGB565, note/velocity→color
│   ├── string_utils.h / .c       itoa, hex, dB format, pad
│   ├── event_queue.h / .c        Lock-free ISR→main event FIFO
│   └── mempool.h / .c            Static block allocator (no malloc)
├── Driver/
│   ├── bsp_system.h / .c         RCU 48MHz IRC48M, NVIC, SysTick
│   ├── bsp_uart_midi.h / .c      USART0 31250 baud, DMA CH2/CH3
│   ├── bsp_lcd_spi.h / .c        SPI1 DMA CH4, DC/RST/CS GPIO
│   ├── bsp_lcd_ctrl.h / .c       ST7789 / SSD1306 init sequence
│   ├── bsp_key_matrix.h / .c     4×4 row/col GPIO scan
│   ├── bsp_adc.h / .c            8-ch continuous scan, DMA CH0
│   ├── bsp_encoder.h / .c        TIM1/TIM2 quadrature decode
│   ├── bsp_ht16k33.h / .c        I2C0 LED driver (2×8 button indicators)
│   ├── bsp_ws2812.h / .c         TIM0 PWM 800kHz + DMA CH5 (RGB strip)
│   ├── bsp_jack_detect.h / .c    EXTI insert + ADC AWD click detect
│   ├── bsp_audio_adc.h / .c      TIM2 trigger 16kHz ADC capture
│   ├── bsp_eeprom_i2c.h / .c     I2C0 AT24C64, page R/W, ACK polling
│   └── bsp_fmc.h / .c            Flash erase/program (app region only)
├── Middleware/
│   ├── ring_buffer.h / .c        512B lock-free SPSC FIFO
│   ├── soft_timer.h / .c         16-slot SysTick software timer
│   ├── crc_engine.h / .c         CRC16-CCITT + CRC32
│   ├── midi_parser.h / .c        Byte-stream FSM, running status
│   ├── usb_midi_codec.h / .c     4-byte USB-MIDI event packet CIN codec
│   ├── midi_router.h / .c        Multi-port routing + channel/type filter
│   ├── sysex_assembler.h / .c    F0..F7 chunked reassembly, timeout
│   ├── key_engine.h / .c         Debounce, long-press, auto-repeat FSM
│   ├── knob_engine.h / .c        IIR filter, deadzone, MIDI scaling
│   ├── encoder_engine.h / .c     Velocity-aware acceleration
│   ├── audio_engine.h / .c       RMS, peak, peak-hold, clip detect
│   ├── jack_click_detect.h / .c  Multi-click FSM (single/double/triple/long)
│   ├── btn_led_engine.h / .c     HT16K33 2×8 buffer, SW blink/pulse
│   ├── strip_led_engine.h / .c   WS2812 per-LED effects + rainbow/VU
│   ├── dfu_engine.h / .c         SysEx DFU protocol, FMC programming
│   ├── eeprom_abstraction.h / .c Wear-leveling, CRC, RAM cache
│   ├── preset_store.h / .c       16-slot serialization, active slot
│   ├── gui_engine.h / .c         Dirty-rect strip-buffer renderer
│   ├── font.h / .c               Bitmap font 6×8, 8×16
│   └── widget.h / .c             Label, progress bar, VU meter, hbar
├── App/
│   ├── led_controller.h / .c     Unified dual LED facade (HT16K33 + WS2812)
│   ├── ui_manager.h / .c         Page system with event dispatch
│   ├── midi_api.h / .c           Convenience MIDI send wrappers
│   └── pages/
│       ├── page_home.h / .c      Channel, preset name, connection status
│       ├── page_monitor.h / .c   VU meter, MIDI activity
│       ├── page_config.h / .c    Routing, key/knob mapping
│       ├── page_mixer.h / .c     Fader, pan display
│       └── page_dfu.h / .c       Firmware upgrade progress
├── Bootloader/
│   └── boot_main.c               DFU flag check, app jump, vector reloc
├── GD32Lib/
│   ├── CMSIS/
│   ├── GD32F3x0_standard_peripheral/
│   └── GD32F3x0_usbfs_library/
├── SEGGER_RTT/
│   ├── SEGGER_RTT.h / .c
│   └── SEGGER_RTT_Conf.h
└── main.c                         Init 7 phases + main loop + IRQ handlers
```

## Hardware

### MCU

- **GD32F350RBT6** — Cortex-M4, 48MHz (IRC48M), 64KB Flash, 16KB SRAM
- USB Full-Speed (internal PHY), 2× USART, 2× SPI, 2× I2C, 12-bit ADC, 6× TIM

### Pin Assignment

| Function | Pin | Peripheral | Notes |
|---|---|---|---|
| UART TX (MIDI OUT) | PA9 | USART0 AF1 | DMA CH2 |
| UART RX (MIDI IN) | PA10 | USART0 AF1 | DMA CH3 + IDLE IRQ |
| Key matrix rows | PA0-PA3 | GPIO output | Active low scan |
| Key matrix cols | PA4-PA5, PB0-PB1 | GPIO input | Internal pull-up |
| ADC knobs | CH0-CH7 | ADC | DMA CH0 circular |
| Encoder A/B | TIM1 CH0/CH1 | TIMER1 | Hardware quadrature |
| LCD SCK | PB13 | SPI1 AF0 | 24MHz |
| LCD MOSI | PB15 | SPI1 AF0 | DMA CH4 |
| LCD DC | PB12 | GPIO output | Data/Command select |
| LCD RST | PB11 | GPIO output | Hardware reset |
| LCD CS | PB10 | GPIO output | Chip select |
| LCD Backlight | PA6 | TIM2 CH0 PWM | 0-255 brightness |
| I2C SCL | PB6 | I2C0 AF1 | 400kHz (shared bus) |
| I2C SDA | PB7 | I2C0 AF1 | EEPROM + HT16K33 |
| WS2812 Data | PB8 | TIM0 CH0 AF2 | DMA CH5, 800kHz PWM |
| Jack Detect | PB2 | EXTI | Pull-up, low = inserted |
| Jack Click | ADC CH15 | ADC AWD | Threshold < 620 |
| USB D+ | PA12 | USBFS | Internal pull-up |
| USB D- | PA11 | USBFS | |

### I2C Bus (shared, 400kHz)

| Device | Address | Function |
|---|---|---|
| AT24C64 | 0xA0 | 8KB EEPROM, 32-byte pages |
| HT16K33 | 0xE0 | 2×8 button indicator LED driver |

### DMA Channel Allocation

| Channel | Peripheral | Direction | Priority | Usage |
|---|---|---|---|---|
| CH0 | ADC | P→M | Medium | 8-knob continuous scan |
| CH2 | USART0 TX | M→P | High | MIDI OUT |
| CH3 | USART0 RX | P→M | High | MIDI IN (circular) |
| CH4 | SPI1 TX | M→P | Low | LCD frame refresh |
| CH5 | TIM0 CH0 | M→P | Medium | WS2812 bitstream |

### NVIC Priority

| Priority | IRQ | Source |
|---|---|---|
| 0 (highest) | USBFS | USB enumeration + data |
| 1 | DMA CH2/3 | UART MIDI TX/RX |
| 2 | USART0 | IDLE frame detect |
| 2 | EXTI4-15 | Jack insert/remove |
| 3 | SysTick | 1ms system tick |
| 3 | ADC_CMP | Audio click (AWD) |
| 4 (lowest) | DMA CH4/5/6 | LCD SPI + WS2812 LED |

### Flash Memory Map

```
0x0800_0000 ┌─────────────────────┐
            │  Bootloader (8KB)   │  Standalone project
0x0800_2000 ├─────────────────────┤
            │  Application (48KB) │  Main firmware (VTOR relocated)
0x0800_E000 ├─────────────────────┤
            │  Config (4KB)       │  NV config backup
0x0800_F000 ├─────────────────────┤
            │  DFU Staging (4KB)  │  Upgrade flag + temp params
0x0801_0000 └─────────────────────┘
```

### EEPROM Memory Map (AT24C64 8KB)

```
0x0000-0x001F   Global config (active preset slot, etc.)
0x0020-0x07FF   Preset slots × 16 (128 bytes each)
0x0800-0x0BFF   Wear-level mirror area
0x0C00-0x0FFF   Factory default backup
0x1000-0x1FFF   Reserved
```

## Main Loop Architecture

```
while (1) {
    router_poll_all();              // Highest: MIDI data (every cycle)
    
    if (st_expired(TM_INPUT))       // 1ms: key scan, ADC read, encoder
        poll_input();
    
    if (st_expired(TM_JACK))        // 5ms: jack click state machine
        jack_click(...);
    
    if (st_expired(TM_LCD))         // 33ms: LCD dirty-rect flush (30fps)
        gui_flush();
    
    if (st_expired(TM_LED))         // 20ms: HT16K33 + WS2812 update (50fps)
        lc_update(now);
    
    if (st_expired(TM_EEPROM))      // 1000ms: deferred EEPROM write
        if (ea_dirty()) ea_flush();
}
```

## Data Flow

```
MIDI TX:   App → midi_api → Router → Codec/Encode → Ring buf → USB ISR / UART DMA → wire
MIDI RX:   ISR → Ring buf → Parser → Router → rx_callback → event_post() → App
Input:     Key/ADC/Encoder → Engine (debounce/filter) → Mapper → MIDI TX + LED
Audio:     Jack ADC → TIM trigger DMA → audio_engine RMS/peak → VU widget → LCD/LED strip
Jack:      EXTI insert → click FSM → event_post() → App dispatch (start/stop/page switch)
LCD:       App page → GUI dirty-rect → Widget render → Strip buffer → SPI DMA → ST7789
Btn LED:   App → btn_led_engine → RAM mirror (dirty flag) → I2C burst → HT16K33
Strip LED: App → strip_led_engine → RGB buffer → TIM PWM DMA → WS2812B
DFU:       Host SysEx → sysex_assembler → dfu_engine → CRC32 verify → FMC program → reboot
```

## DFU Firmware Upgrade Protocol

SysEx-based application-mode DFU (no USB DFU Class needed):

```
Host → Device:
  F0 00 00 7F 01 <cmd> <payload...> F7

Commands:
  0x01 START   — total_size(4B), expected_crc32(4B)
  0x02 DATA    — block_num(2B), data(128B)
  0x03 VERIFY  — triggers CRC32 comparison
  0x04 REBOOT  — clear DFU flag, NVIC_SystemReset()

Device → Host:
  ACK SysEx with block_num + status after each DATA block
```

LCD can display progress bar + percentage during upgrade. Button LEDs show 16-step progress.

## Build

### Toolchain

- **Keil MDK** (ARM Compiler 6) or **arm-none-eabi-gcc**
- SEGGER J-Link for programming and RTT debug

### Keil Include Paths

```
Common
Utilities
Driver
Middleware
App
App/pages
GD32Lib/CMSIS
GD32Lib/GD32F3x0_standard_peripheral/Include
GD32Lib/GD32F3x0_usbfs_library/driver/Include
GD32Lib/GD32F3x0_usbfs_library/device/Include
SEGGER_RTT
```

### Preprocessor Defines

| Define | Purpose |
|---|---|
| `GD32F350` | Target MCU selection |
| `USE_STDPERIPH_DRIVER` | Enable GD32 standard peripheral library |
| `USE_ASSERT` | Enable runtime assert checks (debug only) |
| `DBG_LEVEL=4` | RTT log verbosity (0=off, 4=debug) |
| `BUILD_BOOTLOADER` | Compile bootloader instead of app |
| `STRIP_LEDS=16` | WS2812 strip length |

### Linker Settings

**Application:**
```
ROM: 0x08002000 (size: 0xC000 = 48KB)
RAM: 0x20000000 (size: 0x4000 = 16KB)
```

**Bootloader:**
```
ROM: 0x08000000 (size: 0x2000 = 8KB)
RAM: 0x20000000 (size: 0x4000 = 16KB)
```

## Dependencies

| Library | Version | Source |
|---|---|---|
| GD32F3x0 Firmware Library | V2.2.x | [GigaDevice](http://www.gd32mcu.com) |
| GD32F3x0 USBFS Library | V2.5.0 | GigaDevice |
| SEGGER RTT | V7.x | [SEGGER](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/) |

## RAM Budget (16KB)

```
Stack:              2KB
Ring buffers (×4):  2KB  (512B each: USB TX/RX, UART TX/RX)
LCD strip buffer:   7.5KB (240 × 16 × 2 bytes)
WS2812 DMA buffer:  1.6KB (32 LEDs × 24 bits × 2 bytes + reset)
EEPROM cache:       2KB  (64 records × 32 bytes)
MIDI/SysEx:         0.5KB
Others:             0.4KB (keys, knobs, encoders, timers, globals)
Total:             ~16KB
```

## License

Proprietary — Music Tribe internal use.
