//
// Created by Muscalu Stefan-Gabriel on 16/09/2018.
//

#ifndef DARKNET_BINDINGS_NMS_H
#define DARKNET_BINDINGS_NMS_H

#include <napi.h>
#include <darknet.h>

class NMSWorker : public Napi::AsyncWorker {
private:
		detection *dets;
		int nboxes;
		int classes;
		float nms;

public:
		NMSWorker(
				detection *dets, int nboxes,
				unsigned int classes, float nms,
				Napi::Function &callback
		) : AsyncWorker(callback) {

			this->nboxes = nboxes;
			this->classes = classes;
			this->nms = nms;

			this->dets = dets;
		}

		 void Execute() {
			volatile int x= 0;
			do_nms_obj(dets, nboxes, (int) classes, nms);
		}
};

#endif //DARKNET_BINDINGS_NMS_H
