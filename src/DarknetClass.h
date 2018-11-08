//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_DARKNETCLASS_H
#define DARKNET_BINDINGS_DARKNETCLASS_H

#include <napi.h>
#include "darknet.h"
#include "DarknetImage.h"

class DarknetClass : public Napi::ObjectWrap<DarknetClass> {

public:
		static Napi::Object Init(Napi::Env env, Napi::Object exports);

		DarknetClass(const Napi::CallbackInfo &info);

		~DarknetClass();

		void resetMemory(const Napi::CallbackInfo &info);

		void rememberNet();

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

namespace DarknetClassWorkers {
		class Predict : public Napi::AsyncWorker {
		private:
				DarknetImage *darknetImage;
				DarknetClass *darknetClass;
				network *net;

				int nboxes = 0;
				detection *dets = nullptr;

		public:

				Predict(
						Napi::Function &callback,
						DarknetClass *darknetClass,
						DarknetImage *darknetImage,
						network *net
				) : Napi::AsyncWorker(callback) {

					this->darknetClass = darknetClass;
					this->darknetImage = darknetImage;
					this->net = net;
				}

				void Execute() {
					network_predict(net, darknetImage->get_image().data);
					darknetClass->rememberNet();
					// todo add toggle
					// dets = darknetClass->predictWithoutMemory(&nboxes, w, h);
					dets = darknetClass->predictWithMemory(&nboxes,
																								 darknetImage->original_width(),
																								 darknetImage->original_height());
				}

				void OnOK() {
					Napi::Env env = Env();
					Napi::HandleScope scope(env);

					// todo make this as DarknetDetection
					Napi::Object ret = Napi::Object::New(Env());
					ret["count"] = Napi::Number::New(Env(), nboxes);
					ret["data_pointer"] = Napi::External<detection>::New(Env(), dets);
					Callback().Call({Env().Undefined(), ret});
				}
		};
}

#endif //DARKNET_BINDINGS_DARKNETCLASS_H
