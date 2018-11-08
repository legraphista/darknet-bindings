//
// Created by Muscalu Stefan-Gabriel on 2018-11-08.
//

#ifndef DARKNET_BINDINGS_DARKNETDETECTIONS_H
#define DARKNET_BINDINGS_DARKNETDETECTIONS_H

#include "darknet_external.h"
#include <napi.h>

class DarknetDetections : public Napi::ObjectWrap<DarknetDetections> {
public:
		static Napi::FunctionReference constructor;

		static Napi::Object Init(Napi::Env env, Napi::Object exports);

		DarknetDetections(const Napi::CallbackInfo &info);

		~DarknetDetections();

		void Release(const Napi::CallbackInfo &info);

		Napi::Value GetDetectionCount(const Napi::CallbackInfo &info);

		void DoNMS(const Napi::CallbackInfo &info);

		Napi::Value Interepret(const Napi::CallbackInfo &info);

private:
		void release();

		detection *_detections;
		uint32_t _nb_detections;
		std::vector<std::string> *_names;
		uint32_t _nb_classes;
		uint32_t _w, _h;
		float _thresh;
};

namespace DarknetDetectionsWorkers {
		class NMS : public Napi::AsyncWorker {
		private:
				detection *dets;
				uint32_t nboxes;
				uint32_t classes;
				float nms;

		public:
				NMS(
						Napi::Function &callback,
						detection *dets,
						uint32_t nboxes,
						uint32_t classes,
						float nms
				) : AsyncWorker(callback) {

					this->dets = dets;
					this->nboxes = nboxes;
					this->classes = classes;
					this->nms = nms;
				}

				void Execute() {
					do_nms_obj(dets, nboxes, classes, nms);
				}

				void OnOK() {
					Napi::Env env = Env();
					Napi::HandleScope scope(env);
					Callback().Call({env.Undefined(), env.Undefined()});
				}
		};
}

#endif //DARKNET_BINDINGS_DARKNETDETECTIONS_H
