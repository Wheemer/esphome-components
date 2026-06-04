## TEST!!!! ##

``` yaml

uart:
  tx_pin: GPIO1
  rx_pin: GPIO3
  baud_rate: 9600

# Основной компонент
polaris_kettle:
  id: kettle
  uart_id: uart_bus

# Водонагреватель (главная карточка)
water_heater:
  - platform: polaris_kettle
    id: kettle_heater
    name: "Чайник Polaris"
    min_temperature: 40
    max_temperature: 100
    target_temperature_step: 1
    supported_modes:
      - OFF
      - ECO
      - PERFORMANCE
      - GAS

# Сенсоры
sensor:
  - platform: polaris_kettle
    type: current_temperature
    polaris_kettle_id: kettle
    name: "Текущая температура"
    
  - platform: polaris_kettle
    type: target_temperature
    polaris_kettle_id: kettle
    name: "Установленная температура"

# Текстовый сенсор
text_sensor:
  - platform: polaris_kettle
    type: mode
    polaris_kettle_id: kettle
    name: "Режим чайника"

# Бинарные сенсоры
binary_sensor:
  - platform: polaris_kettle
    type: no_kettle
    polaris_kettle_id: kettle
    name: "Нет чайника"
    
  - platform: polaris_kettle
    type: no_water
    polaris_kettle_id: kettle
    name: "Нет воды"

# Кнопки для чайной церемонии
button:
  - platform: polaris_kettle
    type: black_tea
    polaris_kettle_id: kettle
    name: "Черный чай / Кофе 95°C"
    icon: "mdi:coffee"
    
  - platform: polaris_kettle
    type: mix_tea
    polaris_kettle_id: kettle
    name: "Смесь 40°C"
    icon: "mdi:cup"
    
  - platform: polaris_kettle
    type: white_tea
    polaris_kettle_id: kettle
    name: "Белый чай 65°C"
    icon: "mdi:tea"
    
  - platform: polaris_kettle
    type: green_tea
    polaris_kettle_id: kettle
    name: "Зеленый цветочный 80°C"
    icon: "mdi:flower"
    
  - platform: polaris_kettle
    type: oolong_tea
    polaris_kettle_id: kettle
    name: "Красный улун 90°C"
    icon: "mdi:leaf"
    
  - platform: polaris_kettle
    type: bag_tea
    polaris_kettle_id: kettle
    name: "Пакетированный 100°C"
    icon: "mdi:tea"
    
  - platform: polaris_kettle
    type: boil
    polaris_kettle_id: kettle
    name: "Кипячение"
    icon: "mdi:kettle-steam"
    
  - platform: polaris_kettle
    type: keep_warm
    polaris_kettle_id: kettle
    name: "Подогрев с удержанием"
    icon: "mdi:heat-wave"


```
