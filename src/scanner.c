#include "scanner.h"

#include <string.h>

static bool is_at_end();
static token_t make_token(token_kind_t kind);
static token_t error_token(const char *errmsg);
static char advance();
static bool match(char expected);
static void skip_whitespace();
static char peek();
static char peek_next();
static bool is_digit(char c);
static bool is_alpha(char c);
static token_t string();
static token_t number();
static token_t identifier();
static token_kind_t identifier_type();
static token_kind_t check_keyword(int start, int length, const char *rest, token_kind_t type);

typedef struct
{
    const char *start;
    const char *current;
    int line;
} scanner_t;

scanner_t scanner;

void init_scanner(const char *source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

token_t scan_token()
{
    skip_whitespace();

    scanner.start = scanner.current;

    if (is_at_end())
        return make_token(TOKEN_EOF);

    char c = advance();
    if (is_digit(c))
        return number();
    if (is_alpha(c))
        return identifier();

    switch (c)
    {
    case '(':
        return make_token(TOKEN_LEFT_PAREN);
    case ')':
        return make_token(TOKEN_RIGHT_PAREN);
    case '{':
        return make_token(TOKEN_LEFT_BRACE);
    case '}':
        return make_token(TOKEN_RIGHT_BRACE);
    case ';':
        return make_token(TOKEN_SEMICOLON);
    case ',':
        return make_token(TOKEN_COMMA);
    case '.':
        return make_token(TOKEN_DOT);
    case '-':
        return make_token(TOKEN_MINUS);
    case '+':
        return make_token(TOKEN_PLUS);
    case '/':
        return make_token(TOKEN_SLASH);
    case '*':
        return make_token(TOKEN_STAR);
    case '!':
        return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
        return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
        return string();
    };

    return error_token("Unexpected character.");
}

static bool is_at_end()
{
    return *scanner.current == '\0';
}

static token_t make_token(token_kind_t kind)
{
    token_t token;
    token.kind = kind;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static token_t error_token(const char *errmsg)
{
    token_t token;
    token.kind = TOKEN_ERROR;
    token.start = errmsg;
    token.length = (int)strlen(errmsg);
    token.line = scanner.line;

    return token;
}

static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static bool match(char expected)
{
    if (is_at_end())
        return false;
    if (*scanner.current != expected)
        return false;

    scanner.current++;

    return true;
}

static void skip_whitespace()
{
    while (true)
    {
        char c = peek();

        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            scanner.line++;
            advance();
            break;
        case '/':
            if (peek_next() == '/')
            {
                while (peek() != '\n' && !is_at_end())
                    advance();
            }
            else
                return;

            break;
        default:
            return;
        }
    }
}

static char peek()
{
    return *scanner.current;
}

static char peek_next()
{
    if (is_at_end())
        return '\0';

    return scanner.current[1];
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c)
{
    // LOL.
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c >= 'Z') || c == '_';
}

static token_t string()
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n')
            scanner.line++;
        advance();
    }

    if (is_at_end())
        return error_token("Unterminated string.");

    advance();

    return make_token(TOKEN_STRING);
}

static token_t number()
{
    while (is_digit(peek()))
        advance();

    if (peek() == '.' && is_digit(peek_next()))
    {
        advance();

        while (is_digit(peek()))
            advance();
    }

    return make_token(TOKEN_NUMBER);
}

static token_t identifier()
{
    while (is_alpha(peek()) || is_digit(peek()))
        advance();

    return make_token(identifier_type());
}

static token_kind_t identifier_type()
{
    switch (scanner.start[0])
    {
    case 'a':
        return check_keyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return check_keyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return check_keyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'a':
                return check_keyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return check_keyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return check_keyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return check_keyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return check_keyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return check_keyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return check_keyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return check_keyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return check_keyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (scanner.current - scanner.start > 1)
        {
            switch (scanner.start[1])
            {
            case 'h':
                return check_keyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return check_keyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return check_keyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static token_kind_t check_keyword(int start, int length, const char *rest, token_kind_t type)
{
    if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, (size_t)length) == 0)
        return type;

    return TOKEN_IDENTIFIER;
}
