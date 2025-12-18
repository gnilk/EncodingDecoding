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
    if (currentSection == nullptr) {
        return {};
    }
    for(auto &[key, value] : currentSection->values) {
        if (key == name) {
            return {convert_to<bool>(value)};
        }
    }
    return {};
}

std::optional<int> IniDecoder::ReadIntField(const std::string &name) {
    if (currentSection == nullptr) {
        return {};
    }
    for(auto &[key, value] : currentSection->values) {
        if (key == name) {
            return {convert_to<int>(value)};
        }
    }
    return {};
}

std::optional<int64_t> IniDecoder::ReadInt64Field(const std::string &name) {
    if (currentSection == nullptr) {
        return {};
    }
    for(auto &[key, value] : currentSection->values) {
        if (key == name) {
            return {convert_to<int64_t>(value)};
        }
    }
    return {};
}

std::optional<float> IniDecoder::ReadFloatField(const std::string &name) {
    if (currentSection == nullptr) {
        return {};
    }
    for(auto &[key, value] : currentSection->values) {
        if (key == name) {
            return {convert_to<float>(value)};
        }
    }
    return {};
}
std::optional<std::string> IniDecoder::ReadTextField(const std::string &name) {
    if (currentSection == nullptr) {
        return {};
    }
    for(auto &[key, value] : currentSection->values) {
        if (key == name) {
            return {value};
        }
    }
    return {};
}

// Support for Unmarshalling through IUnmarshal - this makes life SO much easier...
bool IniDecoder::Unmarshal(IUnmarshal *rootObject) {
    for (auto &[name, section] : parser->sectionMap) {
        if (section == nullptr) {
            return false;
        }
        auto workObject = rootObject->GetUnmarshalForField(section->name);
        for(auto &[fieldName, fieldValue] : section->values) {
            workObject->SetField(fieldName, fieldValue);
        }
    }
    return true;
}
