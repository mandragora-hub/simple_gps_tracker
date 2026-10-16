#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include "modem.h"

void captive_portal_start(modem_ctx_t *modem, const char *web_page_mount_point_in_fs);
void captive_portal_stop();

#endif // CAPTIVE_PORTAL_H

