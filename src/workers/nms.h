//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_NMS_H
#define DARKNET_BINDINGS_NMS_H

#include <napi.h>
#include <darknet.h>
#include "helpers/ref-unref.h"

class NMSWorker : public Napi::AsyncWorker {
private:
		detection *dets;
		int nboxes;
		int classes;
		float nms;

public:
		NMSWorker(
				Napi::Buffer<char> dets_data_pointer, int nboxes,
				unsigned int classes, float nms,
				Napi::Function &callback
		) : AsyncWorker(callback) {

			this->nboxes = nboxes;
			this->classes = classes;
			this->nms = nms;

			dets = ref_unref_from_napi_buffer<detection *>(dets_data_pointer);
		}

		void Execute() {
			do_nms_obj(dets, nboxes, (int) classes, nms);
		}
};

#endif //DARKNET_BINDINGS_NMS_H
