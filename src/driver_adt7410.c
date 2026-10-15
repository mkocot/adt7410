/**
 * Copyright (c) 2026 Marcin Kocot All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_adt7410.c
 * @brief     driver adt7410 source file
 * @version   1.0.0
 * @author    Marcin Kocot
 * @date      2026-01-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2026/01/30  <td>1.0      <td>Marcin Kocot  <td>first upload
 * </table>
 */

#include "driver_adt7410.h"

#define CHIP_NAME                 "ADT7410"            /**< chip name */
#define MANUFACTURER_NAME         "ANALOG DEVICES"     /**< manufacturer name */
#define SUPPLY_VOLTAGE_MIN        2.7f                 /**< chip min supply voltage */
#define SUPPLY_VOLTAGE_MAX        5.5f                 /**< chip max supply voltage */
#define MAX_CURRENT               210.0f               /**< chip max current at 3.3V */
#define TEMPERATURE_MIN           -55.0f               /**< chip min operating temperature */
#define TEMPERATURE_MAX           150.0f               /**< chip max operating temperature */
#define DRIVER_VERSION            1000                 /**< driver version */

/* register addresses */
#define ADT7410_REG_TEMP_MSB      0x00                 /**< temperature value most significant byte */
#define ADT7410_REG_TEMP_LSB      0x01                 /**< temperature value least significant byte */
#define ADT7410_REG_STATUS        0x02                 /**< status register (RDY = bit 7) */
#define ADT7410_REG_CONFIG        0x03                 /**< configuration register */
#define ADT7410_REG_ID            0x0B                 /**< ID register (upper 5 bits = 0x19) */
#define ADT7410_REG_RESET         0x2F                 /**< software reset (write-only) */

/* configuration register bit fields */
#define ADT7410_CFG_ONE_SHOT      (1 << 5)             /**< one-shot mode bit */
#define ADT7410_CFG_RESOLUTION    (1 << 7)             /**< 16-bit resolution bit */

/* ID register mask: upper 5 bits = 11001 = 0x19 → 0xC8..0xCF */
#define ADT7410_ID_MASK           0xF8
#define ADT7410_ID_VALUE          0xC8

/* waits exceed the datasheet typical values (power-up conversion typ 6 ms,
   one-shot conversion typ 240 ms) */
#define ADT7410_POWERUP_WAIT_MS   10                   /**< power-up first conversion wait */
#define ADT7410_CONV_WAIT_MS      288                  /**< one-shot conversion wait */

/**
 * @brief      write the chip register
 * @param[in]  *handle points to an adt7410 handle structure
 * @param[in]  reg is the chip register address
 * @param[in]  *buf points to a data buffer
 * @param[in]  len is the data buffer length
 * @return     status code
 *             - 0 success
 *             - 1 write failed
 * @note       none
 */
static uint8_t a_adt7410_write(adt7410_handle_t *handle, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return handle->iic_write(handle->iic_addr, reg, buf, len);
}

/**
 * @brief      read the chip register
 * @param[in]  *handle points to an adt7410 handle structure
 * @param[in]  reg is the chip register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the data buffer length
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
static uint8_t a_adt7410_read(adt7410_handle_t *handle, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return handle->iic_read(handle->iic_addr, reg, buf, len);
}

/**
 * @brief      get chip's information
 * @param[out] *info points to an adt7410 info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t adt7410_info(adt7410_info_t *info)
{
    if (info == NULL)
    {
        return 2;
    }

    memset(info, 0, sizeof(adt7410_info_t));
    strncpy(info->chip_name, CHIP_NAME, 32);
    strncpy(info->manufacturer_name, MANUFACTURER_NAME, 32);
    strncpy(info->interface, "IIC", 8);
    info->supply_voltage_min_v = SUPPLY_VOLTAGE_MIN;
    info->supply_voltage_max_v = SUPPLY_VOLTAGE_MAX;
    info->max_current_ma = MAX_CURRENT;
    info->temperature_min = TEMPERATURE_MIN;
    info->temperature_max = TEMPERATURE_MAX;
    info->driver_version = DRIVER_VERSION;

    return 0;
}

/**
 * @brief     initialize the chip
 *
 * Reads the ID register (0x0B) to verify identity (manufacturer ID
 * bits must match 0xC8..0xCF), then writes the configuration register
 * with 16-bit resolution and one-shot mode selected.
 *
 * @param[in] *handle points to an adt7410 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 iic failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 *            - 4 id register mismatched
 * @note      none
 */
uint8_t adt7410_init(adt7410_handle_t *handle)
{
    uint8_t id;
    uint8_t cfg;

    if (handle == NULL)
    {
        return 2;
    }
    if (handle->iic_init == NULL || handle->iic_deinit == NULL ||
        handle->iic_read == NULL || handle->iic_write == NULL ||
        handle->delay_ms == NULL)
    {
        return 3;
    }

    if (handle->iic_init() != 0)
    {
        return 1;
    }
    handle->inited = 0;

    /* read ID register (0x0B): interface stays alive even if the device
       was left in shutdown by a previous session */
    if (a_adt7410_read(handle, ADT7410_REG_ID, &id, 1) != 0)
    {
        return 1;
    }
    if ((id & ADT7410_ID_MASK) != ADT7410_ID_VALUE)
    {
        return 4;
    }

    /* write config: 16-bit resolution (bit 7) + one-shot mode (bit 5);
       this also wakes the device if it was in shutdown */
    cfg = ADT7410_CFG_RESOLUTION | ADT7410_CFG_ONE_SHOT;
    if (a_adt7410_write(handle, ADT7410_REG_CONFIG, &cfg, 1) != 0)
    {
        return 1;
    }

    /* wait past the first power-up conversion (typ 6 ms) */
    handle->delay_ms(ADT7410_POWERUP_WAIT_MS);

    handle->inited = 1;

    return 0;
}

/**
 * @brief     close the chip
 *
 * Enters shutdown mode (config bits [6:5] = 11).
 *
 * @param[in] *handle points to an adt7410 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t adt7410_deinit(adt7410_handle_t *handle)
{
    uint8_t cfg;

    if (handle == NULL)
    {
        return 2;
    }
    if (handle->inited != 1)
    {
        return 3;
    }

    /* shutdown: bits [6:5] = 11 */
    cfg = ADT7410_CFG_RESOLUTION | (3 << 5);
    if (a_adt7410_write(handle, ADT7410_REG_CONFIG, &cfg, 1) != 0)
    {
        return 1;
    }

    handle->inited = 0;

    return 0;
}

/**
 * @brief      read the temperature data in one-shot mode
 *
 * Triggers a one-shot conversion (config bit 5 = 1), waits out the
 * conversion time, then reads the 16-bit temperature value from the
 * temperature registers (auto-increment walks from 0x00 into 0x01).
 *
 * @param[in]  *handle points to an adt7410 handle structure
 * @param[out] *temperature_raw points to a raw temperature buffer
 * @param[out] *temperature_s points to a converted temperature buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
uint8_t adt7410_read_temperature(adt7410_handle_t *handle, int16_t *temperature_raw, float *temperature_s)
{
    uint8_t buf[2];

    if (handle == NULL)
    {
        return 2;
    }
    if (handle->inited != 1)
    {
        return 3;
    }

    /* trigger one-shot: set bit 5 = 1, keep bit 7 = 1 (resolution) */
    uint8_t cfg = ADT7410_CFG_ONE_SHOT | ADT7410_CFG_RESOLUTION;
    if (a_adt7410_write(handle, ADT7410_REG_CONFIG, &cfg, 1) != 0)
    {
        return 1;
    }

    /* wait past the conversion time (typ 240 ms) */
    handle->delay_ms(ADT7410_CONV_WAIT_MS);

    /* read temperature: auto-increment walks from 0x00 (MSB) into 0x01 (LSB) */
    if (a_adt7410_read(handle, ADT7410_REG_TEMP_MSB, buf, 2) != 0)
    {
        return 1;
    }

    /* combine into signed 16-bit two's complement */
    *temperature_raw = (int16_t)((buf[0] << 8) | buf[1]);

    /* optional float conversion: raw / 128 (16-bit mode: 0.0078 C/LSB) */
    if (temperature_s != NULL)
    {
        *temperature_s = (float)*temperature_raw / 128.0f;
    }

    return 0;
}

/**
 * @brief     set the chip register
 * @param[in] *handle points to an adt7410 handle structure
 * @param[in] reg is the chip register address
 * @param[in] *buf points to a data buffer
 * @param[in] len is the data buffer length
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t adt7410_set_reg(adt7410_handle_t *handle, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (handle == NULL)
    {
        return 2;
    }
    if (handle->inited != 1)
    {
        return 3;
    }

    return a_adt7410_write(handle, reg, buf, len);
}

/**
 * @brief      get the chip register
 * @param[in]  *handle points to an adt7410 handle structure
 * @param[in]  reg is the chip register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the data buffer length
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
uint8_t adt7410_get_reg(adt7410_handle_t *handle, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (handle == NULL)
    {
        return 2;
    }
    if (handle->inited != 1)
    {
        return 3;
    }

    return a_adt7410_read(handle, reg, buf, len);
}

/**
 * @brief     set the chip address
 * @param[in] *handle points to an adt7410 handle structure
 * @param[in] addr_pin is the iic device address
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 * @note      none
 */
uint8_t adt7410_set_addr_pin(adt7410_handle_t *handle, adt7410_address_t addr_pin)
{
    if (handle == NULL)
    {
        return 2;
    }

    handle->iic_addr = (uint8_t)addr_pin;

    return 0;
}

/**
 * @brief      get the chip address
 * @param[in]  *handle points to an adt7410 handle structure
 * @param[out] *addr_pin points to an address pin buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t adt7410_get_addr_pin(adt7410_handle_t *handle, adt7410_address_t *addr_pin)
{
    if (handle == NULL)
    {
        return 2;
    }

    *addr_pin = (adt7410_address_t)handle->iic_addr;

    return 0;
}
