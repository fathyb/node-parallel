//
// Created by fathy on 26/05/2018.
//

#include <nan.h>

#include "IPC_Callee.h"
#include "IPC_Caller.h"
#include "Buffer.h"

#include "../utils/Configuration.h"
#include "Store.h"

using namespace std;
using namespace v8;

namespace nsb {
	NAN_METHOD(queueIPCMessage) {
		nsb::addon::QueueIPCMessage(info.GetIsolate(), info[0], info[1], info[2]);
	}

	NAN_METHOD(waitForIPCMessage) {
		addon::WaitForMessage(info[0]);
	}

	NAN_METHOD(exportBuffer) {
		info.GetReturnValue().Set(
			addon::ExportBuffer(info.GetIsolate(), info[0])
		);
	}

	NAN_METHOD(importBuffer) {
		info.GetReturnValue().Set(
			addon::ImportBuffer(info[0])
		);
	}

    NAN_METHOD(isMaster) {
		info.GetReturnValue().Set(
			Boolean::New(info.GetIsolate(), GlobalConfiguration.master)
		);
    }

    NAN_METHOD(alloc) {
    	info.GetReturnValue().Set(
    		addon::AllocBuffer(info[0])
		);
    }

    NAN_METHOD(getOrSet) {
		addon::GetOrSet(info.GetIsolate(), info[0], info[1], info[2]);

		info.GetReturnValue().SetUndefined();
	}

    NAN_MODULE_INIT(Init) {
		NAN_EXPORT(target, isMaster);
		NAN_EXPORT(target, alloc);
		NAN_EXPORT(target, importBuffer);
		NAN_EXPORT(target, exportBuffer);
		NAN_EXPORT(target, waitForIPCMessage);
		NAN_EXPORT(target, queueIPCMessage);
		NAN_EXPORT(target, getOrSet);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
}
