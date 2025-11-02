#pragma once

#include "core.h"
#include "string.h"

static char *PythonKeywords[] = {
    // 0..=2: Constant
    "False",  "True",    "None",

    "await",  "else",    "import", "pass",   "break",    "except",   "in",     "raise",
    "class",  "finally", "is",     "return", "and",      "continue", "for",    "lambda",
    "try",    "as",      "def",    "from",   "nonlocal", "while",    "assert", "del",
    "global", "not",     "with",   "async",  "elif",     "if",       "or",     "yield",
};

typedef enum TokenType {
    TokenTypeNone = 0,

    TokenTypeKeyword,

    TokenTypePunctuation,
    TokenTypeWhitespace,

    TokenTypeIdent,
    TokenTypeString,
    TokenTypeNumber,
    TokenTypeConstant,
    TokenTypeOperator,

    TokenTypeNewLine,

    TokenTypeComment
} TokenType;

typedef struct Token {
    TokenType type;
    String    s;
} Token;

typedef struct Lexer {
    String input;
    u32    pos;
} Tokenizer;

Token TokenizerNext(Tokenizer *tokenizer);
Token TokenizerTryParseNumber(Tokenizer *tokenizer);
Token TokenizerGetKeywordOrIdent(Tokenizer *tokenizer);

char   TokenizerPeek(Tokenizer *tokenizer);
char   TokenizerConsume(Tokenizer *tokenizer);
String TokenizerConsumeWhile(Tokenizer *tokenizer, bool (*predicate)(char));
void   TokenizerSkipUntil(Tokenizer *tokenizer, char until);

bool TokenizerIsPunct(Tokenizer *tokenizer);

Token TokenizerNext(Tokenizer *tokenizer) {
    if (tokenizer->pos >= tokenizer->input.len) return (Token){0};

    u32  start = tokenizer->pos;
    char current_char = TokenizerPeek(tokenizer);

    if (current_char == '#') {
        String sliced = StringSliceFrom(&tokenizer->input, start);
        u32    end = StringSearchNth(&sliced, 1, '\n');
        tokenizer->pos += end + 1;
        String token_string = StringSliceFromTo(&tokenizer->input, start, start + end);
        return (Token){.type = TokenTypeComment, .s = token_string};
    }

    if (CharIsQuote(current_char)) {
        tokenizer->pos = StringSearchNthAddOne(&tokenizer->input, 2, current_char);
        String token_string = StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
        assert(StringCount(&token_string, '\n') == 0 && "tokens should be on one line");
        return (Token){.type = TokenTypeString, .s = token_string};
    }

    if (CharIsSpace(current_char)) {
        String token_string = TokenizerConsumeWhile(tokenizer, CharIsSpace);
        assert(StringCount(&token_string, '\n') == 0 && "tokens should be on one line");
        return (Token){.type = TokenTypeWhitespace, .s = token_string};
    }

    if (CharIsOperator(current_char)) {
        String token_string = TokenizerConsumeWhile(tokenizer, CharIsOperator);
        assert(StringCount(&token_string, '\n') == 0 && "tokens should be on one line");
        return (Token){.type = TokenTypeOperator, .s = token_string};
    }

    if (CharIsPunct(current_char)) {
        String token_string = TokenizerConsumeWhile(tokenizer, CharIsPunct);
        assert(StringCount(&token_string, '\n') == 0 && "tokens should be on one line");
        return (Token){.type = TokenTypePunctuation, .s = token_string};
    }

    if (current_char == '\n') {
        tokenizer->pos += 1;
        return (Token){.type = TokenTypeNewLine, .s = S("\n")};
    }

    if (CharIsDigit(current_char) || current_char == '.') {
        Token maybe_number = TokenizerTryParseNumber(tokenizer);
        if (maybe_number.type == TokenTypeNumber) return maybe_number;
    }

    return TokenizerGetKeywordOrIdent(tokenizer);
}

Token TokenizerTryParseNumber(Tokenizer *tokenizer) {
    bool is_decimal = false;
    u32  start = tokenizer->pos, radix = 10, exponent_idx = start;
    while (tokenizer->pos < tokenizer->input.len) {
        switch (TokenizerPeek(tokenizer)) {
        case '.': {
            if (is_decimal || radix != 10) goto end;
            is_decimal = true;
        } break;

        case '_': {
            if (tokenizer->pos == start) goto end;
            char prev_char = StringGetChar(&tokenizer->input, tokenizer->pos - 1);
            if (prev_char == 'e' || prev_char == 'E') goto end;
        } break;

        /* 0 and 1 are valid in all radices */
        case '0': {
            if (tokenizer->pos == start && tokenizer->pos < tokenizer->input.len - 1) {
                tokenizer->pos += 1;
                char maybe_radix = TokenizerPeek(tokenizer);
                switch (maybe_radix) {
                case 'x':
                case 'X':
                    radix = 16;
                    break;

                case 'o':
                case 'O':
                    radix = 8;
                    break;

                case 'b':
                case 'B':
                    radix = 2;
                    break;

                default:
                    break;
                }
                continue;
            }
        }
        case '1':
            break;

        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            if (radix < 8) goto end;
        } break;

        case '8':
        case '9': {
            if (radix < 10) goto end;
        } break;

        case 'e':
        case 'E': {
            if (tokenizer->pos == start) goto end;
            if (radix == 10 && exponent_idx == 0) {
                char prev_char = StringGetChar(&tokenizer->input, tokenizer->pos - 1);
                if (prev_char == '_') goto end;
                exponent_idx = tokenizer->pos;
            } else if (radix != 16) {
                goto end;
            }
        } break;

        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        // `e` and `E` are handled above
        case 'f':
        case 'F': {
            if (radix < 16) goto end;
        } break;

        default:
            goto end;
        }
        tokenizer->pos += 1;
    }
end:
    if (tokenizer->pos == start) return (Token){0};
    String token_string = StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
    return (Token){.s = token_string, .type = TokenTypeNumber};
}

Token TokenizerGetKeywordOrIdent(Tokenizer *tokenizer) {
    char current_char = TokenizerPeek(tokenizer);
    assert(CharIsAlnum(current_char));
    String token_string = TokenizerConsumeWhile(tokenizer, CharIsAlnum);
    assert(StringCount(&token_string, '\n') == 0 && "tokens should be on one line");

    for (u32 i = 0; i < sizeof(PythonKeywords) / sizeof(char *); i += 1) {
        u32 len = strlen(PythonKeywords[i]);
        if (len != token_string.len) continue;
        if (memcmp(token_string.buffer, PythonKeywords[i], len) == 0) {
            TokenType type = i <= 2 ? TokenTypeConstant : TokenTypeKeyword;
            return (Token){.type = type, .s = token_string};
        }
    }

    return (Token){.type = TokenTypeIdent, .s = token_string};
}

char TokenizerPeek(Tokenizer *tokenizer) {
    assert(tokenizer->pos < tokenizer->input.len && "tokenizer should be in bounds");
    char c = StringGetChar(&tokenizer->input, tokenizer->pos);
    return c;
}

char TokenizerConsume(Tokenizer *tokenizer) {
    char c = TokenizerPeek(tokenizer);
    tokenizer->pos += 1;
    return c;
}

String TokenizerConsumeWhile(Tokenizer *tokenizer, bool (*Predicate)(char)) {
    u32 start = tokenizer->pos;
    while (tokenizer->pos < tokenizer->input.len && Predicate(TokenizerPeek(tokenizer))) {
        tokenizer->pos += 1;
    }
    return StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
}

bool TokenizerIsPunct(Tokenizer *tokenizer) {
    assert(tokenizer->pos < tokenizer->input.len && "tokenizer should be in bounds");
    if (TokenizerPeek(tokenizer) == '.') {
        // number may start with a dot
        if (tokenizer->pos + 1 < tokenizer->input.len) {
            return !CharIsDigit(StringGetChar(&tokenizer->input, tokenizer->pos + 1));
        } else {
            return true;
        }
    }
    return false;
}
