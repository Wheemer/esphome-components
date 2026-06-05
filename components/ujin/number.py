import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_MIN_VALUE, CONF_MAX_VALUE, CONF_STEP
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinNumber = ujin_ns.class_("UjinNumber", number.Number, cg.Component)

CONF_SETTING = "setting"

CONFIG_SCHEMA = number.NUMBER_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(UjinNumber),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_SETTING): cv.enum({
        "buzzer_volume": 0,
        "timer1": 1,
        "timer2": 2,
        "min_brightness1": 3,
        "max_brightness1": 4,
        "min_brightness2": 5,
        "max_brightness2": 6,
    }),
    cv.Optional(CONF_MIN_VALUE, default=0): cv.float_,
    cv.Optional(CONF_MAX_VALUE, default=100): cv.float_,
    cv.Optional(CONF_STEP, default=1): cv.float_,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    cg.add(var.set_setting(config[CONF_SETTING]))
    
    await number.register_number(var, config)
