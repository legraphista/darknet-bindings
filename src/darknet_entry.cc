//
// Created by Muscalu Stefan-Gabriel on 2018-11-08.
//

#include "darknet_entry.h"

using namespace Napi;

Object Init(Napi::Env env, Object exports) {

	DarknetImage::Init(env, exports);
	DarknetClass::Init(env, exports);
	DarknetDetections::Init(env, exports);

	return exports;
}

NODE_API_MODULE(darknet, Init);
