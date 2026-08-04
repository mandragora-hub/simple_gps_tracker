#ifndef STRING_UTILS_H
#define STRING_UTILS_H

char *trim_string(char *src);
char *strip_string(char *src, char c);
char *extract_between(const char *src, const char *start_str, const char *end_str, char *dest, size_t dest_size);
char *extract_middle_lines(char *src);

#endif // STRING_UTILS_H

