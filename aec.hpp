#include <print>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

using namespace std;

template<typename T, size_t default_capacity = 512>
class Array {
private: 
    size_t capacity, count;
    T *items;

public:
    Array(size_t capacity = default_capacity);
    ~Array();
    T *operator[](size_t index);
    size_t length();
    void append(T item);
};

class Position {
public:
    char   *begin;
    char   *end;
    char   *offset;
    size_t line;

    Position(char *begin = NULL, char *end = NULL, char *offset = NULL, size_t line = 1) :
        begin(begin), end(end), offset(offset), line(line) {}
};

enum class TokenKind {
    undefined,
    identifier,
    integer,
    equal,
    plus,
    minus,
    colon,
    semicolon,
    count,
};

const string TOKEN_KIND[] = {
    "undefined",
    "identifier",
    "integer",
    "equal",
    "plus",
    "minus",
    "colon",
    "semicolon",
    "count",
};

class Token {
public:
    TokenKind kind;
    Position  position;

    Token(TokenKind kind = TokenKind::undefined, Position position = Position()) :
        kind(kind), position(position) {}
};

class Lexer {
public:
    string   path;
    string   text;
    Position pos;
    
    Lexer(string path);
    Array<Token> tokenise();

private:
    bool range();
    bool range(char *c);
    bool whitespace(char c);
    bool digit(char c);
    bool upper(char c);
    bool lower(char c);
    bool alpha(char c);
    bool begin_identifier(char c);
    bool end_identifier(char c);
    Token get_token();
};

class Scope;

class Type {
public:
    string name;

    Type(string name = "") : 
        name(name) {}
};

enum class ExprKind {
    undefined,
    value,
    infix,
    count,
};

template<typename T>
class Expression {
public:
    ExprKind kind;
    Position position;
    T&       expr;

    Expression(ExprKind kind = ExprKind::undefined, Position position = Position(), T& expr = new T()) :
        kind(kind), position(position), expr(expr) {}
};

enum class InfixKind {
    undefined,
    assignment,
    addition,
    subtraction,
    count,
};

template<typename T, typename U>
class InfixExpr {
public:
    Expression<T> lvalue;
    InfixKind      kind;
    Expression<U> rvalue;

    InfixExpr(
        Expression<T> lvalue = Expression<T>(), 
        InfixKind kind = InfixKind::undefined, 
        Expression<U> rvalue = Expression<U>()
    ) : lvalue(lvalue), kind(kind), rvalue(rvalue) {}
};

class Variable {
public:
    string name;
    Type   type;

    Variable(string name = "", Type type = Type()) : 
        name(name), type(type) {}
};

template<
    typename Variables = Array<Variable&>, 
    typename Expressions = Array<Expression<typename T>>
> 
class Block {
public:
    Variables   variables;
    Expressions expressions;

    Block(Variables variables = Variables(), Expressions expressions = Expressions()) : 
        variables(variables), expressions(expressions) {}
};

template<typename T>
class Argument {
public:
    string         name;
    Type           type;
    Expression<T>& exprs;

    Argument(string name, Type type, Expression<T>& value) : 
        name(name), type(type), exprs(exprs) {}
};

template<typename ReturnType = Expression<typename T>&, typename Arguments = Array<Expression<typename U>&>>
class Function {
public:
    string     name;
    ReturnType return_type;
    Arguments  arguments;
    Block      block;

    Function(string name, ReturnType return_type, Arguments arguments, Block block) : 
        name(name), return_type(return_type), arguments(arguments), block(bolck) {}
};

template<
    typename Types = Array<Type&>, 
    typename Variables = Array<Variable&>, 
    typename Functions = Array<Function&>
> 
class Scope {
public:
    Types     types;
    Variables variables;
    Functions functions;

    Scope(Types types, Variables variables, Functions functions) : 
        types(types), variables(variables), functions(functions) {}
};

class Porgram {
public:
    Scope _intern;
    Scope _extern;

    Program(Scope& _intern, Scope& _extern) :
        _intern(_intern), _extern(_extern) {}
};

class Parser {
public:
    Parser(string path);
    Parser(Lexer lexer);
    Porgram parse();

private:
    size_t       index;
    Lexer        lexer;
    Array<Token> tokens;
    Array<Scope> globals;

    Expresssion expression(size_t priority);
};
