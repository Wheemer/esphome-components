``` yaml
uart:
  tx_pin: GPIO1
  rx_pin: GPIO3
  baud_rate: 9600

water_heater:
  - platform: polaris_kettle
    id: kettle
    name: "Чайник Polaris"
    icon: "mdi:kettle"
    min_temperature: 40
    max_temperature: 100
    target_temperature_step: 1
    supported_modes:
      - OFF
      - ECO
      - PERFORMANCE
      - GAS
    
    # Сенсоры
    current_temperature_sensor:
      name: "Текущая температура"
    target_temperature_sensor:
      name: "Установленная температура"
    mode_text_sensor:
      name: "Режим чайника"
    no_kettle_sensor:
      name: "Нет чайника"
    no_water_sensor:
      name: "Нет воды"
    
    # Кнопки
    black_tea_button:
      name: "Черный чай / Кофе 95°C"
    mix_tea_button:
      name: "Смесь 40°C"
    white_tea_button:
      name: "Белый чай 65°C"
    green_tea_button:
      name: "Зеленый цветочный 80°C"
    oolong_tea_button:
      name: "Красный улун 90°C"
    bag_tea_button:
      name: "Пакетированный 100°C"
    boil_button:
      name: "Кипячение"
    keep_warm_button:
      name: "Подогрев с удержанием"
```
