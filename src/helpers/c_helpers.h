//
// Created by Muscalu Stefan-Gabriel on 2018-11-08.
//

#ifndef DARKNET_BINDINGS_C_HELPERS_H
#define DARKNET_BINDINGS_C_HELPERS_H

#include <napi.h>

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

inline Napi::Value float2external(Napi::Env &env, float *data) {
  return Napi::External<float>::New(env, data);
}

inline float *external2float(Napi::Value val) {
  return val.As<Napi::External<float>>().Data();
}

#endif //DARKNET_BINDINGS_C_HELPERS_H
