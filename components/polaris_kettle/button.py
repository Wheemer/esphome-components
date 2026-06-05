import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID, CONF_TYPE

from . import generate, polaris_ns
from .const import CONF_POLARIS_ID

DEPENDENCIES = ["polaris"]

PolarisButton = polaris_ns.class_("PolarisButton", button.Button, cg.Component)

CONF_BUTTON_TYPE = "type"
BUTTON_TYPES = [
    "black_tea", "mix_tea", "white_tea", "green_tea",
    "oolong_tea", "bag_tea", "boil", "keep_warm"
]

CONFIG_SCHEMA = button.button_schema(PolarisButton).extend({
    cv.GenerateID(CONF_ID): cv.declare_id(PolarisButton),
    cv.GenerateID(CONF_POLARIS_ID): cv.use_id(generate.PolarisKettle),
    cv.Required(CONF_BUTTON_TYPE): cv.one_of(*BUTTON_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_ID])
    cg.add(var.set_parent(parent))
    
    button_type = config[CONF_BUTTON_TYPE]
    cg.add(var.set_type(button_type))
    cg.add(parent.set_button(var, button_type))
