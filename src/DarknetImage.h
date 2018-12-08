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

		const float byte_float_LUT[] = {0 / 255.f, 1 / 255.f, 2 / 255.f, 3 / 255.f, 4 / 255.f, 5 / 255.f, 6 / 255.f,
																		7 / 255.f, 8 / 255.f, 9 / 255.f, 10 / 255.f, 11 / 255.f, 12 / 255.f, 13 / 255.f,
																		14 / 255.f, 15 / 255.f, 16 / 255.f, 17 / 255.f, 18 / 255.f, 19 / 255.f, 20 / 255.f,
																		21 / 255.f, 22 / 255.f, 23 / 255.f, 24 / 255.f, 25 / 255.f, 26 / 255.f, 27 / 255.f,
																		28 / 255.f, 29 / 255.f, 30 / 255.f, 31 / 255.f, 32 / 255.f, 33 / 255.f, 34 / 255.f,
																		35 / 255.f, 36 / 255.f, 37 / 255.f, 38 / 255.f, 39 / 255.f, 40 / 255.f, 41 / 255.f,
																		42 / 255.f, 43 / 255.f, 44 / 255.f, 45 / 255.f, 46 / 255.f, 47 / 255.f, 48 / 255.f,
																		49 / 255.f, 50 / 255.f, 51 / 255.f, 52 / 255.f, 53 / 255.f, 54 / 255.f, 55 / 255.f,
																		56 / 255.f, 57 / 255.f, 58 / 255.f, 59 / 255.f, 60 / 255.f, 61 / 255.f, 62 / 255.f,
																		63 / 255.f, 64 / 255.f, 65 / 255.f, 66 / 255.f, 67 / 255.f, 68 / 255.f, 69 / 255.f,
																		70 / 255.f, 71 / 255.f, 72 / 255.f, 73 / 255.f, 74 / 255.f, 75 / 255.f, 76 / 255.f,
																		77 / 255.f, 78 / 255.f, 79 / 255.f, 80 / 255.f, 81 / 255.f, 82 / 255.f, 83 / 255.f,
																		84 / 255.f, 85 / 255.f, 86 / 255.f, 87 / 255.f, 88 / 255.f, 89 / 255.f, 90 / 255.f,
																		91 / 255.f, 92 / 255.f, 93 / 255.f, 94 / 255.f, 95 / 255.f, 96 / 255.f, 97 / 255.f,
																		98 / 255.f, 99 / 255.f, 100 / 255.f, 101 / 255.f, 102 / 255.f, 103 / 255.f,
																		104 / 255.f, 105 / 255.f, 106 / 255.f, 107 / 255.f, 108 / 255.f, 109 / 255.f,
																		110 / 255.f, 111 / 255.f, 112 / 255.f, 113 / 255.f, 114 / 255.f, 115 / 255.f,
																		116 / 255.f, 117 / 255.f, 118 / 255.f, 119 / 255.f, 120 / 255.f, 121 / 255.f,
																		122 / 255.f, 123 / 255.f, 124 / 255.f, 125 / 255.f, 126 / 255.f, 127 / 255.f,
																		128 / 255.f, 129 / 255.f, 130 / 255.f, 131 / 255.f, 132 / 255.f, 133 / 255.f,
																		134 / 255.f, 135 / 255.f, 136 / 255.f, 137 / 255.f, 138 / 255.f, 139 / 255.f,
																		140 / 255.f, 141 / 255.f, 142 / 255.f, 143 / 255.f, 144 / 255.f, 145 / 255.f,
																		146 / 255.f, 147 / 255.f, 148 / 255.f, 149 / 255.f, 150 / 255.f, 151 / 255.f,
																		152 / 255.f, 153 / 255.f, 154 / 255.f, 155 / 255.f, 156 / 255.f, 157 / 255.f,
																		158 / 255.f, 159 / 255.f, 160 / 255.f, 161 / 255.f, 162 / 255.f, 163 / 255.f,
																		164 / 255.f, 165 / 255.f, 166 / 255.f, 167 / 255.f, 168 / 255.f, 169 / 255.f,
																		170 / 255.f, 171 / 255.f, 172 / 255.f, 173 / 255.f, 174 / 255.f, 175 / 255.f,
																		176 / 255.f, 177 / 255.f, 178 / 255.f, 179 / 255.f, 180 / 255.f, 181 / 255.f,
																		182 / 255.f, 183 / 255.f, 184 / 255.f, 185 / 255.f, 186 / 255.f, 187 / 255.f,
																		188 / 255.f, 189 / 255.f, 190 / 255.f, 191 / 255.f, 192 / 255.f, 193 / 255.f,
																		194 / 255.f, 195 / 255.f, 196 / 255.f, 197 / 255.f, 198 / 255.f, 199 / 255.f,
																		200 / 255.f, 201 / 255.f, 202 / 255.f, 203 / 255.f, 204 / 255.f, 205 / 255.f,
																		206 / 255.f, 207 / 255.f, 208 / 255.f, 209 / 255.f, 210 / 255.f, 211 / 255.f,
																		212 / 255.f, 213 / 255.f, 214 / 255.f, 215 / 255.f, 216 / 255.f, 217 / 255.f,
																		218 / 255.f, 219 / 255.f, 220 / 255.f, 221 / 255.f, 222 / 255.f, 223 / 255.f,
																		224 / 255.f, 225 / 255.f, 226 / 255.f, 227 / 255.f, 228 / 255.f, 229 / 255.f,
																		230 / 255.f, 231 / 255.f, 232 / 255.f, 233 / 255.f, 234 / 255.f, 235 / 255.f,
																		236 / 255.f, 237 / 255.f, 238 / 255.f, 239 / 255.f, 240 / 255.f, 241 / 255.f,
																		242 / 255.f, 243 / 255.f, 244 / 255.f, 245 / 255.f, 246 / 255.f, 247 / 255.f,
																		248 / 255.f, 249 / 255.f, 250 / 255.f, 251 / 255.f, 252 / 255.f, 253 / 255.f,
																		254 / 255.f, 255 / 255.f};

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
										byte_float_LUT[interlaced_row_channel_offset[j * c]];
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
            output_channel[i] = byte_float_LUT[input_r[i]];
          }
          output_channel = output + channel_size;
          for (i = 0; i < channel_size; i++) {
            output_channel[i] = byte_float_LUT[input_g[i]];
          }
          output_channel = output + channel_size + channel_size;
          for (i = 0; i < channel_size; i++) {
            output_channel[i] = byte_float_LUT[input_b[i]];
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
