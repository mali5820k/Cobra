#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
//typedef void (*ParseFn)();
typedef void (*ParseFn)(bool canAssign); // Function Pointer Type


/**
 * This is the struct that will hold function pointers and the precedence of a particular token
 * so when a token needs to be consumed and 
*/
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;

/** 
 * Function prototypes for all methods.
*/
static Chunk* currentChunk();
static void errorAt(Token* token, const char* message);
static void error(const char* message);
static void errorAtCurrent(const char* message);
static void advance();
static void consume(TokenType type, const char* message);
static bool check(TokenType type);
static bool match(TokenType type);
static void emitByte(uint8_t byte);
static void emitBytes(uint8_t byte1, uint8_t byte2);
static int emitJump(uint8_t instruction);
static void emitReturn();
static uint8_t makeConstant(Value value);
static void emitConstant(Value value);
static void patchJump(int offset);
static void initCompiler(Compiler* compiler);
static void endCompiler();
static void beginScope();
static void endScope();
static uint8_t identifierConstant(Token* name);
static bool identifiersEqual(Token* a, Token* b);
static int resolveLocal(Compiler* compiler, Token* name);
static void addLocal(Token name);
static void declareVariable();
static uint8_t parseVariable(const char* errorMessage);
static void markInitialized();
static void defineVariable(uint8_t global);
static void binary(bool canAssign);
static void literal(bool canAssign);
static void grouping(bool canAssign);
static void number(bool canAssign);
static void or_(bool canAssign);
static void string(bool canAssign);
static void namedVariable(Token name, bool canAssign);
static void variable(bool canAssign);
static void unary(bool canAssign);
static void parsePrecedence(Precedence precedence);
static ParseRule* getRule(TokenType type);
static void expression();
static void block();
static void varDeclaration();
static void expressionStatement();
static void forStatement();
static void ifStatement();
static void printStatement();
static void whileStatement();
static void synchronize();
static void declaration();
static void statement();
bool compile(const char* source, Chunk* chunk);

Parser parser;
Compiler* current = NULL;
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
    fprintf(stderr, "[line%d] Error", token->line);

    if(token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if(token->type == TOKEN_ERROR) {
        // Do nothing
    }
    else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
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

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if(offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
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

static void patchJump(int offset) {
    int jump = currentChunk()->count - offset - 2;

    if(jump > UINT16_MAX) {
        // potentially add  another function for jumping farther
        // will also need to either create new emitJumpLong and edit vm.c for OP_JUMP_IF_FALSE case
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler) {
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
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

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;

    while(current->localCount > 0 && current->locals[current->localCount -1].depth > current->scopeDepth) {
        emitByte(OP_POP);
        current->localCount--;
    }
}

/**
 * The name of a variable is stored as a string in a Chunk's constant table.
*/
static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if(a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
    for(int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if(identifiersEqual(name, &local->name)) {
            if(local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static void addLocal(Token name) {
    // Ideally, try to increase this count to a much larger value or revamp the bytecode representation for more
    // Currently, you can't have more than 256 local variables in a scope at a time.
    // An idea is to have a dynamic array of local variables.
    // Or a drop easy solution would be to use UINT32 (4 bytes) or some large value that will be highly unlikely to be acheived, like UINT64 (8 bytes)
    if(current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

static void declareVariable() {
    if(current->scopeDepth == 0) return; // If in global scope, return

    Token* name = &parser.previous;
    for(int i = current->localCount -1; i >= 0; i--) {
        Local* local = &current->locals[i];
        if(local->depth != -1 &&  local->depth < current->scopeDepth) {
            break;
        }

        if(identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

/**
 * Reads in a variable declaration with name and value.
*/
static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if(current->scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

static void markInitialized() {
    current->locals[current->localCount -1].depth = current->scopeDepth;
}

/**
 * The index that a variable's name is assigned in the constant table within a Chunk.
*/
static void defineVariable(uint8_t global) {
    // If the variable that's being defined isn't in the global scope depth, then return
    if(current->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void and_(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

/**
 * For Mathematical operators and those respective operations.
*/ 
static void binary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

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
static void literal(bool canAssign) {
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
static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/**
 * Emmitting constant numerical values.
*/ 
static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
    // This can be made even more efficient
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

/**
 * Produces instructions for strings. Future goals include handling of escape characters.
*/
static void string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * Calls identifierConstant() for taking a token and adding it to a Chunk's constant table as a string.
 * "Helper Function"
*/
static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if(arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if(canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

/**
 * Variable parser function.
*/
static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

/**
 * Unary operations are in-place logic operations, for example:
 * var x = -(x + y) * 2;
 * The -(x + y) block will be processed first as a unary (one) operation
 * then the processing on this expression will halt until another hander function is called
 * like binary. 
*/
static void unary(bool canAssign) {
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
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NULL]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
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
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if(prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);
    
    while(precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if(canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
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

static void block() {
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/**
 * Compiles variable declarations.
*/
static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if(match(TOKEN_EQUAL)) {
        expression();
    }
    else {
        emitByte(OP_NULL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
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

static void forStatement() {
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if(match(TOKEN_SEMICOLON)) {} // This is one of those infinite loop conditions or no variable declared
    else if(match(TOKEN_VAR)) {
        varDeclaration();
    }
    else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;
    int exitJump = -1;
    if(!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    if(!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if(exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);
    }

    endScope();
}

static void ifStatement() {
    // A way to really make this multi-functional would be to have a switch-case statement like we do for statement() and vm.c's run()
    // that way, we can have multiple ways to write an if statement, even the one like python
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);

    if(match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

/**
 * Evaluates the expression provided and prints it out.
*/
static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "EXPECT ';' after value.");
    emitByte(OP_PRINT);
}

static void whileStatement() {
    int loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
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
    if(match(TOKEN_VAR)) {
        varDeclaration();
    }
    else {
        statement();
    }
    
    if(parser.panicMode) synchronize();
}

/**
 * Compiles statements.
*/
static void statement() {
    if(match(TOKEN_PRINT)) {
        printStatement();
    }
    else if(match(TOKEN_FOR)) {
        forStatement();
    }
    else if(match(TOKEN_IF)) {
        ifStatement();
    }
    else if(match(TOKEN_WHILE)) {
        whileStatement();
    }
    else if(match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
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
    Compiler compiler;
    initCompiler(&compiler);
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