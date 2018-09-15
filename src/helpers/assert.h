//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_ASSERT_H
#define DARKNET_BINDINGS_ASSERT_H

#include "fail.h"

#define assert(condition, message) { if(!(condition)) { js_fail(message); } }
#define assertv(condition, message) { if(!(condition)) { js_failv(message); } }

#endif //DARKNET_BINDINGS_ASSERT_H
