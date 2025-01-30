#ifndef _POWER_BUTTON_H_
#define _POWER_BUTTON_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void handlePowerButton();
void powerbuttonirq(uint gpio, uint32_t event_mask);

#ifdef __cplusplus
}
#endif

#endif