import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinSelect = ujin_ns.class_("UjinSelect", select.Select, cg.Component)

CONF_SELECT_TYPE = "select_type"

CONFIG_SCHEMA = select.SELECT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(UjinSelect),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_SELECT_TYPE): cv.enum({
        "operation_mode": 0,
        "dimming_mode": 1,
        "touch_sensitivity": 2,
    }),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    cg.add(var.set_select_type(config[CONF_SELECT_TYPE]))
    
    await select.register_select(var, config)
