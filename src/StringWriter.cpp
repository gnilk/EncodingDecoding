//
// Created by gnilk on 3/1/2021.
//

#include "StringWriter.h"

using namespace gnilk;

#define SW_FLAG_NONE 0x00
#define SW_FLAG_APPEND_NL 0x01

// Static in class
#ifdef WIN32
const std::string gnilk::StringWriter::endl = std::string("\r\n");
const std::string gnilk::StringWriter::eol = std::string("\r\n");
const std::string gnilk::StringWriter::eos = std::string("\0");
#else
const std::string gnilk::StringWriter::endl = std::string("\n");
const std::string gnilk::StringWriter::eol = std::string("\n");
const std::string gnilk::StringWriter::eos = std::string("\0");
#endif



void StringWriter::Begin(IWriter::Ref _writer) {
    writer = _writer;
}

int StringWriter::DoPrint(int flags, const char *format, va_list &values) {
    char newstr[256];


    assert(writer != nullptr);

    vsnprintf(newstr, 255, format, values);

    int res = writer->Write((uint8_t *)newstr, strlen(newstr));
    if (flags & SW_FLAG_APPEND_NL) {
        res += Write(endl);
    }
    return res;
}

GNILK_PRINTF_ATTRIB(1)
int StringWriter::printf(const char *format, ...) {
    va_list	values;
    va_start( values, format );
    auto res = DoPrint(SW_FLAG_NONE, format, values);
    va_end(values);
    return res;
}

GNILK_PRINTF_ATTRIB(1)
int StringWriter::println(const char *format,...) {
    va_list	values;
    va_start( values, format );
    auto res = DoPrint(SW_FLAG_APPEND_NL, format, values);
    va_end(values);
    return res;

}

GNILK_PRINTF_ATTRIB(1)
size_t StringWriter::WriteFormat(const char *format, ...) {
    va_list	values;
    va_start( values, format );
    auto res = DoPrint(SW_FLAG_NONE, format, values);
    va_end(values);
    return res;
}

GNILK_PRINTF_ATTRIB(1)
size_t StringWriter::WriteLine(const char *format, ...) {
    va_list	values;
    va_start( values, format );
    auto res = DoPrint(SW_FLAG_APPEND_NL, format, values);
    va_end(values);
    return res;
}

