//
// Created by Muscalu Stefan-Gabriel on 2018-11-07.
//

#include "DarknetImage.h"

using namespace Napi;
Napi::FunctionReference DarknetImage::constructor;

Object DarknetImage::Init(Napi::Env env, Object exports) {
  Napi::Function func = DefineClass(env, "DarknetImage", {
//			InstanceAccessor("width", &FFFrame::GetWidth, nullptr,
//											 napi_property_attributes::napi_enumerable),

      InstanceMethod("release", &DarknetImage::Release,
                     napi_property_attributes::napi_enumerable),
      InstanceMethod("toRGB", &DarknetImage::ToRGB,
                     (napi_property_attributes) (napi_property_attributes::napi_enumerable |
                                                 napi_property_attributes::napi_writable)),
      InstanceMethod("letterbox", &DarknetImage::LetterBox,
                     (napi_property_attributes) (napi_property_attributes::napi_enumerable |
                                                 napi_property_attributes::napi_writable)),

      StaticMethod("fromRGB", &DarknetImage::FromRGB,
                   (napi_property_attributes) (napi_property_attributes::napi_enumerable |
                                               napi_property_attributes::napi_static |
                                               napi_property_attributes::napi_writable)),
      StaticMethod("fromPlanarRGB", &DarknetImage::FromPlanarRGB,
                   (napi_property_attributes) (napi_property_attributes::napi_enumerable |
                                               napi_property_attributes::napi_static |
                                               napi_property_attributes::napi_writable))
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("DarknetImage", func);
  return exports;
}


DarknetImage::DarknetImage(const Napi::CallbackInfo &info) : Napi::ObjectWrap<DarknetImage>(info) {
  auto env = info.Env();
  if (info[0].IsExternal()) {
    // if it's external, i own the data
    _owns_data = true;

    _image.data = external2float(info[0]);
  } else if (info[0].IsTypedArray()) {
    // if it's a Float32Array, JS owns the data
    _owns_data = false;

    _original_data = Napi::Persistent<Float32Array>(info[0].As<Float32Array>());

    _image.data = _original_data.Value().Data();
  } else {
    js_fail("Data is not a typed array!");
  }

  _image.w = info[1].ToNumber();
  _image.h = info[2].ToNumber();
  _image.c = info[3].ToNumber();

  _original_w = info.Length() > 4 ?
                info[4].ToNumber() :
                (uint32_t) _image.w;
  _original_h = info.Length() > 5 ?
                info[5].ToNumber() :
                (uint32_t) _image.h;
}

uint32_t const DarknetImage::original_width() const {
  return _original_w;
}

uint32_t const DarknetImage::original_height() const {
  return _original_h;
}

uint32_t const DarknetImage::width() const {
  return (uint32_t) _image.w;
}

uint32_t const DarknetImage::height() const {
  return (uint32_t) _image.h;
}

DarknetImage::~DarknetImage() {
  this->release();
}

void DarknetImage::Release(const Napi::CallbackInfo &info) {
  this->release();
}

void DarknetImage::release() {
  if (_released) return;
  _released = true;
  // we would unref here, but at destructor time, Napi::Reference releases the data
  // _original_data.Unref();
  if (_owns_data) {
    free_image(_image);
  }
}

image const &DarknetImage::get_image() const {
  return _image;
}

Napi::Value DarknetImage::FromRGB(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto buff = info[0].As<Uint8Array>();
  uint32_t w = info[1].ToNumber();
  uint32_t h = info[2].ToNumber();
  uint32_t c = info[3].ToNumber();
  Function callback = info[4].As<Napi::Function>();

  auto *worker = new DarknetImageWorkers::RGB2DarknetImage(
      callback,
      buff,
      w, h, c
  );
  worker->Queue();
  return env.Undefined();
}

Napi::Value DarknetImage::FromPlanarRGB(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto buff_r = info[0].As<Uint8Array>();
  auto buff_g = info[1].As<Uint8Array>();
  auto buff_b = info[2].As<Uint8Array>();

  uint32_t w = info[3].ToNumber();
  uint32_t h = info[4].ToNumber();

  Function callback = info[5].As<Napi::Function>();

  auto *worker = new DarknetImageWorkers::PlanarRGB2DarknetImage(
      callback,
      buff_r,
      buff_g,
      buff_b,
      w, h
  );
  worker->Queue();
  return env.Undefined();
}

Napi::Value DarknetImage::LetterBox(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  uint32_t w = info[0].ToNumber();
  uint32_t h = info[1].ToNumber();
  Function callback = info[2].As<Napi::Function>();

  auto *worker = new DarknetImageWorkers::Letterbox(
      callback,
      this,
      w, h
  );
  worker->Queue();
  return env.Undefined();
}

Napi::Value DarknetImage::ToRGB(const Napi::CallbackInfo &info) {
  // todo
  Napi::Env env = info.Env();
  js_failv("Not implemented yet!");
}
