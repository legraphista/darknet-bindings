#include <napi.h>
#include "DarknetClass.h"

#include "helpers/fail.h"
#include "helpers/assert.h"
#include "helpers/object.h"

#include "node_api_types.h"
#include "workers/rgb-to-darknet.h"
#include "workers/letterbox.h"
#include "workers/predict.h"
#include "workers/nms.h"
#include "workers/interpret.h"

using namespace Napi;
using namespace std;

Object Init(Env env, Object exports) {
	Napi::HandleScope scope(env);

	Napi::Function func = DarknetClass::DefineClass(env, "DarknetClass", {
			DarknetClass::InstanceMethod("resetMemory", &DarknetClass::resetMemory),
			DarknetClass::InstanceMethod("rgbToDarknet", &DarknetClass::rgbToDarknet),
			DarknetClass::InstanceMethod("letterbox", &DarknetClass::letterbox),
			DarknetClass::InstanceMethod("predict", &DarknetClass::predict),
			DarknetClass::InstanceMethod("nms", &DarknetClass::nms),
			DarknetClass::InstanceMethod("interpret", &DarknetClass::interpret)
	});

	exports.Set("DarknetClass", func);
	return exports;
}

DarknetClass::DarknetClass(const Napi::CallbackInfo &info) : Napi::ObjectWrap<DarknetClass>(info) {

	Napi::Env env = info.Env();
	Napi::HandleScope scope(env);

	assert(info.Length() == 1, "Must pass exactly one parameter!");

	assertValueTypeObject("0", info[0]);

	auto options = info[0].As<Napi::Object>();

	objectGetString(this->cfgFile, options, "cfg");
	objectGetString(this->weightsFile, options, "weights");

	assertObjectMustHave(options, "names");
	Napi::Array names = options.Get("names").As<Napi::Array>();

	unpackStringArray(this->names, names);
	this->classes = (unsigned int) this->names.size();

	if (options.Has("memoryCount")) {
		this->memoryCount = options.Get("memoryCount").ToNumber();
	}
	if (options.Has("nms")) {
		this->nms_thresh = options.Get("nms").ToNumber();
	}
	if (options.Has("thresh")) {
		this->thresh = options.Get("thresh").ToNumber();
	}
	if (options.Has("hier_thresh")) {
		this->hier_thresh = options.Get("hier_thresh").ToNumber();
	}


	this->net = load_network(
			(char *) this->cfgFile.c_str(),
			(char *) this->weightsFile.c_str(),
			0
	);
	set_batch_network(this->net, 1);
	this->net_size_total = network_output_size(this->net);
	this->makeMemory();
	srand(2222222);
}

DarknetClass::~DarknetClass() {
	this->freeMemory();
	free_network(this->net);
	this->names.clear();
}

void DarknetClass::makeMemory() {
	this->memoryIndex = 0;
	this->memorySlotsUsed = 0;
	this->memory = network_memory_make(this->memoryCount, this->net_size_total);
}

void DarknetClass::freeMemory() {
	network_memory_free(this->memory, this->memoryCount);
}

void DarknetClass::resetMemory(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	this->freeMemory();

	if (info.Length() > 0 && info[0].IsNumber()) {
		int n = info[0].ToNumber();
		if (n < 0) {
			js_fail("Cannot set memory lower than 0");
		}
		this->memoryCount = n;
	}

	this->makeMemory();
}

void DarknetClass::rememberNet() {
	network_remember_memory(this->net, this->memory, this->memoryIndex);
	this->memoryIndex = (this->memoryIndex + 1) % this->memoryCount;
	this->memorySlotsUsed = this->memorySlotsUsed + 1;
	if (this->memorySlotsUsed > this->memoryCount) {
		this->memorySlotsUsed = this->memoryCount;
	}
}

void DarknetClass::rgbToDarknet(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	assert(info.Length() == 5, "There must be 4 params passed");

	auto imageBuffer = info[0].As<Uint8Array>();
	int w = info[1].ToNumber();
	int h = info[2].ToNumber();
	int c = info[3].ToNumber();
	auto callback = info[4].As<Function>();

	auto *worker = new RGB2DarknetWorker(imageBuffer, w, h, c, callback);
	worker->Queue();
}

void DarknetClass::letterbox(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	assert(info.Length() == 5, "There must be 5 params passed");
	auto imageBuffer = info[0].As<Float32Array>();
	int w = info[1].ToNumber();
	int h = info[2].ToNumber();
	int c = info[3].ToNumber();
	auto callback = info[4].As<Function>();

	int dw = this->net->w;
	int dh = this->net->h;

	auto *worker = new DarknetLetterboxWorker(imageBuffer, w, h, c, dw, dh, callback);
	worker->Queue();
}

void DarknetClass::predict(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	assert(info.Length() == 4, "There must be 1 param passed");
	auto image_pointer_buffer = info[0].As<Napi::Buffer<char>>();
	int w = info[1].ToNumber();
	assert(w > 0, "invalid width");
	int h = info[2].ToNumber();
	assert(h > 0, "invalid height");

	auto callback = info[3].As<Function>();

	auto *worker = new PredictWorker(
			this,
			this->net,
			w, h,
			this->thresh,
			this->hier_thresh,
			image_pointer_buffer,
			callback);
	worker->Queue();
}

void DarknetClass::nms(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	assert(info.Length() == 3, "There must be 3 param passed");
	auto dets_pointer_buffer = info[0].As<Napi::Buffer<char>>();
	int nboxes = info[1].ToNumber();
	assert(nboxes >= 0, "cannot have negative box count");
	auto callback = info[2].As<Function>();

	auto *worker = new NMSWorker(dets_pointer_buffer, nboxes, this->classes, this->nms_thresh, callback);
	worker->Queue();
}

void DarknetClass::interpret(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	assert(info.Length() == 5, "There must be 5 param passed");

	Napi::Buffer<char> dets_pointer_buffer = info[0].As<Napi::Buffer<char>>();
	int nboxes = info[1].ToNumber();
	assert(nboxes >= 0, "cannot have negative box count");

	int w = info[2].ToNumber();
	assert(w > 0, "invalid width");
	int h = info[3].ToNumber();
	assert(h > 0, "invalid height");
	auto callback = info[4].As<Function>();

	auto *worker = new InterpretWorker(
			dets_pointer_buffer, nboxes,
			w, h,
			this->thresh,
			this->classes,
			this->names,
			callback
	);
	worker->Queue();
}

NODE_API_MODULE(darknet, Init);
