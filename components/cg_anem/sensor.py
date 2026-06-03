import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins, automation
from esphome.components import i2c, sensor, binary_sensor
from esphome.const import (
    CONF_ID,
    CONF_SLEEP_PIN,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_WIND_SPEED,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLUME,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_PROBLEM,
    UNIT_CELSIUS,
    UNIT_WATT,
    UNIT_VOLT,
    UNIT_CUBIC_METER_PER_HOUR,
    STATE_CLASS_MEASUREMENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@ananevgv"]
AUTO_LOAD = ["binary_sensor"]

# Константы
CONF_DIAMETER = "diameter"
CONF_ENABLE_SLEEP = "enable_sleep"

# Конфигурационные ключи для сенсоров
CONF_AMBIENT_TEMPERATURE = "ambient_temperature"
CONF_HOTEND_TEMPERATURE = "hotend_temperature"
CONF_TEMPERATURE_DIFFERENCE = "temperature_difference"
CONF_HEAT_POWER = "heat_power"
CONF_AIR_CONSUMPTION = "air_consumption"
CONF_AIR_FLOW_RATE = "air_flow_rate"
CONF_FIRMWARE_VERSION = "firmware_version"
CONF_MIN_AIR_FLOW_RATE = "min_air_flow_rate"
CONF_MAX_AIR_FLOW_RATE = "max_air_flow_rate"
CONF_SUPPLY_VOLTAGE = "supply_voltage"

# Конфигурационные ключи для бинарных сенсоров
CONF_STATUS_UP = "status_up"
CONF_STATUS_OV = "status_ov"
CONF_STATUS_WDT = "status_wdt"
CONF_STATUS_INCORRECT_TARING = "status_incorrect_taring"
CONF_STATUS_INCORRECT_TARING_RANGE = "status_incorrect_taring_range"

UNIT_METER_PER_SECOND = "m/s"
UNIT_EMPTY = ""

cg_anem_ns = cg.esphome_ns.namespace("cg_anem")
CGAnemComponent = cg_anem_ns.class_(
    "CGAnemComponent", cg.PollingComponent, i2c.I2CDevice
)

# Схема бинарного сенсора (общая для всех)
def binary_sensor_schema():
    return binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_PROBLEM,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )

# Основная схема конфигурации
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CGAnemComponent),
            
            cv.Optional(CONF_DIAMETER): cv.All(
                cv.float_with_unit("diameter", "(mm)"),
                cv.float_range(min=1, max=1000),
            ),
            
            cv.Optional(CONF_SLEEP_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_ENABLE_SLEEP, default=False): cv.boolean,
            
            # Сенсоры
            cv.Optional(CONF_AMBIENT_TEMPERATURE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_TEMPERATURE,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HOTEND_TEMPERATURE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_TEMPERATURE,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_DIFFERENCE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_TEMPERATURE,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_AIR_FLOW_RATE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_WIND_SPEED,
                unit_of_measurement=UNIT_METER_PER_SECOND,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_AIR_FLOW_RATE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_WIND_SPEED,
                unit_of_measurement=UNIT_METER_PER_SECOND,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_AIR_FLOW_RATE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_WIND_SPEED,
                unit_of_measurement=UNIT_METER_PER_SECOND,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HEAT_POWER): sensor.sensor_schema(
                device_class=DEVICE_CLASS_POWER,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SUPPLY_VOLTAGE): sensor.sensor_schema(
                device_class=DEVICE_CLASS_VOLTAGE,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_AIR_CONSUMPTION): sensor.sensor_schema(
                device_class=DEVICE_CLASS_VOLUME,
                unit_of_measurement=UNIT_CUBIC_METER_PER_HOUR,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FIRMWARE_VERSION): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            
            # Бинарные сенсоры
            cv.Optional(CONF_STATUS_UP): binary_sensor_schema(),
            cv.Optional(CONF_STATUS_OV): binary_sensor_schema(),
            cv.Optional(CONF_STATUS_WDT): binary_sensor_schema(),
            cv.Optional(CONF_STATUS_INCORRECT_TARING): binary_sensor_schema(),
            cv.Optional(CONF_STATUS_INCORRECT_TARING_RANGE): binary_sensor_schema(),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x11))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    if CONF_DIAMETER in config:
        cg.add(var.set_diameter(config[CONF_DIAMETER]))
    
    if CONF_SLEEP_PIN in config:
        sleep_pin = await cg.gpio_pin_expression(config[CONF_SLEEP_PIN])
        cg.add(var.set_sleep_pin(sleep_pin))
    
    if CONF_ENABLE_SLEEP in config:
        cg.add(var.set_enable_sleep(config[CONF_ENABLE_SLEEP]))
    
    # Регистрация сенсоров
    sensors = [
        (CONF_AMBIENT_TEMPERATURE, var.set_ambient_temperature_sensor),
        (CONF_HOTEND_TEMPERATURE, var.set_hotend_temperature_sensor),
        (CONF_TEMPERATURE_DIFFERENCE, var.set_temperature_difference_sensor),
        (CONF_AIR_FLOW_RATE, var.set_air_flow_rate_sensor),
        (CONF_MIN_AIR_FLOW_RATE, var.set_min_air_flow_rate_sensor),
        (CONF_MAX_AIR_FLOW_RATE, var.set_max_air_flow_rate_sensor),
        (CONF_HEAT_POWER, var.set_heat_power_sensor),
        (CONF_SUPPLY_VOLTAGE, var.set_supply_voltage_sensor),
        (CONF_FIRMWARE_VERSION, var.set_firmware_version_sensor),
        (CONF_AIR_CONSUMPTION, var.set_air_consumption_sensor),
    ]
    
    for key, setter in sensors:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(setter(sens))
    
    # Регистрация бинарных сенсоров
    binary_sensors = [
        (CONF_STATUS_UP, var.set_status_up_binary_sensor),
        (CONF_STATUS_OV, var.set_status_ov_binary_sensor),
        (CONF_STATUS_WDT, var.set_status_wdt_binary_sensor),
        (CONF_STATUS_INCORRECT_TARING, var.set_status_incorrect_taring_binary_sensor),
        (CONF_STATUS_INCORRECT_TARING_RANGE, var.set_status_incorrect_taring_range_binary_sensor),
    ]
    
    for key, setter in binary_sensors:
        if key in config:
            sens = await binary_sensor.new_binary_sensor(config[key])
            cg.add(setter(sens))
    