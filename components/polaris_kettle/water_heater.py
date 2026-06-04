import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, water_heater
from esphome.const import CONF_ID, CONF_UART_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["water_heater"]

polaris_kettle_ns = cg.esphome_ns.namespace("polaris_kettle")
PolarisKettle = polaris_kettle_ns.class_(
    "PolarisKettle", water_heater.WaterHeater, cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(CONF_ID): cv.declare_id(PolarisKettle),
        cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    })
    .extend(water_heater.WATER_HEATER_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await water_heater.register_water_heater(var, config)
    
    uart_parent = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(uart_parent))
