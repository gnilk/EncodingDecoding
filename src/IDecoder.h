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
        class ArrayIterator {
        public:
            using Ref = std::shared_ptr<ArrayIterator>;
        public:
            ArrayIterator() = default;
            virtual ~ArrayIterator() = default;

            ArrayIterator &operator++ () { Next(); return *this; }
            ArrayIterator operator++ (int) { ArrayIterator tmp = *this; Next(); return tmp; }
            ArrayIterator &operator-- () { Previous(); return *this; }
            bool operator == (const ArrayIterator::Ref &other) const { return Equals(other); }
            bool operator != (const ArrayIterator::Ref &other) const { return !Equals(other); }

        public:
            virtual void Next() {};
            virtual void Previous() {};
            virtual bool Equals(const ArrayIterator::Ref &other) const { return false; };
            virtual bool End() const { return true; }
        public:
            virtual bool IsArray() { return false; }
            virtual bool IsObject() { return false; }

            // Read current item as an Int...
            virtual bool ReadBool() { return false; }
            virtual int ReadInt() { return 0; }
            virtual int64_t ReadInt64() { return 0; }
            virtual float ReadFloat() { return 0.0; }
            virtual std::string ReadText() { return {}; }
        };
    public:
        virtual ~IDecoder() = default;

        virtual void Begin(IReader::Ref in) = 0;
        // Traverse data until the object with 'name' is found

        virtual bool BeginObject(const std::string &name) = 0;
        virtual void EndObject() = 0;
        virtual bool HasObject(const std::string &name) = 0;

        virtual ArrayIterator::Ref BeginArray(const std::string &name) = 0;
        virtual void EndArray() = 0;


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

        ArrayIterator::Ref BeginArray(const std::string &name) override {  return std::make_shared<ArrayIterator>(); }
        void EndArray() override {  }

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
