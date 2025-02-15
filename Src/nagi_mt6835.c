#include "nagi_mt6835.h"

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

nagi_mt6835_error_t nagi_mt6835_get_id(nagi_mt6835_t *pmt6835, uint8_t *custom_id) {
  if (pmt6835 == NULL) {
    return NAGI_MT6835_HANDLE_NULL;
  }
  if (custom_id == NULL) {
    return NAGI_MT6835_POINTER_NULL;
  }

  return mt6835_read_reg(pmt6835, NAGI_MT6835_REG_ID, custom_id);
}