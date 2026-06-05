import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UART_ID
from esphome.components import uart
from esphome.core import CORE, coroutine_with_priority

CODEOWNERS = ["@ujin"]
DEPENDENCIES = ["uart"]

ujin_ns = cg.esphome_ns.namespace("ujin")
UjinComponent = ujin_ns.class_("UjinComponent", cg.Component, uart.UARTDevice)

CONF_UJIN_ID = "ujin_id"
CONF_ON_STATE = "on_state"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(UjinComponent),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_ON_STATE): cv.validate_automation({
        cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    }),
}).extend(cv.COMPONENT_SCHEMA)

@coroutine_with_priority(100.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(uart_component))
    
    for conf in config.get(CONF_ON_STATE, []):
        trigger = cg.new_Pvariable(conf[CONF_ID], var)
        await cg.register_automation(trigger, conf)
