//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_DARKNETCLASS_H
#define DARKNET_BINDINGS_DARKNETCLASS_H

#include <napi.h>
#include "darknet.h"
#include "DarknetImage.h"
#include "DarknetDetections.h"

class DarknetClass : public Napi::ObjectWrap<DarknetClass> {

public:
		static Napi::Object Init(Napi::Env env, Napi::Object exports);

		DarknetClass(const Napi::CallbackInfo &info);

		~DarknetClass();

		Napi::Value GetNetWidth(const Napi::CallbackInfo &info);
		Napi::Value GetNetHeight(const Napi::CallbackInfo &info);

		void resetMemory(const Napi::CallbackInfo &info);

		void rememberNet();

		void predict(const Napi::CallbackInfo &info);

		detection *predictWithoutMemory(int *nboxes, int w, int h);

		detection *predictWithMemory(int *nboxes, int w, int h);

		std::vector<std::string> &get_names();

		uint32_t get_classes_count();

		float get_thresh();

private:

		std::string cfgFile;
		std::string weightsFile;
		std::vector<std::string> names;
		uint32_t classes = 0;

		float thresh = 0.5;
		float hier_thresh = 0.5;

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

					Napi::Object r = DarknetDetections::constructor
							.New({
											 Napi::External<detection>::New(env, dets),
											 Napi::Number::New(env, nboxes),
											 Napi::External<std::vector<std::string>>::New(env, &darknetClass->get_names()),
											 Napi::Number::New(env, darknetClass->get_classes_count()),
											 Napi::Number::New(env, darknetImage->original_width()),
											 Napi::Number::New(env, darknetImage->original_height()),
											 Napi::Number::New(env, darknetClass->get_thresh())
									 });

					// todo make this as DarknetDetection
					Callback().Call({Env().Undefined(), r});
				}
		};
}

#endif //DARKNET_BINDINGS_DARKNETCLASS_H
