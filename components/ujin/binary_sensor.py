import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinBinarySensor = ujin_ns.class_("UjinBinarySensor", binary_sensor.BinarySensor, cg.Component)

CONF_INPUT_TYPE = "input_type"
CONF_INPUT_NUMBER = "input_number"

INPUT_TYPES = {
    "external_input_1": 0,
    "external_input_2": 1,
    "any_external_input": 2,
    "dimmer1_enabled": 3,
    "dimmer2_enabled": 4,
    "channel1_on": 5,
    "channel2_on": 6,
    "power_phase_only": 7,
}

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(UjinBinarySensor),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_INPUT_TYPE): cv.enum(INPUT_TYPES),
    cv.Optional(CONF_INPUT_NUMBER, default=1): cv.int_range(min=1, max=2),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    
    input_type = config[CONF_INPUT_TYPE]
    cg.add(var.set_input_type(input_type))
    
    if input_type in [0, 1]:
        cg.add(var.set_input_number(config[CONF_INPUT_NUMBER]))
    
    if input_type == 0:
        cg.add(ujin.register_external_input_1_sensor(var))
    elif input_type == 1:
        cg.add(ujin.register_external_input_2_sensor(var))
    elif input_type == 2:
        cg.add(ujin.register_any_external_input_sensor(var))
    elif input_type == 3:
        cg.add(ujin.register_dimmer1_enabled_sensor(var))
    elif input_type == 4:
        cg.add(ujin.register_dimmer2_enabled_sensor(var))
    elif input_type == 5:
        cg.add(ujin.register_channel1_on_sensor(var))
    elif input_type == 6:
        cg.add(ujin.register_channel2_on_sensor(var))
    elif input_type == 7:
        cg.add(ujin.register_power_phase_only_sensor(var))
    
    await binary_sensor.register_binary_sensor(var, config)
