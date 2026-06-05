import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import water_heater
from esphome.const import CONF_ID

from . import PolarisKettle, polaris_kettle_ns

DEPENDENCIES = ["polaris_kettle"]

PolarisWaterHeater = polaris_kettle_ns.class_(
    "PolarisKettle", water_heater.WaterHeater
)

CONFIG_SCHEMA = water_heater._WATER_HEATER_SCHEMA'.extend({
    cv.GenerateID(CONF_ID): cv.use_id(PolarisKettle),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
