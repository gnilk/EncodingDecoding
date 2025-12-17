//
// Created by gnilk on 17.12.2025.
//

#ifndef ENCDEC_STRINGREADER_H
#define ENCDEC_STRINGREADER_H

#include <string>
#include <string.h>
#include "IReader.h"

namespace gnilk {
    class StringReader : public IReader {
    public:
        using Ref = StringReader::Ref;
    public:
        StringReader() = delete;
        StringReader(const std::string &buffer) : data(buffer), idx(0) {

        }
        virtual ~StringReader() = default;

        static IReader::Ref Create(const std::string &buffer) {
            return std::make_shared<StringReader>(buffer);
        }

        int32_t Read(void *out, size_t maxbytes) override {
            int32_t ncopy = maxbytes;
            if ((idx + ncopy) >= data.size()) {
                ncopy = data.size() - idx;
            }
            memcpy(out, &data[idx], ncopy);
            idx += ncopy;
            return ncopy;
        }
        bool Available() override {
            return (idx < data.size());
        };


    private:
        size_t idx = 0;
        const std::string &data;
    };
}


#endif //ENCDEC_STRINGREADER_H
