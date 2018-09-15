//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_OBJECT_H
#define DARKNET_BINDINGS_OBJECT_H

#include "assert.h"
#include "fail.h"

#define assertObjectMustHave(object, name) { assert(object.Has(name), std::string(name) + " is not defined"); }

#define assertObjectValueMustBeString(object, name) { \
	assertObjectMustHave(object, name); \
	assert(object.Get(name).IsString(), std::string(name) + " must be a string"); \
}

#define assertObjectValueMustBeArray(object, name) { \
	assertObjectMustHave(object, name); \
	assert(object.Get(name).IsObject(), std::string(name) + " must be an array"); \
}

#define objectGetString(target, object, name) { \
	assertObjectValueMustBeString(object, name); \
	target = object.Get(name).ToString().Utf8Value(); \
}

#define objectGetArray(target, object, name) { \
	assertObjectValueMustBeArray(object, name); \
	try { \
		target = object.Get(name).As<Napi::Array>(); \
  } catch (auto e) { \
		fail(std::string(name) + " must be an array") \
	} \
}

#define unpackStringArray(target, array) { \
  uint32_t len = array.Length(); \
	for(uint32_t i = 0; i < len; i ++) { \
		target.push_back(array.Get(i).ToString().Utf8Value());\
	} \
}

#define assertValueTypeObject(name, value) { assert(value.IsObject(), std::string(name) + " is not of type Object"); }

#endif //DARKNET_BINDINGS_OBJECT_H
