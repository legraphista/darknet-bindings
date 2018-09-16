//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_PREDICT_H
#define DARKNET_BINDINGS_PREDICT_H

#include <napi.h>
#include <darknet.h>

class PredictWorker : public Napi::AsyncWorker {
private:
		float *input;
		DarknetClass *darknetClass;

		network *net;
		int net_size_total;

		float **memory;
		int memorySlotsUsed;

		int nboxes = 0;
		detection *dets;

		int w, h;
		float thresh, hier;
public:
		PredictWorker(
				DarknetClass *darknetClass,
				network *net, int net_size_total,
				float **memory, int memorySlotsUsed,
				int w, int h,
				float thresh, float hier,
				float *image_pointer,
				Napi::Function &callback
		) : Napi::AsyncWorker(callback) {

			// todo clean unnecessary arguments
			input = image_pointer;

			this->darknetClass = darknetClass;
			this->net = net;
			this->net_size_total = net_size_total;
			this->memory = memory;
			this->memorySlotsUsed = memorySlotsUsed;
			this->w = w;
			this->h = h;
			this->thresh = thresh;
			this->hier = hier;
		}

		void Execute() {
			network_predict(net, input);
			darknetClass->rememberNet();
			// todo add toggle
//			dets = darknetClass->predictWithoutMemory(&nboxes, w, h);
			dets = darknetClass->predictWithMemory(&nboxes, w, h);
		}

		void OnOK() {
			Napi::HandleScope scope(Env());

			free(input);
			Napi::Object ret = Napi::Object::New(Env());

			ret["count"] = Napi::Number::New(Env(), nboxes);
			ret["data_pointer"] = Napi::External<detection>::New(Env(), dets);
			Callback().Call({Env().Undefined(), ret});
		}
};

#endif //DARKNET_BINDINGS_PREDICT_H
