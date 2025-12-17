//
// Created by gnilk on 12/12/2022.
//

#ifndef GNILK_INIPARSER_H
#define GNILK_INIPARSER_H

#include <functional>
#include <vector>
#include <string>
#include <utility>
#include <memory>

#include "IReader.h"
#include "IDecoder.h"

namespace gnilk {

    class IniParser {
    public:
        struct Section {

            using Ref = std::shared_ptr<Section>;

            Section() = delete;
            Section(const std::string &sName) : name(sName) {

            }
            static Ref Create(const std::string &name) {
                return std::make_shared<Section>(name);
            }

            std::string name;
            std::vector<std::pair<std::string, std::string>> values;
        };
    public:
        using Ref = std::shared_ptr<IniParser>;
        using ValueDelegate = std::function<void(const std::string &section, const std::string &key, const std::string &value)>;

    public:
        IniParser() = delete;
        IniParser(const std::string &data);
        IniParser(IReader::Ref pStream);
        virtual ~IniParser() = default;

        static Ref Create(const std::string &data) {
            return std::make_shared<IniParser>(data);
        }
        static Ref Create(IReader::Ref inStream) {
            return std::make_shared<IniParser>(inStream);
        }

        IniParser::Section::Ref GetSection(const std::string &section) {
            if (!sectionMap.contains(section)) {
                return nullptr;
            }
            return sectionMap[section];
        }

        void SetValueDelegate(ValueDelegate valueDelegate) { cbValue = valueDelegate; }
        bool ProcessData();
    private:
        void Commit();

        __inline bool PushToSection(char next) {
            section.push_back(next);
            return true;
        }

        __inline bool PushToKey(char next) {
            key.push_back(next);
            return true;
        }
        __inline bool PushToValue(char next) {
            value.push_back(next);
            return true;
        }

        __inline void ResetSection() {
            section.clear();
        }
        __inline void ResetKey() {
            key.clear();
        }
        __inline void ResetValue() {
            value.clear();
        }

    private:
        typedef enum : int {
            kUnknown,   // 0
            kWhiteSpace,       // 1
            kSectionStart,   // 2
            kSectionName,         // 3
            kKey,  // 4
            kSeparator,    // 5
            kValue,   // 6
        } kState;

        std::unordered_map<std::string, Section::Ref> sectionMap = {};

    private:
        std::string section = {};
        std::string key = {};
        std::string value = {};

        kState state = kSectionStart;
        kState stateAfterWhiteSpace = kUnknown;
        IReader::Ref inStream = {};
        ValueDelegate cbValue = nullptr;


    };
}


#endif //GNILK_INIPARSER_H
