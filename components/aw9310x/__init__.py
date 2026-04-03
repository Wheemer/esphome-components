import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
    CONF_INTERRUPT_PIN,
    CONF_THRESHOLD,
    CONF_CHANNEL,
    CONF_NAME,
    CONF_TYPE,
    CONF_GAIN,
    CONF_RESOLUTION,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor", "sensor"]
CODEOWNERS = ["ananyevgv"]

aw9310x_ns = cg.esphome_ns.namespace("aw9310x")
CONF_AW9310X_ID = "aw9310x_id"

AW9310XComponent = aw9310x_ns.class_("AW9310XComponent", cg.Component, i2c.I2CDevice)
AW9310XChannel = aw9310x_ns.class_("AW9310XChannel", binary_sensor.BinarySensor)

# Режимы работы
OperationMode = aw9310x_ns.enum("OperationMode")
OPERATION_MODES = {
    "ACTIVE": OperationMode.ACTIVE_MODE,
    "SLEEP": OperationMode.SLEEP_MODE,
    "DOZE": OperationMode.DOZE_MODE,
    "DEEPSLEEP": OperationMode.DEEPSLEEP_MODE,
}

# Конфигурация канала
ChannelConfig = aw9310x_ns.struct("ChannelConfig")

CHANNEL_SCHEMA = cv.Schema({
    cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=2),
    cv.Optional("type", default="touch"): cv.enum(["touch", "proximity", "both"]),
    cv.Optional("proximity_threshold", default=50000): cv.int_range(min=0, max=200000),
    cv.Optional("touch_threshold", default=80000): cv.int_range(min=0, max=200000),
    cv.Optional(CONF_GAIN, default=4): cv.int_range(min=0, max=7),
    cv.Optional(CONF_RESOLUTION, default=6): cv.int_range(min=0, max=7),
    cv.Optional("freq", default=3): cv.int_range(min=0, max=7),
    cv.Optional("doze_period_ms", default=400): cv.int_range(min=100, max=5000),
})

