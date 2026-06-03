import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor, binary_sensor, switch, number
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
    CONF_UPDATE_INTERVAL,
    CONF_STEP,
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
    ICON_RADIOACTIVE,
    STATE_CLASS_MEASUREMENT,
)

# Импортируем константы из локального файла
from .const import (
    UNIT_MICROROENTGEN_PER_HOUR,
    UNIT_COUNT_PER_MINUTE,
    CONF_DYNAMIC_INTENSITY,
    CONF_STATIC_INTENSITY,
    CONF_COUNTS_PER_MINUTE,
    CONF_FIRMWARE_VERSION
)

DEPENDENCIES = ['i2c']
AUTO_LOAD = ['sensor', 'binary_sensor', 'switch', 'number']

rad_sens_ns = cg.esphome_ns.namespace('rad_sens')
RadSensComponent = rad_sens_ns.class_('RadSensComponent', i2c.I2CDevice, cg.Component)

# Используем импортированные константы
CONF_PULSES = CONF_COUNTS_PER_MINUTE
CONF_HV_GENERATOR_STATE = "hv_generator_state"
CONF_HV_GENERATOR_SWITCH = "hv_generator_switch"
CONF_SENSITIVITY_NUMBER = "sensitivity"

# Конфигурация сенсоров
RAD_SENS_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_MICROROENTGEN_PER_HOUR,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=3,
    state_class=STATE_CLASS_MEASUREMENT,
)

RAD_SENS_PULSES_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_COUNT_PER_MINUTE,
    icon=ICON_RADIOACTIVE,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
)

# Схема для number (чувствительность)
RAD_SENS_SENSITIVITY_NUMBER_SCHEMA = cv.Schema({
    cv.Optional(CONF_MIN_VALUE, default=100): cv.float_,
    cv.Optional(CONF_MAX_VALUE, default=1100): cv.float_,
    cv.Optional(CONF_STEP, default=1): cv.float_,
    cv.Optional(CONF_UNIT_OF_MEASUREMENT, default="imp/µR"): cv.string_strict,
    cv.Optional(CONF_ICON, default="mdi:tune"): cv.icon,
}).extend(number.NUMBER_SCHEMA)

# Схема конфигурации
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RadSensComponent),
    cv.Optional(CONF_ADDRESS, default=0x66): cv.i2c_address,
    cv.Optional(CONF_DYNAMIC_INTENSITY): RAD_SENS_SENSOR_SCHEMA,
    cv.Optional(CONF_STATIC_INTENSITY): RAD_SENS_SENSOR_SCHEMA,
    cv.Optional(CONF_PULSES): RAD_SENS_PULSES_SCHEMA,
    cv.Optional(CONF_HV_GENERATOR_STATE): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_HV_GENERATOR_SWITCH): switch.switch_schema(),
    cv.Optional(CONF_SENSITIVITY_NUMBER): RAD_SENS_SENSITIVITY_NUMBER_SCHEMA,
    cv.Optional(CONF_UPDATE_INTERVAL, default='60s'): cv.update_interval,
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x66))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    
    if CONF_DYNAMIC_INTENSITY in config:
        sens = await sensor.new_sensor(config[CONF_DYNAMIC_INTENSITY])
        cg.add(var.set_dynamic_intensity_sensor(sens))
    
    if CONF_STATIC_INTENSITY in config:
        sens = await sensor.new_sensor(config[CONF_STATIC_INTENSITY])
        cg.add(var.set_static_intensity_sensor(sens))
    
    if CONF_PULSES in config:
        sens = await sensor.new_sensor(config[CONF_PULSES])
        cg.add(var.set_pulses_sensor(sens))
    
    if CONF_HV_GENERATOR_STATE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_HV_GENERATOR_STATE])
        cg.add(var.set_hv_generator_state_sensor(bs))
    
    if CONF_HV_GENERATOR_SWITCH in config:
        sw = await switch.new_switch(config[CONF_HV_GENERATOR_SWITCH])
        cg.add(var.set_hv_generator_switch(sw))
    
    if CONF_SENSITIVITY_NUMBER in config:
        num = await number.new_number(
            config[CONF_SENSITIVITY_NUMBER],
            min_value=config[CONF_SENSITIVITY_NUMBER][CONF_MIN_VALUE],
            max_value=config[CONF_SENSITIVITY_NUMBER][CONF_MAX_VALUE],
            step=config[CONF_SENSITIVITY_NUMBER][CONF_STEP]
        )
        cg.add(var.set_sensitivity_number(num))
