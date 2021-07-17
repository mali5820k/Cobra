#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/**
 *  This is the parser "object" which will keep track of the current and previous tokens
 *  that are being read from the user's code.
*/
typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode; // Since we don't have exceptions in C, we need to have a panic mode incase of an error
} Parser;

/**
 * The enums to enumerate all of the precedence types.
*/
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

/**
 * This is a function pointer type that allows us to use the name of this
 * type without having to handle that declaration within consecutive structs.
*/
typedef void (*ParseFn)(); // Function Pointer Type


/**
 * This is the struct that will hold function pointers and the precedence of a particular token
 * so when a token needs to be consumed and 
*/
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk* compilingChunk;

/**
 * Returns the current chunk that is being compiled.
*/
static Chunk* currentChunk() {
    return compilingChunk;
}

/**
 * Handler of an error. Prints out the message and prepares to break out of compiling
 * by setting up error flags in the parser.
*/
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

/**
 * Output error with message.
*/
static void error(const char* message) {
    errorAt(&parser.previous, message);
}

/**
 * Output message for error at current token.
*/
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

/**
 * Parses through the current token and stops.
 * Will handle the first error encountered if one is encountered.
*/
static void advance() {
    parser.previous = parser.current;

    for(;;) {
        parser.current = scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

/**
 * Consumes current Token or outputs error message.
*/
static void consume(TokenType type, const char* message) {
    if(parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

/**
 * Returns true if the current token has the given type.
*/
static bool check(TokenType type) {
    return parser.current.type == type;
}

/**
 * If the provided token is the same as the one we compare it to,
 * then advance and return true, 
 * otherwise return false;
*/
static bool match(TokenType type) {
    if(!check(type)) return false;
    advance();
    return true;
}

/**
 * Emits a singular byte to a chunk.
*/
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

/**
 * Emits bytes for input bytes 1 and 2 to the current chunk by calling emitByte.
*/
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

/**
 * Return instruction opcode.
*/
static void emitReturn() {
    emitByte(OP_RETURN);
}

/**
 * Creates an instruction from a constant value.
*/
static uint8_t makeConstant(Value value) {
    // Future Note:
    // May want to make an instruction that is larger than UINT8_MAX, like OP_CONST_16 for two-byte operands.
    int constant = addConstant(currentChunk(), value);
    if(constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

/**
 * Handles emitting a constant number.
*/
static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

/**
 * Ends the compiling process.
*/
static void endCompiler() {
    emitReturn();
    
    // This is only for printing out chunks
    #ifdef DEBUG_PRINT_CODE
    if(!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
    #endif
}

/** 
 * Function prototypes for these select methods.
*/
static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

/**
 * For Mathematical operators and those respective operations.
*/ 
static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule -> precedence + 1));

    // Later will use OP_GREATER_EQUAL as well as OP_LESS_EQUAL
    // for the OP codes of TOKEN_GREATER_LESS and TOKEN_LESS_EQUAL.
    switch(operatorType) {
        case TOKEN_BANG_EQUAL:      emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:     emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:         emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:   emitBytes(OP_LESS, OP_NOT); break; 
        case TOKEN_LESS:            emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:      emitBytes(OP_GREATER_EQUAL, OP_NOT); break;
        case TOKEN_PLUS:            emitByte(OP_ADD); break;
        case TOKEN_MINUS:           emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:            emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:           emitByte(OP_DIVIDE); break;
        default:
            return; // Shouldn't reach this part ever!
    }
}

/**
 * Literals are things you see on the right-hand side of an expression,
 * or even terms used for depicting logic
 * like false, null, and true.
*/
static void literal() {
    switch(parser.previous.type) {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NULL: emitByte(OP_NULL); break;
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        default: return;
    }
}

/**
 * For parenthesis grouping. May need to edit the error messages later.
*/ 
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "expect ')' after expression.");
}

/**
 * Emmitting constant numerical values.
*/ 
static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

/**
 * Produces instructions for strings. Future goals include handling of escape characters.
*/
static void string() {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length -2)));
}

/**
 * Unary operations are in-place logic operations, for example:
 * var x = -(x + y) * 2;
 * The -(x + y) block will be processed first as a unary (one) operation
 * then the processing on this expression will halt until another hander function is called
 * like binary. 
*/
static void unary() {
    // Checkout https://craftinginterpreters.com/compiling-expressions.html#unary-negation
    // For the note on the multi-line negation expression error handling

    TokenType operatorType = parser.previous.type;

    // Compile the operand
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch(operatorType) {
        case TOKEN_BANG: emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return; // Unreachable
    }
}

/**
 * This is what is known as a
 * Lookup Designated Initializer
 * This is essentially an array of ParseRule structs which are created
 * This is basically a look-up table of ParseRules.
*/
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
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_LESS]          = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NULL]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

/**
 * Read the next token and then look up the rule for that token.
 * If a rule isn't found for a token, then that's a syntax error.
 * Both the prefix and infix parts of an expression get parsed.
*/
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

/**
 * Looks up the rule for a particular token type.
*/
static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

/**
 * Calls parsePrecedence to parse an expression with precedence.
*/
static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

/**
 * An expression that is followed by a semicolon is an expression statement.
 * This function will compile said expression.
*/
static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

/**
 * Evaluates the expression provided and prints it out.
*/
static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "EXPECT ';' after value.");
    emitByte(OP_PRINT);
}

/**
 * Synchronize after a compile error.
 * Essentially the last token or a "boundary marker" is found
 * so the current error can have a point of reference with the prevention of
 * subsequent errors.
*/
static void synchronize() {
    parser.panicMode = false;

    while(parser.current.type != TOKEN_EOF) {
        if(parser.previous.type == TOKEN_SEMICOLON) return;
        switch(parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUNCTION:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                ;
        }

        advance();
    }
}

/**
 * Compiles a single declaration.
*/
static void declaration() {
    statement();

    if(parser.panicMode) synchronize();
}

/**
 * Compiles statements.
*/
static void statement() {
    if(match(TOKEN_PRINT)) {
        printStatement();
    }
    else {
        expressionStatement();
    }
}

/**
 * The function that calls and pieces the functions of the compiler together and runs.
 * Think of this as the "main" method of the compiler.
*/
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    
    while (!match(TOKEN_EOF)) {
        declaration();
    }

    endCompiler();
    return !parser.hadError;
}