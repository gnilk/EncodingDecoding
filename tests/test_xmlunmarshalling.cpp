//
// Created by gnilk on 19.12.2025.
// Extend with more tests...
//
#include <testinterface.h>
#include "../src/XMLDecoder.h"
#include "../src/IUnmarshal.h"
#include "../src/DecoderHelpers.h"
#include <cmath>

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

extern "C" int test_xmlunmarshal_simple(ITesting *t) {
    return kTR_Pass;
}

// NOTE: THIS WILL FAIL!!!
extern "C" int test_xmlunmarshal_attrib_whitespace(ITesting *t) {
    std::string json = {"<root field = \"123\" />"};
    TestJSONField<int32_t> numberA;
    numberA.expected = 123;
    XMLDecoder decoderA(json);
    TR_ASSERT(t, decoderA.Unmarshal(&numberA));
    TR_ASSERT(t, numberA.actual == numberA.expected);

    return kTR_Pass;
}


extern "C" int test_xmlunmarshal_t_number(ITesting *t) {
    // Basic integer
    {
        std::string json = {"<root field=\"123\" />"};
        TestJSONField<int32_t> numberA;
        numberA.expected = 123;
        XMLDecoder decoderA(json);
        TR_ASSERT(t, decoderA.Unmarshal(&numberA));
        TR_ASSERT(t, numberA.actual == numberA.expected);
    }
    // Basic float
    {
        std::string json = {"<root field=\"123.05\" />"};
        TestJSONField<float> numberA;
        numberA.expected = 123.05;
        XMLDecoder decoderA(json);
        TR_ASSERT(t, decoderA.Unmarshal(&numberA));
        // NOTE: DO NOT COMPARE FLOAT's directly!!!
        TR_ASSERT(t, fabs(numberA.actual - numberA.expected) < 0.01);
    }
    // bool:true
    {
        std::string json = {"<root field=\"true\" />"};
        TestJSONField<bool> mBool;
        mBool.expected = true;
        XMLDecoder decoderTrue(json);
        decoderTrue.Unmarshal(&mBool);
        TR_ASSERT(t, mBool.actual == mBool.expected);
    }
    // bool:false
    {
        std::string json = {"<root field=\"false\" />"};
        TestJSONField<bool> mBool;
        mBool.expected = false;
        XMLDecoder decoderTrue(json);
        decoderTrue.Unmarshal(&mBool);
        TR_ASSERT(t, mBool.actual == mBool.expected);
    }
    // string
    {
        std::string json={"<root field=\"hello xml\" />"};
        TestJSONField<std::string> numberA;
        numberA.expected = "hello xml";
        XMLDecoder decoderA(json);
        decoderA.Unmarshal(&numberA);
        TR_ASSERT(t, numberA.actual == numberA.expected);

    }
    return kTR_Pass;
}
