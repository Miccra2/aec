#include "aec.hpp"

template<typename T>
Array<T>::Array(size_t capacity) {
    capacity = capacity;
    count    = 0;
    items    = new T[capacity];
}

template<typename T>
Array<T>::~Array() {
    delete[] items;
}

template<typename T>
T *Array<T>::operator[](size_t index) {
    if (index >= count) {
        println(stderr, "[ERROR] Tryed to access array outside of range (count: {}, index: {})!", count, index);
        _exit(1);
    }
    return &items[index];
}

template<typename T>
size_t Array<T>::length() {
    return count;
}

template<typename T>
void Array<T>::append(T item) {
    T *_items;
    if (capacity == 0) {
        capacity = default_capacity;
        _items   = items;
        items    = new T[capacity];
    } else if (count + 1 >= capacity) {
        size_t cap = capacity;
        capacity *= 2;
        _items      = items;
        items = new T[capacity];
        items = (Token*)memcpy(items, _items, cap * sizeof(T));
    }
    items[count++] = item;
}

Lexer::Lexer(string path) {
    stringstream buffer;
    ifstream file;

    file.open(path);
    buffer << file.rdbuf();
    file.close();
    
    path = path;
    text = buffer.str();
    pos  = Position(&text[0], &text[0], &text[0]);
}

Array<Token> Lexer::tokenise() {
    Array<Token> tokens = Array<Token>();
    Token        token  = get_token();
    println("text: {}", text.length());
    while (range() && token.kind != TokenKind::undefined) {
        println("first: {} or {}", range(), token.kind != TokenKind::undefined);
        println("token.position.end: {}, {:02X}", (size_t)token.position.end - (size_t)&text[0] + 1, *(token.position.end));
        tokens.append(token);
        token = get_token();
        println("second: {} or {}", range(), token.kind != TokenKind::undefined);
    }
    return tokens;
}

bool Lexer::range() {
    return range(pos.end);
}

bool Lexer::range(char *c) {
    return c >= &text[0] && c < (char*)((size_t)&text[0] + (size_t)text.length());
}

bool Lexer::whitespace(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

bool Lexer::digit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::upper(char c) {
    return c >= 'A' && c <= 'Z';
}

bool Lexer::lower(char c) {
    return c >= 'a' && c <= 'z';
}

bool Lexer::alpha(char c) {
    return upper(c) || lower(c);
}

bool Lexer::begin_identifier(char c) {
    return c == '_' || alpha(c);
}

bool Lexer::end_identifier(char c) {
    return digit(c) || begin_identifier(c);
}

Token Lexer::get_token() {
    // reassignment of pos.end for better readebilety
    char **cur = &pos.end;

    // skip whitesapces and comments
    while (range()) {
        if (whitespace(**cur)) {
            if (**cur == '\n') {
                pos.line++;
                pos.offset = *cur + 1;
            }
            (*cur)++;
        } else if (range(*cur + 1) && **cur == '/' && (*cur)[1] == '/') {
            while (range() && **cur != '\n') {
                (*cur)++;
            }
        } else if (range(*cur + 1) && **cur == '/' && (*cur)[1] == '*') {
            *cur += 2; // skip `/*` of block comment
            while (range(*cur + 1) && **cur != '*' && (*cur)[1] == '/') {
                if (range() && **cur == '\n') {
                    pos.line++;
                    pos.offset = *cur + 1;
                }
                (*cur)++;
            }
            *cur += 2; // skip `*/` of block comment
        } else {
            break;
        }
    }

    TokenKind kind = TokenKind::undefined;
    pos.begin = *cur;
    if (!range()) {
        return Token(kind, pos);
    }

    if (**cur == '=') {
        kind = TokenKind::equal;
    } else if (**cur == '+') {
        kind = TokenKind::plus;
    } else if (**cur == '-') {
        kind = TokenKind::minus;
    } else if (**cur == ':') {
        kind = TokenKind::colon;
    } else if (**cur == ';') {
        kind = TokenKind::semicolon;
    } else if (digit(**cur)) {
        while (range(*cur + 1) && digit((*cur)[1])) {
            (*cur)++;
        }
        kind = TokenKind::integer;
    } else if (begin_identifier(**cur)) {
        while (range(*cur + 1) && end_identifier((*cur)[1])) {
            (*cur)++;
        }
        kind = TokenKind::identifier;
    } else {
        size_t col = (size_t)pos.begin - (size_t)pos.offset + 1;
        println(stderr, "[ERROR:{}:{}:{}] Encountered an invaid or undefined character 0x{:02X}!", pos.line, col, path, **cur);
        _exit(1);
    }

    (*cur)++;
    return Token(kind, pos);
}

Parser::Parser(string path) {
    Parser(Lexer(path));
}

Parser::Parser(Lexer lexer) {
    lexer   = lexer;
    tokens  = lexer.tokenise();
    index   = 0;
    globals = Array<Scope&>();
}

int main() {
    string path = "test.aec";
    Lexer lexer = Lexer(path);
    Array<Token> tokens = lexer.tokenise();
    println("tokens: {}", tokens.length());
    for (size_t i = 0; i < tokens.length(); i++) {
        Token token = *tokens[i];
        println(
            "[INFO:{}:{}:{}] Token(kind: {}({})) \"{}\"", 
            token.position.line, 
            (size_t)token.position.begin - (size_t)token.position.offset + 1, 
            lexer.path, 
            TOKEN_KIND[(size_t)token.kind], 
            (size_t)token.kind,
            string(token.position.begin, token.position.end)
        );
    }
    return 0;
}
