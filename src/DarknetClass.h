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

		void resetMemory(const Napi::CallbackInfo &info);

		void rememberNet();

		void rgbToDarknet(const Napi::CallbackInfo &info);

		void letterbox(const Napi::CallbackInfo &info);

		void predict(const Napi::CallbackInfo &info);

		void nms(const Napi::CallbackInfo &info);

		void interpret(const Napi::CallbackInfo &info);

		detection *predictWithoutMemory(int *nboxes, int w, int h);
		detection *predictWithMemory(int *nboxes, int w, int h);

private:

		std::string cfgFile;
		std::string weightsFile;
		std::vector<std::string> names;
		unsigned int classes = 0;

		float thresh = 0.5;
		float hier_thresh = 0.5;
		float nms_thresh = 0.4;

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
