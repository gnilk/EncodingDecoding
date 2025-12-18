//
// Created by gnilk on 17.12.2025.
//

#ifndef GNILK_INIDECODER_H
#define GNILK_INIDECODER_H

#include <string>
#include <memory>
#include <stack>

#include "IDecoder.h"
#include "IniParser.h"
#include "IUnmarshal.h"

namespace gnilk {
    class IniDecoder : public BaseDecoder {
    public:
        using Ref = std::shared_ptr<IniDecoder>;
    public:
        IniDecoder() = default;
        explicit IniDecoder(IReader::Ref incoming);
        explicit IniDecoder(const std::string &data);
        virtual ~IniDecoder() = default;

        bool Unmarshal(IUnmarshal *rootObject) override;


        void Begin(IReader::Ref incoming) override;
        void Begin(const std::string &data);

        bool BeginObject(const std::string &name) override;
        bool HasObject(const std::string &name) override;
        void EndObject() override;

        std::optional<bool> ReadBoolField(const std::string &name) override;
        std::optional<int> ReadIntField(const std::string &name) override;
        std::optional<int64_t> ReadInt64Field(const std::string &name) override;
        std::optional<float> ReadFloatField(const std::string &name) override;
        std::optional<std::string> ReadTextField(const std::string &name) override;
    protected:
        void Initialize();
    protected:
        IniParser::Ref parser;
        std::stack<IniParser::Section::Ref> objectStack;
        IniParser::Section::Ref currentSection = {};

    };
}


#endif //GNILK_INIDECODER_H
