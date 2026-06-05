import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID

from . import polaris_kettle_ns, PolarisKettle

DEPENDENCIES = ["polaris_kettle"]

BlackTeaButton = polaris_kettle_ns.class_("BlackTeaButton", button.Button, cg.Component)
MixTeaButton = polaris_kettle_ns.class_("MixTeaButton", button.Button, cg.Component)
WhiteTeaButton = polaris_kettle_ns.class_("WhiteTeaButton", button.Button, cg.Component)
GreenTeaButton = polaris_kettle_ns.class_("GreenTeaButton", button.Button, cg.Component)
OolongTeaButton = polaris_kettle_ns.class_("OolongTeaButton", button.Button, cg.Component)
BagTeaButton = polaris_kettle_ns.class_("BagTeaButton", button.Button, cg.Component)
BoilButton = polaris_kettle_ns.class_("BoilButton", button.Button, cg.Component)
KeepWarmButton = polaris_kettle_ns.class_("KeepWarmButton", button.Button, cg.Component)

CONF_POLARIS_KETTLE_ID = "polaris_kettle_id"

BUTTON_SCHEMA = button._BUTTON_SCHEMA.extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
}).extend(cv.COMPONENT_SCHEMA)

async def register_black_tea_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_black_tea_button(var))

async def register_mix_tea_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_mix_tea_button(var))

async def register_white_tea_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_white_tea_button(var))

async def register_green_tea_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_green_tea_button(var))

async def register_oolong_tea_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_oolong_tea_button(var))

async def register_bag_tea_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_bag_tea_button(var))

async def register_boil_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_boil_button(var))

async def register_keep_warm_button(var, config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_keep_warm_button(var))
