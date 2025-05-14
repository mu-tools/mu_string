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
 * @file mu_string.h
 *
 * @brief Defines a lightweight, non-owning string view type (`mu_string_t`)
 * and a mutable string view type (`mu_string_mut_t`) for embedded systems.
 * Provides functions for common string operations like comparison, searching,
 * slicing, trimming, and splitting, operating on these views without
 * heap allocations.
 *
 * The `mu_string_t` and `mu_string_mut_t` types are simple structs containing a
 * pointer to the start of the string data and its length. They do not manage
 * the memory of the string data itself.
 *
 * Functions generally return new `mu_string_t` or `mu_string_mut_t` views
 * derived from the input view(s), or booleans/integers for comparisons and
 * searches.
 *
 * Error handling: Functions may return a special `MU_STRING_INVALID` sentinel
 * value to indicate an invalid operation due to invalid input (e.g., NULL
 * buffer with non-zero length) or an internal error state. Search functions
 * typically return `MU_STRING_EMPTY` if the item is not found, consistent with
 * returning a zero-length view.
 */

#ifndef MU_STRING_H
#define MU_STRING_H

// *****************************************************************************
// Includes

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

/**
 * @brief A read-only string view.
 *
 * Does not own the memory it points to. Represents a sequence of characters
 * within a larger buffer.
 */
typedef struct {
    const char *buf; ///< Pointer to the start of the string data.
    size_t len;      ///< The number of characters in the string.
} mu_string_t;

/**
 * @brief A mutable string view.
 *
 * Does not own the memory it points to. Represents a writable buffer
 * segment with a given capacity.
 */
typedef struct {
    char *buf;  ///< Pointer to the start of the mutable buffer.
    size_t len; ///< The capacity of the mutable buffer segment.
} mu_string_mut_t;

/**
 * @brief Represents an empty read-only string.
 *
 * Defined as a mu_string_t with a pointer to an empty string literal and length
 * 0. Used for empty strings and typically returned by search functions when an
 * item is not found.
 */
#define MU_STRING_EMPTY                                                        \
    (mu_string_t) { .buf = "", .len = 0 }

/**
 * @brief Represents an empty mutable string (zero capacity).
 *
 * Defined as a mu_string_mut_t with a NULL pointer and length 0.
 */
#define MU_STRING_MUT_EMPTY                                                    \
    (mu_string_mut_t) { .buf = NULL, .len = 0 }

/**
 * @brief Special sentinal value indicating string not found.
 */
#define MU_STRING_NOT_FOUND                                                    \
    (mu_string_t) { .buf = NULL, .len = 0 }

/**
 * @brief Special sentinel value indicating an invalid string view result.
 *
 * Typically returned by functions when input parameters are invalid (e.g.,
 * NULL buffer with non-zero length) or the operation cannot produce a
 * valid string view.
 * Defined as a mu_string_t with a NULL buffer and SIZE_MAX length.
 */
#define MU_STRING_INVALID                                                      \
    (mu_string_t) { .buf = NULL, .len = SIZE_MAX }

/**
 * @brief Special index value for string slicing functions.
 *
 * Used to indicate the end of the string. Equivalent to INT_MAX, which
 * is treated as the string's length after clamping in slice operations.
 */
#define MU_STRING_END INT_MAX

/**
 * @brief Predicate function type used by find/trim/split functions.
 *
 * @param ch The character being tested.
 * @param arg An optional user-provided argument.
 * @return true if the character satisfies the predicate, false otherwise.
 */
typedef bool (*mu_string_pred_t)(char ch, void *arg);

// *****************************************************************************
// Public function prototypes

/**
 * @brief Checks if a string view is valid.
 *
 * A string view is considered valid if its buffer pointer is not NULL
 * or if its length is 0. This distinguishes valid strings (including empty)
 * from invalid states like MU_STRING_INVALID or {NULL, non-zero_len}.
 *
 * @param s The string view.
 * @return true if the string view is valid, false otherwise.
 */
bool mu_string_is_valid(mu_string_t s);

/**
 * @brief Creates a read-only string view from a null-terminated C string.
 *
 * The resulting view's buffer points to the original C string data, and its
 * length is calculated using strlen.
 *
 * @param cstr The null-terminated C string.
 * @return A mu_string_t view of the C string, or MU_STRING_EMPTY if cstr is
 * NULL.
 */
mu_string_t mu_string_from_cstr(const char *cstr);

/**
 * @brief Creates a read-only string view from a buffer and length.
 *
 * Creates a mu_string_t view pointing to the specified buffer with the
 * specified length. Assumes the buffer is valid for the given length.
 *
 * @param buf The buffer containing the string data.
 * @param len The number of characters in the string.
 * @return A mu_string_t view. Returns MU_STRING_EMPTY if buf is NULL and len is
 * 0. Returns MU_STRING_INVALID if buf is NULL and len > 0.
 */
mu_string_t mu_string_from_buf(const char *buf, size_t len);

/**
 * @brief Creates a mutable string view from a buffer and capacity.
 *
 * Creates a mu_string_mut_t view pointing to the specified buffer with the
 * specified capacity. Assumes the buffer is valid and writable for the given
 * capacity.
 *
 * @param buf The mutable buffer.
 * @param len The capacity of the buffer.
 * @return A mu_string_mut_t view. Returns MU_STRING_MUT_EMPTY if buf is NULL
 * and len is 0. Returns a view with NULL buffer and specified len if buf is
 * NULL and len > 0, representing an invalid mutable view state.
 */
mu_string_mut_t mu_string_mut_from_buf(char *buf, size_t len);

/**
 * @brief Gets the length of a string view.
 *
 * @param s The string view.
 * @return The length of the string, or SIZE_MAX if the string view is invalid
 * (e.g. MU_STRING_INVALID).
 */
size_t mu_string_len(mu_string_t s);

/**
 * @brief Checks if a string view is empty.
 *
 * A string is considered empty if its length is 0. MU_STRING_EMPTY is empty.
 * MU_STRING_INVALID is NOT considered empty.
 *
 * @param s The string view.
 * @return true if the string is empty, false otherwise.
 */
bool mu_string_is_empty(mu_string_t s);

/**
 * @brief Gets the buffer pointer of a string view.
 *
 * @param s The string view.
 * @return The buffer pointer, or NULL if the string is empty or invalid.
 */
const char *mu_string_buf(mu_string_t s);

/**
 * @brief Gets the buffer pointer of a mutable string view.
 *
 * @param s_mut The mutable string view.
 * @return The buffer pointer. Returns NULL if the view has a NULL buffer.
 */
char *mu_string_mut_buf(mu_string_mut_t s_mut);

/**
 * @brief Gets the capacity (length) of a mutable string view.
 *
 * @param s_mut The mutable string view.
 * @return The capacity of the view.
 */
size_t mu_string_mut_len(mu_string_mut_t s_mut);

/**
 * @brief Checks if two string views are equal (same length and content).
 *
 * Returns true if both are MU_STRING_INVALID. Returns false if one is
 * MU_STRING_INVALID and the other is not. Otherwise, compares lengths
 * and content.
 *
 * @param s1 The first string view.
 * @param s2 The second string view.
 * @return true if the strings are equal, false otherwise.
 */
bool mu_string_eq(mu_string_t s1, mu_string_t s2);

/**
 * @brief Compares two string views lexicographically.
 *
 * Handles MU_STRING_EMPTY and MU_STRING_INVALID. MU_STRING_INVALID is
 * considered less than any valid string view (including empty).
 *
 * @param s1 The first string view.
 * @param s2 The second string view.
 * @return An integer less than, equal to, or greater than zero if s1 is found,
 * respectively, to be less than, to match, or be greater than s2.
 */
int mu_string_cmp(mu_string_t s1, mu_string_t s2);

/**
 * @brief Checks if a string view starts with another string view.
 *
 * Returns false if s or prefix is MU_STRING_INVALID, or if prefix is longer
 * than s. Handles MU_STRING_EMPTY inputs.
 *
 * @param s The string view to check.
 * @param prefix The prefix string view.
 * @return true if s starts with prefix, false otherwise.
 */
bool mu_string_starts_with(mu_string_t s, mu_string_t prefix);

/**
 * @brief Checks if a string view ends with another string view.
 *
 * Returns false if s or suffix is MU_STRING_INVALID, or if suffix is longer
 * than s. Handles MU_STRING_EMPTY inputs.
 *
 * @param s The string view to check.
 * @param suffix The suffix string view.
 * @return true if s ends with suffix, false otherwise.
 */
bool mu_string_ends_with(mu_string_t s, mu_string_t suffix);

/**
 * @brief Finds the first occurrence of a character in a string view.
 *
 * @param s The string view to search in.
 * @param c The character to find.
 * @return A mu_string_t view starting from the first occurrence of the
 * character to the end of the string, or MU_STRING_EMPTY if the character is
 * not found or if s is empty. Returns MU_STRING_INVALID if s is
 * MU_STRING_INVALID.
 */
mu_string_t mu_string_find_char(mu_string_t s, char c);

/**
 * @brief Finds the last occurrence of a character in a string view.
 *
 * @param s The string view to search in.
 * @param c The character to find.
 * @return A mu_string_t view starting from the last occurrence of the character
 * to the end of the string, or MU_STRING_EMPTY if the character is not found
 * or if s is empty. Returns MU_STRING_INVALID if s is MU_STRING_INVALID.
 */
mu_string_t mu_string_rfind_char(mu_string_t s, char c);

/**
 * @brief Finds the first character in a string view that matches a predicate.
 *
 * @param s The string view to search in.
 * @param pred The predicate function to test characters.
 * @param arg An optional argument passed to the predicate function.
 * @return A mu_string_t view starting from the first character matching the
 * predicate to the end of the string. Returns MU_STRING_EMPTY if no character
 * matches or if s is empty or pred is NULL. Returns MU_STRING_INVALID if s is
 * MU_STRING_INVALID.
 */
mu_string_t mu_string_find_pred(mu_string_t s, mu_string_pred_t pred,
                                void *arg);

/**
 * @brief Finds the last character in a string view that matches a predicate.
 *
 * @param s The string view to search in.
 * @param pred The predicate function to test characters.
 * @param arg An optional argument passed to the predicate function.
 * @return A mu_string_t view starting from the last character matching the
 * predicate to the end of the string. Returns MU_STRING_EMPTY if no character
 * matches or if s is empty or pred is NULL. Returns MU_STRING_INVALID if s is
 * MU_STRING_INVALID.
 */
mu_string_t mu_string_rfind_pred(mu_string_t s, mu_string_pred_t pred,
                                 void *arg);

/**
 * @brief Finds the first character in a string view that does NOT match a
 * predicate.
 *
 * Useful for skipping leading characters that match a condition (e.g.,
 * trimming).
 *
 * @param s The string view to search in.
 * @param pred The predicate function to test characters.
 * @param arg An optional argument passed to the predicate function.
 * @return A mu_string_t view starting from the first character NOT matching the
 * predicate to the end of the string. Returns MU_STRING_EMPTY if all characters
 * match or if s is empty. If pred is NULL, returns s. Returns MU_STRING_INVALID
 * if s is MU_STRING_INVALID.
 */
mu_string_t mu_string_find_first_not_pred(mu_string_t s, mu_string_pred_t pred,
                                          void *arg);

/**
 * @brief Finds the first occurrence of a substring (needle) within a string
 * (haystack).
 *
 * @param haystack The string view to search within.
 * @param needle The substring view to search for.
 * @return A mu_string_t view starting from the first occurrence of the needle
 * to the end of the haystack. Returns MU_STRING_EMPTY if the needle is
 * not found, if haystack is empty, or if needle is longer than haystack
 * (and not empty). Returns MU_STRING_INVALID if haystack or needle is
 * MU_STRING_INVALID. If needle is empty, returns haystack.
 */
mu_string_t mu_string_find_str(mu_string_t haystack, mu_string_t needle);

/**
 * @brief Creates a slice (substring view) of a string view.
 *
 * Indices can be positive (0-based from start) or negative (0-based from end).
 * Negative indices are clamped: index = (len + index). Final indices are
 * clamped to [0, s.len]. If start >= end after clamping, an empty string is
 * returned. MU_STRING_END can be used for the 'end' parameter to represent the
 * end of the string.
 *
 * @param s The string view to slice.
 * @param start The starting index (inclusive). Can be negative or positive.
 * MU_STRING_END is not typically used for start.
 * @param end The ending index (exclusive). Can be negative or positive.
 * MU_STRING_END can be used to include the rest of the string.
 * @return A mu_string_t view representing the slice, or MU_STRING_EMPTY for an
 * empty slice or if s is empty. Returns MU_STRING_INVALID if s is
 * MU_STRING_INVALID.
 */
mu_string_t mu_string_slice(mu_string_t s, int start, int end);

/**
 * @brief Trims leading characters from a string view based on a predicate.
 *
 * Returns a view starting from the first character that does NOT match the
 * predicate.
 *
 * @param s The string view to trim.
 * @param pred The predicate function to identify characters to trim.
 * @param arg An optional argument passed to the predicate function.
 * @return A mu_string_t view with leading characters matching the predicate
 * removed. Returns the original string if no leading characters match, or if
 * pred is NULL. Returns MU_STRING_EMPTY if the entire string matches or s is
 * empty. Returns MU_STRING_INVALID if s is MU_STRING_INVALID.
 */
mu_string_t mu_string_ltrim(mu_string_t s, mu_string_pred_t pred, void *arg);

/**
 * @brief Trims trailing characters from a string view based on a predicate.
 *
 * Returns a view ending before the last character that does NOT match the
 * predicate.
 *
 * @param s The string view to trim.
 * @param pred The predicate function to identify characters to trim.
 * @param arg An optional argument passed to the predicate function.
 * @return A mu_string_t view with trailing characters matching the predicate
 * removed. Returns the original string if no trailing characters match, or if
 * pred is NULL. Returns MU_STRING_EMPTY if the entire string matches or s is
 * empty. Returns MU_STRING_INVALID if s is MU_STRING_INVALID.
 */
mu_string_t mu_string_rtrim(mu_string_t s, mu_string_pred_t pred, void *arg);

/**
 * @brief Trims leading and trailing characters from a string view based on a
 * predicate.
 *
 * Combines the behavior of mu_string_ltrim and mu_string_rtrim.
 *
 * @param s The string view to trim.
 * @param pred The predicate function to identify characters to trim.
 * @param arg An optional argument passed to the predicate function.
 * @return A mu_string_t view with leading and trailing characters matching the
 * predicate removed. Returns the original string if no leading/trailing
 * characters match, or if pred is NULL. Returns MU_STRING_EMPTY if the entire
 * string matches or s is empty. Returns MU_STRING_INVALID if s is
 * MU_STRING_INVALID.
 */
mu_string_t mu_string_trim(mu_string_t s, mu_string_pred_t pred, void *arg);

/**
 * @brief Splits a string view into two parts at the first occurrence of a
 * character.
 *
 * Finds the first occurrence of `delimiter` in the string `s`.
 *
 * - If `s` is `MU_STRING_INVALID`, the function returns `MU_STRING_INVALID`.
 * If `after` is non-NULL, `*after` is also set to `MU_STRING_INVALID`.
 * - If `delimiter` is found in `s`, the function returns a `mu_string_t` view
 * representing the part of `s` *before* the delimiter (which may be empty if
 * the delimiter is the first character). If `after` is non-NULL, `*after` is
 * set to a `mu_string_t` view of the part of `s` starting *at* the first
 * occurrence of the `delimiter` and going to the end of `s` (including the
 * delimiter).
 * - If `delimiter` is not found in `s` (and `s` is valid), the function
 * returns `MU_STRING_NOT_FOUND`. If `after` is non-NULL, `*after` is
 * also set to `MU_STRING_NOT_FOUND`.
 *
 * Note: If the `after` pointer is NULL, the 'after' part is not returned
 * or modified. An empty string (`MU_STRING_EMPTY`) is a valid string view
 * and searching in it will result in `MU_STRING_NOT_FOUND`.
 *
 * @param s The string view to split. Must be a valid `mu_string_t`.
 * @param after Optional pointer to a `mu_string_t` to store the part after
 * the delimiter (if found), or `MU_STRING_NOT_FOUND` (if not found),
 * or `MU_STRING_INVALID` (if s is invalid). Can be NULL.
 * @param delimiter The character to split by.
 * @return A `mu_string_t` view representing the part of `s` before the
 * delimiter (if found), or `MU_STRING_NOT_FOUND` (if not found),
 * or `MU_STRING_INVALID` (if s is invalid).
 */
mu_string_t mu_string_split_at_char(mu_string_t s, mu_string_t *after,
                                    char delimiter);

/**
 * @brief Splits a string at the first character satisfying `pred`.
 *
 * Returns the slice of `s` up to (but not including) the first character
 * for which `pred(ch, arg)` is true.  Updates `*after` to the remainder
 * of the string after that character.  If no character satisfies `pred`,
 * returns the entire input as the token and sets `*after` to the empty slice.
 *
 * @param s      Input string slice. Must be valid.
 * @param after  Optional out‐parameter to receive the remainder; may be NULL.
 * @param pred   Predicate function to identify the split point. Must not be NULL.
 * @param arg    Optional argument passed through to `pred`.
 * @return       Slice before the first matching character, or entire `s` if no match.
 */
mu_string_t mu_string_split_by_pred(mu_string_t s,
                                    mu_string_t *after,
                                    mu_string_pred_t pred,
                                    void *arg);

/**
 * @brief Splits a string view into two parts at the first character that does
 * NOT match a predicate.
 *
 * Finds the first character in `s` that does NOT match the `pred` predicate.
 *
 * - If `s` is `MU_STRING_INVALID` or `pred` is NULL, the function returns
 * `MU_STRING_INVALID`. If `after` is non-NULL, `*after` is also set to
 * `MU_STRING_INVALID`.
 * - If a character in `s` at index `idx` is the first that does NOT match the
 * predicate, the function returns a `mu_string_t` view representing the part
 * of `s` *before* this character (`{s.buf, idx}`). If `after` is non-NULL,
 * `*after` is set to a `mu_string_t` view of the part of `s` starting *at* this
 * character and going to the end of `s` (including the non-matching character).
 * - If all characters in `s` match the predicate (and `s` is valid and `pred`
 * is non-NULL), the function returns `MU_STRING_NOT_FOUND`. If `after` is
 * non-NULL, `*after` is also set to `MU_STRING_NOT_FOUND`.
 *
 * Note: If the `after` pointer is NULL, the 'after' part is not returned
 * or modified. An empty string (`MU_STRING_EMPTY`) is a valid string view.
 *
 * @param s The string view to split. Must be a valid `mu_string_t`.
 * @param after Optional pointer to a `mu_string_t` to store the part from
 * the first non-matching character onwards (if found), or
 * `MU_STRING_NOT_FOUND` (if all characters match), or `MU_STRING_INVALID`
 * (if s is invalid or pred is NULL). Can be NULL.
 * @param pred The predicate function. The split occurs at the first character
 * for which this predicate returns false. Must be non-NULL.
 * @param arg An optional argument passed to the predicate function.
 * @return A `mu_string_t` view representing the part of `s` before the
 * first non-matching character (if found), or `MU_STRING_NOT_FOUND` (if
 * all characters match), or `MU_STRING_INVALID` (if s is invalid or
 * pred is NULL).
 */
mu_string_t mu_string_split_by_not_pred(mu_string_t s, mu_string_t *after,
                                        mu_string_pred_t pred, void *arg);

/**
 * @brief Copies content from a read-only string view into a mutable string
 * view.
 *
 * Copies up to `dst.len` bytes from `src.buf` to `dst.buf`. The number of bytes
 * copied is the minimum of `src.len` and `dst.len`.
 *
 * @param dst The mutable string view (destination buffer and capacity). Must
 * have a non-NULL buffer if dst.len > 0.
 * @param src The read-only string view (source data and length). Must be valid.
 * @return A read-only mu_string_t view representing the data that was
 * successfully copied into the destination buffer (starting at dst.buf with the
 * number of bytes actually copied). Returns MU_STRING_EMPTY if dst.len is 0 or
 * src is empty. Returns MU_STRING_INVALID if dst.buf is NULL (regardless of
 * dst.len), or if src is MU_STRING_INVALID.
 */
mu_string_t mu_string_copy(mu_string_mut_t dst, mu_string_t src);

/**
 * @brief Appends content from a read-only string view to a mutable string view.
 *
 * Copies up to `dst_segment.len` bytes from `src.buf` to `dst_segment.buf`. The
 * number of bytes copied is the minimum of `src.len` and `dst_segment.len`.
 * This function is designed to be used in a cursor-style pattern for building
 * strings in a fixed-size buffer.
 *
 * @param dst_segment The mutable string view representing the current available
 * space in the destination buffer (buffer pointer and remaining capacity). Must
 * have a non-NULL buffer if dst_segment.len > 0.
 * @param src The read-only string view (source data and length) to append. Must
 * be valid.
 * @return A mutable mu_string_mut_t view representing the *remaining* space in
 * the destination buffer after the append operation. Returns dst_segment
 * unchanged if dst_segment.buf is NULL, src is invalid/empty, or
 * dst_segment.len was initially 0. Returns MU_STRING_MUT_EMPTY if the entire
 * destination buffer is consumed.
 */
mu_string_mut_t mu_string_append(mu_string_mut_t dst_segment, mu_string_t src);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif // MU_STRING_H