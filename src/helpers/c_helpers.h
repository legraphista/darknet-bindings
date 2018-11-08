//
// Created by Muscalu Stefan-Gabriel on 2018-11-08.
//

#ifndef DARKNET_BINDINGS_C_HELPERS_H
#define DARKNET_BINDINGS_C_HELPERS_H

inline Napi::Float32Array float2js(Napi::Env &env, float *data, uint32_t size) {
	auto arrView = Napi::ArrayBuffer::New(
			env,
			data,
			size * sizeof(float),
			[](Napi::Env /*env*/, void *finalizeData) {
					free(finalizeData);
			}
	);

	return Napi::Float32Array::New(env, size, arrView, 0);
}

#endif //DARKNET_BINDINGS_C_HELPERS_H
