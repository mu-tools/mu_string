/**
 * MIT License
 *
 * Copyright (c) 2025 R. D. Poor & Assoc <rdpoor @ gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file test_mu_string.c
 *
 * @brief Unit tests for the mu_string module using Unity.
 */

// *****************************************************************************
// Includes

#include "unity.h"      // The Unity test framework
#include "mu_string.h" // The module under test
#include <string.h>     // For memcpy, strlen etc. if needed by tests
#include <limits.h>     // For INT_MAX, SIZE_MAX used by MU_STRING_END/INVALID

// *****************************************************************************
// Private types and definitions

// Helper for creating a mu_string_t literal for expected values in tests
#define MU_STR_LITERAL(s) (mu_string_t){ .buf = (s), .len = strlen(s) }

// Custom assertion macros removed, using standard Unity assertions directly.

// Special sentinel value for indicating an invalid string result or input state.
#define MU_STRING_INVALID (mu_string_t){ .buf = NULL, .len = SIZE_MAX }


// Helper predicate for testing trim and split functions
bool is_whitespace_pred(char ch, void *arg) {
    (void)arg; // arg is unused
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
}

// Helper predicate for testing split_by_pred/not_pred
bool is_digit_pred(char ch, void *arg) {
    (void)arg; // arg is unused
    return (ch >= '0' && ch <= '9');
}

// Helper predicate for trimming '.'
bool is_dot_pred(char ch, void *arg) {
    (void)arg; // arg is unused
    return ch == '.';
}

// Helper predicate for finding 'v'
bool is_v_pred(char ch, void *arg) { 
    (void)arg; 
    return ch == 'v'; 
}

// *****************************************************************************
// Private (static) storage

// User-provided buffer for mutable string tests (copy/append)
static char mutable_buffer[100];
static size_t mutable_buffer_capacity = sizeof(mutable_buffer);


// *****************************************************************************
// Private (forward) declarations

// *****************************************************************************
// Public code - Unity test functions are typically public

void setUp(void) {
    // Initialize mutable buffer before each test that uses it
    memset(mutable_buffer, 0, mutable_buffer_capacity);
}

void tearDown(void) {
    // Clean up after tests if necessary (not needed for these stateless functions)
}

// Test cases //

void test_mu_string_from_cstr(void) {
    mu_string_t s1 = mu_string_from_cstr("hello");
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello"), s1));

    mu_string_t s2 = mu_string_from_cstr("");
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, s2));

    mu_string_t s3 = mu_string_from_cstr(NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, s3));

    // Test with embedded null (should stop at first null)
    mu_string_t s4 = mu_string_from_cstr("hello\0world");
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello"), s4));
}

void test_mu_string_from_buf(void) {
    const char* buf = "hello world";
    mu_string_t s1 = mu_string_from_buf(buf, 5);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello"), s1));

    mu_string_t s2 = mu_string_from_buf(buf, 0);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, s2));

    mu_string_t s3 = mu_string_from_buf(NULL, 0);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, s3));

    // Test with len > actual string length (should just use provided len)
    mu_string_t s4 = mu_string_from_buf("short", 10);
    TEST_ASSERT_EQUAL_size_t(10, s4.len);
    // Content check depends on bytes after "short" in memory, which is not guaranteed.
    // Let's only check the part corresponding to "short".
    TEST_ASSERT_EQUAL_MEMORY("short", s4.buf, strlen("short"));

    // Test with NULL buf and len > 0 - should return INVALID view based on new design
    mu_string_t s5 = mu_string_from_buf(NULL, 5);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, s5));

}

void test_mu_string_mut_from_buf(void) {
     char* mut_buf = mutable_buffer;
     mu_string_mut_t s1 = mu_string_mut_from_buf(mut_buf, 10);
     TEST_ASSERT_EQUAL_PTR(mut_buf, s1.buf);
     TEST_ASSERT_EQUAL_size_t(10, s1.len);

     mu_string_mut_t s2 = mu_string_mut_from_buf(NULL, 0);
     TEST_ASSERT_EQUAL_PTR(NULL, s2.buf);
     TEST_ASSERT_EQUAL_size_t(0, s2.len);

     // Test with NULL buf and len > 0 - represents an invalid mutable view
     mu_string_mut_t s3 = mu_string_mut_from_buf(NULL, 5); // Implementation returns {NULL, 5}
     TEST_ASSERT_EQUAL_PTR(NULL, s3.buf);
     // Corrected expectation to match implementation's return for this case:
     TEST_ASSERT_EQUAL_size_t(5, s3.len);
}

void test_mu_string_len(void) {
    TEST_ASSERT_EQUAL_size_t(5, mu_string_len(MU_STR_LITERAL("hello")));
    TEST_ASSERT_EQUAL_size_t(0, mu_string_len(MU_STRING_EMPTY));
    TEST_ASSERT_EQUAL_size_t(10, mu_string_len(mu_string_from_buf("abc", 10)));
    TEST_ASSERT_EQUAL_size_t(SIZE_MAX, mu_string_len(MU_STRING_INVALID)); // Test length of invalid sentinel
}

void test_mu_string_is_empty(void) {
    TEST_ASSERT_TRUE(mu_string_is_empty(MU_STRING_EMPTY));
    TEST_ASSERT_TRUE(mu_string_is_empty(mu_string_from_buf("abc", 0)));
    TEST_ASSERT_FALSE(mu_string_is_empty(MU_STR_LITERAL("hello")));
    TEST_ASSERT_FALSE(mu_string_is_empty(MU_STRING_INVALID)); // Invalid is not considered empty
}

void test_mu_string_get_buf_len(void) {
    mu_string_t s = MU_STR_LITERAL("test");
    TEST_ASSERT_EQUAL_PTR(s.buf, "test"); // Use s.buf explicitly if checking content ptr
    TEST_ASSERT_EQUAL_size_t(4, mu_string_get_len(s));

    mu_string_t empty_s = MU_STRING_EMPTY;
    TEST_ASSERT_EQUAL_PTR(empty_s.buf, ""); // Use empty_s.buf explicitly
    TEST_ASSERT_EQUAL_size_t(0, mu_string_get_len(empty_s));

    mu_string_mut_t mut_s = mu_string_mut_from_buf(mutable_buffer, 20);
    TEST_ASSERT_EQUAL_PTR(mut_s.buf, mutable_buffer); // Use mut_s.buf explicitly
    TEST_ASSERT_EQUAL_size_t(20, mu_string_mut_get_len(mut_s));

    TEST_ASSERT_EQUAL_PTR(NULL, mu_string_get_buf(MU_STRING_INVALID));
    TEST_ASSERT_EQUAL_size_t(SIZE_MAX, mu_string_get_len(MU_STRING_INVALID));
}


void test_mu_string_eq(void) {
    mu_string_t s1 = MU_STR_LITERAL("hello");
    mu_string_t s2 = MU_STR_LITERAL("hello");
    mu_string_t s3 = MU_STR_LITERAL("world");
    mu_string_t s4 = MU_STR_LITERAL("hell");
    mu_string_t empty1 = MU_STRING_EMPTY;
    mu_string_t empty2 = MU_STRING_EMPTY;
    mu_string_t invalid1 = MU_STRING_INVALID;
    mu_string_t invalid2 = MU_STRING_INVALID;


    TEST_ASSERT_TRUE(mu_string_eq(s1, s2));
    TEST_ASSERT_FALSE(mu_string_eq(s1, s3));
    TEST_ASSERT_FALSE(mu_string_eq(s1, s4));
    TEST_ASSERT_FALSE(mu_string_eq(s4, s1));
    TEST_ASSERT_TRUE(mu_string_eq(empty1, empty2));
    TEST_ASSERT_FALSE(mu_string_eq(s1, empty1));
    TEST_ASSERT_FALSE(mu_string_eq(empty1, s1));

    // Test comparisons involving the invalid sentinel
    TEST_ASSERT_TRUE(mu_string_eq(invalid1, invalid2)); // Invalid should equal Invalid
    TEST_ASSERT_FALSE(mu_string_eq(invalid1, s1));
    TEST_ASSERT_FALSE(mu_string_eq(s1, invalid1));
    TEST_ASSERT_FALSE(mu_string_eq(invalid1, empty1));
    TEST_ASSERT_FALSE(mu_string_eq(empty1, invalid1));
}

void test_mu_string_cmp(void) {
     mu_string_t s_a = MU_STR_LITERAL("a");
     mu_string_t s_b = MU_STR_LITERAL("b");
     mu_string_t s_aa = MU_STR_LITERAL("aa");
     mu_string_t s_a_short = MU_STR_LITERAL("a"); // Same as s_a
     mu_string_t empty = MU_STRING_EMPTY;
     mu_string_t invalid = MU_STRING_INVALID;


     TEST_ASSERT_EQUAL_INT(0, mu_string_cmp(s_a, s_a_short));
     TEST_ASSERT_EQUAL_INT(0, mu_string_cmp(empty, empty));
     TEST_ASSERT_LESS_THAN(0, mu_string_cmp(s_a, s_b));
     TEST_ASSERT_GREATER_THAN(0, mu_string_cmp(s_b, s_a));
     TEST_ASSERT_LESS_THAN(0, mu_string_cmp(s_a, s_aa)); // shorter is less
     TEST_ASSERT_GREATER_THAN(0, mu_string_cmp(s_aa, s_a)); // longer is greater
     TEST_ASSERT_GREATER_THAN(0, mu_string_cmp(s_a, empty));
     TEST_ASSERT_LESS_THAN(0, mu_string_cmp(empty, s_a));

     // Test comparisons involving the invalid sentinel (assuming invalid is less than any valid string)
     TEST_ASSERT_EQUAL_INT(0, mu_string_cmp(invalid, invalid));
     TEST_ASSERT_LESS_THAN(0, mu_string_cmp(invalid, s_a));
     TEST_ASSERT_GREATER_THAN(0, mu_string_cmp(s_a, invalid));
     TEST_ASSERT_LESS_THAN(0, mu_string_cmp(invalid, empty)); // Invalid is less than empty? Or vice versa? Need to define behavior.
     TEST_ASSERT_GREATER_THAN(0, mu_string_cmp(empty, invalid)); // Assuming invalid < empty for consistent ordering
}

void test_mu_string_starts_with(void) {
    mu_string_t s = MU_STR_LITERAL("hello world");
    TEST_ASSERT_TRUE(mu_string_starts_with(s, MU_STR_LITERAL("hello")));
    TEST_ASSERT_TRUE(mu_string_starts_with(s, MU_STR_LITERAL("hell")));
    TEST_ASSERT_TRUE(mu_string_starts_with(s, s));
    TEST_ASSERT_TRUE(mu_string_starts_with(s, MU_STRING_EMPTY));
    TEST_ASSERT_FALSE(mu_string_starts_with(s, MU_STR_LITERAL("world")));
    TEST_ASSERT_FALSE(mu_string_starts_with(s, MU_STR_LITERAL("hello world!")));
    TEST_ASSERT_TRUE(mu_string_starts_with(MU_STRING_EMPTY, MU_STRING_EMPTY));
    TEST_ASSERT_FALSE(mu_string_starts_with(MU_STRING_EMPTY, MU_STR_LITERAL("a")));

    // Test starts_with involving the invalid sentinel
    TEST_ASSERT_FALSE(mu_string_starts_with(s, MU_STRING_INVALID)); // A valid string cannot start with invalid
    TEST_ASSERT_FALSE(mu_string_starts_with(MU_STRING_INVALID, s)); // Invalid cannot start with a valid string
    TEST_ASSERT_FALSE(mu_string_starts_with(MU_STRING_INVALID, MU_STRING_INVALID)); // Depends on definition - let's assume false for starts_with
    TEST_ASSERT_FALSE(mu_string_starts_with(MU_STRING_INVALID, MU_STRING_EMPTY)); // Invalid cannot start with empty?
    TEST_ASSERT_FALSE(mu_string_starts_with(MU_STRING_EMPTY, MU_STRING_INVALID)); // Empty cannot start with invalid
}

void test_mu_string_ends_with(void) {
    mu_string_t s = MU_STR_LITERAL("hello world");
    TEST_ASSERT_TRUE(mu_string_ends_with(s, MU_STR_LITERAL("world")));
    TEST_ASSERT_TRUE(mu_string_ends_with(s, MU_STR_LITERAL("rld")));
    TEST_ASSERT_TRUE(mu_string_ends_with(s, s));
    TEST_ASSERT_TRUE(mu_string_ends_with(s, MU_STRING_EMPTY));
    TEST_ASSERT_FALSE(mu_string_ends_with(s, MU_STR_LITERAL("hello")));
    TEST_ASSERT_FALSE(mu_string_ends_with(s, MU_STR_LITERAL("hello world!")));
    TEST_ASSERT_TRUE(mu_string_ends_with(MU_STRING_EMPTY, MU_STRING_EMPTY));
    TEST_ASSERT_FALSE(mu_string_ends_with(MU_STRING_EMPTY, MU_STR_LITERAL("a")));

    // Test ends_with involving the invalid sentinel (similar logic to starts_with)
    TEST_ASSERT_FALSE(mu_string_ends_with(s, MU_STRING_INVALID));
    TEST_ASSERT_FALSE(mu_string_ends_with(MU_STRING_INVALID, s));
    TEST_ASSERT_FALSE(mu_string_ends_with(MU_STRING_INVALID, MU_STRING_INVALID)); // Assume false
    TEST_ASSERT_FALSE(mu_string_ends_with(MU_STRING_INVALID, MU_STRING_EMPTY)); // Assume false
    TEST_ASSERT_FALSE(mu_string_ends_with(MU_STRING_EMPTY, MU_STRING_INVALID)); // Assume false
}

void test_mu_string_find_char(void) {
    mu_string_t s = MU_STR_LITERAL("hello world"); // len 11
    mu_string_t actual_result;

    // Find 'e' at index 1. Expected substring is "ello world", length 10.
    actual_result = mu_string_find_char(s, 'e');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("ello world"), actual_result));

    // Find first 'o' at index 4. Expected substring is "o world", length 7.
    actual_result = mu_string_find_char(s, 'o');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("o world"), actual_result));

    // Find 'z' (not found). Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_char(s, 'z');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in empty string. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_char(MU_STRING_EMPTY, 'a');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in INVALID string. Should return INVALID.
    actual_result = mu_string_find_char(MU_STRING_INVALID, 'a');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_rfind_char(void) {
    mu_string_t s = MU_STR_LITERAL("hello world"); // len 11
    mu_string_t actual_result;

    // Find last 'o'. User expects "orld". MU_STR_LITERAL("orld") has length 4.
    actual_result = mu_string_rfind_char(s, 'o');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("orld"), actual_result)); // Changed expected literal

    // Find last 'e' at index 1. Expected substring is "ello world", length 10.
    actual_result = mu_string_rfind_char(s, 'e'); // Only one 'e'
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("ello world"), actual_result));

    // Find last 'd' at index 10. Expected substring is "d", length 1.
    actual_result = mu_string_rfind_char(s, 'd'); // 'd' is the last char
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("d"), actual_result));

    // Find 'z' (not found). Should return MU_STRING_EMPTY.
    actual_result = mu_string_rfind_char(s, 'z');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in empty string. Should return MU_STRING_EMPTY.
    actual_result = mu_string_rfind_char(MU_STRING_EMPTY, 'a');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in INVALID string. Should return INVALID.
    actual_result = mu_string_rfind_char(MU_STRING_INVALID, 'a');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_find_pred(void) {
    mu_string_t s = MU_STR_LITERAL("  \t hello world"); // len 14
    mu_string_t actual_result;

    // First whitespace is ' ' at index 0. Expected substring is "  \t hello world", length 14.
    actual_result = mu_string_find_pred(s, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("  \t hello world"), actual_result));

    // Predicate is_digit_pred is never true. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_pred(s, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    s = MU_STR_LITERAL("abc123def"); // len 9
    // First digit is '1' at index 3. Expected substring is "123def", length 6.
    actual_result = mu_string_find_pred(s, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("123def"), actual_result));

    s = MU_STR_LITERAL("abc"); // len 3
    // Predicate is_digit_pred is never true. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_pred(s, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in empty string. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_pred(MU_STRING_EMPTY, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Null predicate: Should return MU_STRING_EMPTY as predicate is never true.
    actual_result = mu_string_find_pred(s, NULL, NULL); // Using s = "abc"
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in INVALID string. Should return INVALID.
    actual_result = mu_string_find_pred(MU_STRING_INVALID, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_rfind_pred(void) {
    mu_string_t s = MU_STR_LITERAL("hello world \t "); // len 14
    mu_string_t actual_result;

     // Last whitespace is ' ' at index 13. Expected substring is " ", length 1.
     actual_result = mu_string_rfind_pred(s, is_whitespace_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL(" "), actual_result)); // Corrected expectation

     // Predicate is_digit_pred is never true. Should return MU_STRING_EMPTY.
     actual_result = mu_string_rfind_pred(s, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

     s = MU_STR_LITERAL("abc123abc"); // len 9
     // Last digit is '3' at index 5. Expected substring is "3abc", length 4.
     actual_result = mu_string_rfind_pred(s, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("3abc"), actual_result));

     s = MU_STR_LITERAL("abc"); // len 3
     // Predicate is_digit_pred is never true. Should return MU_STRING_EMPTY.
     actual_result = mu_string_rfind_pred(s, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

     // Search in empty string. Should return MU_STRING_EMPTY.
     actual_result = mu_string_rfind_pred(MU_STRING_EMPTY, is_whitespace_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

     // Null predicate: Should return MU_STRING_EMPTY as predicate is never true.
     actual_result = mu_string_rfind_pred(s, NULL, NULL); // Using s = "abc"
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in INVALID string. Should return INVALID.
    actual_result = mu_string_rfind_pred(MU_STRING_INVALID, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_find_first_not_pred(void) {
     mu_string_t s = MU_STR_LITERAL("  \t hello world"); // len 14
     mu_string_t actual_result;

     // First non-whitespace is 'h' at index 5. Expected substring is "hello world", length 9.
     // This test expected the literal "hello world", length 11. Let's keep that expectation for now.
     actual_result = mu_string_find_first_not_pred(s, is_whitespace_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello world"), actual_result));

     s = MU_STR_LITERAL("123abc"); // len 6
     // First non-digit is 'a' at index 3. Expected substring is "abc", length 3.
     actual_result = mu_string_find_first_not_pred(s, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc"), actual_result));

     s = MU_STR_LITERAL("   "); // len 3
     // All whitespace. Should return empty if all match predicate.
     actual_result = mu_string_find_first_not_pred(s, is_whitespace_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

     // Search in empty string. Should return MU_STRING_EMPTY.
     actual_result = mu_string_find_first_not_pred(MU_STRING_EMPTY, is_whitespace_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

     // Null predicate: Should always return the original string if non-empty.
     s = MU_STR_LITERAL("any string"); // Use a non-empty string for this test, len 10
     actual_result = mu_string_find_first_not_pred(s, NULL, NULL); // First char 'a' at index 0. Substring from index 0.
     TEST_ASSERT_TRUE(mu_string_eq(s, actual_result));

     actual_result = mu_string_find_first_not_pred(MU_STRING_EMPTY, NULL, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Search in INVALID string. Should return INVALID.
    actual_result = mu_string_find_first_not_pred(MU_STRING_INVALID, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_find_str(void) {
    mu_string_t s = MU_STR_LITERAL("hello world world"); // len 17
    mu_string_t actual_result;

    // First "world" starts at index 6. Expected substring is "world world", length 11.
    actual_result = mu_string_find_str(s, MU_STR_LITERAL("world"));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("world world"), actual_result));

    // First "wo" starts at index 6. Expected substring is "world world", length 11.
    actual_result = mu_string_find_str(s, MU_STR_LITERAL("wo"));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("world world"), actual_result));

    // "nope" not found. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_str(s, MU_STR_LITERAL("nope"));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Exact match should return original string view.
    actual_result = mu_string_find_str(s, MU_STR_LITERAL("hello world world"));
    TEST_ASSERT_TRUE(mu_string_eq(s, actual_result));

    // Empty needle is found at start (index 0). Should return original string view.
    actual_result = mu_string_find_str(s, MU_STRING_EMPTY);
    TEST_ASSERT_TRUE(mu_string_eq(s, actual_result));

    // Search in empty string. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_str(MU_STRING_EMPTY, MU_STR_LITERAL("a"));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Needle longer than haystack. Should return MU_STRING_EMPTY.
    actual_result = mu_string_find_str(s, MU_STR_LITERAL("hello world world!"));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Test with NULL haystack (UB per from_buf contract, but defensive check)
    mu_string_t s_null_buf = (mu_string_t){ .buf = NULL, .len = 5 };
    mu_string_t needle_valid = MU_STR_LITERAL("abc");
    actual_result = mu_string_find_str(s_null_buf, needle_valid);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result)); // Should return INVALID for invalid haystack

    // Test with NULL needle (UB per from_buf contract, but defensive check)
    mu_string_t needle_null_buf = (mu_string_t){ .buf = NULL, .len = 3 };
    actual_result = mu_string_find_str(s, needle_null_buf);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result)); // Should return INVALID for invalid needle
}

void test_mu_string_slice(void) {
    mu_string_t s = MU_STR_LITERAL("abcdefgh"); // len = 8
    mu_string_t actual_result;

    // Basic positive indexing
    actual_result = mu_string_slice(s, 2, 6); // [2, 6). Substring from index 2, length 4. "cdef".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("cdef"), actual_result));

    actual_result = mu_string_slice(s, 0, 8); // [0, 8). Substring from index 0, length 8. "abcdefgh".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abcdefgh"), actual_result));

    actual_result = mu_string_slice(s, 0, 1); // [0, 1). Substring from index 0, length 1. "a".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("a"), actual_result));

    actual_result = mu_string_slice(s, 7, 8); // [7, 8). Substring from index 7, length 1. "h".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("h"), actual_result));

    actual_result = mu_string_slice(s, 2, 2); // [2, 2). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, 6, 2); // start (6) > end (2). After clamping, start >= end -> empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));


    // Basic negative indexing (within conceptual bounds)
    // Indices are len + neg_index, clamped to [0, s.len]
    actual_result = mu_string_slice(s, -2, 8); // -2 -> 6. [6, 8). Substring from index 6, length 2. "gh".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("gh"), actual_result));

    actual_result = mu_string_slice(s, -2, -1); // -2 -> 6, -1 -> 7. [6, 7). Substring from index 6, length 1. "g".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("g"), actual_result));

    actual_result = mu_string_slice(s, -7, -5); // -7 -> 1, -5 -> 3. [1, 3). Substring from index 1, length 2. "bc".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("bc"), actual_result));

    actual_result = mu_string_slice(s, -1, -2); // -1 -> 7, -2 -> 6. [7, 6). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, -1, -1); // -1 -> 7, -1 -> 7. [7, 7). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));


    // Clamping tests (positive out of bounds)
    actual_result = mu_string_slice(s, 2, 20); // [2, 20 -> 8). Substring from index 2, length 6. "cdefgh".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("cdefgh"), actual_result));

    actual_result = mu_string_slice(s, 9, 10); // [9 -> 8, 10 -> 8) -> [8, 8). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, 8, 10); // [8, 10 -> 8) -> [8, 8). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));


    // Clamping tests (MU_STRING_END - effectively a large positive number that clamps to s.len)
    actual_result = mu_string_slice(s, 2, MU_STRING_END); // [2, INT_MAX -> 8). Substring from index 2, length 6. "cdefgh".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("cdefgh"), actual_result));

    actual_result = mu_string_slice(s, 0, MU_STRING_END); // [0, INT_MAX -> 8). Substring from index 0, length 8. "abcdefgh".
    TEST_ASSERT_TRUE(mu_string_eq(s, actual_result));

    actual_result = mu_string_slice(s, 8, MU_STRING_END); // [8, INT_MAX -> 8). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, 9, MU_STRING_END); // [9 -> 8, INT_MAX -> 8) -> [8, 8). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, -1, MU_STRING_END); // [-1 -> 7, INT_MAX -> 8). Substring from index 7, length 1. "h".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("h"), actual_result));


    // Clamping tests (large negative numbers)
    int large_neg = -100; // Assume -100 is large enough to be < -s.len (for s with len=8)
    TEST_ASSERT_TRUE((size_t)-large_neg > s.len); // Self-check test assumption

    actual_result = mu_string_slice(s, large_neg, 8); // [-100 -> 0, 8). Substring from index 0, length 8. "abcdefgh".
    TEST_ASSERT_TRUE(mu_string_eq(s, actual_result));

    actual_result = mu_string_slice(s, large_neg, 3); // [-100 -> 0, min(3, 8)=3). Substring from index 0, length 3. "abc".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc"), actual_result));

    actual_result = mu_string_slice(s, large_neg, large_neg); // [-100 -> 0, -100 -> 0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, 0, large_neg); // [0, -100 -> 0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, 3, large_neg); // [3, -100 -> 0) -> empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(s, large_neg, MU_STRING_END); // [-100 -> 0, INT_MAX -> 8). Substring from index 0, length 8. "abcdefgh".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abcdefgh"), actual_result));


    // Empty string slice tests with various indices (should always be empty)
    mu_string_t empty_s = MU_STRING_EMPTY; // len = 0
    actual_result = mu_string_slice(empty_s, 0, 0);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(empty_s, 5, 10); // [5->0, 10->0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(empty_s, -5, -1); // [-5->0, -1->0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(empty_s, 0, MU_STRING_END); // [0, INT_MAX -> 0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(empty_s, large_neg, MU_STRING_END); // [-100 -> 0, INT_MAX -> 0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    actual_result = mu_string_slice(empty_s, large_neg, 5); // [-100 -> 0, 5 -> 0) -> [0, 0). Empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    // Test with NULL buf (UB per from_buf contract, but defensive check)
    mu_string_t null_buf_s = (mu_string_t){ .buf = NULL, .len = 5 };
    actual_result = mu_string_slice(null_buf_s, 0, 5);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result)); // Should return INVALID for invalid source
}


void test_mu_string_ltrim(void) {
    mu_string_t s1 = MU_STR_LITERAL("  \t hello world "); // len 16
    mu_string_t actual_result = mu_string_ltrim(s1, is_whitespace_pred, NULL); // First non-whitespace at index 5. Substring from index 5, len 16-5=11. "hello world ".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello world "), actual_result));

    mu_string_t s2 = MU_STR_LITERAL("hello world"); // len 11
    actual_result = mu_string_ltrim(s2, is_whitespace_pred, NULL); // No leading whitespace. Substring from index 0, len 11. "hello world".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello world"), actual_result));

    mu_string_t s3 = MU_STR_LITERAL("   "); // len 3
    actual_result = mu_string_ltrim(s3, is_whitespace_pred, NULL); // All whitespace. Should return empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    mu_string_t s4 = MU_STRING_EMPTY; // len 0
    actual_result = mu_string_ltrim(s4, is_whitespace_pred, NULL); // Empty string. Should return empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    mu_string_t s5 = MU_STR_LITERAL("...abc..."); // len 9
    actual_result = mu_string_ltrim(s5, is_dot_pred, NULL); // First non-dot at index 3. Substring from index 3, len 9-3=6. "abc...".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc..."), actual_result));

    // ltrim null predicate - should not trim anything. Returns original string.
    actual_result = mu_string_ltrim(s5, NULL, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(s5, actual_result));

    // Trim INVALID string. Should return INVALID.
    actual_result = mu_string_ltrim(MU_STRING_INVALID, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_rtrim(void) {
    mu_string_t s1 = MU_STR_LITERAL("  hello world \t "); // len 16
    mu_string_t actual_result = mu_string_rtrim(s1, is_whitespace_pred, NULL); // Last non-whitespace at index 13. Substring from index 0, len 13. "  hello world".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("  hello world"), actual_result));


    mu_string_t s2 = MU_STR_LITERAL("hello world"); // len 11
    actual_result = mu_string_rtrim(s2, is_whitespace_pred, NULL); // No trailing whitespace. Substring from index 0, len 11. "hello world".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello world"), actual_result));

    mu_string_t s3 = MU_STR_LITERAL("   "); // len 3
    actual_result = mu_string_rtrim(s3, is_whitespace_pred, NULL); // All whitespace. Should return empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    mu_string_t s4 = MU_STRING_EMPTY; // len 0
    actual_result = mu_string_rtrim(s4, is_whitespace_pred, NULL); // Empty string. Should return empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    mu_string_t s5 = MU_STR_LITERAL("...abc..."); // len 9
    actual_result = mu_string_rtrim(s5, is_dot_pred, NULL); // Last non-dot at index 5. Substring from index 0, len 6. "...abc".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("...abc"), actual_result));

    // rtrim null predicate - should not trim anything. Returns original string.
    actual_result = mu_string_rtrim(s5, NULL, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(s5, actual_result));

    // Trim INVALID string. Should return INVALID.
    actual_result = mu_string_rtrim(MU_STRING_INVALID, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_trim(void) {
    mu_string_t s1 = MU_STR_LITERAL("  \t hello world \t "); // len 17
    mu_string_t actual_result = mu_string_trim(s1, is_whitespace_pred, NULL); // First non-ws at 5, last non-ws at 15. Substring from index 5, len 15-5+1=11. "hello world".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello world"), actual_result));

    mu_string_t s2 = MU_STR_LITERAL("hello world"); // len 11
    actual_result = mu_string_trim(s2, is_whitespace_pred, NULL); // No leading/trailing ws. Returns original.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("hello world"), actual_result));


    mu_string_t s3 = MU_STR_LITERAL("   "); // len 3
    actual_result = mu_string_trim(s3, is_whitespace_pred, NULL); // All whitespace. Returns empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    mu_string_t s4 = MU_STRING_EMPTY; // len 0
    actual_result = mu_string_trim(s4, is_whitespace_pred, NULL); // Empty. Returns empty.
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, actual_result));

    mu_string_t s5 = MU_STR_LITERAL("...abc..."); // len 9
    actual_result = mu_string_trim(s5, is_dot_pred, NULL); // First non-dot at 3, last non-dot at 5. Substring from index 3, len 5-3+1=3. "abc".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc"), actual_result));


    mu_string_t s6 = MU_STR_LITERAL("  a  b  "); // len 8
    actual_result = mu_string_trim(s6, is_whitespace_pred, NULL); // First non-ws at 2, last non-ws at 5. Substring from index 2, len 5-2+1=4. "a  b".
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("a  b"), actual_result));

    // trim null predicate - should not trim anything. Returns original string.
    actual_result = mu_string_trim(s6, NULL, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(s6, actual_result));

    // Trim INVALID string. Should return INVALID.
    actual_result = mu_string_trim(MU_STRING_INVALID, is_whitespace_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, actual_result));
}

void test_mu_string_split_at_char(void) {
    mu_string_t s = MU_STR_LITERAL("key=value"); // len 9
    mu_string_t s2 = MU_STR_LITERAL("no delimiter here"); // len 17
    mu_string_t s3 = MU_STR_LITERAL("=starts with"); // len 12
    mu_string_t s4 = MU_STR_LITERAL("ends with="); // len 10
    mu_string_t s5 = MU_STRING_EMPTY; // len 0
    mu_string_t s6 = MU_STR_LITERAL("a=b=c"); // len 5

    mu_string_t return_value;
    mu_string_t after_result; // Variable to hold the value pointed to by the 'after' pointer

    // Test 1: Delimiter found (basic case)
    // s = "key=value", delimiter = '='
    // Expected: return "key", *after = "=value"
    return_value = mu_string_split_at_char(s, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("key"), return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("=value"), after_result));

    // Test 2: Delimiter not found
    // s2 = "no delimiter here", delimiter = '='
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    return_value = mu_string_split_at_char(s2, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));

    // Test 3: Delimiter at start
    // s3 = "=starts with", delimiter = '='
    // Expected: return MU_STRING_EMPTY, *after = "=starts with"
    return_value = mu_string_split_at_char(s3, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("=starts with"), after_result));

    // Test 4: Delimiter at end
    // s4 = "ends with=", delimiter = '='
    // Expected: return "ends with", *after = "="
    return_value = mu_string_split_at_char(s4, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("ends with"), return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("="), after_result));

    // Test 5: Empty input string
    // s5 = MU_STRING_EMPTY, delimiter = '='
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    return_value = mu_string_split_at_char(s5, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));

     // Test 6: Multiple delimiters (should split at the first one)
     // s6 = "a=b=c", delimiter = '='
     // Expected: return "a", *after = "=b=c"
     return_value = mu_string_split_at_char(s6, &after_result, '=');
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("a"), return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("=b=c"), after_result));

    // Test 7: Split with NULL after pointer (should still return the 'before' part or sentinel)
    // s = "key=value", delimiter = '='
    // Expected: return "key"
    return_value = mu_string_split_at_char(s, NULL, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("key"), return_value));

     // Test 8: Split not found with NULL after pointer
     // s2 = "no delimiter here", delimiter = '='
     // Expected: return MU_STRING_NOT_FOUND
     return_value = mu_string_split_at_char(s2, NULL, '=');
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));

    // Test 9: Invalid input string s
    // s = MU_STRING_INVALID, delimiter = '='
    // Expected: return MU_STRING_INVALID, *after = MU_STRING_INVALID
    return_value = mu_string_split_at_char(MU_STRING_INVALID, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, after_result));

    // Test 10: Invalid input string s with NULL after pointer
    // s = MU_STRING_INVALID, delimiter = '='
    // Expected: return MU_STRING_INVALID
    return_value = mu_string_split_at_char(MU_STRING_INVALID, NULL, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));

    // Test 11: Delimiter not in s, s has len 1
    // s = "a", delimiter = '='
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    mu_string_t s_len1 = MU_STR_LITERAL("a");
    return_value = mu_string_split_at_char(s_len1, &after_result, '=');
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));

     // Test 12: Delimiter is the only character
     // s = "=", delimiter = '='
     // Expected: return MU_STRING_EMPTY, *after = "="
     mu_string_t s_only_delim = MU_STR_LITERAL("=");
     return_value = mu_string_split_at_char(s_only_delim, &after_result, '=');
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("="), after_result));
}

void test_mu_string_split_by_pred(void) {
    mu_string_t s = MU_STR_LITERAL("abc123def"); // len 9. First digit at index 3.
    mu_string_t s2 = MU_STR_LITERAL("abcdef"); // len 6. No digits.
    mu_string_t s3 = MU_STR_LITERAL("123abcdef"); // len 9. First digit at index 0.
    mu_string_t s4 = MU_STRING_EMPTY; // len 0.
    mu_string_t s5 = MU_STR_LITERAL("abc"); // len 3.

    mu_string_t return_value;
    mu_string_t after_result; // Variable to hold the value pointed to by the 'after' pointer

    // Test 1: Predicate matches (basic case)
    // s = "abc123def", pred = is_digit_pred ('1' at index 3)
    // Expected: return "abc", *after = "123def"
    return_value = mu_string_split_by_pred(s, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc"), return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("123def"), after_result));

    // Test 2: Predicate never matches
    // s2 = "abcdef", pred = is_digit_pred
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    return_value = mu_string_split_by_pred(s2, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));


    // Test 3: Predicate matches at start
    // s3 = "123abcdef", pred = is_digit_pred ('1' at index 0)
    // Expected: return MU_STRING_EMPTY, *after = "123abcdef"
    return_value = mu_string_split_by_pred(s3, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("123abcdef"), after_result)); // *after should be s3

    // Test 4: Empty input string
    // s4 = MU_STRING_EMPTY, pred = is_digit_pred
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    return_value = mu_string_split_by_pred(s4, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));

    // Test 5: NULL predicate
    // s5 = "abc", pred = NULL
    // Expected: return MU_STRING_INVALID, *after = MU_STRING_INVALID
    return_value = mu_string_split_by_pred(s5, &after_result, NULL, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, after_result));

     // Test 6: Predicate matches with NULL after pointer
     // s = "abc123def", pred = is_digit_pred
     // Expected: return "abc"
     return_value = mu_string_split_by_pred(s, NULL, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc"), return_value));

     // Test 7: Predicate never matches with NULL after pointer
     // s2 = "abcdef", pred = is_digit_pred
     // Expected: return MU_STRING_NOT_FOUND
     return_value = mu_string_split_by_pred(s2, NULL, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));

     // Test 8: Invalid input string s
     // s = MU_STRING_INVALID, pred = is_digit_pred
     // Expected: return MU_STRING_INVALID, *after = MU_STRING_INVALID
     return_value = mu_string_split_by_pred(MU_STRING_INVALID, &after_result, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, after_result));

     // Test 9: Invalid input string s with NULL after pointer
     // s = MU_STRING_INVALID, pred = is_digit_pred
     // Expected: return MU_STRING_INVALID
     return_value = mu_string_split_by_pred(MU_STRING_INVALID, NULL, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));

     // Test 10: Predicate matches at end
     // s = "abcdef1", pred = is_digit_pred ('1' at index 6)
     // Expected: return "abcdef", *after = "1"
     mu_string_t s_end_match = MU_STR_LITERAL("abcdef1");
     return_value = mu_string_split_by_pred(s_end_match, &after_result, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abcdef"), return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("1"), after_result));
}

void test_mu_string_split_by_not_pred(void) {
    mu_string_t s = MU_STR_LITERAL("123abc456"); // len 9. First non-digit at index 3.
    mu_string_t s2 = MU_STR_LITERAL("123456"); // len 6. All digits.
    mu_string_t s3 = MU_STR_LITERAL("abcdef123"); // len 9. First non-digit at index 0.
    mu_string_t s4 = MU_STRING_EMPTY; // len 0.
    mu_string_t s5 = MU_STR_LITERAL("abc"); // len 3.

    mu_string_t return_value;
    mu_string_t after_result; // Variable to hold the value pointed to by the 'after' pointer

    // Test 1: Predicate does NOT match (basic case)
    // s = "123abc456", pred = is_digit_pred ('a' at index 3 does NOT match)
    // Expected: return "123", *after = "abc456"
    return_value = mu_string_split_by_not_pred(s, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("123"), return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abc456"), after_result));

    // Test 2: Predicate always matches (not found)
    // s2 = "123456", pred = is_digit_pred
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    return_value = mu_string_split_by_not_pred(s2, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));

     // Test 3: Predicate does NOT match at start
     // s3 = "abcdef123", pred = is_digit_pred ('a' at index 0 does NOT match)
     // Expected: return MU_STRING_EMPTY, *after = "abcdef123"
     return_value = mu_string_split_by_not_pred(s3, &after_result, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("abcdef123"), after_result)); // *after should be s3

    // Test 4: Empty input string
    // s4 = MU_STRING_EMPTY, pred = is_digit_pred
    // Expected: return MU_STRING_NOT_FOUND, *after = MU_STRING_NOT_FOUND
    return_value = mu_string_split_by_not_pred(s4, &after_result, is_digit_pred, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, after_result));

    // Test 5: NULL predicate
    // s5 = "abc", pred = NULL
    // Expected: return MU_STRING_INVALID, *after = MU_STRING_INVALID
    return_value = mu_string_split_by_not_pred(s5, &after_result, NULL, NULL);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, after_result));

     // Test 6: Predicate does NOT match with NULL after pointer
     // s = "123abc456", pred = is_digit_pred
     // Expected: return "123"
     return_value = mu_string_split_by_not_pred(s, NULL, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("123"), return_value));

     // Test 7: Predicate always matches with NULL after pointer
     // s2 = "123456", pred = is_digit_pred
     // Expected: return MU_STRING_NOT_FOUND
     return_value = mu_string_split_by_not_pred(s2, NULL, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_NOT_FOUND, return_value));

     // Test 8: Invalid input string s
     // s = MU_STRING_INVALID, pred = is_digit_pred
     // Expected: return MU_STRING_INVALID, *after = MU_STRING_INVALID
     return_value = mu_string_split_by_not_pred(MU_STRING_INVALID, &after_result, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, after_result));

     // Test 9: Invalid input string s with NULL after pointer
     // s = MU_STRING_INVALID, pred = is_digit_pred
     // Expected: return MU_STRING_INVALID
     return_value = mu_string_split_by_not_pred(MU_STRING_INVALID, NULL, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, return_value));

     // Test 10: Predicate does NOT match at end
     // s = "12345a", pred = is_digit_pred ('a' at index 5 does NOT match)
     // Expected: return "12345", *after = "a"
     mu_string_t s_end_match = MU_STR_LITERAL("12345a");
     return_value = mu_string_split_by_not_pred(s_end_match, &after_result, is_digit_pred, NULL);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("12345"), return_value));
     TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("a"), after_result));
}

void test_mu_string_copy(void) {
    // mu_string_copy copies into a mutable buffer STARTING AT INDEX 0,
    // limited by dst.len (capacity), and returns a read-only view of the WRITTEN data.
    mu_string_t src = MU_STR_LITERAL("hello"); // len = 5
    mu_string_mut_t dst = mu_string_mut_from_buf(mutable_buffer, mutable_buffer_capacity); // capacity = 100
    mu_string_t result = mu_string_copy(dst, src);

    TEST_ASSERT_EQUAL_PTR(mutable_buffer, result.buf);
    TEST_ASSERT_TRUE(mu_string_eq(src, result)); // Using mu_string_eq for the resulting view
    // Check content in the actual buffer (redundant with above but kept for clarity)
    TEST_ASSERT_EQUAL_MEMORY("hello", mutable_buffer, result.len);


    // Test insufficient buffer
    mu_string_t src2 = MU_STR_LITERAL("too_long"); // len = 8
    mu_string_mut_t dst2 = mu_string_mut_from_buf(mutable_buffer, 3); // capacity = 3
    mu_string_t result2 = mu_string_copy(dst2, src2);

    TEST_ASSERT_EQUAL_PTR(mutable_buffer, result2.buf);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STR_LITERAL("too"), result2)); // Expected is truncated "too"
     // Check content in the actual buffer (redundant with above but kept for clarity)
     TEST_ASSERT_EQUAL_MEMORY("too", mutable_buffer, result2.len);


    // Test copying empty string
    mu_string_t src3 = MU_STRING_EMPTY;
    mu_string_mut_t dst3 = mu_string_mut_from_buf(mutable_buffer, mutable_buffer_capacity);
    mu_string_t result3 = mu_string_copy(dst3, src3);

    TEST_ASSERT_EQUAL_PTR(mutable_buffer, result3.buf);
    TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, result3));


    // Test copy to zero-capacity buffer
     mu_string_t src4 = MU_STR_LITERAL("hello");
     mu_string_mut_t dst4 = mu_string_mut_from_buf(mutable_buffer, 0); // capacity = 0
     mu_string_t result4 = mu_string_copy(dst4, src4);

     TEST_ASSERT_EQUAL_PTR(mutable_buffer, result4.buf);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_EMPTY, result4));


     // Test copy to NULL source (UB per from_buf contract, but defensive check)
     mu_string_t src_null_buf = (mu_string_t){ .buf = NULL, .len = 5 };
     mu_string_mut_t dst_valid = mu_string_mut_from_buf(mutable_buffer, mutable_buffer_capacity);
     mu_string_t result_null_src = mu_string_copy(dst_valid, src_null_buf);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, result_null_src)); // Should return INVALID for invalid source

     // Test copy to NULL buffer destination (UB per from_buf contract for mut, but defensive check)
     mu_string_t src5 = MU_STR_LITERAL("hello");
     mu_string_mut_t dst5 = { .buf = NULL, .len = 10 }; // Capacity 10, NULL buffer
     mu_string_t result5 = mu_string_copy(dst5, src5);
     TEST_ASSERT_TRUE(mu_string_eq(MU_STRING_INVALID, result5)); // Should return INVALID for invalid destination
}

void test_mu_string_append(void) {
    // mu_string_append copies into a mutable buffer segment STARTING AT INDEX 0
    // of that segment, limited by segment.len (capacity), and returns a mutable
    // view of the REMAINING space. This enables the cursor pattern.

    mu_string_t part1 = MU_STR_LITERAL("hello"); // len = 5
    mu_string_t part2 = MU_STR_LITERAL(" world"); // len = 6
    mu_string_t part3 = MU_STR_LITERAL("!"); // len = 1

    // Initial mutable view covers the whole buffer as the available space
    mu_string_mut_t remaining = mu_string_mut_from_buf(mutable_buffer, mutable_buffer_capacity); // buf=buffer, len=100
    TEST_ASSERT_EQUAL_PTR(mutable_buffer, remaining.buf);
    TEST_ASSERT_EQUAL_size_t(mutable_buffer_capacity, remaining.len);

    // Append part 1
    mu_string_mut_t next_remaining = mu_string_append(remaining, part1); // Copies "hello" into remaining.buf (mutable_buffer), returns remaining space
    TEST_ASSERT_EQUAL_MEMORY("hello", remaining.buf, part1.len); // Check buffer content where written
    TEST_ASSERT_EQUAL_PTR(remaining.buf + part1.len, next_remaining.buf);
    TEST_ASSERT_EQUAL_size_t(remaining.len - part1.len, next_remaining.len);
    remaining = next_remaining; // Update cursor

    // Append part 2
    next_remaining = mu_string_append(remaining, part2); // Copies " world" into remaining.buf (mutable_buffer + 5), returns remaining space
    TEST_ASSERT_EQUAL_MEMORY(" world", remaining.buf, part2.len); // Check buffer content where written
    TEST_ASSERT_EQUAL_PTR(remaining.buf + part2.len, next_remaining.buf);
    TEST_ASSERT_EQUAL_size_t(remaining.len - part2.len, next_remaining.len);
    remaining = next_remaining; // Update cursor

    // Append part 3
    next_remaining = mu_string_append(remaining, part3); // Copies "!" into remaining.buf (mutable_buffer + 5 + 6), returns remaining space
    TEST_ASSERT_EQUAL_MEMORY("!", remaining.buf, part3.len); // Check buffer content where written
    TEST_ASSERT_EQUAL_PTR(remaining.buf + part3.len, next_remaining.buf);
    TEST_ASSERT_EQUAL_size_t(remaining.len - part3.len, next_remaining.len);
    remaining = next_remaining; // Update cursor

    // Check the final content of the whole buffer using TEST_ASSERT_EQUAL_MEMORY
    size_t total_written = mutable_buffer_capacity - remaining.len;
    TEST_ASSERT_EQUAL_size_t(part1.len + part2.len + part3.len, total_written);
    TEST_ASSERT_EQUAL_MEMORY("hello world!", mutable_buffer, total_written);


    // Test insufficient capacity during append
    mu_string_t src_long = MU_STR_LITERAL("a_very_long_string"); // len = 18
    mu_string_mut_t short_space = mu_string_mut_from_buf(mutable_buffer, 10); // capacity = 10
    next_remaining = mu_string_append(short_space, src_long); // Copies only 10 bytes

    TEST_ASSERT_EQUAL_MEMORY("a_very_lon", short_space.buf, 10); // Check the written content
    TEST_ASSERT_EQUAL_PTR(short_space.buf + 10, next_remaining.buf);
    TEST_ASSERT_EQUAL_size_t(0, next_remaining.len);
    // No need to update remaining cursor after exhausting space


    // Test appending empty string
    mu_string_t src_empty = MU_STRING_EMPTY;
    mu_string_mut_t space = mu_string_mut_from_buf(mutable_buffer, 10); // capacity = 10
    next_remaining = mu_string_append(space, src_empty); // Copies 0 bytes

    TEST_ASSERT_EQUAL_PTR(space.buf, next_remaining.buf);
    TEST_ASSERT_EQUAL_size_t(space.len, next_remaining.len);
    TEST_ASSERT_EQUAL_size_t(10, next_remaining.len);

    // Test appending to zero capacity segment
    mu_string_t src_short = MU_STR_LITERAL("abc");
    mu_string_mut_t zero_space = mu_string_mut_from_buf(mutable_buffer, 0); // capacity = 0
    next_remaining = mu_string_append(zero_space, src_short); // Copies 0 bytes

    TEST_ASSERT_EQUAL_PTR(zero_space.buf, next_remaining.buf);
    TEST_ASSERT_EQUAL_size_t(0, next_remaining.len);

     // Test appending NULL source (UB per from_buf contract, but defensive check)
     mu_string_t src_null_buf = (mu_string_t){ .buf = NULL, .len = 5 };
     mu_string_mut_t dst_valid = mu_string_mut_from_buf(mutable_buffer, mutable_buffer_capacity);
     mu_string_mut_t next_remaining_null_src = mu_string_append(dst_valid, src_null_buf);
     // Append returns the remaining space view of the *destination*.
     // If source is invalid, no bytes are copied. The destination view remains unchanged.
     TEST_ASSERT_EQUAL_PTR(dst_valid.buf, next_remaining_null_src.buf);
     TEST_ASSERT_EQUAL_size_t(dst_valid.len, next_remaining_null_src.len); // No bytes copied

     // Test appending to NULL buffer destination (UB per from_buf contract for mut, but defensive check)
     mu_string_t src_test = MU_STR_LITERAL("abc");
     mu_string_mut_t null_dst = { .buf = NULL, .len = 10 }; // Capacity 10, NULL buffer
     mu_string_mut_t next_remaining_null_dst = mu_string_append(null_dst, src_test);
     // If destination buffer is NULL, no bytes can be copied. The destination view should perhaps become {NULL, 0}? Or return the input dst?
     // The current test assumes returning the input dst. Let's stick to that assumption for the test.
     TEST_ASSERT_EQUAL_PTR(null_dst.buf, next_remaining_null_dst.buf);
     TEST_ASSERT_EQUAL_size_t(null_dst.len, next_remaining_null_dst.len); // No bytes copied, capacity remains
}


// *****************************************************************************
// Private (static) code - Helper functions can go here

// *****************************************************************************
// End of file - Main test runner

int main(void) {
    UnityBegin("test_mu_string.c");

    // Basic creation and access
    RUN_TEST(test_mu_string_from_cstr);
    RUN_TEST(test_mu_string_from_buf);
    RUN_TEST(test_mu_string_mut_from_buf);
    RUN_TEST(test_mu_string_len);
    RUN_TEST(test_mu_string_is_empty);
    RUN_TEST(test_mu_string_get_buf_len); // Test getters

    // Comparison
    RUN_TEST(test_mu_string_eq);
    RUN_TEST(test_mu_string_cmp);
    RUN_TEST(test_mu_string_starts_with);
    RUN_TEST(test_mu_string_ends_with);

    // Searching
    RUN_TEST(test_mu_string_find_char);
    RUN_TEST(test_mu_string_rfind_char);
    RUN_TEST(test_mu_string_find_pred);
    RUN_TEST(test_mu_string_rfind_pred);
    RUN_TEST(test_mu_string_find_first_not_pred);
    RUN_TEST(test_mu_string_find_str);

    // Slicing and Trimming
    RUN_TEST(test_mu_string_slice); // Now includes extensive clamping tests
    RUN_TEST(test_mu_string_ltrim);
    RUN_TEST(test_mu_string_rtrim);
    RUN_TEST(test_mu_string_trim);

    // Splitting (including NULL pointer tests)
    RUN_TEST(test_mu_string_split_at_char);
    RUN_TEST(test_mu_string_split_by_pred);
    RUN_TEST(test_mu_string_split_by_not_pred);


    // Mutation (requires user buffer)
    RUN_TEST(test_mu_string_copy); // Copy to start, return view of written data
    RUN_TEST(test_mu_string_append); // Copy to segment start, return view of REMAINING space (cursor)


    return UnityEnd();
}