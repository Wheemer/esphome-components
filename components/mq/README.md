# ESPHome component MQ gaz sensor

основан на

https://github.com/nonameplum/esphome_devices/

https://github.com/miguel5612/MQSensorsLib
# Схема включения ESP8266-ESP32

<img src="https://raw.githubusercontent.com/miguel5612/MQSensorsLib_Docs/master/static/img/MQ_ESP8266.PNG" height="300" alt="MQ">




```yaml



external_components:
  source: github://ananyevgv/esphome-components@mq
  components: [mq]
  refresh: 0s

sensor:
  - platform: adc
    id: mq_adc
    pin: A0
    name: "MQ ADC Voltage"
    unit_of_measurement: "V"
    accuracy_decimals: 3
# ESP32 номинал резисторов  делителя 3.1кОм и 1.9 кОм
    attenuation: auto  # ESPHome handles this
   # update_interval: 1s
# ESP8266 Wemos d1 номинал резисторов  делителя 3.3кОм и 1.7 кОм
    filters:
      - multiply: 3.3
      
  - platform: mq
    model: MQ4
    r0:  4.4
    adc_sensor: mq_adc  # Reference to ADC sensor
    rl: 10  # Load resistor in kOhm
    vr: 3.3  # Voltage resolution
    update_interval: 30s
    sensor_co:
      name: "Carbon Monoxide"
      device_class: carbon_monoxide
    sensor_lpg:
      name: "lpg"
    sensor_ch4:
      name: "ch4"
    sensor_smoke:
      name: "smoke"
    sensor_alcohol:
      name: "Alcohol"
```

# Поддерживаемые датчики и газы

# MQ-2 (Горючие газы и дым)  RS/R0  -> 9.83
model: MQ2
сенсоры:
 sensor_h2, sensor_lpg, sensor_co, sensor_alcohol, sensor_propane
 
# MQ-3 (Алкоголь)  RS/R0  -> 60.0
model: MQ3
Доступные сенсоры:
 sensor_lpg, sensor_ch4, sensor_co, sensor_alcohol, sensor_benzene, sensor_hexane
 
# MQ-4 (Природный газ)  RS/R0  -> 	4.4
model: MQ4
Доступные сенсоры:
 sensor_lpg, sensor_ch4, sensor_co, sensor_alcohol, sensor_smoke
 
# MQ-5 (Горючие газы)  RS/R0  -> 6.5
model: MQ5
Доступные сенсоры:
 sensor_h2, sensor_lpg, sensor_ch4, sensor_co, sensor_alcohol
 
# MQ-6 (Сжиженный газ)  RS/R0  -> 10.0
model: MQ6
 Доступные сенсоры:
 sensor_h2, sensor_lpg, sensor_ch4, sensor_co, sensor_alcohol
 
# MQ-7 (Угарный газ)   RS/R0  ->  27.5
model: MQ7
Доступные сенсоры:
 sensor_h2, sensor_lpg, sensor_ch4, sensor_co, sensor_alcohol
 
# MQ-8 (Водород)   RS/R0  ->  70.0
model: MQ8
Доступные сенсоры:
 sensor_h2, sensor_lpg, sensor_ch4, sensor_co, sensor_alcohol
 
# MQ-9 (Угарный газ и горючие газы)   RS/R0  -> 9.6
model: MQ9
Доступные сенсоры:
 sensor_lpg, sensor_ch4, sensor_co
 
# MQ-131 (Озон)   RS/R0  -> 	15.0
model: MQ131
Доступные сенсоры:
 sensor_nox, sensor_cl2, sensor_o3
 
# MQ-135 (Качество воздуха)   RS/R0  ->  	3.6
model: MQ135
Доступные сенсоры:
 sensor_co, sensor_alcohol, sensor_co2, sensor_tolueno, sensor_nh4, sensor_acetona
 
# MQ-136 (Сероводород)   RS/R0  ->  	3.6
model: MQ136
Доступные сенсоры:
 sensor_h2s, sensor_nh4, sensor_co
 
# MQ-303A (Спирт)   RS/R0  ->  1.0
model: MQ303A
Доступные сенсоры:
 sensor_iso_butano, sensor_hydrogen, sensor_ethanol
 
# MQ-309A (Водород и угарный газ)   RS/R0  -> 11.0
model: MQ309A
Доступные сенсоры:
 sensor_h2, sensor_ch4, sensor_co, sensor_alcohol
    
Номинал RL (R2), значение RL в кОм  (!!!!!!!! на двух одинаковых датчиках номиналы резисторов могут быть разные 1кОм-120кОм !!!!!!!!)

<img src="https://static.insales-cdn.com/files/1/7780/30170724/original/mceclip8-1688735102483.jpg" height="300" alt="RL">

Значение RS/R0 (из спецификации вашего датчика) RS/R0 (Чистый воздух)

 <img src="https://raw.githubusercontent.com/miguel5612/MQSensorsLib_Docs/master/static/img/Graph_Explanation.jpeg" height="300" alt="R0">

https://www.yourduino.ru/blogs/blog/datchika-gaza-mq-7-slozhnosti-izmereniya-urovnya-ugarnogo-gaza

# примеры в sensor.yaml
