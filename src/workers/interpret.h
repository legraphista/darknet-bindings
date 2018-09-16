//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_INTERPRET_H
#define DARKNET_BINDINGS_INTERPRET_H

#include <napi.h>
#include <darknet.h>
#include "helpers/ref-unref.h"

struct Prediction {
		std::vector<std::string> names;
		std::vector<float> probabilities;
		int top, left, bottom, right;
		float x, y, w, h;

		~Prediction() {
			names.clear();
			probabilities.clear();
		}
};

class InterpretWorker : public Napi::AsyncWorker {
private:
		detection *dets;
		int nboxes;
		float thresh;
		int w;
		int h;
		unsigned int classes;
		std::vector<std::string> names;

		std::vector<Prediction> predictions;

public:
		InterpretWorker(
				Napi::Buffer<char> dets_data_pointer, int nboxes,
				int w, int h, float thresh,
				unsigned int classes, std::vector<std::string> names,
				Napi::Function &callback
		) : Napi::AsyncWorker(callback) {

			this->nboxes = nboxes;
			this->classes = classes;
			this->thresh = thresh;
			this->w = w;
			this->h = h;
			this->names = names;
			dets = ref_unref_from_napi_buffer<detection *>(dets_data_pointer);
		}

		void Execute() {
			for (int i = 0; i < nboxes; i++) {
				Prediction pred;
				for (int j = 0; j < classes; j++) {
					if (dets[i].prob[j] > thresh) {
						pred.names.push_back(names[j]);
						pred.probabilities.push_back(dets[i].prob[j]);
					}
				}

				if (pred.names.size() == 0) {
					continue;
				}

				box b = dets[i].bbox;
				pred.left = (b.x - b.w / 2.0) * w;
				pred.right = (b.x + b.w / 2.0) * w;
				pred.top = (b.y - b.h / 2.0) * h;
				pred.bottom = (b.y + b.h / 2.0) * h;

				pred.x = b.x;
				pred.y = b.y;
				pred.w = b.w;
				pred.h = b.h;

				// todo add support for mask?

				predictions.push_back(pred);
			}

			free_detections(dets, nboxes);
		}

		void OnOK() {
			Napi::HandleScope scope(Env());
			const int size = predictions.size();
			auto result = Napi::Array::New(Env(), size);

			for (int i = 0; i < size; i++) {
				auto pred = predictions[i];
				int predLen = pred.names.size();

				auto entry = Napi::Object::New(Env());

				auto names = Napi::Array::New(Env(), predLen);
				for (int j = 0; j < predLen; j++) {
					names[j] = pred.names[j];
				}
				entry["names"] = names;

				auto probabilities = Napi::Array::New(Env(), predLen);
				for (int j = 0; j < predLen; j++) {
					probabilities[j] = pred.probabilities[j];
				}
				entry["probabilities"] = probabilities;

				entry["left"] = pred.left;
				entry["right"] = pred.right;
				entry["top"] = pred.top;
				entry["bottom"] = pred.bottom;

				entry["x"] = pred.x;
				entry["y"] = pred.y;
				entry["w"] = pred.w;
				entry["h"] = pred.h;

				result[i] = entry;
			}

			Callback().Call({Env().Undefined(), result});
		}
};

#endif //DARKNET_BINDINGS_INTERPRET_H
