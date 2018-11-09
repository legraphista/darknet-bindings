//
// Created by Muscalu Stefan-Gabriel on 2018-11-08.
//

#include <node_version.h>
#include "darknet_entry.h"

using namespace Napi;

Object Init(Napi::Env env, Object exports) {

  if (NODE_MAJOR_VERSION != 9) {
    fprintf(stderr, "[Darknet-Bindings] Unsupported major version of node %d detected!\n",
           NODE_MAJOR_VERSION);
    fprintf(stderr, "[Darknet-Bindings] Stable tested version is 9.11, you might experience crashes.\n");
  }

  DarknetImage::Init(env, exports);
  DarknetClass::Init(env, exports);
  DarknetDetections::Init(env, exports);

  return exports;
}

NODE_API_MODULE(darknet, Init);
