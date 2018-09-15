//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_RGB_TO_DARKNET_H
#define DARKNET_BINDINGS_RGB_TO_DARKNET_H

#include <napi.h>

class RGB2DarknetWorker : public Napi::AsyncWorker {
public:

		RGB2DarknetWorker(Napi::Uint8Array imageBuffer, int w, int h, int c, Napi::Function &callback)
				: Napi::AsyncWorker(callback) {
			this->imageBuffer = imageBuffer;// Napi::Uint8Array::From(Env(), imageBuffer);
			this->w = w;
			this->h = h;
			this->c = c;
//			this->output = Napi::Float32Array::New(Env(), (size_t) w * h * c);
		}

		void Execute() {
			this->output = Napi::Float32Array::New(Env(), (size_t) w * h * c);

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
				We can't pass back the output directly even tho' it's been allocated in current env
			  If we do, we end up with a random Value (usually a string) from the stack ¯\_(ツ)_/¯
			  I don't think i understand this properly
			 */

			// use buffer to make a quick copy
			Napi::Buffer<float> copy = Napi::Buffer<float>::Copy(Env(), output.Data(), output.ElementLength());
			// use the ArrayBuffer to make thew new Float32 interpretation
			Napi::Float32Array out = Napi::Float32Array::New(Env(), output.ElementLength(), copy.ArrayBuffer(), 0);

			Callback().Call({
													Env().Undefined(),
													out
											});
		}

private:
		Napi::Uint8Array imageBuffer;
		Napi::Float32Array output;
		int w, h, c;

};

#endif //DARKNET_BINDINGS_RGB_TO_DARKNET_H
