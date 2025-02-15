#ifndef __NAGI_MT6835_H__
#define __NAGI_MT6835_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define NAGI_MT6835_ZERO_REG_STEP    (0.088f)
#define NAGI_MT6835_ANGLE_RESOLUTION (1 << 21)

/// @brief mt6835 error codes.
typedef enum nagi_mt6835_error_t {
  NAGI_MT6835_OK = 0, ///< No error.
  NAGI_MT6835_ERROR,  ///< General error.
  NAGI_MT6835_HANDLE_NULL, ///< Handle is NULL.
  NAGI_MT6835_POINTER_NULL, ///< Pointer is NULL.
  NAGI_MT6835_INVALID_ARGUMENT, ///< Invalid argument.
  NAGI_MT6835_CRC_CHECK_FAILED, ///< CRC check failed.
} nagi_mt6835_error_t;

/// @brief mt6835 command enum.
typedef enum nagi_mt6835_cmd_enum_t {
  NAGI_MT6835_CMD_RD = (0b0011), ///< User read register.
  NAGI_MT6835_CMD_WR = (0b0110), ///< User write register.
  NAGI_MT6835_CMD_EEPROM = (0b1100), ///< User erase and program EEPROM.
  NAGI_MT6835_CMD_ZERO = (0b0101), ///< AUTO setting zero.
  NAGI_MT6835_CMD_CONTINUE = (0b1010), ///< Continue mode.
} nagi_mt6835_cmd_enum_t;

/// @brief mt6835 register enum.
typedef enum nagi_mt6835_reg_enum_t {
  NAGI_MT6835_REG_ID = (0x001), ///< ID.
  NAGI_MT6835_REG_ANGLE3 = (0x003), ///< Angle 3.
  NAGI_MT6835_REG_ANGLE2 = (0x004), ///< Angle 2.
  NAGI_MT6835_REG_ANGLE1 = (0x005), ///< Angle 1 and state.
  NAGI_MT6835_REG_CRC = (0x006), ///< CRC.
  NAGI_MT6835_REG_ABZ_RES2 = (0x007), ///< ABZ res 2.
  NAGI_MT6835_REG_ABZ_RES1 = (0x008), ///< ABZ res 1.
  NAGI_MT6835_REG_ZERO2 = (0x009), ///< Zero 2.
  NAGI_MT6835_REG_ZERO1 = (0x00A), ///< Zero 1.
  NAGI_MT6835_REG_UVW = (0x00B), ///< UVW.
  NAGI_MT6835_REG_PWM = (0x00C), ///< PWM.
  NAGI_MT6835_REG_HYST = (0x00D), ///< HYST.
  NAGI_MT6835_REG_AUTOCAL = (0x00E), ///< AUTO cal.
} nagi_mt6835_reg_enum_t;

/// @brief mt6835 warning enum.
typedef enum nagi_mt6835_warning_t {
  NAGI_MT6835_WARN_NONE = 0x00, ///< No warning.
  NAGI_MT6835_WARN_OVER_SPEED = 0x01, ///< Over speed.
  NAGI_MT6835_WARN_FIELD_WEAK = 0x02, ///< Field weak.
  NAGI_MT6835_WARN_UNDER_VOLTAGE = 0x04, ///< Under voltage.
} nagi_mt6835_warning_t;

/// @brief mt6835 read angle method enum.
typedef enum nagi_mt6835_read_angle_method_enum_t {
  NAGI_MT6835_READ_ANGLE_METHOD_NORMAL = 0, ///< Normal.
  NAGI_MT6835_READ_ANGLE_METHOD_CONTINUE = 1, ///< Continue.
} nagi_mt6835_read_angle_method_enum_t;

/// @brief mt6835 data frame.
typedef struct nagi_mt6835_data_frame_t {
  union {
    uint32_t pack;
    struct {
      uint8_t reserved: 4;
      nagi_mt6835_cmd_enum_t cmd: 4;
      nagi_mt6835_reg_enum_t reg: 8;
      uint8_t normal_byte: 8;
      uint8_t empty_byte: 8;
    };
  };
} nagi_mt6835_data_frame_t;

/// @brief mt6835 chip select function typedef.
typedef void (*nagi_mt6835_chip_select_fn_t)(bool);

/// @brief mt6835 read write and read function typedef.
typedef int (*nagi_mt6835_read_write_fn_t)(uint8_t *, uint8_t *, size_t);

/// @brief mt6835 delay function typedef.
typedef void (*nagi_mt6835_delay_fn_t)(uint32_t);

/// @brief mt6835 configuration structure.
typedef struct nagi_mt6835_config_t {
  /// @brief Chip select function pointer.
  nagi_mt6835_chip_select_fn_t chip_select_fn;
  /// @brief Read write function pointer.
  nagi_mt6835_read_write_fn_t read_write_fn;
  /// @brief Delay function pointer.
  nagi_mt6835_delay_fn_t delay_fn;
  /// @brief Enable CRC check.
  bool enable_crc_check;
} nagi_mt6835_config_t;

/// @brief mt6835 structure.
typedef struct nagi_mt6835_t {
  /// @brief Chip select function pointer.
  nagi_mt6835_chip_select_fn_t chip_select_fn;
  /// @brief Read write function pointer.
  nagi_mt6835_read_write_fn_t read_write_fn;
  /// @brief Delay function pointer.
  nagi_mt6835_delay_fn_t delay_fn;
  /// @brief Enable CRC check.
  bool enable_crc_check;

  /// @brief Data frame.
  nagi_mt6835_data_frame_t data_frame;
  /// @brief CRC result.
  bool crc_res;
  /// @brief Warning.
  nagi_mt6835_warning_t warning;
} nagi_mt6835_t;

/// @brief Initialize the mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] config mt6835 configuration.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_init(nagi_mt6835_t *pmt6835, const nagi_mt6835_config_t *pconfig);

/// @brief Set custom ID to mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] custom_id custom ID.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_id(nagi_mt6835_t *pmt6835, uint8_t custom_id);

/// @brief Get custom ID from mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[out] pcustom_id custom ID.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_get_id(nagi_mt6835_t *pmt6835, uint8_t *pcustom_id);

/// @brief Auto set zero angle to mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_auto_zero_angle(nagi_mt6835_t *pmt6835);

/// @brief Set zero angle to mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] rad zero angle in rad.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_zero_angle(nagi_mt6835_t *pmt6835, float rad);

/// @brief Get raw angle from mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] method read angle method.
/// @param[out] praw_angle raw angle.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_get_raw_angle(
  nagi_mt6835_t *pmt6835,
  nagi_mt6835_read_angle_method_enum_t method,
  uint32_t *praw_angle
);

/// @brief Get raw zero angle from mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[out] praw_zero_angle raw zero angle.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_get_raw_zero_angle(nagi_mt6835_t *pmt6835, uint16_t *praw_zero_angle);

/// @brief Get angle from mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] method read angle method.
/// @param[out] prad_angle angle in rad.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_get_angle(
  nagi_mt6835_t *pmt6835,
  nagi_mt6835_read_angle_method_enum_t method,
  float *prad_angle
);

/// @brief Get zero angle from mt6835.
/// @param[in] pmt6835 mt6835 handle.
/// @param[out] prad_angle zero angle in rad.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_get_zero_angle(nagi_mt6835_t *pmt6835, float *prad_angle);

/// @brief Set mt6835 ABZ output enable.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] enable enable or disable.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_enable_abz_output(nagi_mt6835_t *pmt6835, bool enable);

/// @brief Set mt6835 ABZ ab swap.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] ab_swap AB swap.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_abz_ab_swap(nagi_mt6835_t *pmt6835, bool ab_swap);

/// @brief Set mt6835 ABZ resolution.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] abz_res ABZ resolution.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_abz_resolution(nagi_mt6835_t *pmt6835, uint16_t abz_res);

/// @brief Set mt6835 ABZ z position.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] abz_z_pos ABZ z position(0x000 = 0, 0x001 = 0.088, ... 0xFFF = 359.912).
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_abz_z_position(nagi_mt6835_t *pmt6835, uint16_t abz_z_pos);

/// @brief Set mt6835 ABZ z edge.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] abz_z_edge_up ABZ z edge up.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_abz_z_edge_up(nagi_mt6835_t *pmt6835, bool abz_z_edge_up);

/// @brief Set mt6835 ABZ z pulse width.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] abz_z_pulse_width ABZ z pulse width(0x0 = 1deg, 0x1 = 2deg, 0x2 = 4deg, 0x3 = 8deg, 0x4 = 16deg, 0x5 = 60deg, 0x6 = 120deg, 0x7 = 180deg).
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_abz_z_pulse_width(nagi_mt6835_t *pmt6835, uint8_t abz_z_pulse_width);

/// @brief Set mt6835 ABZ z phase.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] abz_z_phase ABZ z phase(0x0 = a down, 0x1 = b up, 0x2 = a up, 0x3 = b down).
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_set_abz_z_phase(nagi_mt6835_t *pmt6835, uint8_t abz_z_phase);

/// @brief Program mt6835 eeprom.
/// @param[in] pmt6835 mt6835 handle.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_program_eeprom(nagi_mt6835_t *pmt6835);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// If you want to configure more registers, try to use the following functions.
////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Read mt6835 register.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] reg register.
/// @param[out] pdata data.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_read_reg(nagi_mt6835_t *pmt6835, nagi_mt6835_reg_enum_t reg, uint8_t *pdata);

/// @brief Write mt6835 register.
/// @param[in] pmt6835 mt6835 handle.
/// @param[in] reg register.
/// @param[in] data data.
/// @return mt6835 error code.
nagi_mt6835_error_t nagi_mt6835_write_reg(nagi_mt6835_t *pmt6835, nagi_mt6835_reg_enum_t reg, uint8_t data);

#endif // __NAGI_MT6835_H__