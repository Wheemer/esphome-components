# Основной конфиг ESPHome

``` yaml
uart:
  id: uart_0
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

ujin:
  id: ujin_dimmer
  uart_id: uart_0
  on_state:
    - logger.log: "Dimmer state updated"
  on_settings_changed:
    - logger.log: "Settings changed"

# Свет
light:
  - platform: ujin
    name: "Лампа 1"
    channel: 1
    ujin_id: ujin_dimmer
    restore_mode: ALWAYS_OFF
    
  - platform: ujin
    name: "Лампа 2"
    channel: 2
    ujin_id: ujin_dimmer
    restore_mode: ALWAYS_OFF

# Кнопки
button:
  - platform: ujin
    name: "Короткий звук"
    button_type: beep_short
    ujin_id: ujin_dimmer
    
  - platform: ujin
    name: "Длинный звук"
    button_type: beep_long
    ujin_id: ujin_dimmer
    
  - platform: ujin
    name: "Сохранить конфиг"
    button_type: save_config
    ujin_id: ujin_dimmer

# Переключатели
switch:
  - platform: ujin
    name: "Звук кнопок"
    switch_type: buzzer
    ujin_id: ujin_dimmer
    
  - platform: ujin
    name: "Диммирование канала 1"
    switch_type: dimming_channel1
    ujin_id: ujin_dimmer
    
  - platform: ujin
    name: "Диммирование канала 2"
    switch_type: dimming_channel2
    ujin_id: ujin_dimmer

# Числовые настройки
number:
  - platform: ujin
    name: "Громкость зуммера"
    setting: buzzer_volume
    min_value: 0
    max_value: 100
    step: 5
    ujin_id: ujin_dimmer
    
  - platform: ujin
    name: "Таймер канала 1"
    setting: timer1
    min_value: 0
    max_value: 65535
    step: 60
    ujin_id: ujin_dimmer

# Выбор режимов
select:
  - platform: ujin
    name: "Режим работы"
    select_type: operation_mode
    ujin_id: ujin_dimmer
    
  - platform: ujin
    name: "Режим диммирования"
    select_type: dimming_mode
    ujin_id: ujin_dimmer

    
  - platform: ujin
    name: "Чувствительность сенсора"
    select_type: touch_sensitivity
    ujin_id: ujin_dimmer

binary_sensor:
  - platform: ujin
    name: "Внешний вход 1"
    input_type: external_input_1
    input_number: 1
    ujin_id: ujin_dimmer
    device_class: connectivity
    
  - platform: ujin
    name: "Внешний вход 2"
    input_type: external_input_2
    input_number: 2
    ujin_id: ujin_dimmer
    device_class: connectivity
    
  - platform: ujin
    name: "Любой внешний вход"
    input_type: any_external_input
    ujin_id: ujin_dimmer
    device_class: connectivity
    
  - platform: ujin
    name: "Диммирование канала 1"
    input_type: dimmer1_enabled
    ujin_id: ujin_dimmer
    device_class: power
    entity_category: diagnostic
    
  - platform: ujin
    name: "Диммирование канала 2"
    input_type: dimmer2_enabled
    ujin_id: ujin_dimmer
    device_class: power
    entity_category: diagnostic
    
  - platform: ujin
    name: "Канал 1 включен"
    input_type: channel1_on
    ujin_id: ujin_dimmer
    device_class: light
    
  - platform: ujin
    name: "Канал 2 включен"
    input_type: channel2_on
    ujin_id: ujin_dimmer
    device_class: light
    
  - platform: ujin
    name: "Только фаза"
    input_type: power_phase_only
    ujin_id: ujin_dimmer
    device_class: power
    entity_category: diagnostic
```
