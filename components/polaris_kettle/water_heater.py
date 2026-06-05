import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import water_heater
from esphome.const import CONF_ID

from . import polaris_kettle_ns, PolarisKettle, CONF_POLARIS_KETTLE_ID

DEPENDENCIES = ["polaris_kettle"]

# Для water_heater используем стандартный подход
CONFIG_SCHEMA = water_heater.WATER_HEATER_SCHEMA.extend({
    cv.GenerateID(CONF_POLARIS_KETTLE_ID): cv.use_id(PolarisKettle),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_POLARIS_KETTLE_ID])
    # Водонагреватель уже зарегистрирован в основном компоненте
