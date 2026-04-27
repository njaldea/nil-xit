# Number Issues

Bindings and events support the std::int64_t and double. They are both represented in JS as a Number.


Numbers in JavaScript are double-precision 64-bit binary format (IEEE 754), similar to a C++ `double`. This means that not all possible values of `std::int64_t` can be represented exactly in JS. Be aware that very large or very small `std::int64_t` values may lose precision when sent to the frontend.

This can be solved by using BigInt in JS, but is not covered by the current `@nil-/xit` library for simplicity.