//
// Created by gnilk on 17.12.2025.
//

#include <testinterface.h>
#include "JSONDecoder.h"
#include "IDeserializable.h"

class MyOtherObject : public gnilk::IDeserializable {
public:
    MyOtherObject() = default;
    virtual ~MyOtherObject() = default;

    void DeserializeFrom(gnilk::IDecoder &decoder) override {
        if (!decoder.BeginObject("subobj")) {
            return;
        }
        auto optNum = decoder.ReadIntField("other_num");
        if (optNum.has_value()) {
            num = *optNum;
        } else {
            num = -1;
        }

        decoder.EndObject();
    }
public:
    int num;
};

class MyRootObject : public gnilk::IDeserializable {
public:
    MyRootObject() = default;
    virtual ~MyRootObject() = default;

    void DeserializeFrom(gnilk::IDecoder &decoder) override {
        if (!decoder.BeginObject("MyRootObject")) {
            return;
        }
        auto optNum = decoder.ReadIntField("num");
        if (optNum.has_value()) {
            num = *optNum;
        } else {
            num = -1;
        }
        other.DeserializeFrom(decoder);
        decoder.EndObject();
    }
public:
    int num;
    MyOtherObject other;
};

class MyRootArray : public gnilk::IDeserializable {
public:
    MyRootArray() = default;
    virtual ~MyRootArray() = default;

    void DeserializeFrom(gnilk::IDecoder &decoder) override {
        auto it = decoder.BeginArray("MyRootArray");
        while(!it->End()) {
            numbers.push_back(it->ReadInt());
            it->Next();
        }
        decoder.EndArray();
    }
public:
    std::vector<int> numbers;
};

class MyRootArrayObjects : public gnilk::IDeserializable {
public:
    MyRootArrayObjects() = default;
    virtual ~MyRootArrayObjects() = default;

    void DeserializeFrom(gnilk::IDecoder &decoder) override {
        auto it = decoder.BeginArray("MyRootArray");
        while(!it->End()) {
            //auto item = it->Get();
            if (it->IsObject()) {
                MyRootObject other;
                other.DeserializeFrom(decoder);
                objects.push_back(other);
            }
            it->Next();
        }
        decoder.EndArray();
    }
public:
    std::vector<MyRootObject> objects;
};



using namespace gnilk;

extern "C" int test_jsondecoder_object_simple(ITesting *t) {
    static std::string data = "{ " \
                              "\"num\" : 1 " \
                              "}";

    JSONDecoder decoder;
    MyRootObject myObj;
    decoder.Begin(data);
    myObj.DeserializeFrom(decoder);
    TR_ASSERT(t, myObj.num == 1);
    return kTR_Pass;
}

extern "C" int test_jsondecoder_object_wobject(ITesting *t) {
    static std::string data = "{ " \
                              "\"subobj\" : { \"other_num\" : 1 } " \
                              "}";

    JSONDecoder decoder;
    MyRootObject myObj;
    decoder.Begin(data);
    myObj.DeserializeFrom(decoder);
    TR_ASSERT(t, myObj.num == -1);
    TR_ASSERT(t, myObj.other.num == 1);
    return kTR_Pass;
}

extern "C" int test_jsondecoder_array_empty(ITesting *t) {
    static std::string data = "[] ";

    JSONDecoder decoder;
    MyRootArray myObj;
    decoder.Begin(data);
    myObj.DeserializeFrom(decoder);
    TR_ASSERT(t, myObj.numbers.size() == 0);
    return kTR_Pass;
}

extern "C" int test_jsondecoder_array_numbers(ITesting *t) {
    static std::string data = "[1,2,3] ";

    JSONDecoder decoder;
    MyRootArray myObj;
    decoder.Begin(data);
    myObj.DeserializeFrom(decoder);
    TR_ASSERT(t, myObj.numbers.size() == 3);
    TR_ASSERT(t, myObj.numbers[0] == 1);
    TR_ASSERT(t, myObj.numbers[1] == 2);
    TR_ASSERT(t, myObj.numbers[2] == 3);
    return kTR_Pass;
}

extern "C" int test_jsondecoder_array_objects(ITesting *t) {
    static std::string data = "[{ \"num\" : 1}, {\"num\" : 2}, {\"num\" :3}] ";

    JSONDecoder decoder;
    MyRootArrayObjects myObj;
    decoder.Begin(data);
    myObj.DeserializeFrom(decoder);
    TR_ASSERT(t, myObj.objects.size() == 3);
    TR_ASSERT(t, myObj.objects[0].num == 1);
    TR_ASSERT(t, myObj.objects[1].num == 2);
    TR_ASSERT(t, myObj.objects[2].num == 3);
    return kTR_Pass;
}