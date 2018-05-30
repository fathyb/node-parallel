//
// Created by fathy on 26/05/2018.
//

#include <nan.h>
#include "SharedBuffer.h"

namespace nsb {
    NAN_METHOD(SharedBuffer) {
        using namespace Nan;

        Utf8String name(info[0]);

        if(name.length() <= 0) {
            return ThrowTypeError("name must be a non-empty string");
        }

        unsigned int size = info[1]->Uint32Value();
        Buffer buffer(*name, size);
        auto nodeBuffer = NewBuffer(buffer.address(), size, Buffer::free, nullptr);

        info.GetReturnValue().Set(nodeBuffer.ToLocalChecked());
    }

    NAN_MODULE_INIT(Init) {
        NAN_EXPORT(target, getSharedBuffer);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
}
