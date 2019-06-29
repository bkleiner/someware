#include "mpu_6000.h"

#include <cmath>

#include "platform/time.h"

// RA = Register Address

#define MPU_RA_XG_OFFS_TC       0x00    //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU_RA_YG_OFFS_TC       0x01    //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU_RA_ZG_OFFS_TC       0x02    //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU_RA_X_FINE_GAIN      0x03    //[7:0] X_FINE_GAIN
#define MPU_RA_Y_FINE_GAIN      0x04    //[7:0] Y_FINE_GAIN
#define MPU_RA_Z_FINE_GAIN      0x05    //[7:0] Z_FINE_GAIN
#define MPU_RA_XA_OFFS_H        0x06    //[15:0] XA_OFFS
#define MPU_RA_XA_OFFS_L_TC     0x07
#define MPU_RA_YA_OFFS_H        0x08    //[15:0] YA_OFFS
#define MPU_RA_YA_OFFS_L_TC     0x09
#define MPU_RA_ZA_OFFS_H        0x0A    //[15:0] ZA_OFFS
#define MPU_RA_ZA_OFFS_L_TC     0x0B
#define MPU_RA_PRODUCT_ID       0x0C    // Product ID Register
#define MPU_RA_XG_OFFS_USRH     0x13    //[15:0] XG_OFFS_USR
#define MPU_RA_XG_OFFS_USRL     0x14
#define MPU_RA_YG_OFFS_USRH     0x15    //[15:0] YG_OFFS_USR
#define MPU_RA_YG_OFFS_USRL     0x16
#define MPU_RA_ZG_OFFS_USRH     0x17    //[15:0] ZG_OFFS_USR
#define MPU_RA_ZG_OFFS_USRL     0x18
#define MPU_RA_SMPLRT_DIV       0x19
#define MPU_RA_CONFIG           0x1A
#define MPU_RA_GYRO_CONFIG      0x1B
#define MPU_RA_ACCEL_CONFIG     0x1C
#define MPU_RA_FF_THR           0x1D
#define MPU_RA_FF_DUR           0x1E
#define MPU_RA_MOT_THR          0x1F
#define MPU_RA_MOT_DUR          0x20
#define MPU_RA_ZRMOT_THR        0x21
#define MPU_RA_ZRMOT_DUR        0x22
#define MPU_RA_FIFO_EN          0x23
#define MPU_RA_I2C_MST_CTRL     0x24
#define MPU_RA_I2C_SLV0_ADDR    0x25
#define MPU_RA_I2C_SLV0_REG     0x26
#define MPU_RA_I2C_SLV0_CTRL    0x27
#define MPU_RA_I2C_SLV1_ADDR    0x28
#define MPU_RA_I2C_SLV1_REG     0x29
#define MPU_RA_I2C_SLV1_CTRL    0x2A
#define MPU_RA_I2C_SLV2_ADDR    0x2B
#define MPU_RA_I2C_SLV2_REG     0x2C
#define MPU_RA_I2C_SLV2_CTRL    0x2D
#define MPU_RA_I2C_SLV3_ADDR    0x2E
#define MPU_RA_I2C_SLV3_REG     0x2F
#define MPU_RA_I2C_SLV3_CTRL    0x30
#define MPU_RA_I2C_SLV4_ADDR    0x31
#define MPU_RA_I2C_SLV4_REG     0x32
#define MPU_RA_I2C_SLV4_DO      0x33
#define MPU_RA_I2C_SLV4_CTRL    0x34
#define MPU_RA_I2C_SLV4_DI      0x35
#define MPU_RA_I2C_MST_STATUS   0x36
#define MPU_RA_INT_PIN_CFG      0x37
#define MPU_RA_INT_ENABLE       0x38
#define MPU_RA_DMP_INT_STATUS   0x39
#define MPU_RA_INT_STATUS       0x3A
#define MPU_RA_ACCEL_XOUT_H     0x3B
#define MPU_RA_ACCEL_XOUT_L     0x3C
#define MPU_RA_ACCEL_YOUT_H     0x3D
#define MPU_RA_ACCEL_YOUT_L     0x3E
#define MPU_RA_ACCEL_ZOUT_H     0x3F
#define MPU_RA_ACCEL_ZOUT_L     0x40
#define MPU_RA_TEMP_OUT_H       0x41
#define MPU_RA_TEMP_OUT_L       0x42
#define MPU_RA_GYRO_XOUT_H      0x43
#define MPU_RA_GYRO_XOUT_L      0x44
#define MPU_RA_GYRO_YOUT_H      0x45
#define MPU_RA_GYRO_YOUT_L      0x46
#define MPU_RA_GYRO_ZOUT_H      0x47
#define MPU_RA_GYRO_ZOUT_L      0x48
#define MPU_RA_EXT_SENS_DATA_00 0x49
#define MPU_RA_MOT_DETECT_STATUS    0x61
#define MPU_RA_I2C_SLV0_DO      0x63
#define MPU_RA_I2C_SLV1_DO      0x64
#define MPU_RA_I2C_SLV2_DO      0x65
#define MPU_RA_I2C_SLV3_DO      0x66
#define MPU_RA_I2C_MST_DELAY_CTRL   0x67
#define MPU_RA_SIGNAL_PATH_RESET    0x68
#define MPU_RA_MOT_DETECT_CTRL      0x69
#define MPU_RA_USER_CTRL        0x6A
#define MPU_RA_PWR_MGMT_1       0x6B
#define MPU_RA_PWR_MGMT_2       0x6C
#define MPU_RA_BANK_SEL         0x6D
#define MPU_RA_MEM_START_ADDR   0x6E
#define MPU_RA_MEM_R_W          0x6F
#define MPU_RA_DMP_CFG_1        0x70
#define MPU_RA_DMP_CFG_2        0x71
#define MPU_RA_FIFO_COUNTH      0x72
#define MPU_RA_FIFO_COUNTL      0x73
#define MPU_RA_FIFO_R_W         0x74
#define MPU_RA_WHO_AM_I         0x75

#define MPU6000_CONFIG              0x1A
#define MPU_RF_DATA_RDY_EN (1 << 0)

// Bits
#define BIT_SLEEP                   0x40
#define BIT_H_RESET                 0x80
#define BITS_CLKSEL                 0x07
#define MPU_CLK_SEL_PLLGYROX        0x01
#define MPU_CLK_SEL_PLLGYROZ        0x03
#define MPU_EXT_SYNC_GYROX          0x02
#define BITS_FS_250DPS              0x00
#define BITS_FS_500DPS              0x08
#define BITS_FS_1000DPS             0x10
#define BITS_FS_2000DPS             0x18
#define BITS_FS_2G                  0x00
#define BITS_FS_4G                  0x08
#define BITS_FS_8G                  0x10
#define BITS_FS_16G                 0x18
#define BITS_FS_MASK                0x18
#define BITS_DLPF_CFG_256HZ         0x00
#define BITS_DLPF_CFG_188HZ         0x01
#define BITS_DLPF_CFG_98HZ          0x02
#define BITS_DLPF_CFG_42HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF  0x07
#define BITS_DLPF_CFG_MASK          0x07
#define BIT_INT_ANYRD_2CLEAR        0x10
#define BIT_RAW_RDY_EN              0x01
#define BIT_I2C_IF_DIS              0x10
#define BIT_INT_STATUS_DATA         0x01
#define BIT_GYRO                    3
#define BIT_ACC                     2
#define BIT_TEMP                    1


namespace reg {
  enum register_offsets {
    SELF_TEST_X = 0x0D,
    SELF_TEST_Y,
    SELF_TEST_Z
  };
}

int16_t bytes_to_short(uint8_t a, uint8_t b) {
  return int16_t(int16_t(a) << 8 | b);
}

namespace accel {

  
mpu_6000::mpu_6000(spi* bus, gpio::pin* cs)
  : bus(bus)
  , cs(cs)
  , gyro_bias({0, 0, 0})
{
  cs->high();

  platform::time::delay_ms(150);

  bus->set_divisor(spi::SPI_CLOCK_INITIALIZATON);

  bus->bus_write_register(cs, MPU_RA_PWR_MGMT_1, BIT_H_RESET);
  platform::time::delay_ms(150);

  bus->bus_write_register(cs, MPU_RA_SIGNAL_PATH_RESET, BIT_GYRO | BIT_ACC | BIT_TEMP);
  platform::time::delay_ms(150);

  // Clock Source PPL with Z axis gyro reference
  bus->bus_write_register(cs, MPU_RA_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
  platform::time::delay_us(15);

  // Disable Primary I2C Interface
  bus->bus_write_register(cs, MPU_RA_USER_CTRL, BIT_I2C_IF_DIS);
  platform::time::delay_us(15);

  bus->bus_write_register(cs, MPU_RA_PWR_MGMT_2, 0x00);
  platform::time::delay_us(15);

  // Accel Sample Rate 1kHz
  // Gyroscope Output Rate =  1kHz when the DLPF is enabled
  bus->bus_write_register(cs, MPU_RA_SMPLRT_DIV, 0x00);
  platform::time::delay_us(15);

  bus->bus_write_register(cs, MPU6000_CONFIG, BITS_DLPF_CFG_256HZ);
  platform::time::delay_us(15);

  // Gyro +/- 2000 DPS Full Scale
  bus->bus_write_register(cs, MPU_RA_GYRO_CONFIG, BITS_FS_2000DPS);
  platform::time::delay_us(15);

  // Accel +/- 16 G Full Scale
  bus->bus_write_register(cs, MPU_RA_ACCEL_CONFIG, BITS_FS_16G);
  platform::time::delay_us(15);

  bus->bus_write_register(cs, MPU_RA_INT_ENABLE, 0x00);
  platform::time::delay_us(15);

  // bus->bus_write_register(cs, MPU_RA_INT_PIN_CFG, 0 << 7 | 0 << 6 | 0 << 5 | 1 << 4 | 0 << 3 | 0 << 2 | 0 << 1 | 0 << 0);  // INT_ANYRD_2CLEAR
  // platform::time::delay_us(15);

  bus->set_divisor(spi::SPI_CLOCK_FAST);
  platform::time::delay_us(1);
}

float mpu_6000::read_temparture() {
  const auto& temp_raw = bus->bus_read_register_buffer(cs, MPU_RA_TEMP_OUT_H, 2);
  return float(bytes_to_short(temp_raw[0], temp_raw[1])) / 340.f + 36.53f;
}

float tranform_gyro(int16_t value) {
  static constexpr const float factor = 2000.f / 32768.f;
  return float(value) * factor;
}

vector mpu_6000::read_gyro() {
  const auto& data = bus->bus_read_register_buffer(cs, MPU_RA_GYRO_XOUT_H, 6);

  // filter before bias/offset?
  return vector(
    tranform_gyro(bytes_to_short(data[0], data[1])) - gyro_bias[0],
    tranform_gyro(bytes_to_short(data[2], data[3])) - gyro_bias[1],
    tranform_gyro(bytes_to_short(data[4], data[5])) - gyro_bias[2]
  );
}

float tranform_accel(int16_t value) {
  static constexpr const float factor = 16.f / 32768.f;
  return float(value) * factor;
}

vector mpu_6000::read_accel() {
  const auto& data = bus->bus_read_register_buffer(cs, MPU_RA_ACCEL_XOUT_H, 6);

  return vector(
    tranform_accel(bytes_to_short(data[0], data[1])) - accel_bias[0],
    tranform_accel(bytes_to_short(data[2], data[3])) - accel_bias[1],
    tranform_accel(bytes_to_short(data[4], data[5])) - accel_bias[2]
  );
}

static const constexpr uint32_t pass_time_us = 500 * 1000;
static const constexpr int32_t max_samples = 250;
static const constexpr int32_t max_passes = 2;

vector mpu_6000::calibrate_gyro() {
  double gyro_samples[3] = {0, 0, 0};

  for (size_t i = 0; i < max_passes; i++) {
    platform::time::delay_ms(250);
    for (size_t i = 0; i < max_samples; i++) {
      const auto& data = bus->bus_read_register_buffer(cs, MPU_RA_GYRO_XOUT_H, 6);

      gyro_samples[0] += tranform_gyro(bytes_to_short(data[0], data[1]));
      gyro_samples[1] += tranform_gyro(bytes_to_short(data[2], data[3]));
      gyro_samples[2] += tranform_gyro(bytes_to_short(data[4], data[5]));
      platform::time::delay_us(max_samples / pass_time_us);
    }
  }

  gyro_bias[0] = gyro_samples[0] / double(max_passes * max_samples);
  gyro_bias[1] = gyro_samples[1] / double(max_passes * max_samples);
  gyro_bias[2] = gyro_samples[2] / double(max_passes * max_samples);

  return accel_bias;
}

vector mpu_6000::calibrate_accel() {
  double accel_samples[3] = {0, 0, 0};

  for (size_t i = 0; i < max_passes; i++) {
    platform::time::delay_ms(250);
    for (size_t i = 0; i < max_samples; i++) {
      const auto& data = bus->bus_read_register_buffer(cs, MPU_RA_ACCEL_XOUT_H, 6);

      accel_samples[0] += tranform_accel(bytes_to_short(data[0], data[1]));
      accel_samples[1] += tranform_accel(bytes_to_short(data[2], data[3]));
      accel_samples[2] += tranform_accel(bytes_to_short(data[4], data[5]));

      platform::time::delay_us(max_samples / pass_time_us);
    }
  }

  accel_bias[0] = accel_samples[0] / double(max_passes * max_samples);
  accel_bias[1] = accel_samples[1] / double(max_passes * max_samples);
  accel_bias[2] = accel_samples[2] / double(max_passes * max_samples) + 1.0f;

  return accel_bias;
}
}