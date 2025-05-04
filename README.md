# mu_string - Lightweight String Views

The `mu_string` module provides a lightweight, non-owning string view type (`mu_string_t`) and a mutable string view type (`mu_string_mut_t`) for use in embedded C projects. It allows you to perform common string operations without dynamic heap allocations, operating directly on existing character buffers.

The core idea is that `mu_string_t` and `mu_string_mut_t` simply point to a location in memory and store a length; they do not manage the lifetime or allocation of the underlying character data.

## Features

* **Non-Owning Views:** Operate on `const char*` (`mu_string_t`) or `char*` (`mu_string_mut_t`) buffers without memory allocation/deallocation within the module.
* **No Heap Allocation:** All operations work on provided buffers or return derived views.
* **Common String Operations:** Includes functions for:
    * Creating views from C strings or buffers.
    * Length and emptiness checks.
    * Comparison (`eq`, `cmp`, `starts_with`, `ends_with`).
    * Searching for characters or substrings.
    * Slicing and trimming based on characters or predicates.
    * Splitting based on characters or predicates.
    * Copying and appending using mutable views.
* **Sentinel Values:** Uses `MU_STRING_EMPTY`, `MU_STRING_NOT_FOUND`, and `MU_STRING_INVALID` to clearly indicate operation outcomes (empty string, item not found, invalid input/result).
* **Predicate-Based Operations:** Supports flexible searching and trimming using custom predicate functions.

## Concepts

* `mu_string_t`: A read-only view of a character sequence (`const char*` + `size_t`).
* `mu_string_mut_t`: A mutable view of a character buffer (`char*` + `size_t`).
* Functions take views as input and typically return new views derived from the inputs.

## Usage Example

```c
#include "mu_string.h"
#include <stdio.h>    // For printf
#include <stdbool.h>  // For bool (already in mu_string.h)

int main() {
    // Create read-only string views
    mu_string_t s1 = mu_string_from_cstr("Hello, world!");
    mu_string_t s2 = mu_string_from_buf("Another string", 14); // From buffer and length

    printf("String 1: '%.*s'\n", (int)s1.len, s1.buf);
    printf("String 2: '%.*s'\n", (int)s2.len, s2.buf);

    // Get length
    printf("Length of s1: %zu\n", mu_string_len(s1));

    // Comparison
    mu_string_t s1_copy = mu_string_from_cstr("Hello, world!");
    if (mu_string_eq(s1, s1_copy)) {
        printf("s1 is equal to s1_copy\n");
    }

    // Slicing
    mu_string_t slice1 = mu_string_slice(s1, 7, 12); // Slice "world"
    printf("Slice s1 (7, 12): '%.*s'\n", (int)slice1.len, slice1.buf);

    mu_string_t slice2 = mu_string_slice(s1, -6, MU_STRING_END); // Slice "world!" using negative index and END
    printf("Slice s1 (-6, END): '%.*s'\n", (int)slice2.len, slice2.buf);

    // Searching
    mu_string_t found = mu_string_find_char(s1, 'o');
    if (!mu_string_eq(found, MU_STRING_NOT_FOUND)) { // Use MU_STRING_NOT_FOUND for search failure
         printf("First 'o' found: '%.*s'\n", (int)found.len, found.buf);
    } else {
        printf("'o' not found.\n");
    }

    // Mutable string example (Appending)
    char buffer[30]; // User-provided buffer
    mu_string_mut_t dest_view = mu_string_mut_from_buf(buffer, sizeof(buffer));

    mu_string_t part_a = mu_string_from_cstr("Embed");
    mu_string_t part_b = mu_string_from_cstr("ded ");
    mu_string_t part_c = mu_string_from_cstr("Systems");

    mu_string_mut_t remaining_space = mu_string_append(dest_view, part_a);
    remaining_space = mu_string_append(remaining_space, part_b);
    remaining_space = mu_string_append(remaining_space, part_c);

    // To get the final read-only string view from the buffer:
    size_t final_len = sizeof(buffer) - remaining_space.len;
    mu_string_t final_string = mu_string_from_buf(buffer, final_len);

    printf("Appended string: '%.*s'\n", (int)final_string.len, final_string.buf);

    return 0;
}
```
Refer to the `mu_string.h` header file for the complete API documentation.
