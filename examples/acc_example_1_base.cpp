/**
 * Example of the LSM303DLHC usage with STM32F3Discovery board.
 *
 * Base accelerometer usage.
 *
 * Pin map:
 *
 * - PC_4 - UART TX (stdout/stderr)
 * - PC_5 - UART RX (stdin)
 * - PB_7 - I2C SDA of the LSM303DLHC
 * - PB_6 - I2C SCL of the LSM303DLHC
 * - PE_4 - INT1 pin of the LSM303DLHC
 */
#include "lsm303dhlc_driver.h"
#include "mbed.h"

DigitalOut led(LED2);

int main()
{
    // specify I2C pins directly
    LSM303DLHCAccelerometer accelerometer(PB_7, PB_6);
    // perform basic configuration of the accelerometer (set default frequency, enable axes, etc.)
    accelerometer.init();

    float acc_data[3];

    while (true) {
        // read accelerometer data in the m/s^2
        accelerometer.read_data(acc_data);
        printf("----------------\n");
        printf("x: %+.4f m/s^2\n", acc_data[0]);
        printf("y: %+.4f m/s^2\n", acc_data[1]);
        printf("z: %+.4f m/s^2\n", acc_data[2]);

        led = !led;
        wait(2.0);
    }
}
