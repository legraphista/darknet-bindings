#include <napi.h>
#include "DarknetClass.h"

#include "helpers/fail.h"
#include "helpers/assert.h"
#include "helpers/object.h"

using namespace Napi;
using namespace std;

Object Init(Env env, Object exports) {
	Napi::HandleScope scope(env);

	Napi::Function func = DarknetClass::DefineClass(env, "Darknet", {
			DarknetClass::InstanceMethod("resetMemory", &DarknetClass::resetMemory)
	});

	exports.Set("Darknet", func);
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

	if (options.Has("memoryCount")) {
		this->memoryCount = options.Get("memoryCount").ToNumber();
	}
	if (options.Has("nms")) {
		this->nms = options.Get("nms").ToNumber();
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
			fail("Cannot set memory lower than 0");
		}
		this->memoryCount = n;
	}

	this->makeMemory();
}

NODE_API_MODULE(darknet, Init);
