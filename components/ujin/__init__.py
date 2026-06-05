import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.automation as automation
from esphome.const import CONF_ID, CONF_UART_ID
from esphome.components import uart

CODEOWNERS = ["@ujin"]
DEPENDENCIES = ["uart"]

ujin_ns = cg.esphome_ns.namespace("ujin")
UjinComponent = ujin_ns.class_("UjinComponent", cg.Component, uart.UARTDevice)

CONF_UJIN_ID = "ujin_id"
CONF_ON_STATE = "on_state"

# Автоматизация с использованием callback-метода (рекомендованный подход [citation:5])
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(UjinComponent),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_ON_STATE): automation.validate_automation({
        cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    }),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(uart_component))
    
    # Создание автоматизации с использованием build_callback_automation [citation:5]
    for conf in config.get(CONF_ON_STATE, []):
        await automation.build_callback_automation(
            var,
            "add_on_state_callback",
            [(ujin_ns.struct("DeviceState"), "state")],
            conf
        )
