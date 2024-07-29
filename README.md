For design and description, see <https://www.wolai.com/zymbox/qsUPoFaTyy88NWHKrjFKtW>
# RISC-V Simulator Template

A template which enables you to write verilog-like C++ code.

## How to use

Go to [docs/help.md](docs/help.md) for more information.

## Design

We propose this template to better simulate the behavior of real hardware.
That is, register's value will be updated in the next cycle after assigned,
and wire's value will be updated with respect to the connected register or wire.

However, it's not easy to synchronize the values of registers and wires in C++ simulation,
since user may forget to synchronize the value after a cycle is done.

Therefore, we purpose such a framework, which features automatic value
synchronization, to help user to write the code in a safer way.
Since the code is written in C++, IDE will have better code completion
and highlight support (than Verilog), and user can debug the code with ease.

In addition, we provide a debug macro control to perform more runtime check
to help user to debug the code.

## Deficiencies

- We do not support Combination Circuit directly now. You may simulate that by simpling using normal integers as intermediate values, and arrange a good order to update the values.

- We do no support `signed` types now.

## TODO
