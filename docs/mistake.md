# Some Common Mistakes

## Bit-width Mismatch

This occurs when the bit-width of the left-hand side (LHS) and right-hand side (RHS) of an assignment operation are different.
For example, the following code will result in a compile error:

```cpp
Register <8>    r1;
Register <16>   r2;
r1 <= r2; // Error: bit-width mismatch
```

## Register/Wire Passed by Value

Register/Wire can only be passed by reference. We forbid the copy/move constructor for Register/Wire to prevent misuse.

This may cause errors in the lambda function of a wire.

```cpp
Register <8> r1;
Wire <8> w1 = [&]() { return r1; };
```

To fix this issue, you may return by reference or use the + operator to convert the value to a bit type.

```cpp
Register <8> r1;
Wire <8> w1 = [&]() -> auto & { return r1; };
Wire <8> w2 = [&]() { return +r1; }; // +r1 will return Bit<8>b
```

## C-array as Member Variable

We do not support C-arrays as member variables for synchronization. Our C++ static reflection library does not currently support parsing C-arrays as member variables.

Always use `std::array` instead.

```cpp
struct NeedToSync {
    std::array <Register <1>, 8> data;
    std::array <Wire <1>, 8> data2;
};
```

## Others

If you encounter other issues, please feel free to open an issue.
