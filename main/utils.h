#ifndef UTILS_H
#define UTILS_H

#define TRACCAR_URL "https://osmand.traccar.redania.online"
#define DEVICE_ID "12345"

#include "gnss.h"

const char *build_osmand_traccar_url(char *dest_url, size_t dest_url_size, gnss_info_t *gnss_info);
void remaining_task_stack();

#endif //UTILS_H
 
