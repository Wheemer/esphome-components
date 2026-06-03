Dosimeter  ESPHome for Climateguard RadSens 1v2, 1v7, 2v7 +
==========
По мотивам 
https://github.com/med-lasers/Dosimeter_RadSens_NarodMon
``` yaml

sensor:
  - platform: rad_sens
    dynamic_intensity:
      name: "Dynamic Intensity"
    static_intensity:
      name: "Static Intensity"
    pulses:
      name: "Counts Per Minute"
    update_interval: 60s
    sensitivity: 105  
    address: 0x66

binary_sensor:
  - platform: rad_sens
    hv_generator_state:
      name: "HV Generator State"

switch:
  - platform: rad_sens
    hv_generator_switch:
      name: "High Voltage Generator"
    led_switch:
      name: "LED Indicator"
    low_power_switch:
      name: "Low Power Mode"


```
Подключение
|ESP32            |RadSens          |
|-----------------|-----------------|
| GPIO21 | SDA to RadSens| 
| GPIO22 | SCL to RadSens|
| GPIO17 | pulse input from RadSens, + Buzzer| 
| GPIO2 | - Buzzer| 
| GPIO32 | + RadSens |
| GPIO0 | sleep|
| GPIO35|  wakeup|

<img src="https://github.com/ananyevgv/esphome-components/blob/main/components/RadSens/dosimeter.jpg" height="300" alt="Дозимер">

# ESPHome custom component sensor for Climateguard RadSens 1v2, 1v7, 2v7



## Known issues

#### RadSens 1v5
[1v5 boards workaround](https://github.com/maaad/RadSens1v2/issues/3#issuecomment-1289578773)



## References

[Official RadSens library by ClimateGuard](https://github.com/climateguard/RadSens)

[ESPHome Documentation](https://esphome.io/index.html)
