import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID, CONF_NAME
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinButton = ujin_ns.class_("UjinButton", button.Button, cg.Component)

CONF_BUTTON_TYPE = "button_type"
CONF_DURATION = "duration"

CONFIG_SCHEMA = button.BUTTON_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(UjinButton),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_BUTTON_TYPE): cv.enum({
        "beep_short": 0,
        "beep_long": 1,
        "beep_custom": 2,
        "save_config": 3,
        "load_config": 4,
    }),
    cv.Optional(CONF_DURATION, default=0xFA): cv.hex_int_range(min=0, max=255),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    cg.add(var.set_button_type(config[CONF_BUTTON_TYPE]))
    
    if config[CONF_BUTTON_TYPE] == 2:
        cg.add(var.set_duration(config[CONF_DURATION]))
    
    await button.register_button(var, config)
