//
// Created by gnilk on 17.12.2025.
//

#include "DecoderHelpers.h"
#include "JSONDecoder.h"

using namespace gnilk;

static JSONObject::Ref FindObject(const JSONObject::Ref &root, const std::string &name);
static JSONArray::Ref FindArray(const JSONObject::Ref &root, const std::string &name);

void JSONDecoder::Begin(IReader::Ref incoming) {
    doc = JSONParser::Load(incoming);
    Initialize();
}

void JSONDecoder::Begin(const std::string &jsonData) {
    doc = JSONParser::Load(jsonData);
    Initialize();
}

void JSONDecoder::Initialize() {
    if (doc == nullptr) {
        return;
    }
    // Make sure we push the first state...
    ChangeState(kState::kRegular);
}

bool JSONDecoder::BeginObject(const std::string &name) {
    // If we are in an array - we treat this differently...
    if (state == kState::kInArray) {
        auto &arrayWorkItem = arrStack.top();
        return BeginObject(std::dynamic_pointer_cast<JSONArrayIterator>(arrayWorkItem.iterator));
    }

    if (objStack.empty()) {
        // If stack is empty - we assume the first call to 'BeginObject' is the actual object we want to deserialize...
        auto root = doc->GetRoot();
        auto rootObject = std::get_if<JSONObject::Ref>(&root);
        if (rootObject == nullptr) {
            // this is an array - and we don't yet process it..
            return false;
        }
        objStack.push(*rootObject);
        ChangeState(kState::kRegular);
        return true;
    }

    auto node = FindObject(objStack.top(), name);
    if (node == nullptr) {
        return false;
    }
    ChangeState(kState::kRegular);
    objStack.push(node);
    return true;
}

bool JSONDecoder::BeginObject(const JSONArrayIterator::Ref &it) {
    // Dynamic cast might have gone wrong...
    if (it == nullptr) return false;

    auto value = it->GetValue();
    if ((value == nullptr) || (!value->IsObject())) {
        return false;
    }

    auto object = value->GetAsObject();
    ChangeState(kState::kRegular);
    objStack.push(object);
    return true;
}

void JSONDecoder::EndObject()  {
    objStack.pop();
    stateStack.pop();
    state = stateStack.top();
}

bool JSONDecoder::HasObject(const std::string &name) {
    if (objStack.empty()) {
        return false;
    }
    // We don't have named objects while traversing the array
    if (state == kState::kInArray) {
        return false;
    }
    if (FindObject(objStack.top(), name) == nullptr) {
        return false;
    }
    return true;
}



IDecoder::ArrayIterator::Ref JSONDecoder::BeginArray(const std::string &name) {
    if (state == kState::kInArray) {
        // FIXME: Need special handling
        return nullptr;
    }

    if (objStack.empty()) {
        auto root = doc->GetRoot();
        auto rootObject = std::get_if<JSONArray::Ref>(&root);
        if (rootObject == nullptr) {
            // we don't have something so return a dummy iterator...
            return BaseDecoder::BeginArray("");
        }
        auto it = JSONArrayIterator::Create(*rootObject);
        arrStack.push({it, *rootObject});
        ChangeState(kState::kInArray);
        return it;
    }

    auto &value = objStack.top()->GetValue(name);
    if (!value->IsArray()) {
        ChangeState(kState::kInArray);      // Need to do this - since they will/should call 'EndArray'
        return BaseDecoder::BeginArray("");
    }

    auto &arrayNode = value->GetAsArray();
    auto it = JSONArrayIterator::Create(arrayNode);

    ChangeState(kState::kInArray);
    arrStack.push({it, arrayNode});

    return it;
}

// This is for nested array's at the iteration point...
IDecoder::ArrayIterator::Ref JSONDecoder::BeginArray(const JSONArrayIterator::Ref &it) {
    return nullptr;
}

void JSONDecoder::EndArray() {
    stateStack.pop();
    arrStack.pop();
    state = stateStack.top();
}


std::optional<bool> JSONDecoder::ReadBoolField(const std::string &name) {
    if (objStack.empty()) return {};
    // We know this exists
    auto &value = objStack.top()->GetValue(name);
    if (value == nullptr) {
        return {};
    }
    // All except Object & Array are stored as strings...
    if (!value->IsString()) {
        return {};
    }

    auto &strValue = value->GetAsString();
    auto res = convert_to<bool>(strValue);

    return res;
}
std::optional<int> JSONDecoder::ReadIntField(const std::string &name) {
    if (objStack.empty()) return {};
    // We know this exists
    auto &value = objStack.top()->GetValue(name);
    if (value == nullptr) {
        return {};
    }
    // All except Object & Array are stored as strings...
    if (!value->IsString()) {
        return {};
    }

    auto &strValue = value->GetAsString();
    auto res = convert_to<int>(strValue);

    return res;
}
std::optional<int64_t> JSONDecoder::ReadInt64Field(const std::string &name) {
    return {};
}

std::optional<float> JSONDecoder::ReadFloatField(const std::string &name) {
    if (objStack.empty()) return {};
    // We know this exists
    auto &value = objStack.top()->GetValue(name);
    if (value == nullptr) {
        return {};
    }
    // All except Object & Array are stored as strings...
    if (!value->IsString()) {
        return {};
    }

    auto &strValue = value->GetAsString();
    auto res = convert_to<float>(strValue);

    return res;
}

std::optional<std::string> JSONDecoder::ReadTextField(const std::string &name) {
    if (objStack.empty()) return {};
    // We know this exists
    auto &value = objStack.top()->GetValue(name);
    if (value == nullptr) {
        return {};
    }
    // All except Object & Array are stored as strings...
    if (!value->IsString()) {
        return {};
    }

    auto &strValue = value->GetAsString();
    return {strValue};
}


// herlps
static JSONObject::Ref FindObject(const JSONObject::Ref &root, const std::string &name) {
    if (root->GetName() == name) return root;

    for(auto &[memberName, value] : root->GetValues()) {
        if (memberName != name) continue;
        if (value->IsObject()) return value->GetAsObject();
    }

    return {};
}
