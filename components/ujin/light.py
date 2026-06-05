import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_OUTPUT_ID, CONF_ID
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinLight = ujin_ns.class_("UjinLight", light.LightOutput, cg.Component)

CONF_CHANNEL = "channel"

CONFIG_SCHEMA = light.LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(UjinLight),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=2),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    
    await light.register_light(var, config)
