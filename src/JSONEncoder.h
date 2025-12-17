//
// Created by gnilk on 17.09.25.
//

#ifndef GNILK_JSONENCODER_H
#define GNILK_JSONENCODER_H

#include <vector>
#include "IEncoder.h"
#include "StringWriter.h"

namespace gnilk {

    class JSONEncoder : public BaseEncoder {
    public:
        JSONEncoder() = default;
        explicit JSONEncoder(IWriter::Ref outStream);
        virtual ~JSONEncoder() = default;

        void PrettyPrint(bool use);

        // This is an optional call to set the writer
        void Begin(IWriter::Ref outStream) override;

        // This is used by the messages
        void BeginObject(const std::string &name) override;
        void EndObject() override;

        // Note: 'hasNext' is deprecated, parameter ignored
        void WriteBoolField(const std::string &name, bool value) override;
        void WriteIntField(const std::string &name, int value) override;
        void WriteInt64Field(const std::string &name, int64_t value) override;
        void WriteFloatField(const std::string &name, double value) override;
        void WriteTextField(const std::string &name, const std::string &value) override;

//        void BeginArray(const char *name, IEncoder::kArrayTypeSpec typeSpec) override;
//        void EndArray(bool hasNext = false) override;
    private:
        void WriteFieldSeparator();
    private:
        bool pretty = false;
        std::string eol = "";
        StringWriter ss;
        std::vector<int> fieldStack;
        int fieldCount;
    };
}


#endif //GNILK_JSONENCODER_H
