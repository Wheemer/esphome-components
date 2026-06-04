import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID, CONF_NAME
from . import ujin_ns, UjinComponent, CONF_UJIN_ID

UjinSelect = ujin_ns.class_("UjinSelect", select.Select, cg.Component)

CONF_SELECT_TYPE = "select_type"

CONFIG_SCHEMA = select.SELECT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(UjinSelect),
    cv.GenerateID(CONF_UJIN_ID): cv.use_id(UjinComponent),
    cv.Required(CONF_SELECT_TYPE): cv.enum({
        "operation_mode": 0,
        "dimming_mode": 1,
        "touch_sensitivity": 2,
    }),
    cv.Optional("options", default={
        0: ["Двойной", "Одиночный", "Локальный-удаленный", "Удаленный-локальный"],
        1: ["Оба канала", "Только канал 1", "Только канал 2", "Отключено"],
        2: ["Низкая", "Средняя", "Высокая"],
    }): cv.ensure_list(cv.string_strict),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ujin = await cg.get_variable(config[CONF_UJIN_ID])
    cg.add(var.set_ujin_parent(ujin))
    cg.add(var.set_select_type(config[CONF_SELECT_TYPE]))
    
    options = config["options"][config[CONF_SELECT_TYPE]]
    cg.add(var.traits.set_options(options))
    
    await select.register_select(var, config)
