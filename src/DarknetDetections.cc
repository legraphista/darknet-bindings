//
// Created by Muscalu Stefan-Gabriel on 2018-11-08.
//

#include "DarknetDetections.h"
#include <math.h>

using namespace Napi;
Napi::FunctionReference DarknetDetections::constructor;

Object DarknetDetections::Init(Napi::Env env, Object exports) {
  Napi::Function func = DefineClass(env, "DarknetDetections", {
      InstanceAccessor("count", &DarknetDetections::GetDetectionCount, nullptr,
                       napi_property_attributes::napi_enumerable),

      InstanceMethod("release", &DarknetDetections::Release,
                     napi_property_attributes::napi_enumerable),
      InstanceMethod("interpret", &DarknetDetections::Interepret,
                     napi_property_attributes::napi_enumerable),
      InstanceMethod("doNMS", &DarknetDetections::DoNMS,
                     (napi_property_attributes) (napi_property_attributes::napi_enumerable |
                                                 napi_property_attributes::napi_writable)),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("DarknetDetections", func);
  return exports;
}

DarknetDetections::DarknetDetections(const Napi::CallbackInfo &info) : Napi::ObjectWrap<DarknetDetections>(info) {

  auto detection_external = info[0].As<External<detection>>();
  _detections = detection_external.Data();

  _nb_detections = info[1].ToNumber();

  auto names_external = info[2].As<External<std::vector<std::string>>>();
  _names = names_external.Data();

  _nb_classes = info[3].ToNumber();

  _w = info[4].ToNumber();
  _h = info[5].ToNumber();
  _thresh = info[6].ToNumber();
}

DarknetDetections::~DarknetDetections() {
  this->release();
}

void DarknetDetections::release() {
  if (_detections != nullptr) {
    free_detections(_detections, _nb_detections);
    _nb_detections = 0;
  }
  _detections = nullptr;
  _names = nullptr;
}

void DarknetDetections::Release(const Napi::CallbackInfo &info) {
  this->release();
}

Napi::Value DarknetDetections::GetDetectionCount(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, _nb_detections);
}

void DarknetDetections::DoNMS(const Napi::CallbackInfo &info) {
  float nms = info[0].ToNumber();
  Function callback = info[1].As<Napi::Function>();

  auto *worker = new DarknetDetectionsWorkers::NMS(
      callback,
      _detections,
      _nb_detections,
      _nb_classes,
      nms
  );
  worker->Queue();
}

Napi::Value DarknetDetections::Interepret(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Array output = Array::New(env);
  uint32_t output_len = 0;
  for (uint32_t i = 0; i < _nb_detections; i++) {
    detection d = _detections[i];

    Object detection = Object::New(env);

    // --- names & probabilities ---
    uint32_t cnt = 0;
    Array names = Array::New(env);
    Array probabilities = Array::New(env);

    int best_index = -1;
    float best_prob = 0;

    for (uint32_t j = 0; j < _nb_classes; j++) {
      float p = d.prob[j];
      if (p > _thresh) {
        names[cnt] = (*_names)[j];
        probabilities[cnt] = p;

        if (p > best_prob) {
          best_prob = p;
          best_index = cnt;
        }

        cnt++;
      }
    }

    if (cnt == 0) {
      continue;
    }

    detection["names"] = names;
    detection["probabilities"] = probabilities;

    // --- best ---
    Object best = Object::New(env);
    if (best_index > -1) {
      best["name"] = names.Get((uint32_t) best_index);
      best["probability"] = probabilities.Get((uint32_t) best_index);
    }
    detection["best"] = best;

    // --- bbox ---
    box b = d.bbox;
    Object box = Object::New(env);
    box["x"] = b.x;
    box["y"] = b.y;
    box["w"] = b.w;
    box["h"] = b.h;
    detection["box"] = box;

    // bounding box
    auto top = (uint32_t) floor((b.y - b.h / 2) * _h);
    auto bottom = (uint32_t) floor((b.y + b.h / 2) * _h);

    if (bottom > _h - 1) {
      auto diff = (uint32_t) ceil((bottom - (_h - 1)) / 2.0);
      top -= diff;
      bottom -= diff;
    }

    auto left = (uint32_t) floor((b.x - b.w / 2) * _w);
    auto right = (uint32_t) floor((b.x + b.w / 2) * _w);

    if (right > _w - 1) {
      auto diff = (uint32_t) ceil((right - (_w - 1)) / 2.0);
      left -= diff;
      right -= diff;
    }

    auto width = right - left;
    auto height = bottom - top;

    detection["top"] = top;
    detection["bottom"] = bottom;

    detection["left"] = left;
    detection["right"] = right;

    detection["width"] = width;
    detection["height"] = height;

    output[output_len++] = detection;
  }
  return output;
}