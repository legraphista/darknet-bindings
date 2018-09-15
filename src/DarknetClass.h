//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_DARKNET_H
#define DARKNET_BINDINGS_DARKNET_H

#include <napi.h>
#include <darknet.h>

class DarknetClass : public Napi::ObjectWrap<DarknetClass> {

public:
		static Napi::Object Init(Napi::Env env, Napi::Object exports);

		DarknetClass(const Napi::CallbackInfo &info);

		~DarknetClass();

		void resetMemory(const Napi::CallbackInfo& info);

private:
		std::string cfgFile;
		std::string weightsFile;
		std::vector<std::string> names;
		int classes = 0;

		float thresh = 0.5;
		float hier_thresh = 0.5;
		float nms = 0.4;

		int net_size_total = 0;
		network *net;

		int memoryIndex = 0;
		int memorySlotsUsed = 0;
		int memoryCount = 3;
		float **memory = nullptr;

		void makeMemory();

		void freeMemory();

};

#endif //DARKNET_BINDINGS_DARKNET_H
