#pragma once

#include "core.h"
#include "string.h"

typedef enum TokenType {
    TokenTypeNone = 0,

    /// Python keywords
    TokenTypeKeywordAwait,
    TokenTypeKeywordElse,
    TokenTypeKeywordImport,
    TokenTypeKeywordPass,
    TokenTypeKeywordBreak,
    TokenTypeKeywordExcept,
    TokenTypeKeywordIn,
    TokenTypeKeywordRaise,
    TokenTypeKeywordClass,
    TokenTypeKeywordFinally,
    TokenTypeKeywordIs,
    TokenTypeKeywordReturn,
    TokenTypeKeywordAnd,
    TokenTypeKeywordContinue,
    TokenTypeKeywordFor,
    TokenTypeKeywordLambda,
    TokenTypeKeywordTry,
    TokenTypeKeywordAs,
    TokenTypeKeywordDef,
    TokenTypeKeywordFrom,
    TokenTypeKeywordNonlocal,
    TokenTypeKeywordWhile,
    TokenTypeKeywordAssert,
    TokenTypeKeywordDel,
    TokenTypeKeywordGlobal,
    TokenTypeKeywordNot,
    TokenTypeKeywordWith,
    TokenTypeKeywordAsync,
    TokenTypeKeywordElif,
    TokenTypeKeywordIf,
    TokenTypeKeywordOr,
    TokenTypeKeywordYield,

    TokenTypePunctComa,      //> ,
    TokenTypePunctDot,       //> .
    TokenTypePunctColon,     //> :
    TokenTypePunctSemicolon, //> ;

    TokenTypeParenhesisOpen,  //> (
    TokenTypeParenhesisClose, //> )

    TokenTypeSquareBracketOpen,  //> [
    TokenTypeSquareBracketClose, //> ]
    // Just in case Python has these...
    TokenTypeCurlyBracketOpen,  //> {
    TokenTypeCurlyBracketClose, //> }

    TokenTypeQuote,       //> '
    TokenTypeDoubleQuote, //> "

    /// Newlines are parsed since source code has
    /// to be printed back to the terminal
    TokenTypeWhitespace,
    TokenTypeNewLine,

    /// Any alpha numerical identifier
    TokenTypeIdent,

    /// (Multiline) string
    TokenTypeString,

    /// Number, see <https://peps.python.org/pep-0515/#literal-grammar>
    TokenTypeNumber,

    TokenTypeConstantTrue,  //> True
    TokenTypeConstantFalse, //> False
    TokenTypeConstantNone,  //> None

    /// Math operators
    TokenTypeMathAdd,       //> +
    TokenTypeMathSubtract,  //> -
    TokenTypeMathMultiply,  //> *
    TokenTypeMathDivide,    //> /
    TokenTypeMathIntDivide, //> //
    TokenTypeMathModulo,    //> %
    TokenTypeCmpLe,         //> <
    TokenTypeCmpLq,         //> <=
    TokenTypeCmpGe,         //> >
    TokenTypeCmpGq,         //> >=
    TokenTypeCmpEq,         //> ==

    TokenTypeAssignment, //> =

    /// Logical operators
    TokenTypeLogicalAnd, //> &
    TokenTypeLogicalOr,  //> |
    TokenTypeLogicalXor, //> ^
    TokenTypeLogicalNot, //> ~

    TokenTypeHashtag,
    TokenTypeBackslash,

    /// This includes `#` and everything after it til \n
    TokenTypeComment,
} TokenType;

typedef struct Token {
    TokenType type;
    String    s;
} Token;

typedef struct Tokenizer {
    String input;
    u32    pos;
} Tokenizer;

static char *PythonKeywords[] = {
    "False",  "True",  "None",  "await",   "else", "import",   "pass",  "break",    "except",
    "in",     "raise", "class", "finally", "is",   "return",   "and",   "continue", "for",
    "lambda", "try",   "as",    "def",     "from", "nonlocal", "while", "assert",   "del",
    "global", "not",   "with",  "async",   "elif", "if",       "or",    "yield",
};

static TokenType PythonKeywordsTType[] = {
    TokenTypeConstantFalse,  TokenTypeConstantTrue,    TokenTypeConstantNone,
    TokenTypeKeywordAwait,   TokenTypeKeywordElse,     TokenTypeKeywordImport,
    TokenTypeKeywordPass,    TokenTypeKeywordBreak,    TokenTypeKeywordExcept,
    TokenTypeKeywordIn,      TokenTypeKeywordRaise,    TokenTypeKeywordClass,
    TokenTypeKeywordFinally, TokenTypeKeywordIs,       TokenTypeKeywordReturn,
    TokenTypeKeywordAnd,     TokenTypeKeywordContinue, TokenTypeKeywordFor,
    TokenTypeKeywordLambda,  TokenTypeKeywordTry,      TokenTypeKeywordAs,
    TokenTypeKeywordDef,     TokenTypeKeywordFrom,     TokenTypeKeywordNonlocal,
    TokenTypeKeywordWhile,   TokenTypeKeywordAssert,   TokenTypeKeywordDel,
    TokenTypeKeywordGlobal,  TokenTypeKeywordNot,      TokenTypeKeywordWith,
    TokenTypeKeywordAsync,   TokenTypeKeywordElif,     TokenTypeKeywordIf,
    TokenTypeKeywordOr,      TokenTypeKeywordYield,
};

static char *TTypeToString[] = {
    "TokenTypeNone",

    "TokenTypeKeywordAwait",
    "TokenTypeKeywordElse",
    "TokenTypeKeywordImport",
    "TokenTypeKeywordPass",
    "TokenTypeKeywordBreak",
    "TokenTypeKeywordExcept",
    "TokenTypeKeywordIn",
    "TokenTypeKeywordRaise",
    "TokenTypeKeywordClass",
    "TokenTypeKeywordFinally",
    "TokenTypeKeywordIs",
    "TokenTypeKeywordReturn",
    "TokenTypeKeywordAnd",
    "TokenTypeKeywordContinue",
    "TokenTypeKeywordFor",
    "TokenTypeKeywordLambda",
    "TokenTypeKeywordTry",
    "TokenTypeKeywordAs",
    "TokenTypeKeywordDef",
    "TokenTypeKeywordFrom",
    "TokenTypeKeywordNonlocal",
    "TokenTypeKeywordWhile",
    "TokenTypeKeywordAssert",
    "TokenTypeKeywordDel",
    "TokenTypeKeywordGlobal",
    "TokenTypeKeywordNot",
    "TokenTypeKeywordWith",
    "TokenTypeKeywordAsync",
    "TokenTypeKeywordElif",
    "TokenTypeKeywordIf",
    "TokenTypeKeywordOr",
    "TokenTypeKeywordYield",

    "TokenTypePunctComa",
    "TokenTypePunctDot",
    "TokenTypePunctColon",
    "TokenTypePunctSemicolon",
    "TokenTypePunctBackslash",

    "TokenTypeParenhesisOpen",
    "TokenTypeParenhesisClose",

    "TokenTypeQuote",
    "TokenTypeDoubleQuote",

    "TokenTypeSquareBracketOpen",
    "TokenTypeSquareBracketClose",

    "TokenTypeCurlyBracketOpen",
    "TokenTypeCurlyBracketClose",

    "TokenTypeWhitespace",
    "TokenTypeNewLine",

    "TokenTypeIdent",

    "TokenTypeString",

    "TokenTypeNumber",

    "TokenTypeConstantTrue",
    "TokenTypeConstantFalse",
    "TokenTypeConstantNone",

    "TokenTypeMathAdd",
    "TokenTypeMathSubtract",
    "TokenTypeMathMultiply",
    "TokenTypeMathDivide",
    "TokenTypeMathIntDivide",
    "TokenTypeMathModulo",
    "TokenTypeCmpLe",
    "TokenTypeCmpLq",
    "TokenTypeCmpGe",
    "TokenTypeCmpGq",
    "TokenTypeCmpEq",

    "TokenTypeAssignment",

    "TokenTypeLogicalAnd",
    "TokenTypeLogicalOr",
    "TokenTypeLogicalXor",
    "TokenTypeLogicalNot",

    "TokenTypeHashtag",

    "TokenTypeComment",
};

// Maps ASCII characters (in range [0, 127]) to a one-symbol `TokenType`.
// Note that some of them could require additional parsing, like a dot
// (a number may start with a dot, .15)
static TokenType CharToTType[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TokenTypeNewLine,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TokenTypeWhitespace,
    0,
    TokenTypeDoubleQuote,
    TokenTypeHashtag,
    0,
    TokenTypeMathModulo,
    TokenTypeLogicalAnd,
    TokenTypeQuote,
    TokenTypeParenhesisOpen,
    TokenTypeParenhesisClose,
    TokenTypeMathMultiply,
    TokenTypeMathAdd,
    TokenTypePunctComa,
    TokenTypeMathSubtract,
    TokenTypePunctDot,
    TokenTypeMathDivide,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TokenTypePunctColon,
    TokenTypePunctSemicolon,
    TokenTypeCmpLe,
    TokenTypeAssignment,
    TokenTypeCmpGe,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TokenTypeSquareBracketOpen,
    TokenTypeBackslash,
    TokenTypeSquareBracketClose,
    TokenTypeLogicalXor,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TokenTypeCurlyBracketOpen,
    TokenTypeLogicalOr,
    TokenTypeCurlyBracketClose,
    TokenTypeLogicalNot,
    0,
};

Token TokenizerNext(Tokenizer *tokenizer);

bool TokenTypeIsKeyword(TokenType type);

Token TokenizerNumber(Tokenizer *tokenizer);
Token TokenizerKeywordOrIdent(Tokenizer *tokenizer);

char   TokenizerPeek(Tokenizer *tokenizer);
char   TokenizerConsume(Tokenizer *tokenizer);
String TokenizerConsumeWhile(Tokenizer *tokenizer, bool (*predicate)(char));
void   TokenizerSkipUntil(Tokenizer *tokenizer, char until);

bool TokenizerIsAtMultiString(Tokenizer *tokenizer);

Token TokenizerNext(Tokenizer *tokenizer) {
    assert(StringCount(&tokenizer->input, '\n') <= 1 && "only one line inputs are supported");

    if (tokenizer->pos >= tokenizer->input.len) return (Token){0};

    u32  start = tokenizer->pos;
    char current_char = TokenizerPeek(tokenizer);
    assert(current_char <= 127);

    TokenType ttype = CharToTType[current_char];

    if (!ttype) {
        if (CharIsDigit(current_char)) {
            Token maybe_number = TokenizerNumber(tokenizer);
            if (maybe_number.type == TokenTypeNumber) return maybe_number;
        }

        return TokenizerKeywordOrIdent(tokenizer);
    }
    // consume the chacter
    tokenizer->pos += 1;
    switch (ttype) {
        case TokenTypeHashtag: {
            tokenizer->pos = StringSearchNthAddOne(&tokenizer->input, 1, '\n');
            String token_string = StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
            return (Token){.type = TokenTypeComment, .s = token_string};
        } break;

        // TODO: parse multi-lines.
        // TODO: regular lines end at a matching quote or \n
        case TokenTypeDoubleQuote:
        case TokenTypeQuote: {
            if (TokenizerIsAtMultiString(tokenizer)) {
                assert(false && "todo: multistrings");
            }
            // find a matching quote
            tokenizer->pos = StringSearchNthAddOne(&tokenizer->input, 2, current_char);
            String token_string = StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
            return (Token){.type = TokenTypeString, .s = token_string};
        } break;

        case TokenTypePunctDot: {
            if (CharIsDigit(TokenizerPeek(tokenizer))) {
                // return back to the consumed dot
                // and parse number
                tokenizer->pos -= 1;
                return TokenizerNumber(tokenizer);
            }
            // fall-through
        };

        default: {
            String token_string = StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
            return (Token){.type = ttype, .s = token_string};
        }
    }
}

bool TokenTypeIsKeyword(TokenType type) {
    return type >= TokenTypeKeywordAwait && type <= TokenTypeKeywordYield;
}

bool TokenTypeIsPunct(TokenType type) {
    return type >= TokenTypePunctComa && type <= TokenTypeSquareBracketClose;
}

Token TokenizerNumber(Tokenizer *tokenizer) {
    bool is_decimal = false;
    u32  start = tokenizer->pos, radix = 10, exponent_idx = start;
    while (tokenizer->pos < tokenizer->input.len) {
        switch (TokenizerPeek(tokenizer)) {
            case '.': {
                if (is_decimal || radix != 10 || exponent_idx != start) goto end;
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
                    // consume this `0`
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

                        // unknown character,
                        // let the next iteration decide
                        default:
                            continue;
                    }
                    // consume the radix
                    tokenizer->pos += 1;
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
                if (radix == 10 && exponent_idx == start) {
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
    assert(tokenizer->pos != start && "number must be valid");
    String token_string = StringSliceFromTo(&tokenizer->input, start, tokenizer->pos);
    return (Token){.s = token_string, .type = TokenTypeNumber};
}

Token TokenizerKeywordOrIdent(Tokenizer *tokenizer) {
    char current_char = TokenizerPeek(tokenizer);
    assert(CharIsAlnum(current_char));
    String token_string = TokenizerConsumeWhile(tokenizer, CharIsAlnum);
    assert(token_string.len > 0 && "identifiers are at least 1 char long");
    assert(StringCount(&token_string, '\n') == 0 && "tokens should be on one line");

    for (u32 i = 0; i < sizeof(PythonKeywords) / sizeof(char *); i += 1) {
        u32 len = strlen(PythonKeywords[i]);
        if (len != token_string.len) continue;
        if (memcmp(token_string.buffer, PythonKeywords[i], len) == 0) {
            TokenType type = PythonKeywordsTType[i];
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

bool TokenizerIsAtMultiString(Tokenizer *tokenizer) {
    // multistring starts with """
    if (tokenizer->pos + 2 >= tokenizer->input.len) return false;
    for (u32 i = 0; i < 2; i += 1) {
        if (StringGetChar(&tokenizer->input, tokenizer->pos + i) != '"') {
            return false;
        }
    }
    return true;
}
