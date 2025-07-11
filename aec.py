#!/usr/bin/env python3
from dataclasses import dataclass, field, asdict, astuple
from typing import Any, overload
from copy import deepcopy
import sys, subprocess

def cmd(*args) -> str:
    return subprocess.run(*args, capture_output=True).stdout

def defualt(cls: Any) -> Any:
    return field(default_factory=cls())

def str_field(cls: Any, kind: int) -> str:
    cls = asdict(cls())
    for f in cls:
        if cls[f] == kind:
            return f
    return ""

IOTA_COUNTER: int = 0
def iota(reset: int | None = None) -> int:
    global IOTA_COUNTER
    if isinstance(reset, int):
        IOTA_COUNTER = reset
    result: int = IOTA_COUNTER
    IOTA_COUNTER += 1
    return result

@dataclass
class Position:
    begin:  int = 0
    end:    int = 0
    offset: int = 0
    line:   int = 1

@dataclass
class TokenKind:
    undefined:  int = iota(0)
    identifier: int = iota()
    integer:    int = iota()
    equal:      int = iota()
    plus:       int = iota()
    minus:      int = iota()
    semicolon:  int = iota()
    count:      int = iota()

@dataclass
class Token:
    kind:     int      = TokenKind.undefined
    position: Position = defualt(Position)

    def priority(self) -> int:
        if self.kind in (TokenKind.plus, TokenKind.minus):
            return 0
        return 1

class Lexer:
    def __init__(self, path: str) -> None:
        self.path: str      = path
        self.text: str      = ""
        self.pos:  Position = Position()

        try:
            with open(self.path, "r") as file:
                self.text = file.read()
        except:
            print(f"ERROR(Lexer): Could not open file \"{self.path}\", please provide a valid file path!")
            sys.exit(1)

    def diag(self, position: Position | None = None) -> str:
        if not isinstance(position, Position):
            position = self.pos
        column: int = position.begin - position.offset + 1
        return f"{position.line}:{column}:{position.path}"

    def get_str(self, position: Position | None = None) -> str:
        if not isinstance(position, Position):
            position = self.pos
        return self.text[position.begin:position.end]

    def tokenise(self) -> list[Token]:
        tokens: list[Token] = []
        token: Token = self.get_token()
        while self.range() and token.kind != TokenKind.undefined:
            tokens.append(token)
            token = self.get_token()
        return tokens

    def range(self, index: int | None = None) -> bool:
        if not isinstance(index, int):
            index = self.pos.end
        return index < len(self.text)

    def whitespace(self) -> bool:
        c: str = self.text[self.pos.end]
        return c in ' \n\r\t\f\v'

    def digit(self) -> bool:
        c: str = self.text[self.pos.end]
        return ord(c) >= ord('0') and ord(c) <= ord('9')
    
    def upper(self) -> bool:
        c: str = self.text[self.pos.end]
        return ord(c) >= ord('A') and ord(c) <= ord('Z')

    def lower(self) -> bool:
        c: str = self.text[self.pos.end]
        return ord(c) >= ord('a') and ord(c) <= ord('z')

    def alpha(self) -> bool:
        return self.upper() or self.lower()
    
    def identifier_begin(self) -> bool:
        return self.text[self.pos.end] == '_' or self.alpha()

    def identifier_end(self) -> bool:
        return self.identifier_begin() or self.digit()

    def get_token(self) -> Token:
        cur: int = 0

        # skip whitespace and comments
        while self.range():
            cur = self.pos.end
            if self.whitespace():
                if self.text[cur] == '\n':
                    self.pos.line += 1
                    self.pos.offset = self.pos.end + 1
                self.pos.end += 1
            elif self.range(cur + 1) and self.text[cur:cur+2] == "//":
                while self.range() and self.text[self.pos.end] != '\n':
                    self.pos.end += 1
            elif self.range(cur + 1) and self.text[cur:cur+2] == "/*":
                self.pos.end += 2
                while self.range(self.pos.end + 1) and self.text[self.pos.end:self.pos.end+2] == "*/":
                    if self.text[self.pos.end] == '\n':
                        self.pos.line += 1
                        self.pos.offset = self.pos.end + 1
                    self.pos.end += 1
                self.pos.end += 2
            else:
                break

        self.pos.begin = self.pos.end
        if not self.range():
            return Token(TokenKind.undefined, deepcopy(self.pos))

        cur = self.pos.end
        kind: int = TokenKind.undefined
        if self.text[cur] == '=':
            kind = TokenKind.equal
        elif self.text[cur] == '+':
            kind = TokenKind.plus
        elif self.text[cur] == '-':
            kind = TokenKind.minus
        elif self.text[cur] == ';':
            kind = TokenKind.semicolon
        elif self.digit():
            while self.range() and self.digit():
                self.pos.end += 1
            return Token(TokenKind.integer, deepcopy(self.pos))
        elif self.identifier_begin():
            while self.range() and self.identifier_end():
                self.pos.end += 1
            return Token(TokenKind.identifier, deepcopy(self.pos))
        else:
            c: str = self.text[self.pos.end]
            char: str = f" '{c}'" if ord(c) >= 0x20 and ord(c) <= 0x7E else ''
            print(f"ERROR(Lexer): {self.diag()}: Encountered an invalide character at {self.pos.end} 0x%.2X{char}!" % ord(self.text[self.pos.end]))
            sys.exit(1)

        self.pos.end += 1
        return Token(kind, deepcopy(self.pos))

@dataclass
class ExprKind:
    undefined:   int = iota(0)
    addition:    int = iota()
    subtraction: int = iota()
    count:       int = iota()

class Parser:
    @overload
    def __init__(self, path: str) -> None: ...
    def __init__(self, lexer: Lexer) -> None:
        self.lexer:  Lexer       = lexer
        self.tokens: list[Token] = []
        self.index:  int         = 0

        if isinstance(lexer, str):
            self.lexer = Lexer(lexer)

        self.tokens = self.lexer.tokenise()

    def parse(self) -> list[list[tuple[int, Position, Any]]]:
        expressions: list[list[tuple[int, Position, Any]]] = []
        while self.range():
            expr: list[tuple[int, Position, Any]] | None = self.expression()
            if not isinstance(expr, list):
                break
            expressions.append(expr)
        return expressions

    def range(self, index: int | None = None) -> bool:
        if not isinstance(index, int):
            index = self.index
        return index < len(self.tokens)

    def expression(self) -> list[tuple[int, Position, Any]]:
        stack: list[tuple[int, Position, Any]] = []
        expr:  list[tuple[int, Position, Any]] = []
        token: Token = self.tokens[self.index]
        while self.range() and token.kind != TokenKind.semicolon:
            if token.kind == TokenKind.identifier:
                ...
        self.index += 1
        return expr

def main() -> None:
    path: str = "test.ae"
    lexer: Lexer = Lexer(path)
    parser: Parser = Parser(lexer)
    ...
    
    pass
    exit()
    tokens: list[Token] = parser.tokens
    for token in tokens:
        diag: str = f"{token.position.line}:{token.position.begin-token.position.offset+1}:{lexer.path}"
        ran:  str = f"{token.position.offset}:{token.position.begin}-{token.position.end}"
        print(f"INFO(Token): {diag}: [{ran}]: kind={str_field(TokenKind, token.kind)}({token.kind}), \"{lexer.get_str(token.position)}\"!")

if __name__ == "__main__":
    main()
