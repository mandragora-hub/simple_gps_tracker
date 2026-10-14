#ifndef PARSE_AT_H
#define PARSE_AT_H

typedef void (*at_field_handler_t)(int field_idx, const char *token, void *user_ctx);
bool parse_at_command_response(const char *response, const char *prefix, const char* delimiters, at_field_handler_t handler, void *user_ctx);

#endif //PARSE_AT_H

