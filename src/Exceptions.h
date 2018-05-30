//
// Created by fathy on 26/05/2018.
//

#ifndef NODE_SHARED_BUFFER_ERRORS_H
#define NODE_SHARED_BUFFER_ERRORS_H

#include <string>
#include <exception>

namespace nsb {
    // Special exception which we handle, all other types are handled as unknown
    class AddonException: public std::exception {
    public:
        explicit AddonException(std::string message):
            message(std::move(message))
        {}

        const char* what() const noexcept override {
            return message.c_str();
        }
    private:
        std::string message;
    };

    class StdException: public AddonException {
    public:
        explicit StdException(int error):
            AddonException("System error: " + std::string(strerror(error)))
        {}
    };
}

#endif //NODE_SHARED_BUFFER_ERRORS_H
