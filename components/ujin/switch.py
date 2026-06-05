import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinSwitch = ujin_ns.class_("UjinSwitch", switch.Switch, cg.Component)

CONF_SWITCH_TYPE = "switch_type"

CONFIG_SCHEMA = switch.SWITCH_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(UjinSwitch),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_SWITCH_TYPE): cv.enum({
        "buzzer": 0,
        "dimming_channel1": 1,
        "dimming_channel2": 2,
    }),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    cg.add(var.set_switch_type(config[CONF_SWITCH_TYPE]))
    
    await switch.register_switch(var, config)
