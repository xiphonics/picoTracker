/*
** Stub sinc helpers for builds that exclude the sinc converters.
*/

#include "common.h"

const char *sinc_get_name(int src_enum) {
  (void)src_enum;
  return NULL;
}

const char *sinc_get_description(int src_enum) {
  (void)src_enum;
  return NULL;
}
