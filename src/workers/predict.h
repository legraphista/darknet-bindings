//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_PREDICT_H
#define DARKNET_BINDINGS_PREDICT_H

#include <napi.h>
#include <darknet.h>
#include "../helpers/ref-unref.h"

class PredictWorker : public Napi::AsyncWorker {
private:
		float* input;
		network *net;
		DarknetClass *darknetClass;
public:
		PredictWorker(DarknetClass *darknetClass, network *net, Napi::Buffer<char> image_pointer, Napi::Function &callback)
				: Napi::AsyncWorker(callback) {

			input = ref_unref_from_pointer<float*>(image_pointer.Data());
			this->darknetClass = darknetClass;
			this->net = net;
		}

		void Execute() {
			network_predict(this->net, input);
			darknetClass->rememberNet();
		}
};

#endif //DARKNET_BINDINGS_PREDICT_H
