//
// Created by gnilk on 18.12.2025.
//
#include <testinterface.h>
#include "../src/JSONDecoder.h"
#include "../src/IUnmarshal.h"
#include "../src/DecoderHelpers.h"

using namespace gnilk;

namespace {
    template<typename T>
    class TestJSONField : public IUnmarshal {
    public:
        TestJSONField() : convert([](const std::string &value) { return *convert_to<T>(value); }) {

        }
        explicit TestJSONField( std::function<T(const std::string &)> useConvert) : convert(useConvert) {}
        virtual ~TestJSONField() = default;
        bool SetField(const std::string &name, const std::string &value) override {
            printf("TestJSONField::SetField, name=%s, value=%s\n", name.c_str(), value.c_str());

            if (name == std::string("field")) {
                // Just assume this works, the actual test will verify the expected value anyway...
                //actual = *convert_to<T>(value);
                actual = convert(value);
                isEmpty = false;
                return true;
            }

            return false;
        };
        // Called to get an instance to a sub-object in your class
        IUnmarshal *GetUnmarshalForField(const std::string &name) override {
            return nullptr;
        }
        // Called to push things to an array you might have
        bool PushToArray(const std::string &name, IUnmarshal *pData) override {
            printf("SimpleArrayItem::PushToArray, name=%s\n", name.c_str());
            return true;;
        }
    public:
        std::function<T(const std::string &)> convert = {};
        bool isEmpty = true;
        T expected;
        T actual;
    };
}

extern "C" int test_jsonunmarshal_simple(ITesting *t) {
    return kTR_Pass;
}

extern "C" int test_jsonunmarshal_t_number(ITesting *t) {
    // Basic integer
    {
        std::string json = {"{\"field\":123}"};
        TestJSONField<int32_t> numberA;
        numberA.expected = 123;
        JSONDecoder decoderA(json);
        decoderA.Unmarshal(&numberA);

        TR_ASSERT(t, numberA.actual == numberA.expected);
    }
    // Basic float
    {
        std::string json = {"{\"field\":123.05}"};
        TestJSONField<float> numberA;
        numberA.expected = 123.05;
        JSONDecoder decoderA(json);
        decoderA.Unmarshal(&numberA);
        // NOTE: DO NOT COMPARE FLOAT's directly!!!
        TR_ASSERT(t, fabs(numberA.actual - numberA.expected) < 0.01);
    }
    // bool:true
    {
        std::string json = {"{\"field\":true}"};
        TestJSONField<bool> mBool;
        mBool.expected = true;
        JSONDecoder decoderTrue(json);
        decoderTrue.Unmarshal(&mBool);
        TR_ASSERT(t, mBool.actual == mBool.expected);
    }
    // bool:false
    {
        std::string json = {"{\"field\":false}"};
        TestJSONField<bool> mBool;
        mBool.expected = false;
        JSONDecoder decoderTrue(json);
        decoderTrue.Unmarshal(&mBool);
        TR_ASSERT(t, mBool.actual == mBool.expected);
    }
    // string
    {
        std::string json={"{\"field\":\"hello json\"}"};
        TestJSONField<std::string> numberA;
        numberA.expected = "hello json";
        JSONDecoder decoderA(json);
        decoderA.Unmarshal(&numberA);
        TR_ASSERT(t, numberA.actual == numberA.expected);

    }
    return kTR_Pass;
}

extern "C" int test_jsonunmarshal_t_null(ITesting *t) {
    std::string json={"{\"field\":null}"};
    TestJSONField<void *> ptrValue([](const std::string &value){ return (("null" == value)?nullptr:(void *)1); });
    ptrValue.expected = nullptr;
    JSONDecoder decoder(json);
    decoder.Unmarshal(&ptrValue);
    TR_ASSERT(t, ptrValue.actual == ptrValue.expected);

    return kTR_Pass;
}
extern "C" int test_jsonunmarshal_basic(ITesting *t) {
    std::string data("{{[}}");


    JSONDecoder jsonDecoder(data);
    TR_ASSERT(t, !jsonDecoder.IsValid());

//    // This should fail!!!
//    if (!jsonDecoder.ProcessData()) {
//        return kTR_Pass;
//    }
    return kTR_Pass;
}


namespace {
    class IntegerArrayUnmarshall : public IUnmarshal {
    public:
        bool SetField(const std::string &name, const std::string &value) override {
            printf("push: %s\n", value.c_str());
            num.push_back(*convert_to<int>(value));
            return true;
        }
        virtual IUnmarshal *GetUnmarshalForField(const std::string &pname) override { return nullptr; }

        virtual bool PushToArray(const std::string &name, IUnmarshal *pData) override {
            return false;
        }
        std::vector<int> num;
    };
}

extern "C" int test_jsonunmarshal_array_simple(ITesting *t) {
    IntegerArrayUnmarshall intmarshal;
    std::string data("[1,2,3,4]");     // Potentially we should fail on this...



    JSONDecoder decoder(data);
    TR_ASSERT(t, decoder.IsValid());
    decoder.Unmarshal(&intmarshal);

    TR_ASSERT(t, intmarshal.num.size() == 4);
    TR_ASSERT(t, intmarshal.num[0] == 1);
    TR_ASSERT(t, intmarshal.num[1] == 2);
    TR_ASSERT(t, intmarshal.num[2] == 3);
    TR_ASSERT(t, intmarshal.num[3] == 4);

    return kTR_Pass;
}

extern "C" int test_jsonunmarshal_array_trailing(ITesting *t) {
    IntegerArrayUnmarshall intmarshal;
    std::string data("[1,2,3,4,]");     // Potentially we should fail on this...

    JSONDecoder decoder(data);
    if (decoder.IsValid()) {
        return kTR_Fail;
    }
    return kTR_Pass;
}

namespace {
    class MyObject : public BaseUnmarshal {
    public:
        MyObject() = default;
        virtual ~MyObject() = default;
    public: // BaseUnmarshal
        bool SetField(const std::string &fieldName, const std::string &fieldValue) override {
            if (fieldName == "field") {
                value = *convert_to<int>(fieldValue);
                return true;
            }
            return false;
        }
        IUnmarshal *GetUnmarshalForField(const std::string &fieldName) override {
            if (fieldName == "Object") {
                subObject = new MyObject();
                return subObject;
            }
            return nullptr;
        }

    public:
        int value = 0;
        MyObject *subObject = {};
    };
}

extern "C" int test_jsonunmarshal_object_nested(ITesting *t) {
    std::string data = "{ \"field\" : 123, \"Object\" : { \"field\" : 345 } }";

    JSONDecoder decoder(data);
    TR_ASSERT(t, decoder.IsValid());
    MyObject root;
    TR_ASSERT(t, decoder.Unmarshal(&root));
    TR_ASSERT(t, root.value == 123);
    TR_ASSERT(t, root.subObject != nullptr);
    TR_ASSERT(t, root.subObject->value == 345);
    TR_ASSERT(t, root.subObject->subObject == nullptr);

    return kTR_Pass;
}

