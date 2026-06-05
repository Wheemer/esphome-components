import esphome.codegen as cg

polaris_ns = cg.esphome_ns.namespace("polaris")
PolarisKettle = polaris_ns.class_("PolarisKettle", cg.Component, cg.uart.UARTDevice)
