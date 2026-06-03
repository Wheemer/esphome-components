import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID, CONF_ADDRESS, CONF_UPDATE_INTERVAL

DEPENDENCIES = ['i2c']
AUTO_LOAD = ['sensor', 'binary_sensor', 'switch', 'number']

CONF_RADSENS_ID = CONF_ID
rad_sens_ns = rad_sens_ns
rad_sens_ns = cg.esphome_ns.namespace('rad_sens')
RadSensComponent = rad_sens_ns.class_('RadSensComponent', i2c.I2CDevice, cg.Component)

CONF_SENSITIVITY = "sensitivity"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RadSensComponent),
    cv.Optional(CONF_ADDRESS, default=0x66): cv.i2c_address,
    cv.Optional(CONF_UPDATE_INTERVAL, default='60s'): cv.update_interval,
    cv.Optional(CONF_SENSITIVITY, default=105): cv.int_range(min=100, max=1100),
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x66))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    
    if CONF_SENSITIVITY in config:
        cg.add(var.set_sensitivity(config[CONF_SENSITIVITY]))
