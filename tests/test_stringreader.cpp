//
// Created by gnilk on 17.12.2025.
//

#include <testinterface.h>
#include "../src/StringReader.h"

using namespace gnilk;

extern "C" int test_stringreader_simple(ITesting *t) {

    static std::string data = "hello world";
    char buffer[128] = {};

    auto reader = StringReader(data);

    auto nRead = reader.Read(buffer, 126);
    TR_ASSERT(t, nRead == data.size());
    TR_ASSERT(t, data == buffer);

    return kTR_Pass;
}


extern "C" int test_stringreader_available(ITesting *t) {

    static std::string data = "hello world";
    char buffer[128] = {};

    auto reader = StringReader(data);

    int idx = 0;
    while(reader.Available()) {
        reader.Read(&buffer[idx], 1);
        ++idx;
    }
    TR_ASSERT(t, idx == data.size());
    TR_ASSERT(t, data == buffer);

    return kTR_Pass;
}

