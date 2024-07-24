# How to Use

When using this library, it's important to note that all the value types behave like regular integers, with the exception that we have a similar bit-width matching check as with Verilog integers.
(e.g. a 4-bit register can only be assigned from a 4-bit value)

Additionally, it is recommended to use the provided method for auto-synchronization, which can potentially save you from writing a lot of duplicated code.

## Requirements

You will need `g++-12` or later, with the flags `-std=c++20`.

Example: `g++ -std=c++20 ...`

Your code may still run on `g++-11` or earlier, but we do not guarantee it.

## Including the Library

This is a header-only library, which means you simply need to include all the required headers in your project.

We strongly recommend including `include/tools.h` to easily include all the headers.

```cpp
#include "include/tools.h"
```

## Debug Mode

We provide a debug mode, which performs additional checks in the code. To enable this, simply define the macro `_DEBUG` before including the headers.
You can also pass `-D _DEBUG` to the compiler to define the macro, or define it directly in your code.

```cpp
#define _DEBUG
```

We **strongly recommend** enabling the debug mode when developing your project.

Example: `g++ -std=c++20 -D _DEBUG ...`

## Value Types

Initially, you can treat all these types as Verilog integers.
You can assume that all the types below support basic arithmetic operations and will **clip** the value just like Verilog integer operations.

### Register

Registers are similar to those in Verilog.

To simulate registers, a `Register` is only allowed to be assigned once in a cycle.

```cpp
// Declare a 32-bit register
// The maximum bit-width depends on the max_size_t
// Currently, the max_size_t is std::uint32_t
Register<32> reg;
reg <= reg + 1; // OK, allows assignment from a value with the same bit-width
Register<16> reg2;
reg <= reg2 * reg2; // Compile error, the bit-width is different (32 vs 16)
```

### Wire

Wires are also similar to those in Verilog.

They should be assigned exactly once before reading.
They can accept a function-like input (function pointers/lambdas) to extract the value.

```cpp
// Declare a 4-bit wire
Wire<4> wire;
Register<4> reg;

// OK, assign the value from an integer
// Be careful, the value may be clipped
wire = []() { return 0b11010; }; // Clipped to 0b1010

// OK, assign the value from a register
// When the register's value changes,
// the wire's value will also change
Wire <4> wire2 = [&reg]() -> auto & { return reg; };

// Ill formed! The wire is assigned twice
wire = []() { return 0b11010; };

// Ill formed! Wire cannot accept a value
// with a different bit-width
Wire <5> wire3 = [&]() -> auto & { return reg + 4; };
```

### Bit

Bit is an intermediate type, which can be used to represent an integer with a specific bit width.

```cpp
Bit <5> b = 0b111111; // Clipped to 0b11111
b.set <4, 2> (0b110); // Set bit 4, 3, 2 to 1, 1, 0
b.set <4> (0); // Set bit 4 to 0

Bit <3> c = b.range <3, 1>;     // Copy bit 3, 2, 1 to c
Bit <4> d = b.slice <4> (1);    // Copy 4 bits from bit 1 (bit 4, 3, 2, 1) to d

Bit <1> e = d[0]; // Get the 0-th bit of d

Bit f = { b + 3, c, d }; // Concatenate  b + 3, c, d  from  high to low
```

## Synchronization

We support a feature of auto synchronization, which means that you can easily synchronize all the members of a class by simply calling the `sync_member` function.

We support 4 types of synchronization:

1. Register / Wire type synchronization.
2. An array (only std::array is supported) of synchronizable objects.
3. A class which contains only synchronizable objects and satisfies std::is_aggregate.
4. A class which has some basis that are synchronizable objects, and has a special tag.

We will show some examples of 3 and 4.

### Example 1

```cpp
// An aggregate class, just a pure struct with some member functions.
// No constructor! (That means, do not declare any constructor,
// and the compiler will generate a default constructor for you)
// See https://en.cppreference.com/w/cpp/language/aggregate_initialization
// We support at most 14 members currently.
struct case3 {
    Register <3> rs1;
    Register <3> rs2;
    Register <3> rd;
    Wire <3> rs1_data;
    Wire <3> rs2_data;
    Wire <3> rd_data;
    std::array <Register <32>, 32> reg;
};
```

### Example 2

```cpp
struct some_private {
    std::array <Register <16>, 3> private_reg;
};

struct case4 : private some_private, public case3 {
    friend class Visitor;
    using Tags = SyncTags <case3, some_private>;
};

// The synchronization function
void demo() {
    case4 c;
    sync_member(c);
}
```

## Common Mistakes

Refer to the [mistake](mistake.md) page to see some common mistakes.

## Examples

See the demo folder for more examples.
