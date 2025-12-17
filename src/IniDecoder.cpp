//
// Created by gnilk on 17.12.2025.
//

#include "DecoderHelpers.h"
#include "IniDecoder.h"

using namespace gnilk;
IniDecoder::IniDecoder(IReader::Ref incoming) {
    parser = IniParser::Create(incoming);
    Initialize();
}

IniDecoder::IniDecoder(const std::string &data)  {
    parser = IniParser::Create(data);
    Initialize();
}

void IniDecoder::Begin(IReader::Ref incoming) {
    if (!parser) {
        parser = IniParser::Create(incoming);
    }
    Initialize();
}
void IniDecoder::Begin(const std::string &data) {
    if (!parser) {
        parser = IniParser::Create(data);
    }
    Initialize();
}

void IniDecoder::Initialize() {
    parser->ProcessData();
}

bool IniDecoder::BeginObject(const std::string &name) {
    auto section = parser->GetSection(name);
    if (section == nullptr) {
        return false;
    }
    objectStack.push(section);
    currentSection = objectStack.top();
    return true;
}

bool IniDecoder::HasObject(const std::string &name) {
    auto s = parser->GetSection(name);
    return (s != nullptr);
}

void IniDecoder::EndObject() {
    objectStack.pop();
    if (!objectStack.empty()) {
        currentSection = objectStack.top();
    }
}

std::optional<bool> IniDecoder::ReadBoolField(const std::string &name) {
    return {};
}
std::optional<int> IniDecoder::ReadIntField(const std::string &name) {
    return {};
}
std::optional<int64_t> IniDecoder::ReadInt64Field(const std::string &name) {
    return {};
}
std::optional<float> IniDecoder::ReadFloatField(const std::string &name) {
    return {};
}
std::optional<std::string> IniDecoder::ReadTextField(const std::string &name) {
    return {};
}
