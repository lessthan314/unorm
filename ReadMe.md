# A fast unorm implementation

This project provides an implementation of an unsigned normalized decimal class (`unorm`) in a single header file. The class is designed to handle fixed-point arithmetic with normalized values, offering efficient arithmetic operations with user-defined precision

## Features

- A class `unorm<PRECISION, REVERSE>` that represents a fixed-point decimal number with `PRECISION` bits.
- A specialization `mnorm<PRECISION> = unorm<PRECISION, true>` for fast multiplication, with normalization in the reverse order for optimization.
- A specialization `anorm<PRECISION> = unorm<PRECISION, false>` for fast addition, using a different normalization approach.
- Automatic conversion from `float`, `double`, and `long double` to `unorm` for seamless integration with other types.
- Allows for printing `unorm` to a `std::ostream` for easy visualization of values.

## Installation

To use, simply include the "unorm.h" file in your source code. You can simply copy it into your project directory.

Example:
```cpp
#include "unorm.h"
#include <iostream>

int main()
{
    unorm<32, false> u = 0.5;  // Using 32-bit precision with addition optimization
    unorm<32, false> v = 0.375;
    std::cout << u << " * " << v << " = " << u * v << std::endl;  // Multiplication
    std::cout << u << " + " << v << " = " << u + v << std::endl;  // Addition
    std::cout << u << " / " << v << " = " << u / v << std::endl;  // Division
    std::cout << u << " - " << v << " = " << u - v << std::endl;  // Subtraction

    return 0;
}
```

## Limitations

- The class is implemented as a fixed point floating point number. To improve speed the denominator is $2^{(bits - 1)}$. As a result, it represents values in the range $[0,2)$ rather than the expected $[0,1]$.
- The absolute error in representing numbers in the range $[0,1]$ can be as large as $2^{-bits}$ for each number.
- On modern hardware with available floating-point units (FPUs), using this class may be slower than using native floating-point numbers of the same size. You may want to use the included `test.cpp` file to benchmark its performance.

## Using Test file

To run the provided test file compile with any c++ compiler:
```
g++ -o test test.cpp
./test
```

This will perform 229229 multiplications using both normal floating-point numbers and unorms, and display the time taken for each. This can help you determine if unorm is an appropriate choice for your use case based on your hardware.

## License

This project is licensed under the [MIT License](LICENSE).

## Authors

Thorge Chall (CAGE)