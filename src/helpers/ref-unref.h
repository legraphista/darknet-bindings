//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_REF_DERF_H
#define DARKNET_BINDINGS_REF_DERF_H

#include <napi.h>

template<typename T>
int ref_unref_size() {
	return sizeof(T *);
}

template<typename T>
char *ref_unref_to_pointer(T &item) {
	return (char *) &item;
}

template<typename T>
Napi::Buffer<char> ref_unref_to_napi_buffer(Napi::Env env, T &item) {
	char *pointer_value = ref_unref_to_pointer<T>(item);
	int pointer_size = ref_unref_size<T>();

	return Napi::Buffer<char>::New(env, pointer_value, pointer_size);
}

template<typename T>
T ref_unref_from_pointer(char *pointer) {
	return *(T *) pointer;
}

template<typename T>
T ref_unref_from_napi_buffer(Napi::Buffer<char> pointer) {
	char *data = pointer.Data();
	return ref_unref_from_pointer<T>(data);
}

#endif //DARKNET_BINDINGS_REF_DERF_H
