#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arraylist.h"

//===----------------------------------------------------------------------===//
// Common
//===----------------------------------------------------------------------===//

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_attribute(noreturn)
#define NO_RETURN __attribute__((__noreturn__))
#else
#define NO_RETURN
#endif

//===----------------------------------------------------------------------===//
// Error handling
//===----------------------------------------------------------------------===//

#ifndef NDEBUG
#define UNREACHABLE(msg) unreachable_impl(msg, __FILE__, __LINE__)
#elif __has_builtin(__builtin_unreachable)
#define UNREACHABLE(msg) __builtin_unreachable()
#else
#define UNREACHABLE(msg) unreachable_impl(0, 0, 0)
#endif

NO_RETURN static void unreachable_impl(const char *msg, const char *filename,
                                       unsigned int line) {
  if (msg)
    fprintf(stderr, "%s\n", msg);
  fprintf(stderr, "UNREACHABLE executed");
  if (filename)
    fprintf(stderr, " at %s:%u", filename, line);
  fprintf(stderr, "!\n");
  abort();
#if __has_builtin(__builtin_unreachable)
  __builtin_unreachable();
#endif
}

NO_RETURN static void die(const char *msg) {
  fprintf(stderr, "\033[31mFATAL ERROR\033[0m %s\nExiting program\n", msg);
  exit(1);
}

//===----------------------------------------------------------------------===//
// Expressions
//===----------------------------------------------------------------------===//

typedef enum ExpressionType {
  EXPR_NUMBER,
  EXPR_BINARY_OP,
  EXPR_UNARY_OP
} ExpressionType;

typedef enum BinaryOperator {
  BINARY_OP_ADD,
  BINARY_OP_SUB,
  BINARY_OP_MUL,
  BINARY_OP_DIV
} BinaryOperator;

typedef enum UnaryOperator { UNARY_OP_PLUS, UNARY_OP_NEG } UnaryOperator;

typedef enum NumberType { NUMBER_INTEGER } NumberType;

typedef struct Number {
  NumberType type;
  union {
    // TODO: Add fractions
    int integer;
  };
} Number;

typedef struct BinaryOp {
  struct Expression *lhs;
  struct Expression *rhs;
  BinaryOperator op;
} BinaryOp;

typedef struct UnaryOp {
  struct Expression *expr;
  UnaryOperator op;
} UnaryOp;

typedef struct Expression {
  ExpressionType type;
  bool valid;
  union {
    BinaryOp binary_op;
    UnaryOp unary_op;
    Number number;
  };
} Expression;

static Expression *expr_invalid(void) {
  static Expression invalid = (Expression){.valid = false};
  return &invalid;
}

static void free_expr(Expression *expr) {
  if (!expr || expr == expr_invalid())
    return;
  switch (expr->type) {
  case EXPR_BINARY_OP:
    free_expr(expr->binary_op.lhs);
    free_expr(expr->binary_op.rhs);
    break;
  case EXPR_UNARY_OP:
    free_expr(expr->unary_op.expr);
    break;
  default:
    break;
  }
  free(expr);
}

static Expression *expr_integer(int value) {
  Expression *result = calloc(1, sizeof(Expression));
  result->type = EXPR_NUMBER;
  result->number.type = NUMBER_INTEGER;
  result->number.integer = value;
  result->valid = true;
  return result;
}

static Expression *expr_unary_op(UnaryOperator op, Expression *expr) {
  Expression *result = calloc(1, sizeof(Expression));
  result->type = EXPR_UNARY_OP;
  result->unary_op.op = op;
  result->unary_op.expr = expr;
  result->valid = (expr != 0) && expr->valid;
  return result;
}

static Expression *expr_unary_plus(Expression *expr) {
  return expr_unary_op(UNARY_OP_PLUS, expr);
}

static Expression *expr_unary_minus(Expression *expr) {
  return expr_unary_op(UNARY_OP_NEG, expr);
}

static Expression *expr_binary_op(BinaryOperator op, Expression *lhs,
                                  Expression *rhs) {
  Expression *result = calloc(1, sizeof(Expression));
  result->type = EXPR_BINARY_OP;
  result->binary_op.op = op;
  result->binary_op.lhs = lhs;
  result->binary_op.rhs = rhs;
  result->valid = (lhs != 0 && lhs->valid) && (rhs != 0 && rhs->valid);
  return result;
}

static Expression *expr_add(Expression *lhs, Expression *rhs) {
  return expr_binary_op(BINARY_OP_ADD, lhs, rhs);
}

static Expression *expr_sub(Expression *lhs, Expression *rhs) {
  return expr_binary_op(BINARY_OP_SUB, lhs, rhs);
}

static Expression *expr_mul(Expression *lhs, Expression *rhs) {
  return expr_binary_op(BINARY_OP_MUL, lhs, rhs);
}

static Expression *expr_div(Expression *lhs, Expression *rhs) {
  return expr_binary_op(BINARY_OP_DIV, lhs, rhs);
}

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

typedef enum TokenType {
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_TIMES,
  TOKEN_DIVIDE,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LSQUARE,
  TOKEN_RSQUARE,
  TOKEN_INTEGER,
  TOKEN_IDENTIFIER,
  TOKEN_EOF,
  TOKEN_CHARACTER
} TokenType;

typedef struct Location {
  const char *line;
  const char *start;
  const char *end;
} Location;

typedef struct Token {
  TokenType type;
  Location loc;
  union {
    struct {
      int value;
    } integer;
  };
} Token;

static Token *tokenize(const char *input) {
  Token *result = 0;

  const char *curr = input;
  size_t size = strlen(input);
  while (curr != input + size) {
    Token t =
        (Token){.loc.line = input, .loc.start = curr, .loc.end = curr + 1};

    switch (*curr) {
    case '+': {
      t.type = TOKEN_PLUS;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case '-': {
      t.type = TOKEN_MINUS;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case '*': {
      t.type = TOKEN_TIMES;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case '/': {
      t.type = TOKEN_DIVIDE;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case '(': {
      t.type = TOKEN_LPAREN;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case ')': {
      t.type = TOKEN_RPAREN;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case '[': {
      t.type = TOKEN_LSQUARE;
      arraylist_push(result, t);
      curr += 1;
    } break;
    case ']': {
      t.type = TOKEN_RSQUARE;
      arraylist_push(result, t);
      curr += 1;
    } break;
    default:
      if (isdigit(*curr)) {
        int value = 0;
        t.type = TOKEN_INTEGER;
        do {
          value = value * 10 + (*curr - '0');
          curr += 1;
        } while (curr < input + size && isdigit(*curr));
        t.integer.value = value;
        t.loc.end = curr;
        arraylist_push(result, t);
      } else if (isalpha(*curr) || *curr == '_') {
        t.type = TOKEN_IDENTIFIER;
        do {
          curr += 1;
        } while (curr < input + size && (isalnum(*curr) || *curr == '_'));
        t.loc.end = curr;
        arraylist_push(result, t);
      } else {
        // Unknown character
        t.type = TOKEN_CHARACTER;
        arraylist_push(result, t);
        curr += 1;
      }
      break;
    }
  }

  Token eof = (Token){
      .type = TOKEN_EOF, .loc.line = input, .loc.start = curr, .loc.end = curr};
  arraylist_push(result, eof);
  return result;
}

//===----------------------------------------------------------------------===//
// Parsing
//===----------------------------------------------------------------------===//

static Expression *parse_term(Token **tokens);
static Expression *parse_factor(Token **tokens);
static Expression *parse_parenthesize_expression(Token **tokens);
static Expression *parse_expression(Token **tokens);

static void parser_error(const char *msg, const Token *token) {
  printf("\033[31mERROR:\033[0m %s\n%s\n", msg, token->loc.line);
  for (ptrdiff_t i = 0; i < token->loc.start - token->loc.line; ++i)
    putc(' ', stdout);
  for (ptrdiff_t i = 0; i < token->loc.end - token->loc.start; ++i)
    putc('~', stdout);
  putc('\n', stdout);
}

static Expression *parse_factor(Token **const tokens) {
  Token *curr = *tokens;
  *tokens += 1;
  switch (curr->type) {
  case TOKEN_INTEGER:
    return expr_integer(curr->integer.value);
  case TOKEN_PLUS:
    return expr_unary_plus(parse_term(tokens));
  case TOKEN_MINUS:
    return expr_unary_minus(parse_term(tokens));
  case TOKEN_LPAREN:
    return parse_parenthesize_expression(tokens);
  default:
    parser_error("Unexpected token", curr);
    break;
  }
  return expr_invalid();
}

static Expression *parse_parenthesize_expression(Token **const tokens) {
  Expression *expr = parse_expression(tokens);
  if ((*tokens)->type != TOKEN_RPAREN) {
    parser_error("Expected closing ')' at end of expression", *tokens);
    expr->valid = false;
  } else {
    ++(*tokens);
    expr->valid = true;
  }
  return expr;
}

static Expression *parse_term(Token **const tokens) {
  Expression *expr = parse_factor(tokens);

  for (bool done = false; !done;) {
    Token *curr = *tokens;
    switch (curr->type) {
    case TOKEN_TIMES:
      *tokens += 1;
      expr = expr_mul(expr, parse_factor(tokens));
      break;
    case TOKEN_DIVIDE:
      *tokens += 1;
      expr = expr_div(expr, parse_factor(tokens));
      break;
    default:
      done = true;
      break;
    }
  }
  return expr;
}

static Expression *parse_expression(Token **const tokens) {
  Expression *expr = parse_term(tokens);

  for (bool done = false; !done;) {
    Token *curr = *tokens;
    switch (curr->type) {
    case TOKEN_PLUS:
      *tokens += 1;
      expr = expr_add(expr, parse_term(tokens));
      break;
    case TOKEN_MINUS:
      *tokens += 1;
      expr = expr_sub(expr, parse_term(tokens));
      break;
    default:
      done = true;
      break;
    }
  }
  return expr;
}

static Expression *parse(Token *tokens) {
  Expression *result = parse_expression(&tokens);
  if (tokens->type != TOKEN_EOF) {
    parser_error("Unexpected input after expression", tokens);
    result->valid = false;
  }
  return result;
}

//===----------------------------------------------------------------------===//
// Evaluation
//===----------------------------------------------------------------------===//

static int eval(Expression *expr);

static int eval_number(Number n) {
  switch (n.type) {
  case NUMBER_INTEGER:
    return n.integer;
  default:
    UNREACHABLE("Unexpected number type");
  }
}

static int eval_binop(BinaryOp op) {
  switch (op.op) {
  case BINARY_OP_ADD:
    return eval(op.lhs) + eval(op.rhs);
  case BINARY_OP_SUB:
    return eval(op.lhs) - eval(op.rhs);
  case BINARY_OP_MUL:
    return eval(op.lhs) * eval(op.rhs);
  case BINARY_OP_DIV:
    return eval(op.lhs) / eval(op.rhs);
  default:
    UNREACHABLE("Unexpected binary operator");
  }
}

static int eval_unop(UnaryOp op) {
  switch (op.op) {
  case UNARY_OP_PLUS:
    return eval(op.expr);
  case UNARY_OP_NEG:
    return -eval(op.expr);
  default:
    UNREACHABLE("Unexpected unary operator");
  }
}

static int eval(Expression *expr) {
  switch (expr->type) {
  case EXPR_NUMBER:
    return eval_number(expr->number);
  case EXPR_BINARY_OP:
    return eval_binop(expr->binary_op);
  case EXPR_UNARY_OP:
    return eval_unop(expr->unary_op);
  default:
    UNREACHABLE("Unexpected expression type");
  }
}

//===----------------------------------------------------------------------===//
// User interaction
//===----------------------------------------------------------------------===//

#define INIT_USER_INPUT_SIZE 256

char *read_input(size_t size, char s[restrict size], FILE *restrict stream) {
  s[0] = 0;
  char *result = fgets(s, size, stream);
  if (result) {
    char *pos = strchr(s, '\n');
    if (pos)
      *pos = 0;
    else
      result = 0;
  }
  return result;
}

void read_input_line(size_t *restrict size, char **input) {
  printf("> ");
  fflush(stdout);

  char *current_input = *input;
  char *result = 0;
  size_t remaining_size = *size;
  while (!result) {
    result = read_input(remaining_size, current_input, stdin);
    if (!result) {
      *input = realloc(*input, *size * 2);
      if (!*input)
        die("Failed to allocate memory");
      current_input =
          *input + *size - 1; // Account for the ending null character
      remaining_size = *size + 1;
      *size *= 2;
    }
  }
}

int main(void) {
  puts("Calc - A simple calculator\n"
       "This is free and unencumbered software released into the public "
       "domain.\n");

  bool quit = false;
  size_t input_length = INIT_USER_INPUT_SIZE;
  char *input = malloc(input_length);
  if (!input)
    die("Failed to allocate memory");

  do {
    read_input_line(&input_length, &input);
    if (strcmp(input, "exit") == 0) {
      quit = true;
      continue;
    }

    Token *tokens = tokenize(input);

    if (tokens->type != TOKEN_EOF) {
      Expression *result = parse(tokens);
      if (result->valid) {
        printf("%d\n", eval(result));
      }
      free_expr(result);
    }

    arraylist_free(tokens);
  } while (!quit);

  if (input) {
    free(input);
    input = 0;
  }
  return 0;
}
