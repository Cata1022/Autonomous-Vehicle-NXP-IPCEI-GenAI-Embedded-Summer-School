/*
 * Prefer the Config Tools generated RTE_Device.h over the SDK template.
 * The SDK template uses a different include guard, so define both guards
 * after loading the project RTE configuration.
 */
#ifndef RTE_DEVICE_OVERRIDE_H_
#define RTE_DEVICE_OVERRIDE_H_

#include "frdmmcxa153/board/RTE_Device.h"

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H
#endif

#endif /* RTE_DEVICE_OVERRIDE_H_ */
