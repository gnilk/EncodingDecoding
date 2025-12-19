// Implements a fairly speedy XML parser in less than 1000 lines of code
#include <string>
#include <cassert>

#include "XMLParser.h"


//
// FIXME: Attribute values must close with same quotation it was started. Can't mix double/single quotes in a single declaration!
//

using namespace gnilk::xml;

XMLParser::XMLParser(const std::string &_data) : data(_data) {
}

XMLParser::XMLParser(const std::string &_data, IParseEvents *eventHandler) : data(_data), pEventHandler(eventHandler) {
}

std::unique_ptr<Document> XMLParser::Load(const std::string &_data, IParseEvents *pEventHandler) {
    XMLParser p(_data, pEventHandler);
    return p.GetDocument();
}

void XMLParser::Initialize() {
    attrName = "";
    attrValue = "";
    token = "";

    // FIXME: Why do I need this???
    root = Tag::Create("root");
    pDocument = std::make_unique<Document>();
    pDocument->SetRoot(root);
    idxCurrent = 0;
    state =  psConsume;
    parseMode = pmDOMBuild;
}

static void TraverseFrom(Tag::Ref tag, size_t depth) {
    std::string indent(depth, ' ');
    printf("%s%s\n", indent.c_str(), tag->GetName().c_str());
    for(auto &attr : tag->GetAttributes()) {
        printf("  %s%s=%s\n", indent.c_str(), attr->GetName().c_str(), attr->GetValue().c_str());
    }
    for(auto &child : tag->GetChildren()) {
        TraverseFrom(child, depth+2);
    }
}

std::unique_ptr<Document> XMLParser::GetDocument() {
    Initialize();
    DoParseData();
    // Dump doc
    // TraverseFrom(pDocument->GetRoot(),0);
    return std::move(pDocument);
}

void XMLParser::DoParseData() {
    int c;
    tagStack.push(root);
    while ((c = NextChar()) != EOF) {
        switch (state) {
            case psConsume:
                stateConsume(c);
                break;
            case psCommentStart : // Make sure we hit '--'
                stateCommentStart(c);
                break;
            case psCommentConsume:  // parse until -->
                stateCommentConsume(c);
                break;
            case psTagHeader : // <?
                stateTagHeader(c);
                break;
            case psTagStart :    // '<'
                stateTagStart(c);
                break;
            case psEndTagStart : // </
                stateEndTagStart(c);
                break;
            case psTagAttributeName :
                stateAttributeName(c);
                break;
            case psTagAttributeValueStart :
                stateAttributeValueStart(c);
                break;
            case psTagAttributeValue :
                stateAttributeValue(c);
                break;
            case psTagContent:
                stateTagContent(c);
                break;
            case psDocType:
                stateDTDDocTypeContent(c);
                break;
        }
    }
}

void XMLParser::stateConsume(int c) {
    if (c == '<') {
        int next = PeekNextChar();
        if (next == '/') {        // ? '</' - distinguish between token <  and </
            NextChar(); // consume '/'
            ChangeState(psEndTagStart);
        } else if (next == '!') {
            // Action tag started <!--
            NextChar();
            token = ""; // Reset token
            ChangeState(psCommentStart);
        } else if (next == '?') {
            // Header tag started '<?xml
            NextChar();
            token = "";
            ChangeState(psTagHeader);
        } else {
            ChangeState(psTagStart);
        }
        token = "";
    } else {
        token += c;
    }
}

void XMLParser::stateCommentStart(int c) {
    if ((c == '-') && (PeekNextChar() == '-')) {
        NextChar();
        ChangeState(psCommentConsume);
    } else {
#ifdef _DEBUG
        //printf("Warning: Illegal start of tag, expected start of comment ('<!--') but found found '<!-'\n");
#endif
        // TODO: If strict, abort here!
        Rewind();   // rewind '-'
        Rewind();   // rewind '!'
        ChangeState(psTagStart);
    }
}

void XMLParser::stateTagStart(int c) {
    if (isspace(c)) {
        tagCurrent = CreateTag(StringUtilStatic::trim(token));
        token = "";
        ChangeState(psTagAttributeName);
    } else if (c == '/' && PeekNextChar() == '>') {   // catch tags like '<tag/>'
        NextChar(); // consume '>'
        tagCurrent = CreateTag(StringUtilStatic::trim(token));
        token = "";
        CommitTag(tagCurrent);
        ChangeState(psConsume);
    } else if (c == '>') {
        tagCurrent = CreateTag(StringUtilStatic::trim(token));
        token = "";
        ChangeState(psTagContent);
    } else {
        token += c;
    }

}

void XMLParser::stateEndTagStart(int c) {
    if (isspace(c)) {
        // drop them
    } else if (c == '>') {
        std::string tmptok(StringUtilStatic::trim(token));
        token = "";
        EndTag(tmptok);
        ChangeState(psConsume);
    } else {
        token += c;
    }
}

void XMLParser::stateTagHeader(int c) {
    if (isspace(c)) {
        // drop them
        tagCurrent = CreateTag(StringUtilStatic::trim(token));
        token = "";
        ChangeState(psTagAttributeName);
    } else {
        token += c;
    }
}

void XMLParser::stateCommentConsume(int c) {
    if ((c == '-') && (PeekNextChar() == '>')) {
        if (token == "-") {
            NextChar();
            ChangeState(psConsume);
        }
    } else if ((c == 'D') && (PeekNextChar() == 'O')) {
        ChangeState(psDocType);
    } else if (c == '-') {
        token = "-";  // Store this in order to track -->
    }

    // } else if (c=='-') {
    //   token="-";  // Store this in order to track -->
    // }
}

void XMLParser::stateAttributeName(int c) {
    if (isspace(c)) return;
    if ((c == '=') && (PeekNextChar() == '"') || (PeekNextChar() == '\'')) {
        NextChar(); // consume "
        attrName = token;
        token = "";
        ChangeState(psTagAttributeValue);
    } else if ((c == '=') && (PeekNextChar() == '#')) {
        NextChar(); // consume #
        attrName = token;
        attrValue = "#";
        tagCurrent->AddAttribute(attrName, attrValue);
        token = "";
    } else if ((c == '=') && isspace(PeekNextChar())) {
        attrName = token;
        token = "";
        ChangeState(psTagAttributeValueStart);
    } else if (c == '>') {    // End of tag
            token = "";
            ChangeState(psTagContent);
    } else if ((c == '/') && (PeekNextChar() == '>')) {
        NextChar();
        CommitTag(tagCurrent);
        EndTag(StringUtilStatic::trim(token));
        token = "";
        ChangeState(psConsume);
    } else if ((c == '?') && (PeekNextChar() == '>')) {
        NextChar();
        CommitTag(tagCurrent);
        EndTag(StringUtilStatic::trim(token));
        token = "";
        ChangeState(psConsume);
    } else {
        token += c;
    }
}
void XMLParser::stateAttributeValueStart(int c) {
    if (isspace(c)) return;
    if ((c == '\"') || (c == '\'')) {
        token = "";
        ChangeState(psTagAttributeValue);
    }
}

void XMLParser::stateAttributeValue(int c) {
    // FIXME: This requires more elaborate parsing to be compliant (like support for &quot and escapes and stuff...)
    if ((c == '"') || (c == '\'')) {
        attrValue = token;
        tagCurrent->AddAttribute(attrName, attrValue);
        ChangeState(psTagAttributeName);
        token = "";
    } else {
        token += c;
    }
}

void XMLParser::stateTagContent(int c) {
    if (c == '<') {    // can't use 'peekNext' since we might have >< which is legal
        tagCurrent->SetContent(StringUtilStatic::trim(token));
        token = "";
        ChangeState(psConsume);
        Rewind();    // rewind so we will see tag start next time
    } else {
        token += c;
    }
}

void XMLParser::stateDTDDocTypeContent(int c) {
    if (c == '>') {
        token = "";
        ChangeState(psConsume);
    }
}

void XMLParser::Rewind() {
    idxCurrent--;
}

int XMLParser::NextChar() {
    if (idxCurrent >= (int) data.length()) return EOF;
    return data.at(idxCurrent++);
}

int XMLParser::PeekNextChar() {
    if (idxCurrent >= (int) data.length()) return EOF;
    return data.at(idxCurrent);
}

void XMLParser::ChangeState(kParseState newState) {
    state = newState;
    EnterNewState();
}

Tag::Ref XMLParser::CreateTag(const std::string &name) {
    return Tag::Create(name);
}

void XMLParser::EndTag(std::string tok) {
    Tag::Ref popped = nullptr;
    if (!StringUtilStatic::equalsIgnoreCase(tagStack.top()->GetName(), tok)) {
        auto top = tagStack.top();
        // can be an empty tag, like <br />
        if (top->HasContent() == false) {
            popped = tagStack.top();
            tagStack.pop();
        } else {
#ifdef _DEBUG
            printf("WARN: Illegal XML, end-tag has no corrsponding start tag!\n");
#endif
        }
    } else {
        if (!tagStack.empty()) {
            popped = tagStack.top();
            tagStack.pop();
        }
    }

    if ((pEventHandler != nullptr) && (popped != nullptr)) {
        pEventHandler->EndTag(popped);
    }
}

void XMLParser::CommitTag(Tag::Ref pTag) {
    if (pEventHandler != nullptr) {
        pEventHandler->StartTag(pTag);
    }
    // Only store in hierarchy if we are building a 'DOM' tree
    if (parseMode == pmDOMBuild) {
        pTag->SetParent(tagStack.top());
        tagStack.top()->AddChild(pTag);
    }
    tagStack.push(pTag);
}

void XMLParser::EnterNewState() {
    if (state == psTagContent) {
        CommitTag(tagCurrent);
    }
}


//////////////////
// Tag implementation
Tag::Tag(const std::string &_name) : name(_name) {
    content.clear();
    parent = nullptr;
}

void Tag::AddAttribute(const std::string &_name, const std::string &_value) {
    attributes.push_back(Attribute::Create(_name, _value));
}

void Tag::AddChild(Tag::Ref tag) {
    assert((tag->GetParent() != nullptr));
    children.push_back(tag);
}

void Tag::SetParent(Tag::Ref tag) {
    parent = std::move(tag);
}

Tag::Ref Tag::GetParent() {
    return parent;
}


bool Tag::HasContent() {
    return (!content.empty());
}


std::string Tag::ToString() {
    return std::string(name + " (" + content + ")");
}

bool Tag::HasAttribute(const std::string &attrName) {
    auto it = attributes.begin();
    while (it != attributes.end()) {
        auto pAttribute = *it;
        if (pAttribute->GetName() == attrName) return true;
        ++it;

    }
    return false;
}

const std::string &Tag::GetAttributeValue(const std::string &attrName, const std::string &defValue) {
    auto it = attributes.begin();
    while (it != attributes.end()) {
        auto pAttribute = *it;
        if (pAttribute->GetName() == attrName) {
            return pAttribute->GetValue();
        }
        ++it;
    }
    return defValue;
}

Tag::Ref Tag::GetFirstChild(const std::string &childName) {
    auto it = children.begin();
    while (it != children.end()) {
        auto child = *it;
        if (child->GetName() == childName) return child;
        ++it;
    }
    return nullptr;
}

// Get a child with a specific attribute and value
Tag::Ref Tag::GetChildWithAttributeValue(const std::string &childName, const std::string &attribute, const std::string &value) {
    auto it = children.begin();
    while(it != children.end()) {
        auto child = *it;
        if (child->GetName() != name) continue;
        if (!child->HasAttribute(attribute)) continue;

        auto chval = child->GetAttributeValue(attribute, "");
        if (chval == value) {
            return child;
        }
        ++it;
    }
    return nullptr;
}

// -- Document container
Document::Document() {
    root = nullptr;
}

void Document::Traverse(const OnTagDelegate& startHandler, const OnTagDelegate &endHandler) {
    TraverseNodes(startHandler, endHandler, root->GetChildren());
}

void Document::TraverseFromNode(const Tag::Ref& node, const OnTagDelegate& startHandler, const OnTagDelegate &endHandler) {
    TraverseNodes(startHandler, endHandler, node->GetChildren());
}


void Document::TraverseNodes(const OnTagDelegate& startHandler, const OnTagDelegate& endHandler, const std::list<Tag::Ref> &tags) {
    auto it = tags.begin();
    while (it != tags.end()) {
        auto tag = *it;
        startHandler(tag, tag->GetAttributes());
        TraverseNodes(startHandler, endHandler, tag->GetChildren());
        endHandler(tag, tag->GetAttributes());
        ++it;
    }
}

std::string Document::IndentString(int depth) {
    std::string s = "";
    for (int i = 0; i < depth; i++) s += " ";
    return s;
}

// DEBUG HELPER!
void Document::DumpTagTree(const Tag::Ref& fromNode, int depth) {
    std::string indent = IndentString(depth);
    //System.out.println(indent+"T:"+root->getName());
    printf("%sT:%s\n", indent.c_str(), fromNode->GetName().c_str());
    auto &tags = fromNode->GetChildren();

    auto it = tags.begin();
    while (it != tags.end()) {
        auto child = *it;
        DumpTagTree(child, depth + 2);
        ++it;
    }
}



/////////////////////////////////////////////////////////////////////////////////
///////// -------- Class StringUtil
/////////////////////////////////////////////////////////////////////////////////
std::string StringUtilStatic::whiteSpaces(" \f\n\r\t\v");

