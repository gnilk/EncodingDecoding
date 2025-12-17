//
// Created by gnilk on 17.12.2025.
//

//
// Test of the JSON decoder...
// TODO: tests for error detection path's...
//

#include <testinterface.h>
#include "../src/JSONParser.h"
using namespace gnilk;

extern "C" int test_jsonparser_object_empty(ITesting *t) {
    static std::string data = "{ }";
    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) == nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) != nullptr);

    auto rootObject = *std::get_if<JSONObject::Ref>(&root);
    TR_ASSERT(t, rootObject->IsEmpty());

    return kTR_Pass;
}

// Just a single string object
extern "C" int test_jsonparser_object_wstring(ITesting *t) {
    static std::string data = "{ " \
                              "\"string\" : \"value\" " \
                              "}";

    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) == nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) != nullptr);

    auto rootObject = *std::get_if<JSONObject::Ref>(&root);
    TR_ASSERT(t, rootObject->IsEmpty() != true);
    return kTR_Pass;
}


extern "C" int test_jsonparser_object_simple(ITesting *t) {
    static std::string data = "{ " \
                              "\"num\" : 1, " \
                              "\"string\" : \"value\", " \
                              "\"bool\" : true " \
                              "}";


    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) == nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) != nullptr);

    auto rootObject = *std::get_if<JSONObject::Ref>(&root);
    TR_ASSERT(t, rootObject->IsEmpty() != true);

    TR_ASSERT(t, rootObject->HasValue("num"));
    TR_ASSERT(t, rootObject->HasValue("string"));
    TR_ASSERT(t, rootObject->HasValue("bool"));

    auto numValue = rootObject->GetValue("num");
    auto stringValue = rootObject->GetValue("string");
    auto boolValue = rootObject->GetValue("bool");

    TR_ASSERT(t, numValue->IsString());
    TR_ASSERT(t, stringValue->IsString());
    TR_ASSERT(t, boolValue->IsString());

    return kTR_Pass;
}

extern "C" int test_jsonparser_object_warray(ITesting *t) {
    static std::string data = "{ " \
                              "\"array\" : [1,2,3] " \
                              "}";


    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) == nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) != nullptr);

    auto rootObject = *std::get_if<JSONObject::Ref>(&root);
    TR_ASSERT(t, rootObject->IsEmpty() != true);
    auto arrayValue = rootObject->GetValue("array");
    TR_ASSERT(t, arrayValue != nullptr);
    TR_ASSERT(t, arrayValue->IsArray());
    return kTR_Pass;
}

extern "C" int test_jsonparser_object_wobject(ITesting *t) {
    static std::string data = "{ " \
                              "\"obj\" : { \"num\" : 1 } " \
                              "}";


    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) == nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) != nullptr);

    auto rootObject = *std::get_if<JSONObject::Ref>(&root);
    TR_ASSERT(t, rootObject->IsEmpty() != true);

    auto &objectValue = rootObject->GetValue("obj");
    TR_ASSERT(t, objectValue != nullptr);
    TR_ASSERT(t, objectValue->IsObject());

    auto &subObject = objectValue->GetAsObject();
    TR_ASSERT(t, subObject != nullptr);
    TR_ASSERT(t, subObject->HasValue("num"));
    auto &subObjValue = subObject->GetValue("num");
    TR_ASSERT(t, subObjValue != nullptr);
    TR_ASSERT(t, subObjValue->IsString());
    auto &strValue = subObjValue->GetAsString();
    TR_ASSERT(t, strValue == "1");

    return kTR_Pass;
}


extern "C" int test_jsonparser_array_empty(ITesting *t) {
    static std::string data = "[ ]";
    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) != nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) == nullptr);

    auto rootArray = *std::get_if<JSONArray::Ref>(&root);
    TR_ASSERT(t, rootArray->IsEmpty());
    return kTR_Pass;
}

extern "C" int test_jsonparser_array_simple(ITesting *t) {
    static std::string data = "[1,2,3]";
    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) != nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) == nullptr);

    auto rootArray = *std::get_if<JSONArray::Ref>(&root);
    TR_ASSERT(t, !rootArray->IsEmpty());
    TR_ASSERT(t, rootArray->Size() == 3);

    auto &v1 = rootArray->At(0);
    TR_ASSERT(t, v1 != nullptr);
    auto &v2 = rootArray->At(1);
    TR_ASSERT(t, v2 != nullptr);
    auto &v3 = rootArray->At(2);
    TR_ASSERT(t, v3 != nullptr);

    // Out of bounds
    auto &v4 = rootArray->At(3);
    TR_ASSERT(t, v4 == nullptr);

    TR_ASSERT(t, v1->IsString());
    TR_ASSERT(t, v2->IsString());
    TR_ASSERT(t, v3->IsString());

    TR_ASSERT(t, v1->GetAsString() == "1");
    TR_ASSERT(t, v2->GetAsString() == "2");
    TR_ASSERT(t, v3->GetAsString() == "3");

    return kTR_Pass;
}

extern "C" int test_jsonparser_array_wobjects(ITesting *t) {
    static std::string data = "[{},{},{}]";
    auto doc = JSONParser::Load(data);
    TR_ASSERT(t, doc.get() != nullptr);

    auto root = doc->GetRoot();
    TR_ASSERT(t, std::get_if<JSONArray::Ref>(&root) != nullptr);
    TR_ASSERT(t, std::get_if<JSONObject::Ref>(&root) == nullptr);

    auto rootArray = *std::get_if<JSONArray::Ref>(&root);
    TR_ASSERT(t, !rootArray->IsEmpty());
    TR_ASSERT(t, rootArray->Size() == 3);

    for(size_t i=0;i<rootArray->Size();i++) {
        auto &value = rootArray->At(i);
        TR_ASSERT(t, value!= nullptr);
        TR_ASSERT(t, value->IsObject());
        auto &object = value->GetAsObject();
        TR_ASSERT(t, object != nullptr);
        TR_ASSERT(t, object->IsEmpty());
    }
    return kTR_Pass;
}