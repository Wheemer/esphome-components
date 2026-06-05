import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID, CONF_TYPE

from . import polaris_kettle_ns, PolarisKettle, CONF_POLARIS_KETTLE_ID

DEPENDENCIES = ["polaris_kettle"]

PolarisButton = polaris_kettle_ns.class_(
    "PolarisButton", button.Button, cg.Component
)

CONF_BUTTON_TYPE = "type"
BUTTON_TYPES = {
    "black_tea": "black_tea",
    "mix_tea": "mix_tea",
    "white_tea": "white_tea",
    "green_tea": "green_tea",
    "oolong_tea": "oolong_tea",
    "bag_tea": "bag_tea",
    "boil": "boil",
    "keep_warm": "keep_warm",
}

CONFIG_SCHEMA = button.BUTTON_SCHEMA.extend({
    cv.GenerateID(CONF_ID): cv.declare_id(PolarisButton),
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
    cv.Required(CONF_BUTTON_TYPE): cv.enum(BUTTON_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    
    button_type = config[CONF_BUTTON_TYPE]
    cg.add(var.set_type(button_type))
