//
// Created by Muscalu Stefan-Gabriel on 2018-11-07.
//

#ifndef DARKNET_BINDINGS_DARKNETIMAGE_H
#define DARKNET_BINDINGS_DARKNETIMAGE_H

#include <napi.h>
#include "darknet_external.h"
#include "helpers/c_helpers.h"
#include "helpers/fail.h"

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

		uint32_t const original_width() const;

		uint32_t const original_height() const;

		uint32_t const width() const;

		uint32_t const height() const;

private:
		void release();

		Napi::Float32Array _original_data;
		image _image;

		uint32_t _original_w = 0;
		uint32_t _original_h = 0;
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

					auto dnImage = DarknetImage::constructor.New(
							{
									float2js(env, output, w * h * c),
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

					uint32_t len = (
							(uint32_t) output.w *
							(uint32_t) output.h *
							(uint32_t) output.c
					);

					auto dnImage = DarknetImage::constructor.New(
							{
									float2js(env, output.data, len),
									Napi::Number::New(env, output.w),
									Napi::Number::New(env, output.h),
									Napi::Number::New(env, output.c),
									Napi::Number::New(env, darknetImage->width()),
									Napi::Number::New(env, darknetImage->height())
							}
					);

					Callback().Call({env.Undefined(), dnImage});
				}
		};
}

#endif //DARKNET_BINDINGS_DARKNETIMAGE_H
