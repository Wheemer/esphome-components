import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID

from . import RadSensComponent

DEPENDENCIES = ['rad_sens']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.use_id(RadSensComponent),
    cv.Optional("sensitivity"): number._NUMBER_SCHEMA.extend({
        cv.Optional("min_value", default=100): cv.float_,
        cv.Optional("max_value", default=1100): cv.float_,
        cv.Optional("step", default=1): cv.float_,
    }),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if "sensitivity" in config:
        sens_config = config["sensitivity"]
        
        # Если нет ID, создаём его автоматически
        if CONF_ID not in sens_config:
            sens_config[CONF_ID] = cg.global_id("rad_sens_sensitivity")
        
        # Создаём number через new_Pvariable
        num = cg.new_Pvariable(sens_config[CONF_ID])
        
        # Регистрируем number
        await number.register_number(num, sens_config)
        
        # Устанавливаем параметры
        cg.add(num.set_min_value(sens_config["min_value"]))
        cg.add(num.set_max_value(sens_config["max_value"]))
        cg.add(num.set_step(sens_config["step"]))
        
        # Устанавливаем callback для обновления чувствительности
        cg.add(parent.set_sensitivity_number(num))
