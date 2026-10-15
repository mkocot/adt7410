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
 * @file      driver_adt7410.h
 * @brief     driver adt7410 header file
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

#ifndef DRIVER_ADT7410_H
#define DRIVER_ADT7410_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup adt7410_driver adt7410 driver function
 * @brief    adt7410 driver modules
 * @addtogroup adt7410_driver
 * @{
 */

/**
 * @brief adt7410 address enumeration definition
 */
typedef enum
{
    ADT7410_ADDRESS_A0A1_00 = ((0x48) << 1),          /**< A1=0 A0=0: 7-bit 0x48 */
    ADT7410_ADDRESS_A0A1_01 = ((0x49) << 1),          /**< A1=0 A0=1: 7-bit 0x49 */
    ADT7410_ADDRESS_A0A1_10 = ((0x4A) << 1),          /**< A1=1 A0=0: 7-bit 0x4A */
    ADT7410_ADDRESS_A0A1_11 = ((0x4B) << 1),          /**< A1=1 A0=1: 7-bit 0x4B */
} adt7410_address_t;

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

/**
 * @brief adt7410 handle structure definition
 */
typedef struct adt7410_handle_s
{
    uint8_t (*iic_init)(void);                                              /**< point to an iic_init function address */
    uint8_t (*iic_deinit)(void);                                            /**< point to an iic_deinit function address */
    uint8_t (*iic_read)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);    /**< point to an iic_read function address */
    uint8_t (*iic_write)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);   /**< point to an iic_write function address */
    void (*delay_ms)(uint32_t ms);                                          /**< point to a delay_ms function address */
#ifdef DRIVER_ADT7410_WITH_DEBUG_PRINT
    void (*debug_print)(const char *const fmt, ...);                        /**< point to a debug_print function address */
#endif
    uint8_t iic_addr;                                                       /**< iic device address */
    uint8_t inited;                                                         /**< inited flag */
} adt7410_handle_t;

/**
 * @brief adt7410 information structure definition
 */
typedef struct adt7410_info_s
{
    char chip_name[32];                /**< chip name */
    char manufacturer_name[32];        /**< manufacturer name */
    char interface[8];                 /**< chip interface name */
    float supply_voltage_min_v;        /**< chip min supply voltage */
    float supply_voltage_max_v;        /**< chip max supply voltage */
    float max_current_ma;              /**< chip max current */
    float temperature_min;             /**< chip min operating temperature */
    float temperature_max;             /**< chip max operating temperature */
    uint32_t driver_version;           /**< driver version */
} adt7410_info_t;

/**
 * @}
 */

/**
 * @defgroup adt7410_link_driver adt7410 link driver function
 * @brief    adt7410 link driver modules
 * @ingroup  adt7410_driver
 * @{
 */

/**
 * @brief     initialize adt7410_handle_t structure
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] STRUCTURE is adt7410_handle_t
 * @note      none
 */
#define DRIVER_ADT7410_LINK_INIT(HANDLE, STRUCTURE)      memset(HANDLE, 0, sizeof(STRUCTURE))

/**
 * @brief     link iic_init function
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] FUC points to an iic_init function address
 * @note      none
 */
#define DRIVER_ADT7410_LINK_IIC_INIT(HANDLE, FUC)        (HANDLE)->iic_init = FUC

/**
 * @brief     link iic_deinit function
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] FUC points to an iic_deinit function address
 * @note      none
 */
#define DRIVER_ADT7410_LINK_IIC_DEINIT(HANDLE, FUC)      (HANDLE)->iic_deinit = FUC

/**
 * @brief     link iic_read function
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] FUC points to an iic_read function address
 * @note      none
 */
#define DRIVER_ADT7410_LINK_IIC_READ(HANDLE, FUC)        (HANDLE)->iic_read = FUC

/**
 * @brief     link iic_write function
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] FUC points to an iic_write function address
 * @note      none
 */
#define DRIVER_ADT7410_LINK_IIC_WRITE(HANDLE, FUC)       (HANDLE)->iic_write = FUC

/**
 * @brief     link delay_ms function
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] FUC points to a delay_ms function address
 * @note      none
 */
#define DRIVER_ADT7410_LINK_DELAY_MS(HANDLE, FUC)        (HANDLE)->delay_ms = FUC

/**
 * @brief     link debug_print function
 * @param[in] HANDLE points to an adt7410 handle structure
 * @param[in] FUC points to a debug_print function address
 * @note      none
 */
#ifdef DRIVER_ADT7410_WITH_DEBUG_PRINT
#define DRIVER_ADT7410_LINK_DEBUG_PRINT(HANDLE, FUC)     (HANDLE)->debug_print = FUC
#else
#define DRIVER_ADT7410_LINK_DEBUG_PRINT(HANDLE, FUC)     do{}while(0)
#endif

/**
 * @}
 */

/**
 * @defgroup adt7410_base_driver adt7410 base driver function
 * @brief    adt7410 base driver modules
 * @ingroup  adt7410_driver
 * @{
 */

/**
 * @brief      get chip's information
 * @param[out] *info points to an adt7410 info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t adt7410_info(adt7410_info_t *info);

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
uint8_t adt7410_init(adt7410_handle_t *handle);

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
uint8_t adt7410_deinit(adt7410_handle_t *handle);

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
uint8_t adt7410_read_temperature(adt7410_handle_t *handle, int16_t *temperature_raw, float *temperature_s);

/**
 * @}
 */

/**
 * @defgroup adt7410_extend_driver adt7410 extend driver function
 * @brief    adt7410 extend driver modules
 * @ingroup  adt7410_driver
 * @{
 */

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
uint8_t adt7410_set_reg(adt7410_handle_t *handle, uint8_t reg, uint8_t *buf, uint16_t len);

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
uint8_t adt7410_get_reg(adt7410_handle_t *handle, uint8_t reg, uint8_t *buf, uint16_t len);

/**
 * @brief     set the chip address
 * @param[in] *handle points to an adt7410 handle structure
 * @param[in] addr_pin is the iic device address
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 * @note      none
 */
uint8_t adt7410_set_addr_pin(adt7410_handle_t *handle, adt7410_address_t addr_pin);

/**
 * @brief      get the chip address
 * @param[in]  *handle points to an adt7410 handle structure
 * @param[out] *addr_pin points to an address pin buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t adt7410_get_addr_pin(adt7410_handle_t *handle, adt7410_address_t *addr_pin);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
