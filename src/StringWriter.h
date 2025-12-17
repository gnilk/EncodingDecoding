//
// Created by gnilk on 3/1/2021.
//

#ifndef GNILK_STRINGWRITER_H
#define GNILK_STRINGWRITER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string>
#include <assert.h>

#include "IWriter.h"
#include "PrintfAttribute.h"


namespace gnilk {
    /**
     * String writer implements ability to write typed data to a text/human-readable file
     * This is similar to BinTypeWriter..
     */
    class StringWriter {
    public:
        using Ref = std::shared_ptr<StringWriter>;
    public:
        StringWriter() = default;
        explicit StringWriter(IWriter::Ref _writer) : writer(_writer) {

        }
        virtual ~StringWriter() = default;

        static Ref Create(IWriter::Ref _writer) {
            return std::make_shared<StringWriter>(_writer);
        }

        void Begin(IWriter::Ref _writer);

        int printf(const char *format, ...) GNILK_PRINTF_ATTRIB(1);
        int println(const char *format, ...) GNILK_PRINTF_ATTRIB(1);

        // We could alias the printf to Printf (in order to be compatible with 'memfile'
//        template<typename... Args>
//        int Printf(Args&&... args) {
//            return printf(std::forward<Args>(args)...);
//        }

        size_t WriteFormat(const char *format, ...) GNILK_PRINTF_ATTRIB(1);
        size_t WriteLine(const char *format, ...) GNILK_PRINTF_ATTRIB(1);

        __inline int Write(const float value) { return(printf("%f", value)); }
        __inline int Write(const double value) { return(printf("%f",value)); }
        // stdint
        __inline int Write(const char value) { return(printf("%c", value)); }
        __inline int Write(const int8_t value) { return(printf("%" PRIu8, value)); }
        __inline int Write(const uint8_t value) { return(printf("%" PRIu8, value)); }
        __inline int Write(const int16_t value) { return(printf("%" PRIi16, value)); }
        __inline int Write(const uint16_t value) { return(printf("%" PRIu16, value)); }
        __inline int Write(const int32_t value) { return(printf("%" PRIi32, value)); }
        __inline int Write(const uint32_t value) { return(printf("%" PRIu32, value)); }
        __inline int Write(const int64_t value) { return(printf("%" PRIi64, value)); }
        __inline int Write(const uint64_t value) { return(printf("%" PRIu64, value)); }
        __inline int Write(const char *str, size_t len) { return writer->Write((uint8_t *)str, len); }
        __inline int Write(const bool value) { return(printf("%s", value?"true":"false")); }
        __inline int Write(const std::string &value) {
            return(Write(value.c_str(), value.length()));
        }
        inline gnilk::StringWriter& operator<<(const std::string &str) {
            Write(str);
            return *this;
        }

#define SW_OP_WRITE(__T__) inline gnilk::StringWriter& operator<<(const __T__ value) { Write(value); return *this; }
        SW_OP_WRITE(float);
        SW_OP_WRITE(double);
        SW_OP_WRITE(int8_t);
        SW_OP_WRITE(uint8_t);
        SW_OP_WRITE(int16_t);
        SW_OP_WRITE(uint16_t);
        SW_OP_WRITE(int32_t);
        SW_OP_WRITE(uint32_t);
        SW_OP_WRITE(int64_t);
        SW_OP_WRITE(uint64_t);
#undef SW_OP_WRITE

        __inline gnilk::StringWriter &operator<<(const bool value) { Write(value); return *this; }

        inline gnilk::StringWriter& operator<<(const char *cstr) {
            assert(writer != nullptr);
            writer->Write((uint8_t *)cstr, strlen(cstr));
            return *this;
        }

        static const std::string endl;
        static const std::string eol;
        static const std::string eos;

    protected:
        int DoPrint(int flags, const char *format, va_list &values);

    private:
        IWriter::Ref writer;
    };
}


#endif //GNILK_STRINGWRITER_H
