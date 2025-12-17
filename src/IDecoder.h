//
// Created by gnilk on 16.12.25.
//

#ifndef GNILK_IDECODER_H
#define GNILK_IDECODER_H

#include "IReader.h"
#include <optional>
#include <memory>
#include <string>

namespace gnilk {

    class IDecoder {
    public:
        using Ref = std::shared_ptr<IDecoder>;
    public:
        virtual ~IDecoder() = default;

        virtual void Begin(IReader::Ref in) = 0;
        // Traverse data until the object with 'name' is found

        virtual bool BeginObject(const std::string &name) = 0;
        virtual void EndObject() = 0;
        virtual bool HasObject(const std::string &name) = 0;

        virtual std::optional<bool> ReadBoolField(const std::string &name) = 0;
        virtual std::optional<int> ReadIntField(const std::string &name) = 0;
        virtual std::optional<int64_t> ReadInt64Field(const std::string &name) = 0;
        virtual std::optional<float> ReadFloatField(const std::string &name) = 0;
        virtual std::optional<std::string> ReadTextField(const std::string &name) = 0;

    };

    class BaseDecoder : public IDecoder {
    public:
        BaseDecoder() = default;
        virtual ~BaseDecoder() = default;

        void Begin(IReader::Ref incoming) override {
            reader = incoming;
        }

        bool BeginObject(const std::string &name) override { return false; }
        void EndObject() override {}
        bool HasObject(const std::string &name) override { return false; };

        std::optional<bool> ReadBoolField(const std::string &name) override { return {}; }
        std::optional<int> ReadIntField(const std::string &name) override { return {}; }
        std::optional<int64_t> ReadInt64Field(const std::string &name) override { return {}; }
        std::optional<float> ReadFloatField(const std::string &name) override { return {}; }
        std::optional<std::string> ReadTextField(const std::string &name) override { return {}; }

    protected:
        IReader::Ref reader;
    };
}

#endif //GNILK_IDECODER_H
