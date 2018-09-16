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
		float *input;
		network *net;
		DarknetClass *darknetClass;

		int nboxes = 0;
		detection *dets;

		int w, h;
		float thresh, hier;
public:
		PredictWorker(
				DarknetClass *darknetClass, network *net,
				int w, int h,
				float thresh, float hier,
				Napi::Buffer<char> image_pointer,
				Napi::Function &callback
		) : Napi::AsyncWorker(callback) {

			input = ref_unref_from_pointer<float *>(image_pointer.Data());
			this->darknetClass = darknetClass;
			this->net = net;
			this->w = w;
			this->h = h;
			this->thresh = thresh;
			this->hier = hier;
		}

		void Execute() {
			network_predict(net, input);
			darknetClass->rememberNet();
			dets = get_network_boxes(net, w, h, thresh, hier, 0, 1, &nboxes);
			free(input);
		}

		void OnOK() {
			Napi::HandleScope scope(Env());

			Napi::Object ret = Napi::Object::New(Env());

			ret["count"] = Napi::Number::New(Env(), nboxes);
			ret["data_pointer"] = ref_unref_to_napi_buffer<detection *>(Env(), dets);
			Callback().Call({Env().Undefined(), ret});
		}
};

#endif //DARKNET_BINDINGS_PREDICT_H
