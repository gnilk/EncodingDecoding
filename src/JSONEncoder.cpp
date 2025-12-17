//
// Created by gnilk on 17.09.25.
//
// JSON Marshalling/Encoding
//

#include "JSONEncoder.h"

using namespace gnilk;

static const std::string beginobj("{");
static const std::string endobj("}");
static const std::string beginarray("[");
static const std::string endarray("]");
static const std::string quote("\"");
static const std::string separator(":");
static const std::string nextfield(",");
static const std::string jsontrue("true");
static const std::string jsonfalse("false");
static const std::string jsonnull("null");
static const std::string jsonempty("");

JSONEncoder::JSONEncoder(IWriter::Ref outStream) : ss(outStream), fieldCount(0) {

}

void JSONEncoder::PrettyPrint(bool use) {
    pretty = use;
    eol = pretty ? gnilk::StringWriter::eol : "";
}

#define spacing() (pretty?std::string(fieldStack.size(), '\t') : jsonempty)

//std::string &JSONEncoder::spacing() {
//    return pretty ? std::string(fieldStack.size(), '\t') : jsonempty;
//}
// This is an optional call to set the writer
void JSONEncoder::Begin(IWriter::Ref outStream) {
    ss.Begin(writer);
    fieldCount = 0;
}

// This is used by the messages
void JSONEncoder::BeginObject(const std::string &name) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << beginobj << eol;
    } else {
        ss << spacing() << beginobj << eol;
    }
    fieldStack.push_back(fieldCount);
    fieldCount = 0;
}

void JSONEncoder::EndObject() {
    fieldCount = fieldStack.back();
    fieldStack.pop_back();

    ss << eol << spacing() << endobj;

    fieldCount += 1;
}


void JSONEncoder::WriteBoolField(const std::string &name, bool value) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << (value?jsontrue:jsonfalse);
    } else {
        ss << spacing() << (value?jsontrue:jsonfalse);
    }
}
void JSONEncoder::WriteIntField(const std::string &name, int value) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << std::to_string(value);
    } else {
        ss << spacing() << std::to_string(value);
    }
}
void JSONEncoder::WriteInt64Field(const std::string &name, int64_t value) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << std::to_string(value);
    } else {
        ss << spacing() << std::to_string(value);
    }
}

void JSONEncoder::WriteFloatField(const std::string &name, double value) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << std::to_string(value);
    } else {
        ss << spacing() << std::to_string(value);
    }
}
void JSONEncoder::WriteTextField(const std::string &name, const std::string &value) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << quote << value << quote;
    } else {
        ss << spacing() << quote << value << quote;
    }
}

/*
void JSONEncoder::BeginArray(const std::string &name, IEncoder::kArrayTypeSpec typeSpec) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << beginarray << eol;
    } else {
        ss << spacing() << beginarray << eol;
    }
    fieldStack.push_back(fieldCount);
    fieldCount = 0;
}
void JSONEncoder::EndArray() {
    fieldCount = fieldStack.back();
    fieldStack.pop_back();

    ss << eol << spacing() << endarray << eol;

    fieldCount += 1;
}
*/

void JSONEncoder::WriteFieldSeparator() {
    if (fieldStack.empty()) return;

    if (fieldCount > 0) {
        ss << nextfield << eol;
    }

    fieldCount+=1;
}
