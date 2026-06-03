import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_SENSOR,
    CONF_ENABLE_PIN,
    CONF_VOLTAGE,
    UNIT_VOLT,
    UNIT_PERCENT,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_BATTERY,
    ICON_BATTERY,
)

CONF_VOLTAGE_DIVIDER = "voltage_divider"
CONF_BUS_VOLTAGE = "bus_voltage"
CONF_LEVEL = "level"
CONF_REFERENCE_VOLTAGE = "reference_voltage"

ICON_BATTERY_CHARGING = "mdi:battery-charging"

Lilygotbattery_ns = cg.esphome_ns.namespace("lilygo_t_battery")
LilygotBattery = Lilygotbattery_ns.class_("LilygotBattery", cg.PollingComponent)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LilygotBattery),
    cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_VOLTAGE_DIVIDER, default=6.0): cv.float_,
    cv.Optional(CONF_REFERENCE_VOLTAGE, default=3.3): cv.float_,
    cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        icon=ICON_BATTERY,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    cv.Optional(CONF_BUS_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=2,
        icon=ICON_BATTERY_CHARGING,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    cv.Optional(CONF_LEVEL): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        icon=ICON_BATTERY,
        device_class=DEVICE_CLASS_BATTERY,
    ),
}).extend(cv.polling_component_schema("30s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Устанавливаем ADC сенсор
    sens = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_adc_sensor(sens))
    
    # Устанавливаем делитель напряжения
    if CONF_VOLTAGE_DIVIDER in config:
        cg.add(var.set_voltage_divider(config[CONF_VOLTAGE_DIVIDER]))
    
    # Устанавливаем опорное напряжение
    if CONF_REFERENCE_VOLTAGE in config:
        cg.add(var.set_reference_voltage(config[CONF_REFERENCE_VOLTAGE]))
    
    # Устанавливаем сенсоры
    if CONF_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage_sensor(sens))
    
    if CONF_BUS_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BUS_VOLTAGE])
        cg.add(var.set_bus_voltage_sensor(sens))
    
    if CONF_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_LEVEL])
        cg.add(var.set_battery_level_sensor(sens))
    
    # Устанавливаем enable пин
    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))
