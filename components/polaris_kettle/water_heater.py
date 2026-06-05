import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import water_heater
from esphome.const import CONF_ID

from . import polaris_ns
from .const import CONF_POLARIS_ID

DEPENDENCIES = ["polaris"]

PolarisWaterHeater = polaris_ns.class_("PolarisKettle", water_heater.WaterHeater)

CONFIG_SCHEMA = water_heater.WATER_HEATER_SCHEMA.extend({
    cv.GenerateID(CONF_POLARIS_ID): cv.use_id(PolarisWaterHeater),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_POLARIS_ID])
    # Water heater уже зарегистрирован в основном компоненте
