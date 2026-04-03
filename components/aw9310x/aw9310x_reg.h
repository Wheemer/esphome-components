#ifndef AW9310X_REG_H
#define AW9310X_REG_H

// Базовые адреса регистров (из документации)
#define REG_SCANCTRL0          0x0000
#define REG_SCANCTRL1          0x0004
#define REG_HOSTCTRL2          0xFF0C
#define REG_CHIP_ID            0xFF10
#define REG_CMD                0xF008
#define REG_HOSTIRQSRC         0xF080
#define REG_HOSTIRQEN          0xF084
#define REG_STAT0              0x0090

// Регистры Diff значений (32-битные)
#define REG_DIFF_CH0           0x0240
#define REG_DIFF_CH1           0x0244
#define REG_DIFF_CH2           0x0248

// Регистры порогов приближения
#define REG_PROXTH0_CH0        0x00B8
#define REG_PROXTH0_CH1        0x00F4
#define REG_PROXTH0_CH2        0x0130

// Регистры порогов касания
#define REG_TOUCHTH0_CH0       0x00BC
#define REG_TOUCHTH0_CH1       0x00F8
#define REG_TOUCHTH0_CH2       0x0134

// Регистры конфигурации AFE для каждого канала
#define REG_AFECFG1_CH0        0x0020
#define REG_AFECFG1_CH1        0x005C
#define REG_AFECFG1_CH2        0x0098

#define REG_AFECFG2_CH0        0x0024
#define REG_AFECFG2_CH1        0x0060
#define REG_AFECFG2_CH2        0x009C

// Регистры калибровки
#define REG_CALI_CTRL          0x001C
#define REG_CALI_STATUS        0x001D

// Регистры компенсации паразитной емкости
#define REG_CPARA_CH0          0x0030
#define REG_CPARA_CH1          0x006C
#define REG_CPARA_CH2          0x00A8

// Регистры управления режимами сканирования
#define REG_DOZECTRL_CH0       0x00A0
#define REG_DOZECTRL_CH1       0x00DC
#define REG_DOZECTRL_CH2       0x0118

// Значения для регистра CMD
#define CMD_ACTIVE_MODE        0x00000001
#define CMD_SLEEP_MODE         0x00000002
#define CMD_DOZE_MODE          0x00000003
#define CMD_DEEPSLEEP_MODE     0x00000004
#define CMD_CALIBRATE          0x00000010
#define CMD_SAVE_NVM           0x00000020

// ID чипа
#define AW93103_CHIP_ID        0xA931  // Для AW93103
#define AW93105_CHIP_ID        0xA961  // Для AW93105 (если поддерживается)

// Маски битов
#define SCANCTRL0_CH_EN_MASK   0x00000007  // Биты 0-2 для включения каналов 0-2
#define SCANCTRL0_CALI_EN_MASK 0x00000700  // Биты 8-10 для калибровки

// Параметры чувствительности
#define AFECFG_GAIN_MASK       0x00000007
#define AFECFG_RESOLUTION_MASK 0x00000038
#define AFECFG_FREQ_MASK       0x000001C0

#endif // AW9310X_REG_H
