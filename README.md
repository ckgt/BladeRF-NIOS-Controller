# BladeRF-NIOS-Controller - 2.0!
A better version of the NIOS lms_spi_controller for the Nuand BladeRF. 

Did the old one work fine? yes.

This provides Blade hardware abstraction for the NIOS functional code.

This emulates *all* functionality of the old version, now it's just more flexible and modular.
```
blade_hardware.h    - The hardware function abstraction. 'blade' structure defined.
simple_types.h      - Simple types, utilities.
packet_router.h     - Takes a byte stream, routes it to the state machine that takes that magic byte value.
state_machine.h     - A template for a state machine.
sm_fx3uart.h        - The state machine that emulates the old version. Similar performance, but not thoroughly tested.
```

FPGA version changed (erm, if there are bugs).  0.1.2 --> 0.1.3


Less of this...
```
if(IORD_ALTERA_AVALON_UART_STATUS(UART_0_BASE) & ALTERA_AVALON_UART_STATUS_RRDY_MSK){
  val = IORD_ALTERA_AVALON_UART_RXDATA(UART_0_BASE);
}
```

And more of this...
```
blade.devices.host.uart_read(...);
blade.devices.host.uart_write(...);
blade.devices.lms.read(...);
blade.devices.lms.write(...);
blade.devices.si5338.read(...);
blade.devices.si5338.write(...);
blade.devices.dac.read(...);
blade.devices.dac.write(...);
blade.devices.adf4351.read(...);
blade.devices.adf4351.write(...);
```

-- Some more --
```
// All gpio functions accessible by their enum

uint32_t blade.gpio[GDEV_DEVICE_ENUM...].read(uint8_t addr, uint8_t data);
void     blade.gpio[GDEV_DEVICE_ENUM...].write(uint32_t addr, uint32_t data);

// All gpio functions are also accessible by their old address bytes from uart_cmd.
// This is the same functionality as the old device lookup table.

blade_device_procedure proc = blade_find_gpio_procedure(uint8_t addr);
proc.read(...);
proc.write(...);
```

# Better Debugging!  -  (even on a PC)

PC_TEST_lms_spi_controller.c gives an example of how to compile the code on a PC environment.
Why do this? Because it makes it sooo much faster to change and debug a state machine.

Simply #define BLADE_NULL_HARDWARE and you can compile everything (all of it) from the NIOS project in something like MSVC or GDB.
All the hardware mappings are replaced with print statements... so the console gets pretty crazy, but you can map whatever simulated hardware activity you want.

There is also a BLADE_NIOS_DEBUG define that will enable function logging to the JTAG output when running on the NIOS.
Almost all hardware functions will be logged. This is done in blade_hardware.h with DSHOW_CALL and DLOG macros. You can easily modify the file to change how a particular function is displayed when logged.

# More Machines!

The previous lms_spi_controller was organized around a state machine architecture. The machine would activate with a single special 'magic' byte at the beginning of the host transaction. For various reasons, I wanted different magics and multiple state machines. There is a new structure called the packet_router that will allow many state machines to monitor the incoming byte stream (just 2 slots by default, change MAX_STATE_MACHINES for more) and act accordingly. 

The current state machine implementation of the old version does not use switch states like before. Now it uses a function lookup table, using the current state as the lookup index.
