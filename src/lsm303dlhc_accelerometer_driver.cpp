#include "lsm303dlhc_accelerometer_driver.h"
#include "math.h"
#include "mbed_error.h"

using namespace lsm303dlhc;

LSM303DLHCAccelerometer::LSM303DLHCAccelerometer(I2C* i2c_ptr)
    : i2c_device(I2C_ADDRESS, i2c_ptr)
    , sensitivity(0)
{
}

LSM303DLHCAccelerometer::LSM303DLHCAccelerometer(PinName sda, PinName scl, int frequency)
    : i2c_device(I2C_ADDRESS, sda, scl, frequency)
    , sensitivity(0)
{
}

LSM303DLHCAccelerometer::~LSM303DLHCAccelerometer()
{
}

int LSM303DLHCAccelerometer::init()
{
    // check device id
    int device_id = i2c_device.read_register(WHO_AM_I_ADDR);
    if (device_id != DEVICE_ID) {
        return MBED_ERROR_CODE_INITIALIZATION_FAILED;
    }

    // set default modes
    set_data_ready_interrupt_mode(DRDY_DISABLE);
    set_full_scale(FULL_SCALE_2G);
    set_high_pass_filter_mode(HPF_OFF);
    set_high_resolution_output_mode(HRO_ENABLED);
    set_power_mode(NORMAL_POWER_MODE);
    set_output_data_rate(ODR_25HZ);

    // check that ODR is set correctly
    if (get_output_data_rate() != ODR_25HZ) {
        return MBED_ERROR_CODE_INITIALIZATION_FAILED;
    }

    return MBED_SUCCESS;
}

uint8_t LSM303DLHCAccelerometer::read_register(uint8_t reg)
{
    return i2c_device.read_register(reg);
}

void LSM303DLHCAccelerometer::write_register(uint8_t reg, uint8_t val)
{
    i2c_device.write_register(reg, val);
}

void LSM303DLHCAccelerometer::set_power_mode(PowerMode power_mode)
{
    // update power mode bit
    i2c_device.update_register(CTRL_REG1_A, (uint8_t)(power_mode << 3), 0x08);
}

LSM303DLHCAccelerometer::PowerMode LSM303DLHCAccelerometer::get_power_mode()
{
    uint8_t val = i2c_device.read_register(CTRL_REG1_A, 0x08);
    return val ? LOW_POWER_MODE : NORMAL_POWER_MODE;
}

void LSM303DLHCAccelerometer::set_output_data_rate(OutputDataRate odr)
{
    PowerMode power_mode = get_power_mode();

    if (odr == ODR_NONE) {
        i2c_device.update_register(CTRL_REG1_A, 0x00, 0x08);
    } else {
        switch (power_mode) {
        case NORMAL_POWER_MODE:
            if (!(odr & 0x01)) {
                MBED_ERROR(MBED_ERROR_CONFIG_MISMATCH, "Invalid ODR for normal power mode");
            }
            break;
        case LOW_POWER_MODE:
            if (!(odr & 0x02)) {
                MBED_ERROR(MBED_ERROR_CONFIG_MISMATCH, "Invalid ODR for low power mode");
            }
            break;
        }
        // set ODR and enable axes
        i2c_device.update_register(CTRL_REG1_A, (odr & 0xF0) | 0x07, 0xF7);
    }
}

LSM303DLHCAccelerometer::OutputDataRate LSM303DLHCAccelerometer::get_output_data_rate()
{
    uint8_t val = i2c_device.read_register(CTRL_REG1_A, 0xF0);
    PowerMode power_mode;
    OutputDataRate odr;

    switch (val) {
    case 0x00:
        odr = ODR_NONE;
        break;
    case 0x10:
        odr = ODR_1HZ;
        break;
    case 0x20:
        odr = ODR_10HZ;
        break;
    case 0x30:
        odr = ODR_25HZ;
        break;
    case 0x40:
        odr = ODR_50HZ;
        break;
    case 0x50:
        odr = ODR_100HZ;
        break;
    case 0x60:
        odr = ODR_200HZ;
        break;
    case 0x70:
        odr = ODR_400HZ;
        break;
    case 0x80:
        odr = ODR_1620HZ;
        break;
    case 0x90:
        power_mode = get_power_mode();
        odr = power_mode == NORMAL_POWER_MODE ? ODR_1344HZ : ODR_5376HZ;
    default:
        MBED_ERROR(MBED_ERROR_INVALID_DATA_DETECTED, "Invalid CTRL_REG1_A value");
    }
    return odr;
}

float LSM303DLHCAccelerometer::get_output_data_rate_hz()
{
    float f_odr;

    switch (get_output_data_rate()) {
    case LSM303DLHCAccelerometer::ODR_NONE:
        f_odr = 0;
        break;
    case LSM303DLHCAccelerometer::ODR_1HZ:
        f_odr = 1.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_10HZ:
        f_odr = 10.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_25HZ:
        f_odr = 25.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_50HZ:
        f_odr = 50.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_100HZ:
        f_odr = 100.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_200HZ:
        f_odr = 200.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_400HZ:
        f_odr = 400.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_1620HZ:
        f_odr = 1620.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_1344HZ:
        f_odr = 1344.0f;
        break;
    case LSM303DLHCAccelerometer::ODR_5376HZ:
        f_odr = 5376.0f;
        break;
    }
    return f_odr;
}

void LSM303DLHCAccelerometer::set_full_scale(FullScale fs)
{
    i2c_device.update_register(CTRL_REG4_A, fs, 0x30);

    // calculate m/s^2 / lsb
    switch (fs) {
    case LSM303DLHCAccelerometer::FULL_SCALE_2G:
        sensitivity = 0.001f * GRAVITY_OF_EARTH;
        break;
    case LSM303DLHCAccelerometer::FULL_SCALE_4G:
        sensitivity = 0.002f * GRAVITY_OF_EARTH;
        break;
    case LSM303DLHCAccelerometer::FULL_SCALE_8G:
        sensitivity = 0.004f * GRAVITY_OF_EARTH;
        break;
    case LSM303DLHCAccelerometer::FULL_SCALE_16G:
        sensitivity = 0.012f * GRAVITY_OF_EARTH;
        break;
    }
}

LSM303DLHCAccelerometer::FullScale LSM303DLHCAccelerometer::get_full_scale()
{
    uint8_t value = i2c_device.read_register(CTRL_REG4_A, 0x30);
    FullScale fs;

    switch (value) {
    case 0x00:
        fs = FULL_SCALE_2G;
        break;
    case 0x10:
        fs = FULL_SCALE_4G;
        break;
    case 0x20:
        fs = FULL_SCALE_8G;
        break;
    case 0x30:
        fs = FULL_SCALE_16G;
        break;
    default:
        MBED_ERROR(MBED_ERROR_UNKNOWN, "Unreachable code");
    }
    return fs;
}

float LSM303DLHCAccelerometer::get_sensitivity()
{
    return sensitivity;
}

void LSM303DLHCAccelerometer::set_high_pass_filter_mode(LSM303DLHCAccelerometer::HighPassFilterMode hpf)
{
    if (hpf == HPF_OFF) {
        i2c_device.update_register(CTRL_REG2_A, 0x00, 0x08);
    } else {
        i2c_device.update_register(CTRL_REG2_A, hpf | 0x08, 0x38);
    }
}

LSM303DLHCAccelerometer::HighPassFilterMode LSM303DLHCAccelerometer::get_high_pass_filter_mode()
{
    uint8_t val = i2c_device.read_register(CTRL_REG2_A, 0x38);
    HighPassFilterMode hpf = HPF_OFF;
    if (val & 0x08) {
        switch (val & 0x30) {
        case 0x00:
            hpf = HPF_CF0;
            break;
        case 0x10:
            hpf = HPF_CF1;
            break;
        case 0x20:
            hpf = HPF_CF2;
            break;
        case 0x30:
            hpf = HPF_CF3;
            break;
        }
    }
    return hpf;
}

float LSM303DLHCAccelerometer::get_high_pass_filter_cut_off_frequency()
{
    uint8_t val = i2c_device.read_register(CTRL_REG2_A, 0x30);
    float hp_c = val >> 4;
    float f_s = get_output_data_rate_hz();

    float f_cutt_off = -logf(1.0f - 3.0f / (25.0f * powf(2, hp_c))) * f_s / (2 * 3.14159265358979323846f);

    return f_cutt_off;
}

void LSM303DLHCAccelerometer::set_data_ready_interrupt_mode(LSM303DLHCAccelerometer::DatadaReadyInterruptMode drdy_mode)
{
    // Note: enable I1_DRDY1 interrupt
    // there is I2_DRDY2 interrupt bit, that can be set,
    // but I don't find what it means
    i2c_device.update_register(CTRL_REG3_A, drdy_mode == DRDY_ENABLE ? 0x10 : 0x00, 0x18);
}

LSM303DLHCAccelerometer::DatadaReadyInterruptMode LSM303DLHCAccelerometer::get_data_ready_interrupt_mode()
{
    uint8_t val = i2c_device.read_register(CTRL_REG3_A, 0x18);
    return val ? DRDY_ENABLE : DRDY_DISABLE;
}

void LSM303DLHCAccelerometer::set_high_resolution_output_mode(HighResolutionOutputMode hro)
{
    i2c_device.update_register(CTRL_REG4_A, hro == HRO_ENABLED ? 0x08 : 0x00, 0x08);
}

LSM303DLHCAccelerometer::HighResolutionOutputMode LSM303DLHCAccelerometer::get_high_resolution_output_mode()
{
    return i2c_device.read_register(CTRL_REG4_A, 0x08) ? HRO_ENABLED : HRO_DISABLED;
}

void LSM303DLHCAccelerometer::read_data(float data[3])
{
    int16_t data_16[3];
    read_data_16(data_16);
    for (int i = 0; i < 3; i++) {
        data[i] = data_16[i] * sensitivity;
    }
}

void LSM303DLHCAccelerometer::read_data_16(int16_t data[3])
{
    uint8_t raw_data[6];
    i2c_device.read_registers(OUT_X_L_A | 0x80, raw_data, 6);
    // data layout
    // - output resolution 12 bit
    // - assume that LSB is lower address, as it's default value
    //   note: the byte order is controlled by CTRL_REG4_A
    // - the value is left-justified, so we need to shift it to right
    data[0] = (int16_t)(raw_data[1] << 8 | raw_data[0]) >> 4; // X axis
    data[1] = (int16_t)(raw_data[3] << 8 | raw_data[2]) >> 4; // Y axis
    data[2] = (int16_t)(raw_data[5] << 8 | raw_data[4]) >> 4; // Z axis
}
