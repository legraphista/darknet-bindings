//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_REF_DERF_H
#define DARKNET_BINDINGS_REF_DERF_H

template<typename T>
int ref_unref_size() {
	return sizeof(T*);
}

template<typename T>
char *ref_unref_to_pointer(T &item) {
	return (char *) &item;
}

template<typename T>
T ref_unref_from_pointer(char *pointer) {
	return *(T *) pointer;
}

#endif //DARKNET_BINDINGS_REF_DERF_H
