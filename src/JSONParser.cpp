//
// Very small stream based and callback based JSON parser/decoder
// allocation free (i.e. using the buffer pool)
//
// [gnilk, 2024-06-27] This is the third rewrite...
//
// see: https://www.json.org/json-en.html
//
// Left:
//  - Numbers, not properly supported!
//  - strings, no escape char handling
//
// Most of this code was produced before the 'Peek' function was introduced. Using Peek there is room for improvement.
//

#include <string.h>
#include "StringReader.h"
#include "JSONParser.h"


namespace gnilk {
#if defined(DEBUG) || defined(_DEBUG)
template <typename... Args>
void Debug(const char *fmt, Args... args) {
    std::printf(fmt, args...);
    std::printf("\n");
}

template <typename... Args>
void Error(const char *fmt, Args... args) {
    std::fprintf(stdout, fmt, args...);
    std::printf("\n");
}


#else
#define debug(...) {}
#define error(...) {}
#endif

}


#ifndef GNILK_JSON_MAX_DEPTH
#define GNILK_JSON_MAX_DEPTH 255
#endif

using namespace gnilk;

JSONParser::JSONParser(IReader::Ref stream) : inStream(stream) {
}

JSONParser::JSONParser(IReader::Ref stream, ValueDelegate valueDelegate) : inStream(stream), cbValue(valueDelegate) {
}

JSONParser::JSONParser(const std::string &data) {
    inStream = StringReader::Create(data);
}

JSONParser::JSONParser(const std::string &data, ValueDelegate valueDelegate) {
    inStream = StringReader::Create(data);
    cbValue = valueDelegate;
}

std::unique_ptr<JSONDoc> JSONParser::GetDocument() {
    auto res = ProcessData();
    if (res != kResult::Ok) {
        Error("ERR: %s", ErrToString(res).c_str());
        return {};
    }
    return std::move(document);
}

std::unique_ptr<JSONDoc> JSONParser::Load(const std::string &data) {
    JSONParser parser(data);
    return parser.GetDocument();
}



// Process data
JSONParser::kResult JSONParser::ProcessData() {
    Reset();
    document = std::make_unique<JSONDoc>();
    return ProcessDataInternal();
}

// Private/Protected functionality below this point

//
// Reset decoder internal values - currently not much, was more convoluted when decoder was iterative...
//
void JSONParser::Reset() {
    ResetCurrentValue();
}

//
// Top parsing point...
// JS must have a top-node object or array -
//
JSONParser::kResult JSONParser::ProcessDataInternal() {
    std::string emptyLabel = {};
    idxParser = 0;
    int ch;
    kResult procRes = kResult::Ok;
    while((ch = Next()) > 0) {
        if (std::isspace(ch)) {
            continue;
        }
        if (ch == '{') {
            auto obj = CreateJSONObject({});
            if ((procRes = ProcessObject(obj, 0)) != kResult::Ok) {
                return procRes;
            }
            document->root = {obj};
        } else if (ch == '[') {
            auto array = CreateJSONArray({});
            if ((procRes = ProcessArray(array,emptyLabel, 0)) != kResult::Ok) {
                return procRes;
            }
            document->root = {array};
        } else  {
            // any other root object??
        }
    }
    return kResult::Ok;
}

int JSONParser::SkipWhiteSpace() {
    int ch;
    while((ch = Next()) > 0) {
        if (!std::isspace(ch)) {
            return ch;
        }
    }
    return -1;
}

//
// Process object
//
JSONParser::kResult JSONParser::ProcessObject(JSONObject::Ref &currentObject, size_t depth) {
    int ch;
    if ((ch = SkipWhiteSpace()) < 0) {
        return kResult::ErrUnexpectedEOF;
    }
    if (ch == '}') {
        return kResult::Ok;
    }
    if (ch != '\"') {
        return kResult::ErrKeyMissing;
    }

    std::string label = {};
    kResult procRes = {};
    do {
        if ((procRes = ProcessString()) != kResult::Ok) {
            return procRes;
        }
        label = valueCurrent;
        //Debug("ProcessObject, label = '%s'", label.c_str());

        if ((ch = SkipWhiteSpace()) < 0) {
            return kResult::ErrUnexpectedEOF;
        }
        if (ch != ':') {
            //Error("ProcessObject, Missing separator after label");
            return kResult::ErrSeparatorMissing;
        }
        if ((procRes = ProcessValue(Next(), label, currentObject, depth)) != kResult::Ok) {
            return procRes;
        }

        // FIXME: OnValue()
        if ((ch = SkipWhiteSpace()) < 0) {
            return kResult::ErrUnexpectedEOF;
        }
        if (ch == '}') {
            //Debug("ProcessObject, '}' end of object '%s'", label.c_str());
            break;
        }

        if (ch != ',') {
            //Debug("ProcessObject, unepxected token ('%c') - expected either '}' or ','",ch);
            return kResult::ErrUnexpectedToken;
        }

        // Start of next
        // FIXME: Is this an error ???
        if ((ch = SkipWhiteSpace()) < 0) {
            return kResult::ErrUnexpectedEOF;
        }
        if (ch != '\"') {
            return kResult::ErrUnexpectedToken;
        }
    } while(true);

    return kResult::Ok;
}

//
// Process array
//
JSONParser::kResult JSONParser::ProcessArray(JSONArray::Ref &currentObject, const std::string &label, size_t depth) {
    if (depth > GNILK_JSON_MAX_DEPTH) {
        //Error("Max Recursion Depth %zu exceeded", depth);
        return kResult::ErrMaxDepth;
    }

    int ch;
    if ((ch = SkipWhiteSpace()) < 0) {
        return kResult::ErrUnexpectedEOF;
    }

    if (ch == ']') {
        //Debug("ProcessArray, ']' empty array");
        return kResult::Ok;
    }

    // Mark the current object as we are 'in an array...
    kResult procRes;
    while((procRes = ProcessValue(ch, label, currentObject, depth+1)) == kResult::Ok) {
        if ((ch = SkipWhiteSpace()) < 0) {
            return kResult::ErrUnexpectedEOF;
        }
        if (ch == ']') {
            // Debug("ProcessArray, end of array");
            return kResult::Ok;
        }

        if (ch != ',') {
            //Debug("ProcessArray, unepxected token ('%c') - expected either ']' or ','",ch);
            return kResult::ErrUnexpectedToken;
        }
        //Debug("ProcessArray, more items...");
        if ((ch = Next()) < 0) {
            return kResult::ErrUnexpectedEOF;
        }
    }

    //Debug("ProcessArray, failed to process value");
    return procRes;
}

//
// ProcessString, updates 'currentValue' with the string
//
JSONParser::kResult JSONParser::ProcessString() {
    ResetCurrentValue();

    int ch;
    while((ch = Next()) >= 0) {
        if (ch == '\"') {
            // Debug("ProcessString, end of string (string = %s)", valueCurrent);
            return kResult::Ok;
        }
        if (ch == 0) {
//            Error("ProcessString, zero (0) detected in string data - although not explicitly disallowed by standard - this will break shit");
            return kResult::ErrUnexpectedToken;
        }

        // FIXME: Handle escape values
        AppendToValue(ch);
    }
    return kResult::ErrUnexpectedEOF;
}

// Process a value
JSONParser::kResult JSONParser::ProcessValue(int ch, const std::string &label, JSONCoreObject::Ref parent, size_t depth) {
    if (depth > GNILK_JSON_MAX_DEPTH) {
        //Error("Max Recursion Depth %zu exceeded", depth);
        return kResult::ErrMaxDepth;
    }

    if (std::isspace(ch)) {
        if ((ch = SkipWhiteSpace()) < 0) {
            return kResult::ErrUnexpectedEOF;
        }
    }
    // declared here, used to check result of sub-processing...
    kResult procRes = kResult::Ok;

    //Debug("ProcessValue, token after ws = '%c'", ch);
    switch(ch) {
        case '\"' :
            //Debug("ProcessValue, token=\" - '%s' is string", label.c_str());
            if ((procRes = ProcessString()) != kResult::Ok) {
                return procRes;
            }
            OnValue(parent, label);
            // OnValue(currentObject, label);
            break;
        case '{' :
            {
                //Debug("ProcessValue, token='{' - '%s' is object", label.c_str());;
                auto newObject = CreateJSONObject(label);
                if ((procRes = ProcessObject(newObject, depth + 1)) != kResult::Ok) {
                    return procRes;
                }

                parent->AddValue(label, JSONValue::Create(newObject));
            }
            break;
        case '[' : {
                auto newArray = CreateJSONArray(label);
                //Debug("ProcessValue, token='[' - '%s' is array", label.c_str());
                if ((procRes = ProcessArray(newArray, label, depth + 1)) != kResult::Ok) {
                    return procRes;
                }

                parent->AddValue(label, JSONValue::Create(newArray));

            }
            break;
        case 't' :
            if ((procRes = ProcessExpected(ch, "true")) != kResult::Ok) {
                return procRes;
            }
            OnValue(parent,label);
            break;
        case 'f' :
            if ((procRes = ProcessExpected(ch, "false")) != kResult::Ok) {
                return procRes;
            }
            OnValue(parent,label);
            break;
        case 'n' :
            if ((procRes = ProcessExpected(ch, "null")) != kResult::Ok) {
                return procRes;
            }
            OnValue(parent,label);
            break;
        default : {
                if (!IsValidNumberStart(ch)) {
                    //Error("ProcessValue, unexpected token (%c) at start of value", ch);
                    return kResult::ErrUnexpectedToken;
                }
                if ((procRes = ProcessNumber(ch)) != kResult::Ok) {
                    return procRes;
                }
                OnValue(parent, label);
            }
            break;
    }
    return kResult::Ok;
}

// Checks if the token 'ch' is a valid number starter
bool JSONParser::IsValidNumberStart(int ch) {
    if (ch == '-') return true;
    if (std::isdigit(ch)) return true;
    if (ch == '0') return true;
    return false;
}

// Do number parsing
JSONParser::kResult JSONParser::ProcessNumber(int ch) {
    // FIXME: this lacks quite a bit - see link in the file-header
    static std::string valid(".0123456789");
    ResetCurrentValue();

    // FIXME: This needs a more elaborate state machine...
    do {
        AppendToValue(ch);

        if ((ch = Peek()) < -1) {
            return kResult::ErrUnexpectedEOF;
        }
        // End of number - perhaps a better detection is good
        if (ch == ',') {
            break;
        }
        // only valid if we are in an object
        if (ch == '}') {
            break;
        }
        // this is only valid if we are in an array
        if (ch == ']') {
            break;
        }

        // white space breaks number parsing
        if (std::isspace(ch)) {
            break;
        }

        if (valid.find(ch) == std::string::npos) {
            //Error("ProcessNumber, invalid Token '%c'", ch);
            return kResult::ErrUnexpectedToken;
        }
        // Consume peeked value...
        Next();
    } while(true);

    // Here we have advanced ONE token too many...
    return kResult::Ok;
}

//
// Process the expected char and updates the currentValue with the result..
//
// Note: This does not check if the token ('ch') AFTER the last expected char is a valid termination char
//       like 'truea' <- where 'a' is outside the expected range, and will yield a positive result
//       the error will be caught later - but it will generate a call to 'OnValue' before that happens...
//
// Like:
//       {
//          "var" : trueisfine
//       }
//
// If you decode this you will either have an unmarshal::SetField("var","true") or a value-callback before the parser-error is caught.
// The JSONDecoder::ProcessData will return false at the end of the day - due to parser error...
//
JSONParser::kResult JSONParser::ProcessExpected(int ch, const char *expected) {
    auto szExpected = strlen(expected);
    ResetCurrentValue();
    AppendToValue(ch);
    for(size_t i=1;i<szExpected;i++) {
        ch = Peek();
        if (ch != expected[i]) {
            // Error("Unexpected token ('%c') while parsing %s", ch, expected);
            return kResult::ErrUnexpectedToken;
        }
        AppendToValue(ch);
        Next();
    }
    // Note: what-ever comes after will be handled by parser
    return kResult::Ok;

}

// Resets the current work-value
void JSONParser::ResetCurrentValue() {
    valueCurrent = {};
    idxValueCurrent = 0;
}

void JSONParser::AppendToValue(int ch) {
    valueCurrent.push_back(ch);
    idxValueCurrent++;
}

// Private
JSONObject::Ref JSONParser::CreateJSONObject(const std::string &name) {
    return std::make_shared<JSONObject>(name);
}

JSONArray::Ref JSONParser::CreateJSONArray(const std::string &name) {
    return std::make_shared<JSONArray>(name);
}

void JSONParser::OnValue(JSONCoreObject::Ref currentObject, const std::string &label) {
//    if (cbValue != nullptr) {
//        cbValue(currentObject->label, label.c_str(), valueCurrent);
//    }

//    auto newValue = JSONValue();
//    newValue.value = valueCurrent;
    currentObject->AddValue(label, JSONValue::Create(valueCurrent));

    ResetCurrentValue();
}

//
// Returns the current look ahead (unless consumed) or reads one item in the to look ahead and returns it
//
int JSONParser::Peek() {
    // Bring in a new ONLY if the current lookAhead value has been consumed
    if (lookAhead < 0) {
        lookAhead = ReadNext();
    }
    return lookAhead;
}

//
// Returns 'next' or the look-ahead value (resets the look ahead if present)
//
int JSONParser::Next() {
    // In case we have peeked - return the look-ahead value and reset
    if (lookAhead > -1) {
        auto result = lookAhead;
        lookAhead = -1;
        return result;
    }

    // Otherwise just read...
    return ReadNext();
}

//
// Read a single value off the stream
//
int JSONParser::ReadNext() {
    uint8_t ch;
    auto nRead = inStream->Read(&ch, 1);
    // End of stream??
    if (nRead == 0) {
        //Debug("Next, end of stream?");
        return -1;
    }
    if (nRead < 0) {
        //Error("Next, read failure");
        return -1;
    }

    idxParser++;
    return ch;
}

const std::string &JSONParser::ErrToString(JSONParser::kResult err) {
    static std::unordered_map<JSONParser::kResult, std::string> errToStr = {
            {kResult::Ok, "Ok"},
            {kResult::ErrUnexpectedEOF, "Unexpected End of File"},
            {kResult::ErrMaxDepth, "Max recursive depth reached"},
            {kResult::ErrUnexpectedToken, "Unexpected token"},
            {kResult::ErrKeyMissing, "Key missing"},
            {kResult::ErrSeparatorMissing, "Separator missing"},
    };
    static std::string unkErr = "Unknown error";
    if (!errToStr.contains(err)) {
        return unkErr;
    }
    return errToStr[err];
}
