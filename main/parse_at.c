#include <stddef.h>
#include <string.h>
#include "parse_at.h"

bool parse_at_command_response(const char *response, const char *prefix, const char* delimiters, at_field_handler_t handler, void *user_ctx) {
	if (response == NULL || prefix == NULL || handler == NULL || user_ctx == NULL) return false;

	char *str = strstr(response, prefix);
	if (str == NULL) return false;

	char *payload = str + strlen(prefix);
	char *newline = strpbrk(payload, "\r\n");
	if (newline != NULL) *newline = '\0';

	char *saveptr = NULL;
	char *token = NULL;
	int field_idx = 0;
	const char *separator_str = (delimiters == NULL) ? "," : delimiters;

	token = strtok_r(payload, separator_str, &saveptr);
	while (token != NULL) {
		// Trim leading spaces if any
		while (*token == ' ') token++;
    
		handler(field_idx, token, user_ctx);

		token = strtok_r(NULL, separator_str, &saveptr);
		field_idx++;
	}

	return (field_idx > 0);
}


