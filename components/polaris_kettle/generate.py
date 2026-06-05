import esphome.codegen as cg

from . import const

polaris_ns = cg.esphome_ns.namespace("polaris")
PolarisKettle = polaris_ns.class_("PolarisKettle", cg.Component, cg.uart.UARTDevice)
PolarisWaterHeater = polaris_ns.class_("PolarisWaterHeater", cg.Component)

def define_has_component(component_type: str, keys: list[str]) -> None:
    cg.add_define(
        f"POLARIS_{component_type.upper()}_LIST(F, sep)",
        cg.RawExpression(" sep ".join(f"F({key})" for key in keys))
    )
