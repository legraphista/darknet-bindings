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

inline Napi::Function promise2callback(Napi::Env &env, Napi::Promise::Deferred &future) {

  return Napi::Function::New(
      env,
      [future](const Napi::CallbackInfo &info) {
          Napi::Env env2 = info.Env();

          if (info.Length() >= 1 && !info[0].IsUndefined() && !info[0].IsNull()) {
            future.Reject(info[0]);
          } else if (info.Length() >= 2) {
            future.Resolve(info[1]);
          } else {
            future.Resolve(env2.Undefined());
          }

      }
  );
}

#endif //DARKNET_BINDINGS_C_HELPERS_H
