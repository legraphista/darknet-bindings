//
// Created by Muscalu Stefan-Gabriel on 2018-11-07.
//

#ifndef DARKNET_BINDINGS_DARKNETIMAGE_H
#define DARKNET_BINDINGS_DARKNETIMAGE_H

#include <napi.h>
#include "darknet_external.h"

class DarknetImage : public Napi::ObjectWrap<DarknetImage> {
public:
		static Napi::FunctionReference constructor;

		static Napi::Object Init(Napi::Env env, Napi::Object exports);

		DarknetImage(const Napi::CallbackInfo &info);

		~DarknetImage();

		void Release(const Napi::CallbackInfo &info);

		static Napi::Value FromRGB(const Napi::CallbackInfo &info);

		Napi::Value LetterBox(const Napi::CallbackInfo &info);

		Napi::Value ToRGB(const Napi::CallbackInfo &info);

		image const &get_image() const;

private:
		void release();

		Napi::Float32Array _original_data;
		image _image;
};

namespace DarknetImageWorkers {

		class RGB2DarknetImage : public Napi::AsyncWorker {

		private:
				Napi::Uint8Array imageBuffer;
				uint32_t w, h, c;
				float *output;
		public:

				RGB2DarknetImage(
						Napi::Function &callback,
						Napi::Uint8Array &imageBuffer,
						uint32_t w, uint32_t h, uint32_t c
				) : Napi::AsyncWorker(callback) {

					this->imageBuffer = imageBuffer;
					this->w = w;
					this->h = h;
					this->c = c;
				}

				void Execute() {
					this->output = output = (float *) malloc(w * h * c * sizeof(float));

					uint32_t step = w * c;
					uint32_t i, j, k;

					for (i = 0; i < h; ++i) {
						for (k = 0; k < c; ++k) {
							for (j = 0; j < w; ++j) {
								output[k * w * h + i * w + j] = imageBuffer[i * step + j * c + k] / 255.0f;
							}
						}
					}

				}

				void OnOK() {
					Napi::Env env = Env();
					Napi::HandleScope scope(env);

					/*
						we pass back a view with the raw float* data
						and we also make sure we free it
					 */

					auto arrView = Napi::ArrayBuffer::New(
							env,
							output,
							w * h * c * sizeof(float),
							[](Napi::Env /*env*/, void *finalizeData) {
									free(finalizeData);
							}
					);

					Napi::Float32Array out = Napi::Float32Array::New(
							env,
							w * h * c,
							arrView,
							0
					);

					auto dnImage = DarknetImage::constructor.New(
							{
									out,
									Napi::Number::New(env, w),
									Napi::Number::New(env, h),
									Napi::Number::New(env, c)
							}
					);

					Callback().Call({env.Undefined(), dnImage});
				}

		};

		class Letterbox : public Napi::AsyncWorker {
		private:
				DarknetImage *darknetImage;
				uint32_t w, h;
				image output;

		public:
				Letterbox(
						Napi::Function &callback,
						DarknetImage *darknetImage,
						uint32_t w, uint32_t h
				) : Napi::AsyncWorker(callback) {

					this->darknetImage = darknetImage;

					this->w = w;
					this->h = h;
				}

				void Execute() {
					output = letterbox_image(
							darknetImage->get_image(),
							w, h
					);
				}

				void OnOK() {
					Napi::Env env = Env();
					Napi::HandleScope scope(env);

					uint32_t len = output.w * output.h * output.c;

					auto arrView = Napi::ArrayBuffer::New(
							env,
							output.data,
							len * sizeof(float),
							[](Napi::Env /*env*/, void *finalizeData) {
									free(finalizeData);
							}
					);

					Napi::Float32Array out = Napi::Float32Array::New(
							env,
							len,
							arrView,
							0
					);

					auto dnImage = DarknetImage::constructor.New(
							{
									out,
									Napi::Number::New(env, output.w),
									Napi::Number::New(env, output.h),
									Napi::Number::New(env, output.c)
							}
					);

					Callback().Call({env.Undefined(), dnImage});
				}
		};
}

#endif //DARKNET_BINDINGS_DARKNETIMAGE_H
