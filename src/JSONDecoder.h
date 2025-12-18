//
// Created by gnilk on 17.12.2025.
//

#ifndef GNILK_JSONDECODER_H
#define GNILK_JSONDECODER_H

#include "IDecoder.h"
#include "IUnmarshal.h"
#include "JSONParser.h"
#include "DecoderHelpers.h"

namespace gnilk {
    class JSONDecoder : public BaseDecoder {
    public:
    class JSONArrayIterator : public IDecoder::ArrayIterator {
        friend JSONDecoder;
    protected:
        using Ref = std::shared_ptr<JSONArrayIterator>;
    public:
        JSONArrayIterator() = delete;
        JSONArrayIterator(JSONArray::Ref jsArray) : idxCurrent(0), array(jsArray) {

        }
        virtual ~JSONArrayIterator() = default;
    public:
        virtual void Next() override { ++idxCurrent; };
        virtual void Previous() override { --idxCurrent; };
        virtual bool Equals(const ArrayIterator::Ref &other) const override {
            auto itOther = std::dynamic_pointer_cast<JSONArrayIterator>(other);
            return (idxCurrent == itOther->idxCurrent);
        };
        virtual bool End() const override {
            if (array == nullptr) {
                return true;
            }
            return (idxCurrent == array->Size());
        }
    public:
        bool IsArray() override {
            auto &item = array->At(idxCurrent);
            return item->IsArray();
        }

        bool IsObject() override {
            auto &item = array->At(idxCurrent);
            return item->IsObject();
        }


        bool ReadBool() override {
            auto &item = array->At(idxCurrent);
            if (!item->IsString()) {
                return false;
            }
            auto out = convert_to<bool>(item->GetAsString());
            if (!out.has_value()) {
                return false;
            }
            return *out;
        }
        int ReadInt() override {
            auto &item = array->At(idxCurrent);
            if (!item->IsString()) {
                return -1;
            }
            auto out = convert_to<int>(item->GetAsString());
            if (!out.has_value()) {
                return -1;
            }
            return *out;
        }
        int64_t ReadInt64() override {
            auto &item = array->At(idxCurrent);
            if (!item->IsString()) {
                return -1;
            }
            auto out = convert_to<int64_t>(item->GetAsString());
            if (!out.has_value()) {
                return -1;
            }
            return *out;
        }
        float ReadFloat() override {
            auto &item = array->At(idxCurrent);
            if (!item->IsString()) {
                return -1;
            }
            auto out = convert_to<float>(item->GetAsString());
            if (!out.has_value()) {
                return -1;
            }
            return *out;
        }
        std::string ReadText() override {
            auto &item = array->At(idxCurrent);
            if (!item->IsString()) {
                return {};
            }
            return item->GetAsString();
        }

    protected:
        static ArrayIterator::Ref Create(JSONArray::Ref jsArray) {
            return std::make_shared<JSONArrayIterator>(jsArray);
        }
        JSONValue::Ref GetValue() {
            return array->At(idxCurrent);
        }
    protected:
        size_t idxCurrent;
        JSONArray::Ref array;
    };
    public:
        JSONDecoder() = default;
        explicit JSONDecoder(IReader::Ref incoming);
        explicit JSONDecoder(const std::string &jsondata);
        virtual ~JSONDecoder() = default;

        void Begin(IReader::Ref incoming) override;
        void Begin(const std::string &jsondata);

        bool IsValid() {
            return (doc != nullptr);
        }

        void Unmarshal(IUnmarshal *rootObject);

        bool BeginObject(const std::string &name) override;

        void EndObject() override;
        bool HasObject(const std::string &name) override;

        ArrayIterator::Ref BeginArray(const std::string &name) override;
        void EndArray() override;

        std::optional<bool> ReadBoolField(const std::string &name) override;
        std::optional<int> ReadIntField(const std::string &name) override;
        std::optional<int64_t> ReadInt64Field(const std::string &name) override;
        std::optional<float> ReadFloatField(const std::string &name) override;
        std::optional<std::string> ReadTextField(const std::string &name) override;
    protected:
        bool BeginObject(const JSONArrayIterator::Ref &it);
        ArrayIterator::Ref BeginArray(const JSONArrayIterator::Ref &it);
        void UnmarshalObject(IUnmarshal *pObject, const JSONObject::Ref &jsonObject);
        void UnmarshalArray(IUnmarshal *pObject, const JSONArray::Ref &jsonObject);

    protected:
        enum class kState {
            kRegular,
            kInArray,
        };

        void Initialize();
        void ChangeState(kState newState) {
            stateStack.push(newState);
            state = newState;
        };
    protected:
        struct ArrayWorkObject {
            ArrayIterator::Ref iterator;
            JSONArray::Ref array;
        };

        // Assume we are in regular state
        kState state = kState::kRegular;

        std::unique_ptr<JSONDoc> doc;
        // Only objects for now - arrays will come later...
        std::stack<JSONObject::Ref> objStack;
        //std::stack<ArrayWorkItem> arrStack;
        std::stack<ArrayWorkObject> arrStack;
        std::stack<kState> stateStack;




    };
}


#endif //ENCDEC_JSONDECODER_H
