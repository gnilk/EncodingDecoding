//
// Created by gnilk on 19.12.2025.
//
#include <string>
#include <testinterface.h>
#include "../src/XMLParser.h"

using namespace gnilk;

// FIXME: Failing tests - should work...
extern "C" int test_xmlparser_attrib_whitespace(ITesting *t) {
    static std::string data = "<node field = \"value\" />";
    auto doc = xml::XMLParser::Load(data);
    TR_ASSERT(t, doc != nullptr);
    auto root = doc->GetRoot();
    TR_ASSERT(t, root != nullptr);
    auto node = root->GetFirstChild("node");
    TR_ASSERT(t, node != nullptr);
    auto value = node->GetAttributeValue("field", "not_this");
    TR_ASSERT(t, value == "value");

    return kTR_Pass;
}

extern "C" int test_xmlparser_attrib_singlquote(ITesting *t) {
    static std::string data = "<node field=\'value\' />";
    auto doc = xml::XMLParser::Load(data);
    TR_ASSERT(t, doc != nullptr);
    auto root = doc->GetRoot();
    TR_ASSERT(t, root != nullptr);
    auto node = root->GetFirstChild("node");
    TR_ASSERT(t, node != nullptr);
    auto value = node->GetAttributeValue("field", "not_this");
    TR_ASSERT(t, value == "value");

    return kTR_Pass;
}

// Ok
extern "C" int test_xmlparser_attrib_single(ITesting *t) {
    static std::string data = "<node field=\"value\" />";
    auto doc = xml::XMLParser::Load(data);
    TR_ASSERT(t, doc != nullptr);
    auto root = doc->GetRoot();
    TR_ASSERT(t, root != nullptr);
    auto node = root->GetFirstChild("node");
    TR_ASSERT(t, node != nullptr);
    auto value = node->GetAttributeValue("field", "not_this");
    TR_ASSERT(t, value == "value");

    return kTR_Pass;
}

// Ok..
extern "C" int test_xmlparser_attrib_multi(ITesting *t) {
    static std::string data = "<node field=\"value\" field2=\"value2\"/>";
    auto doc = xml::XMLParser::Load(data);
    TR_ASSERT(t, doc != nullptr);
    auto root = doc->GetRoot();
    TR_ASSERT(t, root != nullptr);
    auto node = root->GetFirstChild("node");
    TR_ASSERT(t, node != nullptr);
    std::string value;
    value = node->GetAttributeValue("field", "not_this");
    TR_ASSERT(t, value == "value");

    value = node->GetAttributeValue("field2", "not_this");
    TR_ASSERT(t, value == "value2");

    return kTR_Pass;
}

