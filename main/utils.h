#ifndef UTILS_H
#define UTILS_H

#include "at/gnss.h"

const char *build_osmand_traccar_url(char *dest_url, size_t dest_url_size, gnss_info_t *gnss_info);
void remaining_task_stack();
const char *bool_to_string(bool value);

#endif //UTILS_H
 
