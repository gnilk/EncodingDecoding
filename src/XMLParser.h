#ifndef GNILK_XML_PARSER
#define GNILK_XML_PARSER

// Implements a fairly speedy XML parser
#include <string>
#include <list>
#include <stack>
#include <functional>
#include <memory>

namespace gnilk {
    namespace xml {

        //
        // Classes in reverse order - due to internal type-declaration...  sometimes I do dislike the C++ type system
        //

        // Defines an attribute within an XML tag; like '<tag attribute="value" />'
        class Attribute {
        public:
            using Ref = std::shared_ptr<Attribute>;
        public:
            Attribute() = delete;
            explicit Attribute(const std::string &_name, const std::string &_value) : name(_name), value(_value) {}

            virtual ~Attribute() = default;

            static Attribute::Ref Create(const std::string &_name, const std::string &_value) {
                return std::make_shared<Attribute>(_name, _value);
            }

            const std::string &GetName() { return name; }
            void SetName(std::string &_name) { name = _name; }

            const std::string &GetValue() { return value; }
            void SetValue(std::string &_value) { value = _value; }
        private:
            std::string name;
            std::string value;
        };

        // defines a tag w/wo content and attributes
        class Tag  {
        public:
            using Ref = std::shared_ptr<Tag>;
        public:

            static Tag::Ref Create(const std::string&_name) {
                return std::make_shared<Tag>(_name);
            }


            Tag() = delete;
            explicit Tag(const std::string &_name);
            virtual ~Tag() = default;

            bool HasContent();
            std::string ToString();

            void AddAttribute(const std::string &_name, const std::string &_value);
            void AddChild(Tag::Ref tag);
            void SetParent(Tag::Ref tag);
            Tag::Ref GetParent();

            const std::string &GetName() { return name; }

            const std::string &GetContent() { return content; }
            void SetContent(std::string &_content) { content = _content; }

            bool HasAttribute(const std::string &attrName);
            const std::string &GetAttributeValue(const std::string &attrName, const std::string &defValue);
            std::list<Attribute::Ref> &GetAttributes() { return attributes; }

            const std::list<Tag::Ref> &GetChildren() { return children; }
            Tag::Ref GetFirstChild(const std::string &childName);
            Tag::Ref GetChildWithAttributeValue(const std::string &childName, const std::string &attribute, const std::string &value);

        private:
            std::string name;
            std::string content;

            std::list<Attribute::Ref> attributes;
            std::list<Tag::Ref> children;
            Tag::Ref parent;
        };

        typedef std::function<void(Tag::Ref tag, std::list<Attribute::Ref> &attributes)> OnTagDelegate;


        // Document container
        class Document {
        public:
            using Ref = std::shared_ptr<Document>;
        public:
            Document();
            virtual ~Document() = default;

            static Document::Ref Create() {
                return std::make_shared<Document>();
            }

            Tag::Ref GetRoot() { return root; };

            void Traverse(const OnTagDelegate &startHandler, const OnTagDelegate &endHandler);
            void TraverseFromNode(const Tag::Ref &node, const OnTagDelegate &startHandler, const OnTagDelegate &endHandler);
            void SetRoot(Tag::Ref pRoot) { root = pRoot; }
            void DumpTagTree(const Tag::Ref &root, int depth);
        protected:
            void TraverseNodes(const OnTagDelegate &startHandler, const OnTagDelegate &endHandler, const std::list<Tag::Ref> &tags);
            std::string IndentString(int depth);
        protected:
            Tag::Ref root;
        };

        // If you want to use 'event based' parsing - you can supply this to either CTOR or Load in the parser.
        // this will make the parser operate more like a 'SAX' type of parser...
        class IParseEvents
        {
        public:
            virtual void StartTag(Tag::Ref pTag) = 0;
            virtual void EndTag(Tag::Ref pTag) = 0;
            virtual void ContentTag(Tag::Ref pTag, const std::string &content) = 0;
        };

        //
        // XMLParser, use CTOR or 'Load' function to parse a document
        // Either call the static function 'Load' or use CTOR+GetDocument to retrieve parse it...
        //
        class XMLParser {
        protected:
            enum kParseState {
                psConsume,
                psTagStart,
                psEndTagStart,
                psTagAttributeName,
                psTagAttributeValue,
                psTagContent,
                psTagHeader,
                psCommentStart,
                psCommentConsume,
                psDocType,
            };
            // This is not used...
            enum kParseMode {
                pmStream,
                pmDOMBuild,
            };
            /////////
        public:
            XMLParser(const std::string &_data);
            XMLParser(const std::string &_data, IParseEvents *pEventHandler);
            std::unique_ptr<Document> GetDocument();

            static std::unique_ptr<Document> Load(const std::string &_data, IParseEvents *pEventHandler = nullptr);
        protected:
            void Initialize();
            void DoParseData();
            void ChangeState(kParseState newState);
            Tag::Ref CreateTag(const std::string &name);
            void EndTag(std::string tok);
            void CommitTag(Tag::Ref pTag);
            void Rewind();
            int NextChar();
            int PeekNextChar();
            void EnterNewState();
        protected:
            __inline void stateConsume(int c);
            __inline void stateCommentStart(int c);
            __inline void stateTagStart(int c);
            __inline void stateEndTagStart(int c);
            __inline void stateTagHeader(int c);
            __inline void stateCommentConsume(int c);
            __inline void stateAttributeName(int c);
            __inline void stateAttributeValue(int c);
            __inline void stateTagContent(int c);
            __inline void stateDTDDocTypeContent(int c);
        protected:
            // Global parsing stuff
            Tag::Ref tagCurrent = {};
            std::string attrName = {};
            std::string attrValue = {};

        protected:
            std::unique_ptr<Document> pDocument = {};
            Tag::Ref root = {};
            kParseState state = {};
            kParseState oldState = {};
            kParseMode parseMode = {};
            std::stack<Tag::Ref> tagStack = {};
            int idxCurrent = {};
            const std::string &data;            // Set in CTOR
            IParseEvents *pEventHandler = {};
            // parser variables
            std::string token = {};
        };


        // Helper class for string operations
        class StringUtilStatic {
        private:
            static std::string whiteSpaces;
        public:
            __inline static void trimRight(std::string &str, const std::string &trimChars = whiteSpaces) {
                std::string::size_type pos = str.find_last_not_of(trimChars);
                str.erase(pos + 1);
            }

            __inline static void trimLeft(std::string &str, const std::string &trimChars = whiteSpaces) {
                std::string::size_type pos = str.find_first_not_of(trimChars);
                str.erase(0, pos);
            }

            __inline static std::string &trim(std::string &str, const std::string &trimChars = whiteSpaces) {
                trimRight(str, trimChars);
                trimLeft(str, trimChars);
                return str;
            }

            __inline static std::string toLower(std::string s) {
                std::string res = "";
                for (size_t i = 0; i < s.length(); i++) {
                    res += ((char) tolower(s.at(i)));
                }
                return res;
            }

            __inline static bool equalsIgnoreCase(std::string a, std::string b) {
                std::string sa = toLower(a);
                std::string sb = toLower(b);
                return (sa == sb);
            }

        };


    } // xml namespace
} // gnilk

#endif