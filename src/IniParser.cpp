//
// Created by gnilk on 12/12/2022.
//

#include "IniParser.h"
#include "StringReader.h"

using namespace gnilk;

IniParser::IniParser(const std::string &data) {
    inStream = StringReader::Create(data);
}
IniParser::IniParser(IReader::Ref stream) : inStream(stream)
{

}

bool IniParser::ProcessData() {
    bool res = false;
    state = kSectionStart;

    ResetKey();
    ResetValue();

    int32_t nread = 0;
    char next;
    // FIXME: rewrite as 'while(inStream->Available()) {
    while((nread = inStream->Read((uint8_t *)&next, sizeof(next))) > 0) {
        switch(state) {
            case kUnknown :
                // printf("Unknown state, this should not happen - developer error...\n");
                goto leave;
            case kSectionStart :
                if (next == '[') {
                    state = kSectionName;
                    ResetSection();
                } else if (!std::isspace(next)) {
                    // No starting section, assume this all goes directly to the object supplied
                    state = kKey;
                    ResetKey();
                    ResetValue();
                    PushToKey(next);
                }
                break;
            case kSectionName :
                if (next == ']') {
                    // TODO: resolve current object from token
                    // printf("Root=%s\n", section.CharPtr());
                    state = kWhiteSpace;
                    stateAfterWhiteSpace = kKey;

                    ResetValue();
                    ResetKey();
                } else {
                    if (!PushToSection(next)) {
                        // printf("Token: '%s' too long!\n", section.CharPtr());
                        goto leave;
                    }
                }
                break;

            case kWhiteSpace :
                if (!std::isspace(next)) {
                    state = stateAfterWhiteSpace;
                    // This will be part of the next token (hopefully)
                    if (stateAfterWhiteSpace == kKey) {
                        PushToKey(next);
                    } else if (stateAfterWhiteSpace == kValue) {
                        PushToValue(next);
                    }
                }
                break;

            case kKey :
                if (next == '=') {
                    // printf("key: '%s'\n", key.CharPtr());
                    state = kWhiteSpace;
                    stateAfterWhiteSpace = kValue;

                } else if (std::isspace(next)) {
                    // printf("key: '%s'\n", key.CharPtr());
                    state = kSeparator;
                } else {
                    if (!PushToKey(next)) {
                        goto leave;
                    }
                }
                break;
            case kSeparator :
                if (next == '=') {
                    state = kWhiteSpace;
                    stateAfterWhiteSpace = kValue;
                }
                break;
            case kValue :
                if (next < ' ') {
                    state = kWhiteSpace;
                    stateAfterWhiteSpace = kKey;

//                    printf("Value='%s'\n", value.CharPtr());
//                    printf("** %s.%s = '%s' **\n", section.CharPtr(), key.CharPtr(), value.CharPtr());
                    Commit();

                    ResetKey();
                    ResetValue();
                } else {
                    if (!PushToValue(next)) {
                        goto leave;
                    }
                }
                break;

        }
    }
    // Assume we are all good...
    res = true;

    // Check if we are running of data and we are in the Value state, assume file terminates after the last char of value...
    if ((nread == 0) && (state == kValue)) {
        Commit();
    }

leave:
    return res;
}

void IniParser::Commit() {
    if (cbValue != nullptr) {
        cbValue(section, key, value);
    }

    if (!sectionMap.contains(section)) {
        sectionMap[section] = Section::Create(section);
    }
    auto &s = sectionMap[section];
    s->values.push_back({key, value});
}

