import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, water_heater, sensor, text_sensor, binary_sensor, button
from esphome.const import (
    CONF_ID, CONF_UART_ID, CONF_NAME, CONF_ICON,
    CONF_CURRENT_TEMPERATURE, CONF_TARGET_TEMPERATURE,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["water_heater", "sensor", "text_sensor", "binary_sensor", "button"]

CONF_CURRENT_TEMP_SENSOR = "current_temperature_sensor"
CONF_TARGET_TEMP_SENSOR = "target_temperature_sensor"
CONF_MODE_TEXT_SENSOR = "mode_text_sensor"
CONF_NO_KETTLE_SENSOR = "no_kettle_sensor"
CONF_NO_WATER_SENSOR = "no_water_sensor"
CONF_BLACK_TEA_BUTTON = "black_tea_button"
CONF_MIX_TEA_BUTTON = "mix_tea_button"
CONF_WHITE_TEA_BUTTON = "white_tea_button"
CONF_GREEN_TEA_BUTTON = "green_tea_button"
CONF_OOLONG_TEA_BUTTON = "oolong_tea_button"
CONF_BAG_TEA_BUTTON = "bag_tea_button"
CONF_BOIL_BUTTON = "boil_button"
CONF_KEEP_WARM_BUTTON = "keep_warm_button"

polaris_kettle_ns = cg.esphome_ns.namespace("polaris_kettle")
PolarisKettle = polaris_kettle_ns.class_(
    "PolarisKettle", water_heater.WaterHeater, cg.PollingComponent, uart.UARTDevice
)

BlackTeaButton = polaris_kettle_ns.class_("BlackTeaButton", button.Button, cg.Component)
MixTeaButton = polaris_kettle_ns.class_("MixTeaButton", button.Button, cg.Component)
WhiteTeaButton = polaris_kettle_ns.class_("WhiteTeaButton", button.Button, cg.Component)
GreenTeaButton = polaris_kettle_ns.class_("GreenTeaButton", button.Button, cg.Component)
OolongTeaButton = polaris_kettle_ns.class_("OolongTeaButton", button.Button, cg.Component)
BagTeaButton = polaris_kettle_ns.class_("BagTeaButton", button.Button, cg.Component)
BoilButton = polaris_kettle_ns.class_("BoilButton", button.Button, cg.Component)
KeepWarmButton = polaris_kettle_ns.class_("KeepWarmButton", button.Button, cg.Component)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(CONF_ID): cv.declare_id(PolarisKettle),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        
        # Сенсоры
        cv.Optional(CONF_CURRENT_TEMP_SENSOR): cv.maybe_simple_value(
            sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1, icon="mdi:thermometer"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_TARGET_TEMP_SENSOR): cv.maybe_simple_value(
            sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1, icon="mdi:thermometer"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_MODE_TEXT_SENSOR): cv.maybe_simple_value(
            text_sensor.text_sensor_schema(icon="mdi:kettle"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_NO_KETTLE_SENSOR): cv.maybe_simple_value(
            binary_sensor.binary_sensor_schema(icon="mdi:kettle-off"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_NO_WATER_SENSOR): cv.maybe_simple_value(
            binary_sensor.binary_sensor_schema(icon="mdi:water-off"),
            key=CONF_NAME,
        ),
        
        # Кнопки
        cv.Optional(CONF_BLACK_TEA_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:coffee"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_MIX_TEA_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:cup"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_WHITE_TEA_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:tea"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_GREEN_TEA_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:flower"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_OOLONG_TEA_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:leaf"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_BAG_TEA_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:tea"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_BOIL_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:kettle-steam"),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_KEEP_WARM_BUTTON): cv.maybe_simple_value(
            button.button_schema(icon="mdi:heat-wave"),
            key=CONF_NAME,
        ),
    })
    .extend(water_heater.WATER_HEATER_SCHEMA)
    .extend(cv.polling_component_schema("1000ms"))
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await water_heater.register_water_heater(var, config)
    
    uart_parent = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(uart_parent))
    
    # Сенсоры
    if CONF_CURRENT_TEMP_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_TEMP_SENSOR])
        cg.add(var.set_current_temperature_sensor(sens))
        
    if CONF_TARGET_TEMP_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_TARGET_TEMP_SENSOR])
        cg.add(var.set_target_temperature_sensor(sens))
        
    if CONF_MODE_TEXT_SENSOR in config:
        sens = await text_sensor.new_text_sensor(config[CONF_MODE_TEXT_SENSOR])
        cg.add(var.set_mode_text_sensor(sens))
        
    if CONF_NO_KETTLE_SENSOR in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_NO_KETTLE_SENSOR])
        cg.add(var.set_no_kettle_binary_sensor(sens))
        
    if CONF_NO_WATER_SENSOR in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_NO_WATER_SENSOR])
        cg.add(var.set_no_water_binary_sensor(sens))
    
    # Кнопки
    if CONF_BLACK_TEA_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_BLACK_TEA_BUTTON]))
        await cg.register_component(btn, config[CONF_BLACK_TEA_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_black_tea_button(btn))
        
    if CONF_MIX_TEA_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_MIX_TEA_BUTTON]))
        await cg.register_component(btn, config[CONF_MIX_TEA_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_mix_tea_button(btn))
        
    if CONF_WHITE_TEA_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_WHITE_TEA_BUTTON]))
        await cg.register_component(btn, config[CONF_WHITE_TEA_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_white_tea_button(btn))
        
    if CONF_GREEN_TEA_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_GREEN_TEA_BUTTON]))
        await cg.register_component(btn, config[CONF_GREEN_TEA_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_green_tea_button(btn))
        
    if CONF_OOLONG_TEA_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_OOLONG_TEA_BUTTON]))
        await cg.register_component(btn, config[CONF_OOLONG_TEA_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_oolong_tea_button(btn))
        
    if CONF_BAG_TEA_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_BAG_TEA_BUTTON]))
        await cg.register_component(btn, config[CONF_BAG_TEA_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_bag_tea_button(btn))
        
    if CONF_BOIL_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_BOIL_BUTTON]))
        await cg.register_component(btn, config[CONF_BOIL_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_boil_button(btn))
        
    if CONF_KEEP_WARM_BUTTON in config:
        btn = cg.new_Pvariable(await button.new_button(config[CONF_KEEP_WARM_BUTTON]))
        await cg.register_component(btn, config[CONF_KEEP_WARM_BUTTON])
        cg.add(btn.set_parent(var))
        cg.add(var.set_keep_warm_button(btn))
