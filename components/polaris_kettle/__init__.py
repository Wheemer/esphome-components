import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UART_ID

from . import const, generate, schema, validate

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["water_heater"]
CODEOWNERS = ["@yourname"]
MULTI_CONF = True

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.declare_id(generate.PolarisKettle),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(const.CONF_SYNC_MODE, False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    uart_parent = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(uart_parent))
    
    if const.CONF_SYNC_MODE in config:
        cg.add(var.set_sync_mode(config[const.CONF_SYNC_MODE]))
