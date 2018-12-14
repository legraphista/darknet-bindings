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

    static Napi::Value FromPlanarRGB(const Napi::CallbackInfo &info);

    Napi::Value LetterBox(const Napi::CallbackInfo &info);

    Napi::Value ToRGB(const Napi::CallbackInfo &info);

    image const &get_image() const;

    uint32_t const original_width() const;

    uint32_t const original_height() const;

    uint32_t const width() const;

    uint32_t const height() const;

private:
    bool _released = false;

    void release();

    Napi::Reference<Napi::Float32Array> _original_data;
    image _image;
    bool _owns_data = false;

    uint32_t _original_w = 0;
    uint32_t _original_h = 0;
};

namespace DarknetImageWorkers {

    const float div255multiplier = 1.0f / 255.0f;

    class RGB2DarknetImage : public Napi::AsyncWorker {

    private:
        Napi::Reference<Napi::Uint8Array> imageBufferReference;
        uint32_t w, h, c;
        float *output;
        uint8_t *input;
    public:

        RGB2DarknetImage(
            Napi::Function &callback,
            Napi::Uint8Array &imageBuffer,
            uint32_t w, uint32_t h, uint32_t c
        ) : Napi::AsyncWorker(callback) {

          this->input = imageBuffer.Data();
          this->imageBufferReference = Napi::Persistent<Napi::Uint8Array>(imageBuffer);
          this->w = w;
          this->h = h;
          this->c = c;
        }

        void Execute() {
          this->output = (float *) malloc(w * h * c * sizeof(float));

          size_t step = w * c;
          size_t i, j, k;

// Original DarkNet Ipm_to_image with optimizations
//
//          size_t kwh_iw = 0;
//          size_t iw = 0;
//          size_t istep = 0;
//          size_t istep_k = 0;
//          for (i = 0; i < h; ++i) {
//            iw = i * w;
//            istep = i * step;
//
//            for (k = 0; k < c; ++k) {
//              kwh_iw = k * w * h + iw;
//              istep_k = istep + k;
//
//              for (j = 0; j < w; ++j) {
//                output[kwh_iw + j] = input[istep_k + j * c] / 255.0f;
//              }
//            }
//          }

// rewritten ipl_to_image using pointer manipulation and caching
// typically 3x faster, unless other optimizations kick in

          float *planar_channel_out = nullptr;
          float *planar_channel_row_out = nullptr;

          uint8_t *interlaced_row_channel_offset = nullptr;

          // interlaced rgb (rgbrgbrgbrgb)
          // to planar rgb (rrrrrr.....bbbbbbb.....ggggggg)
          for (k = 0; k < c; k++) {
            // navigate to the planar channel data block
            planar_channel_out = this->output + w * h * k;

            for (i = 0; i < h; i++) {
              // navigate to the interlace RGB line, offset by the channel
              interlaced_row_channel_offset = this->input + i * step + k;
              // cache planar row offset
              planar_channel_row_out = planar_channel_out + i * w;

              for (j = 0; j < w; j++) {

                // set pixel for channel at i, j
                planar_channel_row_out[j] =
                    // to pixel from row i, column j, channel k
                    interlaced_row_channel_offset[j * c] * div255multiplier;
              }
            }
          }

        }

        void OnOK() {
          Napi::Env env = Env();

          imageBufferReference.Unref();
          /*
            we pass back a view with the raw float* data
            and we also make sure we free it
           */

          auto dnImage = DarknetImage::constructor.New(
              {
                  float2external(env, output),
                  Napi::Number::New(env, w),
                  Napi::Number::New(env, h),
                  Napi::Number::New(env, c)
              }
          );

          Callback().Call({env.Undefined(), dnImage});
        }

    };

    class PlanarRGB2DarknetImage : public Napi::AsyncWorker {

    private:
        Napi::Reference<Napi::Uint8Array> ref_r;
        Napi::Reference<Napi::Uint8Array> ref_g;
        Napi::Reference<Napi::Uint8Array> ref_b;
        uint32_t w, h;
        uint32_t c = 3;
        float *output;
        uint8_t *input_r;
        uint8_t *input_g;
        uint8_t *input_b;
    public:

        PlanarRGB2DarknetImage(
            Napi::Function &callback,
            Napi::Uint8Array &r,
            Napi::Uint8Array &g,
            Napi::Uint8Array &b,
            uint32_t w, uint32_t h
        ) : Napi::AsyncWorker(callback) {

          this->input_r = r.Data();
          this->input_g = g.Data();
          this->input_b = b.Data();
          this->ref_r = Napi::Persistent<Napi::Uint8Array>(r);
          this->ref_g = Napi::Persistent<Napi::Uint8Array>(g);
          this->ref_b = Napi::Persistent<Napi::Uint8Array>(b);
          this->w = w;
          this->h = h;
        }

        void Execute() {
          size_t channel_size = w * h;
          output = (float *) malloc(channel_size * c * sizeof(float));

          float *output_channel = nullptr;
          size_t i = 0;

          output_channel = output;
          for (i = 0; i < channel_size; i++) {
            output_channel[i] = input_r[i] * div255multiplier;
          }
          output_channel = output + channel_size;
          for (i = 0; i < channel_size; i++) {
            output_channel[i] = input_g[i] * div255multiplier;
          }
          output_channel = output + channel_size + channel_size;
          for (i = 0; i < channel_size; i++) {
            output_channel[i] = input_b[i] * div255multiplier;
          }
        }

        void OnOK() {
          Napi::Env env = Env();

          ref_r.Unref();
          ref_g.Unref();
          ref_b.Unref();
          /*
            we pass back a view with the raw float* data
            and we also make sure we free it
           */

          auto dnImage = DarknetImage::constructor.New(
              {
                  float2external(env, output),
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

          auto dnImage = DarknetImage::constructor.New(
              {
                  float2external(env, output.data),
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
