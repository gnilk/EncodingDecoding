//
// Created by gnilk on 4/29/22.
//

#ifndef GNILK_JSONPARSER_H
#define GNILK_JSONPARSER_H

#include <functional>
#include <vector>
#include <string>
#include <utility>
#include <stack>
#include <optional>
#include <memory>
#include <variant>

#include "IReader.h"
#include "JSONParser.h"


namespace gnilk {
    class JSONValue;
    using JSONValueRef = std::shared_ptr<JSONValue>;

    class JSONCoreObject {
    public:
        using Ref = std::shared_ptr<JSONCoreObject>;
    public:
        virtual ~JSONCoreObject() = default;

        virtual void AddValue(const std::string &label, const JSONValueRef &value) = 0;
    };

    class JSONObject : public JSONCoreObject {
    public:
        using Ref = std::shared_ptr<JSONObject>;
    public:
        JSONObject() = default;
        explicit JSONObject(const std::string &objName) : name(objName) {

        }
        virtual ~JSONObject() = default;

        static JSONObject::Ref Create(const std::string &objName = {}) {
            return std::make_shared<JSONObject>(objName);
        }

        void AddValue(const std::string &label, const JSONValueRef &value) override {
            values[label] = value;
        }

        bool IsEmpty() const {
            return values.empty();
        }

        const std::string &GetName() const {
            return name;
        }

        bool HasValue(const std::string &valueName) const {
            return values.contains(valueName);
        }

        const JSONValueRef GetValue(const std::string &valueName) const {
            if (!values.contains(valueName)) {
                return {};
            }
            return values.at(valueName);
        }
        [[nodiscard]]
        const std::unordered_map<std::string, JSONValueRef> &GetValues() const {
            return values;
        }


    protected:
        std::string name;
        std::unordered_map<std::string, JSONValueRef> values;
    };

    class JSONArray : public JSONCoreObject {
    public:
        using Ref = std::shared_ptr<JSONArray>;
    public:
        JSONArray() = default;
        explicit JSONArray(const std::string &arrayName) : name(arrayName) {

        }
        virtual ~JSONArray() = default;

        static JSONArray::Ref Create(const std::string &arrayName = {}) {
            return std::make_shared<JSONArray>(arrayName);
        }

        void AddValue(const std::string &label, const JSONValueRef &value) override {
            values.push_back(value);
        }

        bool IsEmpty() const {
            return values.empty();
        }

        size_t Size() const {
            return values.size();
        }
        const std::vector<JSONValueRef> &GetValues() const {
            return values;
        }

        const std::string &GetName() const {
            return name;
        }

        const JSONValueRef At(size_t idx) const {
            if (idx >= values.size()) {
                return {};
            }
            return values.at(idx);
        }

    protected:
        std::string name;
        std::vector<JSONValueRef> values = {};
    };

    class JSONValue {
    public:
        using Ref = JSONValueRef;
    public:
        JSONValue() = default;

        virtual ~JSONValue() = default;

        template<typename T>
        static JSONValue::Ref Create(T &data) {
            auto refValue = std::make_shared<JSONValue>();
            refValue->value = data;
            return refValue;
        }
        template<typename T>
        const T& As() const {
            if constexpr (std::is_same_v<T, JSONObject::Ref>) {
                auto res = std::get_if<JSONObject::Ref>(&value);
                if (res == nullptr) {
                    return {};
                }
                return *res;
            }

            if constexpr (std::is_same_v<T, JSONArray::Ref>) {
                auto res = std::get_if<JSONArray::Ref>(&value);
                if (res == nullptr) {
                    return {};
                }
                return *res;
            }

            if constexpr (std::is_same_v<T, std::string>) {
               auto res = std::get_if<std::string>(&value);
               if (res == nullptr) {
                   return {};
               }
               return *res;
            }
        }

        bool IsObject() const {
            return (std::holds_alternative<JSONObject::Ref>(value));
        }
        bool IsArray() const {
            return (std::holds_alternative<JSONArray::Ref>(value));
        }
        bool IsString() const {
            return (std::holds_alternative<std::string>(value));
        }

        const JSONObject::Ref &GetAsObject() {
            auto res = std::get_if<JSONObject::Ref>(&value);
            if (res == nullptr) {
                static JSONObject::Ref empty = {};
                return empty;
            }
            return *res;
        }
        const JSONArray::Ref &GetAsArray() {
            auto res = std::get_if<JSONArray::Ref>(&value);
            if (res == nullptr) {
                static JSONArray::Ref empty = {};
                return empty;
            }
            return *res;
        }
        const std::string &GetAsString() {
            auto res = std::get_if<std::string>(&value);
            if (res == nullptr) {
                static std::string empty = {};
                return empty;
            }
            return *res;
        }

    protected:
        std::variant<JSONObject::Ref, JSONArray::Ref, std::string> value;
    };


    class JSONParser;

    class JSONDoc {
        friend JSONParser;
    public:
        JSONDoc() = default;
        virtual ~JSONDoc() = default;

        const std::variant<JSONObject::Ref, JSONArray::Ref> &GetRoot() const {
            return root;
        }
    protected:
        std::variant<JSONObject::Ref, JSONArray::Ref> root;
    };

    class JSONParser {
    public:
        using ValueDelegate = std::function<void(const char *object, const char *name, const char *value)>;
        enum class kResult {
            Ok,
            ErrUnexpectedEOF,
            ErrKeyMissing,
            ErrUnexpectedToken,
            ErrSeparatorMissing,
            ErrMaxDepth,
        };
    public:
        // Note: Factory is unsued - these constructors are only here for API compatibility right now..
        //       should be removed in the future...

        // We can't use default CTOR!
        JSONParser() = delete;

        JSONParser(IReader::Ref stream);
        JSONParser(IReader::Ref stream, ValueDelegate valueDelegate);
        JSONParser(const std::string &data);
        JSONParser(const std::string &data, ValueDelegate valueDelegate);
        virtual ~JSONParser() = default;


        std::unique_ptr<JSONDoc> GetDocument();
        static std::unique_ptr<JSONDoc> Load(const std::string &data);
        static std::unique_ptr<JSONDoc> Load(IReader::Ref stream);

        const std::string &ErrToString(JSONParser::kResult err);

    protected:
        void SetValueDelegate(ValueDelegate valueDelegate) { cbValue = valueDelegate; }
        JSONParser::kResult ProcessData();

        JSONParser::kResult ProcessDataInternal();
        JSONParser::kResult ProcessObject(JSONObject::Ref &currentObject, size_t depth);
        JSONParser::kResult ProcessArray(JSONArray::Ref &currentObject, const std::string &label, size_t depth);
        JSONParser::kResult ProcessString();
        bool IsValidNumberStart(int ch);
        JSONParser::kResult ProcessNumber(int ch);
        JSONParser::kResult ProcessValue(int ch, const std::string &label, JSONCoreObject::Ref currentObject, size_t depth);
        JSONParser::kResult ProcessExpected(int ch, const char *expected);
        int SkipWhiteSpace();

        void OnValue(JSONCoreObject::Ref currentObject, const std::string &label);
    private:

        void ResetCurrentValue();
        void AppendToValue(int ch);

        JSONObject::Ref CreateJSONObject(const std::string &name);
        JSONArray::Ref CreateJSONArray(const std::string &name);


        void Reset();
        int Next();
        int Peek();
        int ReadNext();
    private:
        IReader::Ref inStream = nullptr;
        ValueDelegate cbValue = nullptr;

        int lookAhead = -1;         // one char look-ahead buffer, -1 signal's nothing in the buffer..
        int idxParser = 0;

        int idxValueCurrent = 0;
        std::string valueCurrent = {};

        std::unique_ptr<JSONDoc> document;
    };

}


#endif //JSON_JSONDECODERNEW_H
