//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_RGB_TO_DARKNET_H
#define DARKNET_BINDINGS_RGB_TO_DARKNET_H

#include <napi.h>

class RGB2DarknetWorker : public Napi::AsyncWorker {

private:
		Napi::Uint8Array imageBuffer;
		int w, h, c;
		float *output;
public:

		RGB2DarknetWorker(Napi::Uint8Array imageBuffer, int w, int h, int c, Napi::Function &callback)
				: Napi::AsyncWorker(callback) {
			this->imageBuffer = imageBuffer;
			this->w = w;
			this->h = h;
			this->c = c;
		}

		void Execute() {
			this->output = output = (float *) calloc(w * h * c, sizeof(float));

			int step = w * c;
			int i, j, k;

			for (i = 0; i < h; ++i) {
				for (k = 0; k < c; ++k) {
					for (j = 0; j < w; ++j) {
						output[k * w * h + i * w + j] = imageBuffer[i * step + j * c + k] / 255.0;
					}
				}
			}

		}

		void OnOK() {
			Napi::HandleScope scope(Env());

			/*
				we pass back a view with the raw float* data
			  and we also make sure we free it
			 */

			auto arrView = Napi::ArrayBuffer::New(
					Env(),
					output,
					w * h * c * sizeof(float),
					[](Napi::Env /*env*/, void *finalizeData) {
							free(finalizeData);
					}
			);

			Napi::Float32Array out = Napi::Float32Array::New(
					Env(),
					imageBuffer.ElementLength(),
					arrView,
					0
			);

			Callback().Call({Env().Undefined(), out});
		}

};

#endif //DARKNET_BINDINGS_RGB_TO_DARKNET_H
