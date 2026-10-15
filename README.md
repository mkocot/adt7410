# adt7410 driver

A LibDriver-style driver for the Analog Devices ADT7410 temperature sensor
with IIC interface. Written for the i2c-bridge firmware (see the
`include/adt7410.h` wrapper there), portable to any platform that provides
the interface functions in `interface/`.

Datasheet: `datasheet/ADT7410.pdf` (Rev. C).

## Features

- one-shot conversion: trigger a read, wait out the conversion time, read the
  result; the device auto-shuts down between reads
- 16-bit resolution (0.0078 °C/LSB), configurable via config register bit 7
- raw output always; converted floats behind `DRIVER_ADT7410_WITH_FLOAT_OUTPUT`
- debug printing behind `DRIVER_ADT7410_WITH_DEBUG_PRINT`
- waits exceed the datasheet typical values (10 ms power-up first conversion,
  288 ms one-shot conversion)

## API

```c
adt7410_info(handle, &info)                          /* chip information */
adt7410_init(handle)         /* ID verify (0xC8..0xCF) + config write */
adt7410_deinit(handle)       /* enter shutdown mode */

adt7410_read_temperature(handle, &t_raw, &t_s)       /* one-shot read */

adt7410_set_reg(handle, reg, buf, len)               /* extend API */
adt7410_get_reg(handle, reg, buf, len)
adt7410_set_addr_pin(handle, ADT7410_ADDRESS_*)
adt7410_get_addr_pin(handle, &addr)
```

`adt7410_init` reads the ID register (0x0B), verifies the manufacturer ID
bits (upper 5 bits = 0x19, yielding 0xC8..0xCF), then writes the config
register with 16-bit resolution and one-shot mode selected. After that,
`adt7410_read_temperature` triggers a one-shot conversion (config bit 5 = 1),
waits 288 ms (1.2× typ 240 ms), and reads the 16-bit temperature value from
the auto-incrementing temperature registers (0x00 → 0x01).

## Integration

Link the five interface functions in the handle
(`DRIVER_ADT7410_LINK_*` macros), set the address with
`adt7410_set_addr_pin`, and call `adt7410_read_temperature`. The device
answers at 7-bit addresses 0x48..0x4B, selected by the A1/A0 pins:

| A1 | A0 | 7-bit address |
|----|----|---------------|
| 0  | 0  | 0x48          |
| 0  | 1  | 0x49          |
| 1  | 0  | 0x4A          |
| 1  | 1  | 0x4B          |

## Status codes

| code | meaning |
| ---- | ------- |
| 0    | success |
| 1    | iic write/read failed |
| 2    | handle is NULL |
| 3    | handle is not initialized (or linked functions are NULL in init) |
| 4    | ID register mismatched |

## License

MIT, see `LICENSE`.
