#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode; // Since we don't have exceptions in C, we need to have a panic mode incase of an error
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(); // Function Pointer Type

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

// Handler of an error. Prints out the message and prepares to break out of compiling
// by setting up error flags in the parser.
static void errorAt(Token* token, const char* message) {
    if(parser.panicMode) return; // To prevent any cascading errors from source error from being reported
    parser.panicMode = true;
    fprintf(stderr, "[line%d] Error", token -> line);

    if(token -> type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if(token -> type == TOKEN_ERROR) {
        // Do nothing
    }
    else {
        fprintf(stderr, " at '%.*s'", token -> length, token -> start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Output error with message
static void error(const char* message) {
    errorAt(&parser.previous, message);
}

// Output message for error at current token
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

// Parses through the current token and stops.
// Will handle error if one is encountered.
static void advance() {
    parser.previous = parser.current;

    for(;;) {
        parser.current = scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

// Consumes current Token or outputs error message
static void consume(TokenType type, const char* message) {
    if(parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// Emits Bytes for input bytes 1 and 2
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

// Return instruction
static void emitReturn() {
    emitByte(OP_RETURN);
}

// Future Note:
// May want to make an instruction that is larger than UINT8_MAX, like OP_CONST_16 for two-byte operands.
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if(constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

// Handles emitting a constant number
static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

// Ends the compiling process
static void endCompiler() {
    emitReturn();
    
    // This is only for printing out chunks
    #ifdef DEBUG_PRINT_CODE
    if(!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
    #endif
}

// Function prototypes for these select methods
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

// For Mathematical operators
static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule -> precedence + 1));

    switch(operatorType) {
        case TOKEN_PLUS: 
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
        default:
            return; // Shouldn't reach this part ever!
    }
}

// For parenthesis grouping. May need to edit the error messages later
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "expect ')' after expression.");
}

// Emmitting constant numerical values
static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// Checkout https://craftinginterpreters.com/compiling-expressions.html#unary-negation
// For the note on the multi-line negation expression error handling
static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch(operatorType) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return; // Unreachable
    }
}

// Lookup Designated initializer syntax in C
// This is a rules look-up table
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NULL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

// Read the next token and then look up the rule for that token.
// If a rule isn't found for a token, then that's a syntax error.
// Both the prefix and infix parts of an expression get parsed.
static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type) -> prefix;
    if(prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    prefixRule();
    while(precedence <= getRule(parser.current.type) -> precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type) -> infix;
        infixRule();
    }
}

// Looks up the rule for a particular token type
static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}