#include <napi.h>
#include "DarknetClass.h"

#include "helpers/fail.h"
#include "helpers/assert.h"
#include "helpers/object.h"

#include "node_api_types.h"

#include "DarknetImage.h"

using namespace Napi;
using namespace std;

Object DarknetClass::Init(Napi::Env env, Object exports) {

	Napi::Function func = DarknetClass::DefineClass(env, "DarknetClass", {
			DarknetClass::InstanceAccessor("netWidth", &DarknetClass::GetNetWidth, nullptr),
			DarknetClass::InstanceAccessor("netHeight", &DarknetClass::GetNetHeight, nullptr),
			DarknetClass::InstanceMethod("resetMemory", &DarknetClass::resetMemory),
			DarknetClass::InstanceMethod("predict", &DarknetClass::predict,
																	 (napi_property_attributes) (napi_property_attributes::napi_enumerable |
																															 napi_property_attributes::napi_writable)),
	});

	exports.Set("Darknet", func);
	return exports;
}

DarknetClass::DarknetClass(const Napi::CallbackInfo &info) : Napi::ObjectWrap<DarknetClass>(info) {

	Napi::Env env = info.Env();

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

Napi::Value DarknetClass::GetNetWidth(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	return Number::New(env, this->net->w);
}

Napi::Value DarknetClass::GetNetHeight(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	return Number::New(env, this->net->h);
}

void DarknetClass::makeMemory() {
	this->memoryIndex = 0;
	this->memorySlotsUsed = 0;
	this->memory = network_memory_make(this->memoryCount, this->net_size_total);
}

void DarknetClass::freeMemory() {
	network_memory_free(this->memory, this->memoryCount);
}

// This is dangerous as other threads might be using it
// todo add this in readme
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

detection *DarknetClass::predictWithoutMemory(int *nboxes, int w, int h) {
	return get_network_boxes(net, w, h, thresh, hier_thresh, 0, 1, nboxes);
}

detection *DarknetClass::predictWithMemory(int *nboxes, int w, int h) {
	return network_avg_predictions(net, net_size_total,
																 memory, memorySlotsUsed,
																 nboxes, w, h,
																 thresh, hier_thresh);
}

std::vector<std::string> &DarknetClass::get_names() {
	return names;
}

uint32_t DarknetClass::get_classes_count() {
	return classes;
}

float DarknetClass::get_thresh() {
	return thresh;
}

void DarknetClass::predict(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();

	auto *image = DarknetImage::Unwrap(info[0].ToObject());

	auto callback = info[1].As<Function>();

	auto *worker = new DarknetClassWorkers::Predict(
			callback,
			this,
			image,
			this->net
	);
	worker->Queue();
}

