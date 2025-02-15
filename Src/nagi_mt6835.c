#include "nagi_mt6835.h"

#include <string.h>
#include <math.h>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (0.017453292519943295)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (57.29577951308232)
#endif

static uint8_t crc8_table[256] = {
  0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
  0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
  0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
  0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
  0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
  0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
  0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
  0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
  0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
  0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
  0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
  0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
  0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
  0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
  0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
  0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};

/// @brief CRC check.
/// @param data The data to be checked.
/// @param len The length of the data.
/// @return CRC check result.
static uint8_t crc_table(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0x00; // 初始CRC值

  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i]; // 与数据异或
    crc = crc8_table[crc]; // 查表更新CRC
  }

  return crc;
}

/// @brief Read mt6835 register.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] reg register address, @ref mt6835_reg_enum_t.
/// @param[out] data data.
/// @return mt6835 error code.
static nagi_mt6835_error_t mt6835_read_reg(nagi_mt6835_t *pmt6835, nagi_mt6835_reg_enum_t reg, uint8_t* data) {
  uint8_t result[3] = {0, 0, 0};

  pmt6835->chip_select_fn(false);
  pmt6835->data_frame.cmd = NAGI_MT6835_CMD_RD; // byte read command
  pmt6835->data_frame.reg = reg;

  pmt6835->chip_select_fn(true);
  nagi_mt6835_error_t err = pmt6835->read_write_fn((uint8_t *)&pmt6835->data_frame.pack, (uint8_t *)&result, 3);
  pmt6835->chip_select_fn(false);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  *data = result[2];
  return NAGI_MT6835_OK;
}

/// @brief Write mt6835 register.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] reg register address, @ref mt6835_reg_enum_t.
/// @param[in] data data to write.
/// @return mt6835 error code.
static nagi_mt6835_error_t mt6835_write_reg(nagi_mt6835_t *pmt6835, nagi_mt6835_reg_enum_t reg, uint8_t data) {
  uint8_t result[3] = {0, 0, 0};

  pmt6835->chip_select_fn(false);
  pmt6835->data_frame.cmd = NAGI_MT6835_CMD_WR; // byte write command
  pmt6835->data_frame.reg = reg;
  pmt6835->data_frame.normal_byte = data;

  pmt6835->chip_select_fn(true);
  nagi_mt6835_error_t err = pmt6835->read_write_fn((uint8_t *)&pmt6835->data_frame.pack, (uint8_t *)&result, 3);
  pmt6835->chip_select_fn(false);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_init(nagi_mt6835_t *pmt6835, const nagi_mt6835_config_t *pconfig) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (pconfig == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }
  if (pconfig->chip_select_fn == NULL || pconfig->read_write_fn == NULL || pconfig->delay_fn == NULL) {
    return NAGI_MT6835_INVALID_ARGUMENT;
  }

  pmt6835->chip_select_fn = pconfig->chip_select_fn;
  pmt6835->read_write_fn = pconfig->read_write_fn;
  pmt6835->delay_fn = pconfig->delay_fn;
  pmt6835->enable_crc_check = pconfig->enable_crc_check;

  pmt6835->data_frame.pack = 0;
  pmt6835->crc_res = false;
  pmt6835->warning = NAGI_MT6835_WARN_NONE;

  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_set_id(nagi_mt6835_t *pmt6835, uint8_t custom_id) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  return mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ID, custom_id);
}

nagi_mt6835_error_t nagi_mt6835_get_id(nagi_mt6835_t *pmt6835, uint8_t *pcustom_id) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (pcustom_id == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  return mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ID, pcustom_id);
}

nagi_mt6835_error_t nagi_mt6835_auto_zero_angle(nagi_mt6835_t *pmt6835) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  uint8_t result[3] = {0, 0, 0};

  pmt6835->chip_select_fn(false);
  pmt6835->data_frame.cmd = NAGI_MT6835_CMD_ZERO;
  pmt6835->data_frame.reg = 0x00;
  pmt6835->data_frame.normal_byte = 0x00;

  pmt6835->chip_select_fn(true);
  nagi_mt6835_error_t err = pmt6835->read_write_fn((uint8_t *)&pmt6835->data_frame.pack, (uint8_t *)&result, 3);
  pmt6835->chip_select_fn(false);

  if (result[2] != 0x55) {
    return NAGI_MT6835_ERROR;
  }

  return err;
}

nagi_mt6835_error_t nagi_mt6835_set_zero_angle(nagi_mt6835_t *pmt6835, float rad) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  uint16_t angle = (uint16_t)roundf(rad * RAD_TO_DEG / NAGI_MT6835_ZERO_REG_STEP);
  if (angle > 0xFFF) {
    return NAGI_MT6835_INVALID_ARGUMENT;
  }

  uint8_t tx_buf[2] = {0};

  tx_buf[1] = angle >> 4;
  tx_buf[0] = (angle & 0x0F) << 4;

  uint8_t zero1 = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ZERO1, &zero1);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  tx_buf[0] |= zero1 & 0x0F;

  err = mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ZERO2, tx_buf[1]);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  err = mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ZERO1, tx_buf[0]);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_get_raw_angle(
  nagi_mt6835_t *pmt6835,
  nagi_mt6835_read_angle_method_enum_t method,
  uint32_t *praw_angle
) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (praw_angle == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  uint8_t rx_buf[6] = {0};
  uint8_t tx_buf[6] = {0};

  switch (method) {
    case NAGI_MT6835_READ_ANGLE_METHOD_NORMAL: {
      nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ANGLE3, &rx_buf[0]);
      if (err != NAGI_MT6835_OK) {
        return err;
      }
      err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ANGLE2, &rx_buf[1]);
      if (err != NAGI_MT6835_OK) {
        return err;
      }
      err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ANGLE1, &rx_buf[2]);
      if (err != NAGI_MT6835_OK) {
        return err;
      }
      if (pmt6835->enable_crc_check) {
        err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_CRC, &rx_buf[3]);
        if (err != NAGI_MT6835_OK) {
          return err;
        }
      }
      break;
    }
    case NAGI_MT6835_READ_ANGLE_METHOD_CONTINUE: {
      const uint8_t len = pmt6835->enable_crc_check ? 6 : 5;

      pmt6835->chip_select_fn(false);
      pmt6835->data_frame.cmd = NAGI_MT6835_CMD_CONTINUE;
      pmt6835->data_frame.reg = NAGI_MT6835_REG_ANGLE3;
      tx_buf[0] = pmt6835->data_frame.pack & 0xFF;
      tx_buf[1] = (pmt6835->data_frame.pack >> 8) & 0xFF;

      pmt6835->chip_select_fn(true);
      nagi_mt6835_error_t err = pmt6835->read_write_fn(tx_buf, rx_buf, len);
      pmt6835->chip_select_fn(false);
      if (err != NAGI_MT6835_OK) {
        return err;
      }

      memmove(rx_buf, &rx_buf[2], 3);
      if (pmt6835->enable_crc_check) {
        rx_buf[3] = rx_buf[5];
      }
      break;
    }
  }

  if (pmt6835->enable_crc_check) {
    if (crc_table(rx_buf, 3) != rx_buf[3]) {
      pmt6835->crc_res = false;
      return NAGI_MT6835_CRC_CHECK_FAILED;
    }
    pmt6835->crc_res = true;
  }

  pmt6835->warning = rx_buf[2] & 0x07;
  *praw_angle = (rx_buf[0] << 13) | (rx_buf[1] << 5) | (rx_buf[2] >> 3);

  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_get_raw_zero_angle(nagi_mt6835_t *pmt6835, uint16_t *praw_zero_angle) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (praw_zero_angle == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  uint8_t rx_buf[2] = {0};
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ZERO2, &rx_buf[1]);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ZERO1, &rx_buf[0]);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  *praw_zero_angle = (rx_buf[1] << 4) | (rx_buf[0] >> 4);
  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_get_angle(
  nagi_mt6835_t *pmt6835,
  nagi_mt6835_read_angle_method_enum_t method,
  float *prad_angle
) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (prad_angle == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  uint32_t raw_angle = 0;
  nagi_mt6835_error_t err = nagi_mt6835_get_raw_angle(pmt6835, method, &raw_angle);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  *prad_angle = (float)(raw_angle * 2.996056226329803e-6);
  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_get_zero_angle(nagi_mt6835_t *pmt6835, float *prad_angle) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (prad_angle == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  uint16_t raw_zero_angle = 0;
  nagi_mt6835_error_t err = nagi_mt6835_get_raw_zero_angle(pmt6835, &raw_zero_angle);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  *prad_angle = (float)raw_zero_angle * DEG_TO_RAD * NAGI_MT6835_ZERO_REG_STEP;
  return NAGI_MT6835_OK;
}

nagi_mt6835_error_t nagi_mt6835_enable_abz_output(nagi_mt6835_t *pmt6835, bool enable) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  uint8_t abz_res1_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES1, &abz_res1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  if (enable) {
    abz_res1_reg |= 0b00000010;
  } else {
    abz_res1_reg &= 0b11111101;
  }

  return mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES1, abz_res1_reg);
}

nagi_mt6835_error_t nagi_mt6835_set_abz_ab_swap(nagi_mt6835_t *pmt6835, bool ab_swap) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  uint8_t abz_res1_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES1, &abz_res1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  if (ab_swap) {
    abz_res1_reg |= 0b00000001;
  } else {
    abz_res1_reg &= 0b11111110;
  }

  return mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES1, abz_res1_reg);
}

nagi_mt6835_error_t nagi_mt6835_set_abz_resolution(nagi_mt6835_t *pmt6835, uint16_t abz_res) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (abz_res > 16384) {
    return NAGI_MT6835_INVALID_ARGUMENT;
  }

  uint8_t abz_res1_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES1, &abz_res1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  abz_res1_reg = (abz_res1_reg & 0b00000011) | ((abz_res & 0b00111111) << 2);
  err = mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES2, abz_res >> 8);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  err = mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ABZ_RES1, abz_res1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  return err;
}

nagi_mt6835_error_t nagi_mt6835_set_abz_z_position(nagi_mt6835_t *pmt6835, uint16_t abz_z_pos) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (abz_z_pos > 4095) {
    return NAGI_MT6835_INVALID_ARGUMENT;
  }

  uint8_t abz_zero1_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ZERO1, &abz_zero1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  abz_zero1_reg = (abz_zero1_reg & 0b00001111) | ((abz_z_pos & 0b00001111) << 4);
  err = mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ZERO2, abz_z_pos >> 4);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  err = mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ZERO1, abz_zero1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  return err;
}

nagi_mt6835_error_t nagi_mt6835_set_abz_z_edge_up(nagi_mt6835_t *pmt6835, bool abz_z_edge_up) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  uint8_t abz_zero1_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ZERO1, &abz_zero1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }

  if (abz_z_edge_up) {
    abz_zero1_reg |= 0b00001000;
  } else {
    abz_zero1_reg &= 0b11110111;
  }

  return mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ZERO1, abz_zero1_reg);
}

nagi_mt6835_error_t nagi_mt6835_set_abz_z_pulse_width(nagi_mt6835_t *pmt6835, uint8_t abz_z_pulse_width) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (abz_z_pulse_width > 7) {
    return NAGI_MT6835_INVALID_ARGUMENT;
  }

  uint8_t abz_zero1_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ZERO1, &abz_zero1_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  abz_zero1_reg = (abz_zero1_reg & 0b11111000) | (abz_z_pulse_width & 0b00000111);

  return mt6835_write_reg(pmt6835, NAGI_MT6835_REG_ZERO1, abz_zero1_reg);
}

nagi_mt6835_error_t nagi_mt6835_set_abz_z_phase(nagi_mt6835_t *pmt6835, uint8_t abz_z_phase) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (abz_z_phase > 3) {
    return NAGI_MT6835_INVALID_ARGUMENT;
  }

  uint8_t abz_uvw_reg = 0;
  nagi_mt6835_error_t err = mt6835_read_reg(pmt6835, NAGI_MT6835_REG_UVW, &abz_uvw_reg);
  if (err != NAGI_MT6835_OK) {
    return err;
  }
  abz_uvw_reg = (abz_uvw_reg & 0b00111111) | (abz_z_phase & 0b11000000);

  return mt6835_write_reg(pmt6835, NAGI_MT6835_REG_UVW, abz_uvw_reg);
}

nagi_mt6835_error_t nagi_mt6835_program_eeprom(nagi_mt6835_t *pmt6835) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  uint8_t result[3] = {0, 0, 0};

  pmt6835->chip_select_fn(false);
  pmt6835->data_frame.cmd = NAGI_MT6835_CMD_EEPROM;
  pmt6835->data_frame.reg = 0x00;
  pmt6835->data_frame.normal_byte = 0x00;

  pmt6835->chip_select_fn(true);
  nagi_mt6835_error_t err = pmt6835->read_write_fn((uint8_t *)&pmt6835->data_frame.pack, (uint8_t *)&result, 3);
  pmt6835->chip_select_fn(false);

  if (result[2] != 0x55) {
    return NAGI_MT6835_ERROR;
  }

  return err;
}

nagi_mt6835_error_t nagi_mt6835_read_reg(nagi_mt6835_t *pmt6835, nagi_mt6835_reg_enum_t reg, uint8_t *pdata) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (pdata == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  return mt6835_read_reg(pmt6835, reg, pdata);
}

nagi_mt6835_error_t nagi_mt6835_write_reg(nagi_mt6835_t *pmt6835, nagi_mt6835_reg_enum_t reg, uint8_t data) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }

  return mt6835_write_reg(pmt6835, reg, data);
}