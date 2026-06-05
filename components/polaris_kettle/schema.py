from dataclasses import dataclass
from typing import Any

import esphome.config_validation as cv
from esphome.const import UNIT_CELSIUS

@dataclass
class PolarisEntity:
    """Базовый класс для сущностей Polaris"""
    name: str
    icon: str | None = None
    disabled_by_default: bool = False

@dataclass
class PolarisSensor(PolarisEntity):
    """Схема для сенсора температуры"""
    unit_of_measurement: str = UNIT_CELSIUS
    accuracy_decimals: int = 1

@dataclass
class PolarisTextSensor(PolarisEntity):
    """Схема для текстового сенсора (режим)"""
    pass

@dataclass
class PolarisBinarySensor(PolarisEntity):
    """Схема для бинарного сенсора (ошибки)"""
    device_class: str | None = None

@dataclass
class PolarisButton(PolarisEntity):
    """Схема для кнопки"""
    command: int
    temperature: int

# Предопределённые кнопки для чайной церемонии
BUTTONS = {
    "black_tea": PolarisButton(
        name="Черный чай / Кофе",
        icon="mdi:coffee",
        command=0x3D,
        temperature=95
    ),
    "mix_tea": PolarisButton(
        name="Смесь",
        icon="mdi:cup",
        command=0x3B,
        temperature=40
    ),
    "white_tea": PolarisButton(
        name="Белый чай",
        icon="mdi:tea",
        command=0x3C,
        temperature=65
    ),
    "green_tea": PolarisButton(
        name="Зеленый цветочный",
        icon="mdi:flower",
        command=0x7C,
        temperature=80
    ),
    "oolong_tea": PolarisButton(
        name="Красный улун",
        icon="mdi:leaf",
        command=0x7D,
        temperature=90
    ),
    "bag_tea": PolarisButton(
        name="Пакетированный",
        icon="mdi:tea",
        command=0x7E,
        temperature=100
    ),
    "boil": PolarisButton(
        name="Кипячение",
        icon="mdi:kettle-steam",
        command=0x3B,
        temperature=100
    ),
    "keep_warm": PolarisButton(
        name="Подогрев",
        icon="mdi:heat-wave",
        command=0x63,
        temperature=40
    ),
}
