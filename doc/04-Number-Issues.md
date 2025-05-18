# Number Issues

Bindings and events support the std::int64_t and double. They are both represented in JS as a Number.

Numbers in javascript is a double-precision 64-bit binary format similar to a double in C++. This means that it can’t cover all the possible values of std::int64_t. Just be wary that because of this, huge numbers in std::int64_t might not be represented properly in JS.

This can be solved by using BigInt in JS, but is not covered by the current `@nil-/xit` library for simplicity.