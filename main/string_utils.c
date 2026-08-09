#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "string_utils.h"

char *extract_between(const char *src, const char *start_str, const char *end_str, char *dest, size_t dest_size) {
	char *start_ptr = strstr(src, start_str);
	start_ptr = start_ptr + strlen(start_str);
	if (start_ptr == NULL) return NULL;

	char *end_ptr = strstr(src, end_str);
	if (end_ptr == NULL) return NULL;

	size_t len = (size_t)(end_ptr - start_ptr);
	if (dest_size == 0) return NULL;

	if (len >= dest_size) len = dest_size - 1; // truncate safely	
	memcpy(dest, start_ptr, len);
	dest[len] = '\0';

	return dest;
}

char *trim_string(char *src) {
	size_t start = 0, end = strlen(src) - 1;
	while(isspace(src[start])) {
		start++;
	}

	while (end > start && isspace(src[end])) {
		end--;
	}

	if (start > 0 || end < (strlen(src) - 1)) {
		memmove(src, src + start, end - start + 1);
		src[end - start + 1] = '\0';
	}

	return src;
}

char *strip_string(char *src, char c) {
	size_t start = 0, end = strlen(src) - 1;
	while (src[start] == c) start++;
	while (end > start && src[end] == c) end--;

	if (start > 0 || end < (strlen(src) - 1)) {
		memmove(src, src + start, end - start + 1);
		src[end - start + 1] = '\0';
	}
	return src;
}

char *extract_middle_lines(char *src) {
	if (src == NULL) return NULL;

	char *first_newline = strpbrk(src, "\r\n");
	if (first_newline == NULL) return NULL; // Only 1 line present

	
	if (*first_newline == '\r' && *(first_newline + 1) == '\n') {
		first_newline += 2;
	} else {
		first_newline += 1;
	}

	first_newline = strpbrk(src, "\r\n");
	if (first_newline == NULL) return NULL; 
	if (*first_newline == '\r' && *(first_newline + 1) == '\n') {
		first_newline += 2;
	} else {
		first_newline += 1;
	}


	if (*first_newline == '\0') return NULL; // No content left

	first_newline = strpbrk(src, "\r\n");
	if (first_newline == NULL) return NULL; 
	if (*first_newline == '\r' && *(first_newline + 1) == '\n') {
		first_newline += 2;
	} else {
		first_newline += 1;
	}


	if (*first_newline == '\0') return NULL; // No content left

	memmove(src, first_newline, strlen(first_newline) + 1);

	char *last_newline = strrchr(src, '\n');
	if (last_newline == NULL) {
		last_newline = strrchr(src, '\r');
	}

	if (last_newline != NULL) {
		if (last_newline > src && *(last_newline - 1) == '\r') {
			last_newline--;
		}
		*last_newline = '\0';
	}

	return src;
}

char *remove_digits(char* new_text, size_t new_text_size,	const char* text) {
	size_t use_text_size = 0;
	size_t text_size = strlen(text);
	for (int i = 0; i < text_size; i++) {
		if (use_text_size >= new_text_size - 1) break;

		uint8_t c = text[i];
		if (!isdigit(c)) {
			new_text[use_text_size] = c;
			use_text_size++;
		}
	}
	new_text[use_text_size] = '\0';
	return new_text;
}

char *to_lowercase(char* new_text, size_t new_text_size,	const char* text) {
	size_t use_text_size = 0;
	size_t text_size = strlen(text);
	for (int i = 0; i < text_size; i++) {
		if (use_text_size >= new_text_size - 1) break;

		uint8_t c = text[i];
		new_text[use_text_size] = tolower(c);
		use_text_size++;
	}
	new_text[use_text_size] = '\0';
	return new_text;
}

