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
 * @file mu_string.c
 *
 * @brief Implements the mu_string library functions.
 */

// *****************************************************************************
// Includes

#include "mu_string.h"
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef INT_MAX
#error INT_MAX is not defined
#endif

#ifndef SIZE_MAX
#error SIZE_MAX is not defined
#endif

// *****************************************************************************
// Private types and definitions

// Note: mu_string_is_valid is now public.

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

/**
 * @brief Helper for splitting a string slice at a given index.
 *
 * If `found_idx < s.len`, returns the "before" slice [0..found_idx)
 * and sets `*after` to the remainder starting at `found_idx`, length
 * `s.len - found_idx`.  If `found_idx == s.len`, returns the entire
 * input `s` and sets `*after` to the empty slice at the end of `s`.
 *
 * @param s          The original string slice. Must be valid.
 * @param after      Optional out‐parameter to receive the remainder; may be NULL.
 * @param found_idx  Index at which to split (0..s.len).
 * @return           The "before" slice.
 */
static mu_string_t mu_string_split_handle_result(mu_string_t s,
                                                 mu_string_t *after,
                                                 size_t found_idx);

// *****************************************************************************
// Public code

bool mu_string_is_valid(mu_string_t s) {
    // A string is valid if its buffer is non-NULL, OR if its length is 0.
    // This excludes {NULL, non-zero_len} including the INVALID sentinel.
    return (s.buf != NULL) || (s.len == 0);
}


mu_string_t mu_string_from_cstr(const char* cstr) {
    if (cstr == NULL) {
        return MU_STRING_EMPTY;
    }
    return (mu_string_t){ .buf = cstr, .len = strlen(cstr) };
}

mu_string_t mu_string_from_buf(const char* buf, size_t len) {
    if (buf == NULL && len > 0) {
        return MU_STRING_INVALID;
    }
    // As per header, if buf is NULL and len is 0, return MU_STRING_EMPTY.
    if (buf == NULL && len == 0) {
        return MU_STRING_EMPTY;
    }
    return (mu_string_t){ .buf = buf, .len = len };
}

mu_string_mut_t mu_string_mut_from_buf(char* buf, size_t len) {
    // As per test expectation, return {NULL, len} if buf is NULL and len > 0.
    // This represents an invalid mutable view state.
    return (mu_string_mut_t){ .buf = buf, .len = len };
}


size_t mu_string_len(mu_string_t s) {
    if (!mu_string_is_valid(s)) {
        return SIZE_MAX; // Return SIZE_MAX for invalid strings
    }
    return s.len;
}

bool mu_string_is_empty(mu_string_t s) {
    if (!mu_string_is_valid(s)) {
        return false; // Invalid is not empty
    }
    return s.len == 0;
}


const char* mu_string_buf(mu_string_t s) {
    // Header says NULL if empty or invalid.
    if (!mu_string_is_valid(s) || s.len == 0) {
        // For MU_STRING_EMPTY ({ "", 0 }), buf is not NULL, but returning NULL is consistent
        // with the header comment and test expectations for empty strings.
        return NULL;
    }
    return s.buf;
}

char* mu_string_mut_buf(mu_string_mut_t s_mut) {
    return s_mut.buf;
}

size_t mu_string_mut_len(mu_string_mut_t s_mut) {
    return s_mut.len;
}


bool mu_string_eq(mu_string_t s1, mu_string_t s2) {
    bool s1_is_valid = mu_string_is_valid(s1);
    bool s2_is_valid = mu_string_is_valid(s2);

    if (!s1_is_valid || !s2_is_valid) {
        return !s1_is_valid == !s2_is_valid; // Both must be invalid to be equal
    }

    if (s1.len != s2.len) {
        return false;
    }
    if (s1.len == 0) {
        return true; // Both empty and valid
    }
    // Valid, non-empty, same length - compare content
    return memcmp(s1.buf, s2.buf, s1.len) == 0;
}

int mu_string_cmp(mu_string_t s1, mu_string_t s2) {
    bool s1_is_valid = mu_string_is_valid(s1);
    bool s2_is_valid = mu_string_is_valid(s2);

    if (!s1_is_valid && !s2_is_valid) return 0; // Invalid == Invalid
    if (!s1_is_valid) return -1; // Invalid < valid
    if (!s2_is_valid) return 1; // Valid > invalid

    // Handle comparison with empty strings explicitly for clarity, though memcmp/length check covers it
    if (s1.len == 0 && s2.len == 0) return 0;
    if (s1.len == 0) return -1; // Empty < non-empty
    if (s2.len == 0) return 1; // Non-empty > empty


    size_t min_len = (s1.len < s2.len) ? s1.len : s2.len;
    int cmp_result = memcmp(s1.buf, s2.buf, min_len);

    if (cmp_result != 0) {
        return cmp_result;
    }
    // Content is equal up to min_len, compare lengths
    if (s1.len < s2.len) return -1; // shorter is less
    if (s1.len > s2.len) return 1; // longer is greater
    return 0; // lengths are equal, and content was equal
}


bool mu_string_starts_with(mu_string_t s, mu_string_t prefix) {
    if (!mu_string_is_valid(s) || !mu_string_is_valid(prefix)) {
        return false; // Invalid inputs
    }
    if (prefix.len == 0) {
        return true; // Any string starts with an empty string
    }
    if (prefix.len > s.len) {
        return false; // Prefix longer than string
    }
     // Handle empty string case explicitly although len check covers it
    if (s.len == 0) return false; // Non-empty prefix cannot start an empty string


    return memcmp(s.buf, prefix.buf, prefix.len) == 0;
}

bool mu_string_ends_with(mu_string_t s, mu_string_t suffix) {
    if (!mu_string_is_valid(s) || !mu_string_is_valid(suffix)) {
        return false; // Invalid inputs
    }
    if (suffix.len == 0) {
        return true; // Any string ends with an empty string
    }
    if (suffix.len > s.len) {
        return false; // Suffix longer than string
    }
     // Handle empty string case explicitly although len check covers it
    if (s.len == 0) return false; // Non-empty suffix cannot end an empty string

    return memcmp(s.buf + s.len - suffix.len, suffix.buf, suffix.len) == 0;
}


mu_string_t mu_string_find_char(mu_string_t s, char c) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0) return MU_STRING_EMPTY;

    for (size_t i = 0; i < s.len; ++i) {
        if (s.buf[i] == c) {
            // Return view from found character to the end
            return (mu_string_t){ .buf = s.buf + i, .len = s.len - i };
        }
    }
    return MU_STRING_EMPTY; // Not found
}

mu_string_t mu_string_rfind_char(mu_string_t s, char c) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0) return MU_STRING_EMPTY;

    // Iterate backwards using signed int index to handle i >= 0 condition easily
    for (int i = (int)s.len - 1; i >= 0; --i) {
        if (s.buf[i] == c) {
            // Return view from found character to the end
            return (mu_string_t){ .buf = s.buf + i, .len = s.len - (size_t)i };
        }
    }
    return MU_STRING_EMPTY; // Not found
}

mu_string_t mu_string_find_pred(mu_string_t s, mu_string_pred_t pred, void* arg) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0 || pred == NULL) return MU_STRING_EMPTY;

    for (size_t i = 0; i < s.len; ++i) {
        if (pred(s.buf[i], arg)) {
            // Return view from first matching character to the end
            return (mu_string_t){ .buf = s.buf + i, .len = s.len - i };
        }
    }
    return MU_STRING_EMPTY; // Not found
}

mu_string_t mu_string_rfind_pred(mu_string_t s, mu_string_pred_t pred, void* arg) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0 || pred == NULL) return MU_STRING_EMPTY;

    // Iterate backwards
    for (int i = (int)s.len - 1; i >= 0; --i) {
        if (pred(s.buf[i], arg)) {
            // Return view from last matching character to the end
            return (mu_string_t){ .buf = s.buf + i, .len = s.len - (size_t)i };
        }
    }
    return MU_STRING_EMPTY; // Not found
}

mu_string_t mu_string_find_first_not_pred(mu_string_t s, mu_string_pred_t pred, void* arg) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0) return MU_STRING_EMPTY;
    if (pred == NULL) {
         // If predicate is NULL, !pred is always true.
         // The first character will always satisfy !pred.
         // Return view from the first character to the end.
         return (mu_string_t){ .buf = s.buf, .len = s.len }; // Which is the original string s
    }

    size_t start_idx = 0;
    while (start_idx < s.len && pred(s.buf[start_idx], arg)) {
        start_idx++;
    }

    // If all characters matched the predicate, start_idx will be s.len
    if (start_idx == s.len) {
        return MU_STRING_EMPTY;
    }

    // Return view from the first non-matching character to the end
    return (mu_string_t){ .buf = s.buf + start_idx, .len = s.len - start_idx };
}


mu_string_t mu_string_find_str(mu_string_t haystack, mu_string_t needle) {
    if (!mu_string_is_valid(haystack) || !mu_string_is_valid(needle)) return MU_STRING_INVALID;
    if (needle.len == 0) return haystack; // Empty needle is found at the start
    if (needle.len > haystack.len) return MU_STRING_EMPTY; // Needle longer than haystack

    // Simple brute-force search
    for (size_t i = 0; i <= haystack.len - needle.len; ++i) {
        if (memcmp(haystack.buf + i, needle.buf, needle.len) == 0) {
            // Return view from the start of the match to the end of the haystack
            return (mu_string_t){ .buf = haystack.buf + i, .len = haystack.len - i };
        }
    }

    return MU_STRING_EMPTY; // Not found
}


mu_string_t mu_string_slice(mu_string_t s, int start, int end) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0) return MU_STRING_EMPTY;

    // Clamp and adjust start index
    long long clamped_start_ll = (long long)start;
    if (clamped_start_ll < 0) {
        clamped_start_ll = (long long)s.len + clamped_start_ll;
        if (clamped_start_ll < 0) clamped_start_ll = 0; // Clamp to 0 if still negative
    }
    size_t clamped_start = (size_t)clamped_start_ll;
    if (clamped_start > s.len) clamped_start = s.len; // Clamp to len

    // Clamp and adjust end index
    long long clamped_end_ll = (long long)end;
    if (clamped_end_ll < 0) {
         clamped_end_ll = (long long)s.len + clamped_end_ll;
         if (clamped_end_ll < 0) clamped_end_ll = 0; // Clamp to 0 if still negative
    }
    size_t clamped_end = (size_t)clamped_end_ll;
    if (clamped_end > s.len) clamped_end = s.len; // Clamp to len

    // Ensure start is not after end after clamping
    if (clamped_start >= clamped_end) {
        return MU_STRING_EMPTY;
    }

    // Valid slice
    return (mu_string_t){ .buf = s.buf + clamped_start, .len = clamped_end - clamped_start };
}


mu_string_t mu_string_ltrim(mu_string_t s, mu_string_pred_t pred, void* arg) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0 || pred == NULL) return s;

    size_t start_idx = 0;
    while (start_idx < s.len && pred(s.buf[start_idx], arg)) {
        start_idx++;
    }

    // If all characters matched, start_idx will be s.len
    if (start_idx == s.len) {
        return MU_STRING_EMPTY;
    }

    // Return view from the first non-matching character
    return (mu_string_t){ .buf = s.buf + start_idx, .len = s.len - start_idx };
}

mu_string_t mu_string_rtrim(mu_string_t s, mu_string_pred_t pred, void* arg) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0 || pred == NULL) return s;

    int end_idx = (int)s.len - 1;
    while (end_idx >= 0 && pred(s.buf[end_idx], arg)) {
        end_idx--;
    }

    // If all characters matched, end_idx will be -1
    if (end_idx < 0) {
        return MU_STRING_EMPTY;
    }

    // Return view from the start up to and including the last non-matching character
    return (mu_string_t){ .buf = s.buf, .len = (size_t)end_idx + 1 };
}

mu_string_t mu_string_trim(mu_string_t s, mu_string_pred_t pred, void* arg) {
    if (!mu_string_is_valid(s)) return MU_STRING_INVALID;
    if (s.len == 0 || pred == NULL) return s;

    size_t start_idx = 0;
    while (start_idx < s.len && pred(s.buf[start_idx], arg)) {
        start_idx++;
    }

    // If all characters matched, start_idx will be s.len, return empty
    if (start_idx == s.len) {
        return MU_STRING_EMPTY;
    }

    int end_idx = (int)s.len - 1;
    while (end_idx >= (int)start_idx && pred(s.buf[end_idx], arg)) {
        end_idx--;
    }

    // Return view from the first non-matching char to the last non-matching char
    return (mu_string_t){ .buf = s.buf + start_idx, .len = (size_t)end_idx - start_idx + 1 };
}


mu_string_t mu_string_split_at_char(mu_string_t s, mu_string_t* after, char delimiter) {
    if (!mu_string_is_valid(s)) {
        // Case 1: Invalid input string 's'
        if (after) {
            *after = MU_STRING_INVALID;
        }
        return MU_STRING_INVALID;
    }

    // Search for the delimiter
    // If s.len == 0, this loop doesn't run, found_idx remains s.len (0).
    // This correctly leads to the "not found" case below.
    size_t found_idx = s.len; // Initialize to s.len to indicate not found
    for (size_t i = 0; i < s.len; ++i) {
        if (s.buf[i] == delimiter) {
            found_idx = i;
            break;
        }
    }

    if (found_idx != s.len) {
        // Case 2: Delimiter found
        mu_string_t before_part = { .buf = &s.buf[0], .len = found_idx };
        if (after) {
            // After part includes the delimiter and goes to the end
            *after = (mu_string_t){ .buf = &s.buf[found_idx], .len = s.len - found_idx };
        }
        return before_part;

    } else {
        // Case 3: Delimiter not found (s is valid and the loop finished)
        if (after) {
            // After part is MU_STRING_NOT_FOUND if delimiter is not found
            *after = MU_STRING_NOT_FOUND;
        }
        return s; // Return entire string when not found
    }
}

mu_string_t mu_string_split_by_pred(mu_string_t s,
                                    mu_string_t *after,
                                    mu_string_pred_t pred,
                                    void *arg)
{
    // Invalid input?
    if (!mu_string_is_valid(s) || pred == NULL) {
        if (after) {
            *after = MU_STRING_INVALID;
        }
        return MU_STRING_INVALID;
    }

    // Find index of first character satisfying the predicate
    size_t split_idx = s.len;
    for (size_t i = 0; i < s.len; ++i) {
        if (pred(s.buf[i], arg)) {
            split_idx = i;
            break;
        }
    }

    // If no match, return whole input and make `after` the empty slice
    if (split_idx == s.len) {
        if (after) {
            // Remainder starts at end of s, length zero
            after->buf = s.buf + s.len;
            after->len = 0;
        }
        return s;
    }

    // Otherwise delegate to the existing split handler
    mu_string_t xxx = mu_string_split_handle_result(s, after, split_idx);
    return xxx;
}

mu_string_t mu_string_split_by_not_pred(mu_string_t s, mu_string_t* after, mu_string_pred_t pred, void* arg) {
    // Handle invalid input (s invalid or pred NULL)
    if (!mu_string_is_valid(s) || pred == NULL) {
        if (after) {
            *after = MU_STRING_INVALID;
        }
        return MU_STRING_INVALID;
    }

    // Find the first index where predicate is NOT true
    size_t found_idx = s.len; // Initialize to s.len (not found)
    for (size_t i = 0; i < s.len; ++i) {
        if (!pred(s.buf[i], arg)) {
            found_idx = i;
            break;
        }
    }

    // Handle the result based on whether the index was found
    return mu_string_split_handle_result(s, after, found_idx);
}

mu_string_t mu_string_copy(mu_string_mut_t dst, mu_string_t src) {
    // Check for invalid source or destination buffers
    // A mutable view is considered invalid if its buffer is NULL (cannot write).
    if (dst.buf == NULL || !mu_string_is_valid(src)) {
        // Return INVALID view as per test expectation
        return MU_STRING_INVALID;
    }

    // Determine actual number of bytes to copy
    size_t bytes_to_copy = (src.len < dst.len) ? src.len : dst.len;

    // Perform the copy
    if (bytes_to_copy > 0) {
        memcpy(dst.buf, src.buf, bytes_to_copy);
    }

    // Return a view of the data that was actually copied
    return (mu_string_t){ .buf = dst.buf, .len = bytes_to_copy };
}

mu_string_mut_t mu_string_append(mu_string_mut_t dst_segment, mu_string_t src) {
    // Check for invalid source or destination segment buffers
    // As per test, return the input dst_segment unchanged in case of invalid input.
    // A mutable view segment is unusable if its buffer is NULL (cannot write).
    if (dst_segment.buf == NULL || !mu_string_is_valid(src)) {
        return dst_segment;
    }
     // Also handle appending an empty string - no bytes copied, remaining space is the same
     if (src.len == 0) {
         return dst_segment;
     }


    // Determine actual number of bytes to copy (limited by src len and dst_segment len)
    size_t bytes_to_copy = (src.len < dst_segment.len) ? src.len : dst_segment.len;

    // Perform the copy
    if (bytes_to_copy > 0) {
        memcpy(dst_segment.buf, src.buf, bytes_to_copy);
    }

    // Return a view of the remaining space
    return (mu_string_mut_t){ .buf = dst_segment.buf + bytes_to_copy, .len = dst_segment.len - bytes_to_copy };
}


// *****************************************************************************
// Private (static) code

static mu_string_t mu_string_split_handle_result(mu_string_t s,
                                                 mu_string_t *after,
                                                 size_t found_idx)
{
    if (found_idx < s.len) {
        // Split point found within s
        mu_string_t before = { .buf = s.buf, .len = found_idx };
        if (after) {
            after->buf = s.buf + found_idx;
            after->len = s.len - found_idx;
        }
        return before;
    }
    // No split point: return whole s, and set after to empty slice
    if (after) {
        after->buf = s.buf + s.len;
        after->len = 0;
    }
    return s;
}

// *****************************************************************************
// End of file
