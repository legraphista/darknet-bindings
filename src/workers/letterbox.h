//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_LETTERBOX_H
#define DARKNET_BINDINGS_LETTERBOX_H

#include <napi.h>
#include <darknet.h>
#include "helpers/ref-unref.h"

class DarknetLetterboxWorker : public Napi::AsyncWorker {

private:
		Napi::Float32Array input;
		image output;
		int w, h, c;
		int dw, dh;

public:
		DarknetLetterboxWorker(Napi::Float32Array input, int w, int h, int c, int dw, int dh, Napi::Function &callback)
				: Napi::AsyncWorker(callback) {
			this->input = input;
			this->w = w;
			this->h = h;
			this->c = c;
			this->dw = dw;
			this->dh = dh;
		}

		void Execute() {

			image in = float_to_image(w, h, c, input.Data());

			// this memory is owned by the C binding
			output = letterbox_image(in, dw, dh);

			// no need to free image since memory is held by JS
			// free_image(in);
		}

		void OnOK() {
			Napi::HandleScope scope(Env());

			/*
			 Here we'll just pass the pointer as we need to free it later on anyway
			 no need to copy the letterboxed image around
			 */


			Napi::Object ret = Napi::Object::New(Env());

			ret["w"] = Napi::Number::New(Env(), output.w);
			ret["h"] = Napi::Number::New(Env(), output.h);
			ret["c"] = Napi::Number::New(Env(), output.c);

			char *pointer_value = ref_unref_to_pointer<image>(output);
			int pointer_size = ref_unref_size<image>();

			ret["data_pointer"] = Napi::Buffer<char>::New(Env(), pointer_value, pointer_size);

			Callback().Call({Env().Undefined(), ret});
		}
};

#endif //DARKNET_BINDINGS_LETTERBOX_H
