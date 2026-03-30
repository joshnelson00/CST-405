/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

/* SYNTAX ANALYZER (PARSER)
 * This is the second phase of compilation - checking grammar rules
 * Bison generates a parser that builds an Abstract Syntax Tree (AST)
 * The parser uses tokens from the scanner to verify syntax is correct
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

/* External declarations for lexer interface */
extern int yylex();      /* Get next token from scanner */
extern int yyparse();    /* Parse the entire input */
extern FILE* yyin;       /* Input file handle */
extern int yyline;       /* Current line number from scanner */
extern int yycolumn;     /* Current column number from scanner */

void yyerror(const char* s);  /* Error handling function */
ASTNode* root = NULL;          /* Root of the Abstract Syntax Tree */

/* Function to check for infinite/dead loops */
void checkWhileLoop(ASTNode* condition) {
    if (!condition) return;
    
    // Check for constant true condition (infinite loop)
    if (condition->type == NODE_NUM) {
        if (condition->data.num != 0) {
            fprintf(stderr, "\n⚠️  Warning: Infinite loop detected at line %d\n", yyline);
            fprintf(stderr, "   Loop condition is always true (non-zero constant)\n");
            fprintf(stderr, "💡 Consider adding a break condition or loop counter\n\n");
        } else {
            fprintf(stderr, "\n⚠️  Warning: Dead loop detected at line %d\n", yyline);
            fprintf(stderr, "   Loop condition is always false (zero constant)\n");
            fprintf(stderr, "💡 Loop body will never execute\n\n");
        }
    }
    // Could add more sophisticated checks here for variable analysis
}
/* Function to check for infinite/dead loops in for statements */
void checkForLoop(ASTNode* condition) {
    // NULL condition means for(;;) — always infinite
    if (!condition) {
        fprintf(stderr, "\n⚠️  Warning: Infinite loop detected at line %d\n", yyline);
        fprintf(stderr, "   for(;;) has no condition — loop runs forever\n");
        fprintf(stderr, "💡 Add a condition, e.g. for(i = 0; i < n; i = i + 1)\n\n");
        return;
    }
    if (condition->type == NODE_NUM) {
        if (condition->data.num != 0) {
            fprintf(stderr, "\n⚠️  Warning: Infinite loop detected at line %d\n", yyline);
            fprintf(stderr, "   for-loop condition is always true (non-zero constant)\n");
            fprintf(stderr, "💡 Consider adding a proper loop counter or variable condition\n\n");
        } else {
            fprintf(stderr, "\n⚠️  Warning: Dead loop detected at line %d\n", yyline);
            fprintf(stderr, "   for-loop condition is always false (zero constant) — body never runs\n");
            fprintf(stderr, "💡 Loop body is unreachable code; remove it or fix the condition\n\n");
        }
    }
}
/* Warn when an if-statement condition is a compile-time constant */
void checkIfCondition(ASTNode* condition) {
    if (!condition) return;
    if (condition->type == NODE_NUM) {
        if (condition->data.num != 0) {
            fprintf(stderr, "\n⚠️  Warning: if condition always true at line %d\n", yyline);
            fprintf(stderr, "   The else-branch (if present) is dead code\n\n");
        } else {
            fprintf(stderr, "\n⚠️  Warning: if condition always false at line %d\n", yyline);
            fprintf(stderr, "   The then-body will never execute\n\n");
        }
    }
}

static int break_context_depth = 0;

static void enterBreakContext(void) {
    break_context_depth++;
}

static void exitBreakContext(void) {
    if (break_context_depth > 0) {
        break_context_depth--;
    }
}

static int isIntegralExpr(ASTNode* expr) {
    if (!expr) return 0;

    switch (expr->type) {
        case NODE_NUM:
            return 1;
        case NODE_FLT:
        case NODE_STR:
            return 0;
        case NODE_VAR:
            return getVarType(expr->data.var.name) == TYPE_INT;
        case NODE_ARRAY_ACCESS:
            return getVarType(expr->data.array_access.name) == TYPE_INT;
        case NODE_FUNC_CALL:
            return getFunctionReturnType(expr->data.func_call.name) == TYPE_INT;
        case NODE_MEMBER_ACCESS:
            return 1;
        case NODE_ADDR_OF:
            return 0;
        case NODE_BINOP:
            if (expr->data.binop.op == OP_EQ || expr->data.binop.op == OP_NE ||
                expr->data.binop.op == OP_LT || expr->data.binop.op == OP_GT ||
                expr->data.binop.op == OP_LE || expr->data.binop.op == OP_GE ||
                expr->data.binop.op == OP_AND || expr->data.binop.op == OP_OR) {
                return 1;
            }
            if (expr->data.binop.op == OP_NOT) {
                return isIntegralExpr(expr->data.binop.left);
            }
            return isIntegralExpr(expr->data.binop.left) &&
                   isIntegralExpr(expr->data.binop.right);
        default:
            return 0;
    }
}

static VarType inferExprType(ASTNode* expr) {
    if (!expr) return TYPE_VOID;

    switch (expr->type) {
        case NODE_NUM:
            return TYPE_INT;
        case NODE_FLT:
            return TYPE_FLOAT;
        case NODE_VAR:
            return getVarType(expr->data.var.name);
        case NODE_ARRAY_ACCESS:
            return getVarType(expr->data.array_access.name);
        case NODE_MEMBER_ACCESS:
            return TYPE_INT; /* Session 1 fields are int-only. */
        case NODE_ADDR_OF:
            return TYPE_STRUCT_PTR;
        case NODE_FUNC_CALL:
            return getFunctionReturnType(expr->data.func_call.name);
        case NODE_BINOP: {
            VarType lt = inferExprType(expr->data.binop.left);
            VarType rt = inferExprType(expr->data.binop.right);
            if (expr->data.binop.op == OP_EQ || expr->data.binop.op == OP_NE ||
                expr->data.binop.op == OP_LT || expr->data.binop.op == OP_GT ||
                expr->data.binop.op == OP_LE || expr->data.binop.op == OP_GE ||
                expr->data.binop.op == OP_AND || expr->data.binop.op == OP_OR ||
                expr->data.binop.op == OP_NOT) {
                return TYPE_INT;
            }
            if (lt == TYPE_STRUCT || rt == TYPE_STRUCT ||
                lt == TYPE_STRUCT_PTR || rt == TYPE_STRUCT_PTR) {
                return TYPE_STRUCT;
            }
            return (lt == TYPE_FLOAT || rt == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
        }
        default:
            return TYPE_INT;
    }
}

static StructType* getMemberBaseStruct(ASTNode* base) {
    if (!base) return NULL;
    if (base->type == NODE_VAR) {
        return getVarStructType(base->data.var.name);
    }
    return NULL;
}

static void validateSwitchCases(ASTNode* cases) {
    int seen_values[256];
    int seen_count = 0;
    int default_count = 0;

    for (ASTNode* curr = cases; curr; curr = curr->data.case_stmt.next) {
        if (curr->type != NODE_CASE) continue;

        if (curr->data.case_stmt.is_default) {
            default_count++;
            if (default_count > 1) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Multiple default clauses in one switch are not allowed\n");
                fprintf(stderr, "💡 Suggestion: Keep only one default clause per switch\n\n");
                semantic_error_count++;
            }
            continue;
        }

        for (int i = 0; i < seen_count; i++) {
            if (seen_values[i] == curr->data.case_stmt.value) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Duplicate case value '%d' in switch\n", curr->data.case_stmt.value);
                fprintf(stderr, "💡 Suggestion: Case labels in one switch must be unique\n\n");
                semantic_error_count++;
                break;
            }
        }

        if (seen_count < 256) {
            seen_values[seen_count++] = curr->data.case_stmt.value;
        }
    }

    if (default_count == 0) {
        fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
        fprintf(stderr, "   switch has no default clause\n");
        fprintf(stderr, "💡 Suggestion: Add default: to handle unmatched values\n\n");
    }
}

int syntax_error_count = 0;    /* Counter for syntax errors */
extern int semantic_error_count; /* Counter for semantic errors (in symtab.c) */

#line 286 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NUM = 3,                        /* NUM  */
  YYSYMBOL_FLT = 4,                        /* FLT  */
  YYSYMBOL_ID = 5,                         /* ID  */
  YYSYMBOL_STRING = 6,                     /* STRING  */
  YYSYMBOL_INT = 7,                        /* INT  */
  YYSYMBOL_FLOAT = 8,                      /* FLOAT  */
  YYSYMBOL_VOID = 9,                       /* VOID  */
  YYSYMBOL_PRINT = 10,                     /* PRINT  */
  YYSYMBOL_RETURN = 11,                    /* RETURN  */
  YYSYMBOL_WHILE = 12,                     /* WHILE  */
  YYSYMBOL_FOR = 13,                       /* FOR  */
  YYSYMBOL_IF = 14,                        /* IF  */
  YYSYMBOL_ELSE = 15,                      /* ELSE  */
  YYSYMBOL_SWITCH = 16,                    /* SWITCH  */
  YYSYMBOL_CASE = 17,                      /* CASE  */
  YYSYMBOL_DEFAULT = 18,                   /* DEFAULT  */
  YYSYMBOL_BREAK = 19,                     /* BREAK  */
  YYSYMBOL_STRUCT = 20,                    /* STRUCT  */
  YYSYMBOL_DOT = 21,                       /* DOT  */
  YYSYMBOL_AMP = 22,                       /* AMP  */
  YYSYMBOL_EQ = 23,                        /* EQ  */
  YYSYMBOL_NE = 24,                        /* NE  */
  YYSYMBOL_LT = 25,                        /* LT  */
  YYSYMBOL_GT = 26,                        /* GT  */
  YYSYMBOL_LE = 27,                        /* LE  */
  YYSYMBOL_GE = 28,                        /* GE  */
  YYSYMBOL_AND = 29,                       /* AND  */
  YYSYMBOL_OR = 30,                        /* OR  */
  YYSYMBOL_NOT = 31,                       /* NOT  */
  YYSYMBOL_LOWER_THAN_ELSE = 32,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_33_ = 33,                       /* '+'  */
  YYSYMBOL_34_ = 34,                       /* '-'  */
  YYSYMBOL_35_ = 35,                       /* '*'  */
  YYSYMBOL_36_ = 36,                       /* '/'  */
  YYSYMBOL_37_ = 37,                       /* '{'  */
  YYSYMBOL_38_ = 38,                       /* '}'  */
  YYSYMBOL_39_ = 39,                       /* ';'  */
  YYSYMBOL_40_ = 40,                       /* '('  */
  YYSYMBOL_41_ = 41,                       /* ')'  */
  YYSYMBOL_42_ = 42,                       /* '['  */
  YYSYMBOL_43_ = 43,                       /* ']'  */
  YYSYMBOL_44_ = 44,                       /* ','  */
  YYSYMBOL_45_ = 45,                       /* '='  */
  YYSYMBOL_46_ = 46,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 47,                  /* $accept  */
  YYSYMBOL_program = 48,                   /* program  */
  YYSYMBOL_struct_defs_opt = 49,           /* struct_defs_opt  */
  YYSYMBOL_struct_def = 50,                /* struct_def  */
  YYSYMBOL_field_list = 51,                /* field_list  */
  YYSYMBOL_field_decl = 52,                /* field_decl  */
  YYSYMBOL_func_list = 53,                 /* func_list  */
  YYSYMBOL_func = 54,                      /* func  */
  YYSYMBOL_55_1 = 55,                      /* $@1  */
  YYSYMBOL_56_2 = 56,                      /* $@2  */
  YYSYMBOL_57_3 = 57,                      /* $@3  */
  YYSYMBOL_58_4 = 58,                      /* $@4  */
  YYSYMBOL_59_5 = 59,                      /* $@5  */
  YYSYMBOL_60_6 = 60,                      /* $@6  */
  YYSYMBOL_61_7 = 61,                      /* $@7  */
  YYSYMBOL_62_8 = 62,                      /* $@8  */
  YYSYMBOL_63_9 = 63,                      /* $@9  */
  YYSYMBOL_64_10 = 64,                     /* $@10  */
  YYSYMBOL_param_list = 65,                /* param_list  */
  YYSYMBOL_param = 66,                     /* param  */
  YYSYMBOL_stmt_list = 67,                 /* stmt_list  */
  YYSYMBOL_stmt_list_opt = 68,             /* stmt_list_opt  */
  YYSYMBOL_stmt = 69,                      /* stmt  */
  YYSYMBOL_decl = 70,                      /* decl  */
  YYSYMBOL_assign = 71,                    /* assign  */
  YYSYMBOL_expr = 72,                      /* expr  */
  YYSYMBOL_print_stmt = 73,                /* print_stmt  */
  YYSYMBOL_return_stmt = 74,               /* return_stmt  */
  YYSYMBOL_func_call = 75,                 /* func_call  */
  YYSYMBOL_arg_list = 76,                  /* arg_list  */
  YYSYMBOL_while_stmt = 77,                /* while_stmt  */
  YYSYMBOL_78_11 = 78,                     /* $@11  */
  YYSYMBOL_for_stmt = 79,                  /* for_stmt  */
  YYSYMBOL_80_12 = 80,                     /* $@12  */
  YYSYMBOL_if_stmt = 81,                   /* if_stmt  */
  YYSYMBOL_switch_stmt = 82,               /* switch_stmt  */
  YYSYMBOL_83_13 = 83,                     /* $@13  */
  YYSYMBOL_case_clause_list_opt = 84,      /* case_clause_list_opt  */
  YYSYMBOL_case_clause_list = 85,          /* case_clause_list  */
  YYSYMBOL_case_clause = 86,               /* case_clause  */
  YYSYMBOL_case_value = 87,                /* case_value  */
  YYSYMBOL_break_stmt = 88,                /* break_stmt  */
  YYSYMBOL_for_init = 89,                  /* for_init  */
  YYSYMBOL_for_cond = 90,                  /* for_cond  */
  YYSYMBOL_for_update = 91                 /* for_update  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   905

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  124
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  303

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   287


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      40,    41,    35,    33,    44,    34,     2,    36,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    46,    39,
       2,    45,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    42,     2,    43,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    37,     2,    38,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   265,   265,   269,   273,   274,   278,   327,   328,   332,
     340,   341,   348,   348,   354,   354,   360,   360,   366,   366,
     372,   372,   378,   378,   384,   384,   391,   391,   398,   398,
     405,   405,   412,   417,   420,   427,   431,   435,   439,   443,
     454,   469,   472,   478,   479,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   497,   501,   502,   507,   513,
     519,   535,   552,   569,   586,   597,   612,   668,   720,   756,
     759,   762,   766,   779,   789,   792,   802,   812,   822,   832,
     835,   838,   841,   844,   847,   850,   860,   870,   879,   882,
     922,   943,   950,   953,   960,   964,   972,   975,   982,   982,
     994,   994,  1009,  1013,  1020,  1020,  1037,  1038,  1042,  1043,
    1049,  1052,  1058,  1059,  1063,  1076,  1077,  1084,  1091,  1103,
    1117,  1118,  1123,  1124,  1136
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NUM", "FLT", "ID",
  "STRING", "INT", "FLOAT", "VOID", "PRINT", "RETURN", "WHILE", "FOR",
  "IF", "ELSE", "SWITCH", "CASE", "DEFAULT", "BREAK", "STRUCT", "DOT",
  "AMP", "EQ", "NE", "LT", "GT", "LE", "GE", "AND", "OR", "NOT",
  "LOWER_THAN_ELSE", "'+'", "'-'", "'*'", "'/'", "'{'", "'}'", "';'",
  "'('", "')'", "'['", "']'", "','", "'='", "':'", "$accept", "program",
  "struct_defs_opt", "struct_def", "field_list", "field_decl", "func_list",
  "func", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "$@10", "param_list", "param", "stmt_list", "stmt_list_opt", "stmt",
  "decl", "assign", "expr", "print_stmt", "return_stmt", "func_call",
  "arg_list", "while_stmt", "$@11", "for_stmt", "$@12", "if_stmt",
  "switch_stmt", "$@13", "case_clause_list_opt", "case_clause_list",
  "case_clause", "case_value", "break_stmt", "for_init", "for_cond",
  "for_update", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-89)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-45)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -89,    14,   153,   -89,   -20,    -4,    -2,    27,    49,   -89,
      80,   167,   -89,    18,    33,    47,    52,    57,    68,   -89,
     -89,     5,    94,    55,   113,    65,   117,   128,   144,   150,
     -89,   -37,   -89,   123,   -89,   -18,   130,   -89,    88,   152,
      -1,   -89,   137,   141,    22,   135,   -89,   158,   118,   169,
     -89,   190,   170,   -89,   173,   185,   -89,   168,   186,   -89,
     233,   601,   207,   -89,   -89,   107,   601,   210,   -89,   172,
     601,   220,   -89,   -89,   -89,   -89,   -89,   226,    67,   264,
     267,   238,    62,   244,   246,   247,   248,   250,   280,   601,
     301,   -89,   -89,   -89,   -89,   -89,   251,   -89,   -89,   -89,
     -89,   -89,   601,   254,   -89,   335,   601,   256,   -89,   351,
     601,   -89,   289,    30,   197,   197,   183,   184,   197,   -89,
     -89,   -32,   -89,   290,   197,   -89,   197,   785,   -89,   197,
     213,   197,   197,   -89,    34,   385,   -89,   -89,   -89,   401,
     601,   259,   -89,   435,   601,   260,   -89,   451,   253,   -89,
     225,   189,   606,   802,   -89,     8,   -89,    16,   690,   197,
     -89,   278,   709,   300,   197,   197,   197,   197,   197,   197,
     197,   197,   197,   197,   197,   197,   -89,   728,   194,   302,
     305,   277,   747,   766,   279,   314,   -89,   -89,   485,   601,
     -89,   501,   601,   -89,   197,   -89,   197,   281,   -89,   282,
     320,   284,   321,   291,   627,   -89,   -89,    56,    56,    56,
      56,    56,    56,   869,   853,    -5,    -5,   278,   278,   -89,
     197,   197,   283,   286,   197,   601,   -89,   -89,   293,   -89,
     535,   -89,   551,   819,   225,   197,   294,   292,   295,   298,
     -89,   -89,   307,   648,   225,   197,   197,   225,   311,   322,
     316,   -89,   -89,   -89,   -89,   836,   -89,   318,   -89,   327,
     601,   315,   225,   225,   324,   601,   245,   -89,   -89,   -89,
     585,   197,   198,   328,   -89,    19,   329,   330,   245,   -89,
     -89,   225,   197,   197,   -89,   -89,   371,   331,   103,   -89,
     -89,   669,   225,   601,   -89,   103,   263,   -89,   333,   -89,
     -89,   197,   225
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,     0,     1,     0,     0,     0,     0,     0,     5,
       0,     0,    32,     0,     0,     0,     0,     0,     0,     3,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    33,     0,    18,     0,     0,    22,     0,     0,
       0,     7,    35,    36,     0,     0,    12,     0,     0,     0,
      16,     0,     0,    20,     0,     0,     8,     0,     0,    40,
       0,     0,     0,    34,    26,     0,     0,     0,    30,     0,
       0,     0,     9,     6,    37,    38,    39,    57,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    41,    45,    46,    47,    48,     0,    49,    50,    51,
      52,    53,     0,     0,    24,     0,     0,     0,    28,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,    69,
      70,    72,    71,     0,     0,    93,     0,     0,    74,     0,
     115,     0,     0,   114,     0,     0,    15,    42,    54,     0,
       0,     0,    19,     0,     0,     0,    23,     0,     0,    95,
      96,     0,     0,     0,    58,     0,    59,     0,     0,     0,
      73,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    55,    13,     0,     0,
      17,     0,     0,    21,     0,    94,     0,     0,    66,     0,
       0,     0,     0,     0,     0,    88,    90,    79,    80,    81,
      82,    83,    84,    85,    86,    75,    76,    77,    78,    98,
       0,     0,     0,     0,   120,     0,   104,    60,     0,    27,
       0,    31,     0,     0,    97,     0,     0,     0,     0,     0,
      91,    89,     0,     0,   118,     0,     0,   121,     0,   102,
       0,    61,    25,    29,    68,     0,    62,     0,    63,     0,
       0,     0,   116,   117,   122,     0,   106,    67,    64,    65,
       0,     0,     0,     0,   103,     0,     0,     0,   107,   108,
      99,   119,     0,     0,   100,   112,     0,     0,     0,   105,
     109,     0,   123,     0,   113,     0,     0,   111,     0,   101,
     110,     0,   124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -89,   -89,   -89,   -89,   -89,   336,   368,   -89,   -89,   -89,
     -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   133,   334,
     -46,    85,   -88,   -89,   -89,    21,   -89,   -89,   -61,   -89,
     -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   104,
     -89,   -89,   -89,   -89,   -89
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,     9,    40,    41,    10,    11,    62,    45,
      67,    49,    71,    52,   141,   103,   145,   107,    31,    32,
     296,   297,    91,    92,    93,   127,    94,    95,   128,   151,
      97,   242,    98,   293,    99,   100,   250,   277,   278,   279,
     287,   101,   181,   248,   273
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      96,    13,   137,    15,    46,    96,    39,    47,   113,    96,
     159,   199,    27,    28,     3,    90,   163,   137,    12,   201,
     105,   137,   285,    50,   109,    29,    47,    59,    96,    96,
     174,   175,    17,   119,   120,   121,   122,    55,    14,   184,
      16,    96,   200,   135,    96,    96,    30,   137,    96,    96,
     202,   137,   123,   286,    18,   137,   139,    60,    21,   137,
     143,   124,    27,    28,   147,   119,   120,   121,   122,   185,
     126,   149,    27,    28,    96,    29,    22,   163,    96,    96,
      -2,    19,    96,    96,   123,    29,    96,    23,   112,   172,
     173,   174,   175,   124,   188,    24,    34,    25,   191,    33,
     137,   125,   126,   137,    77,    26,    37,   113,    78,   114,
      79,    80,   115,    81,    82,    83,    84,    85,    36,    86,
     -43,   -43,    87,    88,    39,    27,    28,    96,    96,    53,
      96,    96,    47,    42,   150,   152,   153,   249,    29,   158,
      89,   -43,   137,   230,   137,   161,   232,   162,   104,    43,
     177,    47,   182,   183,     4,    44,    35,    54,    38,    64,
       5,     6,     7,    48,    96,    27,    28,   -10,     4,    96,
      51,    96,    61,     8,     5,     6,     7,   274,    29,    57,
     204,    65,   137,    58,    69,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,    27,    28,    96,
     119,   120,   121,   122,    96,   299,    66,    70,   137,    96,
      29,    74,    72,   108,   270,   233,    47,   234,   178,   123,
     179,   180,   154,   156,    73,   155,   157,    96,   124,    75,
     195,    68,    96,   196,    96,    96,   220,   126,    76,   221,
     282,   243,   244,   283,   102,   247,   163,   106,   164,   165,
     166,   167,   168,   169,   170,   171,   255,   110,   172,   173,
     174,   175,   275,   276,    77,   111,   262,   263,    78,   116,
      79,    80,   117,    81,    82,    83,    84,    85,   118,    86,
     -44,   -44,    87,    88,   129,   134,   130,   131,   132,   133,
     138,   140,   281,   144,   148,   160,   189,   192,   194,   163,
      89,   -44,    77,   291,   292,   206,    78,   222,    79,    80,
     223,    81,    82,    83,    84,    85,   224,    86,   227,   228,
      87,    88,   302,   237,   239,   236,   235,   238,   245,   272,
     240,   246,   251,   256,   258,   257,    77,   265,    89,   136,
      78,   259,    79,    80,   260,    81,    82,    83,    84,    85,
     264,    86,    77,   266,    87,    88,    78,   268,    79,    80,
     271,    81,    82,    83,    84,    85,   269,    86,   289,   284,
      87,    88,    89,   142,   294,   288,    56,   295,   301,    20,
     300,    63,   290,     0,     0,     0,    77,     0,    89,   146,
      78,     0,    79,    80,     0,    81,    82,    83,    84,    85,
       0,    86,    77,     0,    87,    88,    78,     0,    79,    80,
       0,    81,    82,    83,    84,    85,     0,    86,     0,     0,
      87,    88,    89,   186,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,    89,   187,
      78,     0,    79,    80,     0,    81,    82,    83,    84,    85,
       0,    86,    77,     0,    87,    88,    78,     0,    79,    80,
       0,    81,    82,    83,    84,    85,     0,    86,     0,     0,
      87,    88,    89,   190,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,    89,   193,
      78,     0,    79,    80,     0,    81,    82,    83,    84,    85,
       0,    86,    77,     0,    87,    88,    78,     0,    79,    80,
       0,    81,    82,    83,    84,    85,     0,    86,     0,     0,
      87,    88,    89,   229,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,    89,   231,
      78,     0,    79,    80,     0,    81,    82,    83,    84,    85,
       0,    86,    77,     0,    87,    88,    78,     0,    79,    80,
       0,    81,    82,    83,    84,    85,     0,    86,     0,     0,
      87,    88,    89,   252,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,    89,   253,
      78,     0,    79,    80,     0,    81,    82,    83,    84,    85,
       0,    86,    77,     0,    87,    88,    78,     0,    79,    80,
       0,    81,    82,    83,    84,    85,     0,    86,     0,     0,
      87,    88,    89,   280,     0,     0,     0,   163,     0,   164,
     165,   166,   167,   168,   169,   170,   171,     0,    89,   172,
     173,   174,   175,     0,     0,     0,     0,     0,   163,   197,
     164,   165,   166,   167,   168,   169,   170,   171,     0,     0,
     172,   173,   174,   175,     0,     0,     0,     0,     0,   163,
     241,   164,   165,   166,   167,   168,   169,   170,   171,     0,
       0,   172,   173,   174,   175,     0,     0,     0,     0,     0,
     163,   261,   164,   165,   166,   167,   168,   169,   170,   171,
       0,     0,   172,   173,   174,   175,     0,     0,     0,     0,
       0,   163,   298,   164,   165,   166,   167,   168,   169,   170,
     171,     0,     0,   172,   173,   174,   175,     0,     0,     0,
     163,   203,   164,   165,   166,   167,   168,   169,   170,   171,
       0,     0,   172,   173,   174,   175,     0,     0,     0,   163,
     205,   164,   165,   166,   167,   168,   169,   170,   171,     0,
       0,   172,   173,   174,   175,     0,     0,     0,   163,   219,
     164,   165,   166,   167,   168,   169,   170,   171,     0,     0,
     172,   173,   174,   175,     0,     0,     0,   163,   225,   164,
     165,   166,   167,   168,   169,   170,   171,     0,     0,   172,
     173,   174,   175,     0,     0,     0,   163,   226,   164,   165,
     166,   167,   168,   169,   170,   171,     0,     0,   172,   173,
     174,   175,     0,   163,   176,   164,   165,   166,   167,   168,
     169,   170,   171,     0,     0,   172,   173,   174,   175,     0,
     163,   198,   164,   165,   166,   167,   168,   169,   170,   171,
       0,     0,   172,   173,   174,   175,     0,   163,   254,   164,
     165,   166,   167,   168,   169,   170,   171,     0,     0,   172,
     173,   174,   175,     0,   163,   267,   164,   165,   166,   167,
     168,   169,   170,     0,     0,     0,   172,   173,   174,   175,
     163,     0,   164,   165,   166,   167,   168,   169,     0,     0,
       0,     0,   172,   173,   174,   175
};

static const yytype_int16 yycheck[] =
{
      61,     5,    90,     5,    41,    66,     7,    44,    40,    70,
      42,     3,     7,     8,     0,    61,    21,   105,    38,     3,
      66,   109,     3,    41,    70,    20,    44,     5,    89,    90,
      35,    36,     5,     3,     4,     5,     6,    38,    42,     5,
      42,   102,    34,    89,   105,   106,    41,   135,   109,   110,
      34,   139,    22,    34,     5,   143,   102,    35,    40,   147,
     106,    31,     7,     8,   110,     3,     4,     5,     6,    35,
      40,    41,     7,     8,   135,    20,    43,    21,   139,   140,
       0,     1,   143,   144,    22,    20,   147,    40,    21,    33,
      34,    35,    36,    31,   140,    43,    41,    40,   144,     5,
     188,    39,    40,   191,     1,    37,    41,    40,     5,    42,
       7,     8,    45,    10,    11,    12,    13,    14,     5,    16,
      17,    18,    19,    20,     7,     7,     8,   188,   189,    41,
     191,   192,    44,     5,   113,   114,   115,   225,    20,   118,
      37,    38,   230,   189,   232,   124,   192,   126,    41,     5,
     129,    44,   131,   132,     1,     5,    23,     5,    25,    41,
       7,     8,     9,    40,   225,     7,     8,     0,     1,   230,
      40,   232,    37,    20,     7,     8,     9,   265,    20,    42,
     159,    48,   270,    42,    51,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,     7,     8,   260,
       3,     4,     5,     6,   265,   293,    37,    37,   296,   270,
      20,    43,    39,    41,   260,   194,    44,   196,     5,    22,
       7,     8,    39,    39,    39,    42,    42,   288,    31,    43,
      41,    41,   293,    44,   295,   296,    42,    40,     5,    45,
      42,   220,   221,    45,    37,   224,    21,    37,    23,    24,
      25,    26,    27,    28,    29,    30,   235,    37,    33,    34,
      35,    36,    17,    18,     1,    39,   245,   246,     5,     5,
       7,     8,     5,    10,    11,    12,    13,    14,    40,    16,
      17,    18,    19,    20,    40,     5,    40,    40,    40,    39,
      39,    37,   271,    37,     5,     5,    37,    37,    45,    21,
      37,    38,     1,   282,   283,     5,     5,     5,     7,     8,
       5,    10,    11,    12,    13,    14,    39,    16,    39,     5,
      19,    20,   301,     3,     3,    43,    45,    43,    45,     5,
      39,    45,    39,    39,    39,    43,     1,    15,    37,    38,
       5,    43,     7,     8,    37,    10,    11,    12,    13,    14,
      39,    16,     1,    37,    19,    20,     5,    39,     7,     8,
      45,    10,    11,    12,    13,    14,    39,    16,    38,    41,
      19,    20,    37,    38,     3,    46,    40,    46,    45,    11,
     295,    47,   278,    -1,    -1,    -1,     1,    -1,    37,    38,
       5,    -1,     7,     8,    -1,    10,    11,    12,    13,    14,
      -1,    16,     1,    -1,    19,    20,     5,    -1,     7,     8,
      -1,    10,    11,    12,    13,    14,    -1,    16,    -1,    -1,
      19,    20,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    37,    38,
       5,    -1,     7,     8,    -1,    10,    11,    12,    13,    14,
      -1,    16,     1,    -1,    19,    20,     5,    -1,     7,     8,
      -1,    10,    11,    12,    13,    14,    -1,    16,    -1,    -1,
      19,    20,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    37,    38,
       5,    -1,     7,     8,    -1,    10,    11,    12,    13,    14,
      -1,    16,     1,    -1,    19,    20,     5,    -1,     7,     8,
      -1,    10,    11,    12,    13,    14,    -1,    16,    -1,    -1,
      19,    20,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    37,    38,
       5,    -1,     7,     8,    -1,    10,    11,    12,    13,    14,
      -1,    16,     1,    -1,    19,    20,     5,    -1,     7,     8,
      -1,    10,    11,    12,    13,    14,    -1,    16,    -1,    -1,
      19,    20,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    37,    38,
       5,    -1,     7,     8,    -1,    10,    11,    12,    13,    14,
      -1,    16,     1,    -1,    19,    20,     5,    -1,     7,     8,
      -1,    10,    11,    12,    13,    14,    -1,    16,    -1,    -1,
      19,    20,    37,    38,    -1,    -1,    -1,    21,    -1,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    37,    33,
      34,    35,    36,    -1,    -1,    -1,    -1,    -1,    21,    43,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    -1,
      33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,    21,
      43,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      -1,    33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,
      21,    43,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    -1,    33,    34,    35,    36,    -1,    -1,    -1,    -1,
      -1,    21,    43,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    -1,    33,    34,    35,    36,    -1,    -1,    -1,
      21,    41,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    -1,    33,    34,    35,    36,    -1,    -1,    -1,    21,
      41,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      -1,    33,    34,    35,    36,    -1,    -1,    -1,    21,    41,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    -1,
      33,    34,    35,    36,    -1,    -1,    -1,    21,    41,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    -1,    33,
      34,    35,    36,    -1,    -1,    -1,    21,    41,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    -1,    33,    34,
      35,    36,    -1,    21,    39,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    -1,    33,    34,    35,    36,    -1,
      21,    39,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    -1,    33,    34,    35,    36,    -1,    21,    39,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    -1,    33,
      34,    35,    36,    -1,    21,    39,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    -1,    33,    34,    35,    36,
      21,    -1,    23,    24,    25,    26,    27,    28,    -1,    -1,
      -1,    -1,    33,    34,    35,    36
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    48,    49,     0,     1,     7,     8,     9,    20,    50,
      53,    54,    38,     5,    42,     5,    42,     5,     5,     1,
      53,    40,    43,    40,    43,    40,    37,     7,     8,    20,
      41,    65,    66,     5,    41,    65,     5,    41,    65,     7,
      51,    52,     5,     5,     5,    56,    41,    44,    40,    58,
      41,    40,    60,    41,     5,    38,    52,    42,    42,     5,
      35,    37,    55,    66,    41,    65,    37,    57,    41,    65,
      37,    59,    39,    39,    43,    43,     5,     1,     5,     7,
       8,    10,    11,    12,    13,    14,    16,    19,    20,    37,
      67,    69,    70,    71,    73,    74,    75,    77,    79,    81,
      82,    88,    37,    62,    41,    67,    37,    64,    41,    67,
      37,    39,    21,    40,    42,    45,     5,     5,    40,     3,
       4,     5,     6,    22,    31,    39,    40,    72,    75,    40,
      40,    40,    40,    39,     5,    67,    38,    69,    39,    67,
      37,    61,    38,    67,    37,    63,    38,    67,     5,    41,
      72,    76,    72,    72,    39,    42,    39,    42,    72,    42,
       5,    72,    72,    21,    23,    24,    25,    26,    27,    28,
      29,    30,    33,    34,    35,    36,    39,    72,     5,     7,
       8,    89,    72,    72,     5,    35,    38,    38,    67,    37,
      38,    67,    37,    38,    45,    41,    44,    43,    39,     3,
      34,     3,    34,    41,    72,    41,     5,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    41,
      42,    45,     5,     5,    39,    41,    41,    39,     5,    38,
      67,    38,    67,    72,    72,    45,    43,     3,    43,     3,
      39,    43,    78,    72,    72,    45,    45,    72,    90,    69,
      83,    39,    38,    38,    39,    72,    39,    43,    39,    43,
      37,    43,    72,    72,    39,    15,    37,    39,    39,    39,
      67,    45,     5,    91,    69,    17,    18,    84,    85,    86,
      38,    72,    42,    45,    41,     3,    34,    87,    46,    38,
      86,    72,    72,    80,     3,    46,    67,    68,    43,    69,
      68,    45,    72
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    47,    48,    48,    49,    49,    50,    51,    51,    52,
      53,    53,    55,    54,    56,    54,    57,    54,    58,    54,
      59,    54,    60,    54,    61,    54,    62,    54,    63,    54,
      64,    54,    54,    65,    65,    66,    66,    66,    66,    66,
      66,    67,    67,    68,    68,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    70,    70,
      70,    70,    70,    70,    70,    70,    71,    71,    71,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    73,    74,    74,    75,    75,    76,    76,    78,    77,
      80,    79,    81,    81,    83,    82,    84,    84,    85,    85,
      86,    86,    87,    87,    88,    89,    89,    89,    89,    89,
      90,    90,    91,    91,    91
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     3,     0,     2,     6,     1,     2,     3,
       1,     2,     0,     9,     0,     8,     0,     9,     0,     8,
       0,     9,     0,     8,     0,    11,     0,    10,     0,    11,
       0,    10,     2,     1,     3,     2,     2,     4,     4,     4,
       3,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     2,     1,     3,     3,
       4,     5,     6,     6,     7,     7,     4,     7,     6,     1,
       1,     1,     1,     2,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     4,
       3,     5,     3,     2,     4,     3,     1,     3,     0,     8,
       0,    10,     5,     7,     0,     8,     0,     1,     1,     2,
       4,     3,     1,     2,     2,     0,     4,     4,     3,     6,
       0,     1,     0,     3,     6
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: struct_defs_opt func_list  */
#line 265 "parser.y"
                              {
        /* Action: Save the function list as our AST root */
        root = (yyvsp[0].node);
    }
#line 1682 "parser.tab.c"
    break;

  case 3: /* program: struct_defs_opt func_list error  */
#line 269 "parser.y"
                                      { root = (yyvsp[-1].node); yyerrok; }
#line 1688 "parser.tab.c"
    break;

  case 4: /* struct_defs_opt: %empty  */
#line 273 "parser.y"
                { (yyval.node) = NULL; }
#line 1694 "parser.tab.c"
    break;

  case 5: /* struct_defs_opt: struct_defs_opt struct_def  */
#line 274 "parser.y"
                                 { (yyval.node) = (yyvsp[-1].node); }
#line 1700 "parser.tab.c"
    break;

  case 6: /* struct_def: STRUCT ID '{' field_list '}' ';'  */
#line 278 "parser.y"
                                     {
        StructType st;
        memset(&st, 0, sizeof(st));
        st.name = (yyvsp[-4].str);

        int fieldIndex = 0;
        for (ASTNode* f = (yyvsp[-2].node); f; f = f->data.field_decl.next) {
            if (fieldIndex >= MAX_STRUCT_FIELDS) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Struct '%s' exceeds max field count (%d)\n\n", (yyvsp[-4].str), MAX_STRUCT_FIELDS);
                semantic_error_count++;
                break;
            }

            int duplicate = 0;
            for (int i = 0; i < fieldIndex; i++) {
                if (strcmp(st.fields[i].name, f->data.field_decl.name) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Duplicate field '%s' in struct '%s'\n", f->data.field_decl.name, (yyvsp[-4].str));
                fprintf(stderr, "💡 Suggestion: use unique field names in each struct definition\n\n");
                semantic_error_count++;
                continue;
            }

            st.fields[fieldIndex].name = f->data.field_decl.name;
            st.fields[fieldIndex].offset = fieldIndex * 4;
            fieldIndex++;
        }
        st.numFields = fieldIndex;
        st.totalSize = fieldIndex * 4;

        if (registerStruct(&st) != 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct '%s' is already defined\n", (yyvsp[-4].str));
            fprintf(stderr, "💡 Suggestion: rename or remove the duplicate struct definition\n\n");
            semantic_error_count++;
        }

        (yyval.node) = createStructDef((yyvsp[-4].str), (yyvsp[-2].node));
        free((yyvsp[-4].str));
    }
#line 1751 "parser.tab.c"
    break;

  case 7: /* field_list: field_decl  */
#line 327 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1757 "parser.tab.c"
    break;

  case 8: /* field_list: field_list field_decl  */
#line 328 "parser.y"
                            { (yyval.node) = appendField((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1763 "parser.tab.c"
    break;

  case 9: /* field_decl: INT ID ';'  */
#line 332 "parser.y"
               {
        (yyval.node) = createFieldDecl((yyvsp[-1].str));
        free((yyvsp[-1].str));
    }
#line 1772 "parser.tab.c"
    break;

  case 10: /* func_list: func  */
#line 340 "parser.y"
         { (yyval.node) = (yyvsp[0].node); }
#line 1778 "parser.tab.c"
    break;

  case 11: /* func_list: func func_list  */
#line 341 "parser.y"
                   { 
        (yyvsp[-1].node)->data.func.next = (yyvsp[0].node);
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1787 "parser.tab.c"
    break;

  case 12: /* $@1: %empty  */
#line 348 "parser.y"
                              { prepareFunctionScope((yyvsp[-3].str), TYPE_INT); addParamsToScope((yyvsp[-1].node)); }
#line 1793 "parser.tab.c"
    break;

  case 13: /* func: INT ID '(' param_list ')' $@1 '{' stmt_list '}'  */
#line 348 "parser.y"
                                                                                                              { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_INT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Function with return type and parameters */
        addFunction((yyvsp[-7].str), TYPE_INT, (yyval.node));         /* Update with AST */
        free((yyvsp[-7].str));
    }
#line 1804 "parser.tab.c"
    break;

  case 14: /* $@2: %empty  */
#line 354 "parser.y"
                     { prepareFunctionScope((yyvsp[-2].str), TYPE_INT); }
#line 1810 "parser.tab.c"
    break;

  case 15: /* func: INT ID '(' ')' $@2 '{' stmt_list '}'  */
#line 354 "parser.y"
                                                                               { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_INT, NULL, (yyvsp[-1].node));  /* Function with no parameters */
        addFunction((yyvsp[-6].str), TYPE_INT, (yyval.node));            /* Update with AST */
        free((yyvsp[-6].str));
    }
#line 1821 "parser.tab.c"
    break;

  case 16: /* $@3: %empty  */
#line 360 "parser.y"
                                  { prepareFunctionScope((yyvsp[-3].str), TYPE_FLOAT); addParamsToScope((yyvsp[-1].node)); }
#line 1827 "parser.tab.c"
    break;

  case 17: /* func: FLOAT ID '(' param_list ')' $@3 '{' stmt_list '}'  */
#line 360 "parser.y"
                                                                                                                    { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_FLOAT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Float function with parameters */
        addFunction((yyvsp[-7].str), TYPE_FLOAT, (yyval.node));           /* Update with AST */
        free((yyvsp[-7].str));
    }
#line 1838 "parser.tab.c"
    break;

  case 18: /* $@4: %empty  */
#line 366 "parser.y"
                       { prepareFunctionScope((yyvsp[-2].str), TYPE_FLOAT); }
#line 1844 "parser.tab.c"
    break;

  case 19: /* func: FLOAT ID '(' ')' $@4 '{' stmt_list '}'  */
#line 366 "parser.y"
                                                                                   { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_FLOAT, NULL, (yyvsp[-1].node));  /* Float function with no parameters */
        addFunction((yyvsp[-6].str), TYPE_FLOAT, (yyval.node));            /* Update with AST */
        free((yyvsp[-6].str));
    }
#line 1855 "parser.tab.c"
    break;

  case 20: /* $@5: %empty  */
#line 372 "parser.y"
                                 { prepareFunctionScope((yyvsp[-3].str), TYPE_VOID); addParamsToScope((yyvsp[-1].node)); }
#line 1861 "parser.tab.c"
    break;

  case 21: /* func: VOID ID '(' param_list ')' $@5 '{' stmt_list '}'  */
#line 372 "parser.y"
                                                                                                                  { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_VOID, (yyvsp[-5].node), (yyvsp[-1].node));  /* Void function with parameters */
        addFunction((yyvsp[-7].str), TYPE_VOID, (yyval.node));           /* Update with AST */
        free((yyvsp[-7].str));
    }
#line 1872 "parser.tab.c"
    break;

  case 22: /* $@6: %empty  */
#line 378 "parser.y"
                      { prepareFunctionScope((yyvsp[-2].str), TYPE_VOID); }
#line 1878 "parser.tab.c"
    break;

  case 23: /* func: VOID ID '(' ')' $@6 '{' stmt_list '}'  */
#line 378 "parser.y"
                                                                                 { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_VOID, NULL, (yyvsp[-1].node));  /* Void function with no parameters */
        addFunction((yyvsp[-6].str), TYPE_VOID, (yyval.node));            /* Update with AST */
        free((yyvsp[-6].str));
    }
#line 1889 "parser.tab.c"
    break;

  case 24: /* $@7: %empty  */
#line 384 "parser.y"
                                        { prepareFunctionScope((yyvsp[-3].str), TYPE_INT); addParamsToScope((yyvsp[-1].node)); }
#line 1895 "parser.tab.c"
    break;

  case 25: /* func: INT '[' ']' ID '(' param_list ')' $@7 '{' stmt_list '}'  */
#line 384 "parser.y"
                                                                                                                        { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_INT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Function returning int array with parameters */
        (yyval.node)->data.func.return_type = TYPE_INT;
        addFunction((yyvsp[-7].str), TYPE_INT, (yyval.node));
        free((yyvsp[-7].str));
    }
#line 1907 "parser.tab.c"
    break;

  case 26: /* $@8: %empty  */
#line 391 "parser.y"
                             { prepareFunctionScope((yyvsp[-2].str), TYPE_INT); }
#line 1913 "parser.tab.c"
    break;

  case 27: /* func: INT '[' ']' ID '(' ')' $@8 '{' stmt_list '}'  */
#line 391 "parser.y"
                                                                                       { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_INT, NULL, (yyvsp[-1].node));  /* Function returning int array, no parameters */
        (yyval.node)->data.func.return_type = TYPE_INT;
        addFunction((yyvsp[-6].str), TYPE_INT, (yyval.node));
        free((yyvsp[-6].str));
    }
#line 1925 "parser.tab.c"
    break;

  case 28: /* $@9: %empty  */
#line 398 "parser.y"
                                          { prepareFunctionScope((yyvsp[-3].str), TYPE_FLOAT); addParamsToScope((yyvsp[-1].node)); }
#line 1931 "parser.tab.c"
    break;

  case 29: /* func: FLOAT '[' ']' ID '(' param_list ')' $@9 '{' stmt_list '}'  */
#line 398 "parser.y"
                                                                                                                            { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_FLOAT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Function returning float array with parameters */
        (yyval.node)->data.func.return_type = TYPE_FLOAT;
        addFunction((yyvsp[-7].str), TYPE_FLOAT, (yyval.node));
        free((yyvsp[-7].str));
    }
#line 1943 "parser.tab.c"
    break;

  case 30: /* $@10: %empty  */
#line 405 "parser.y"
                               { prepareFunctionScope((yyvsp[-2].str), TYPE_FLOAT); }
#line 1949 "parser.tab.c"
    break;

  case 31: /* func: FLOAT '[' ']' ID '(' ')' $@10 '{' stmt_list '}'  */
#line 405 "parser.y"
                                                                                           { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_FLOAT, NULL, (yyvsp[-1].node));  /* Function returning float array, no parameters */
        (yyval.node)->data.func.return_type = TYPE_FLOAT;
        addFunction((yyvsp[-6].str), TYPE_FLOAT, (yyval.node));
        free((yyvsp[-6].str));
    }
#line 1961 "parser.tab.c"
    break;

  case 32: /* func: error '}'  */
#line 412 "parser.y"
                { yyerrok; }
#line 1967 "parser.tab.c"
    break;

  case 33: /* param_list: param  */
#line 417 "parser.y"
          { 
        (yyval.node) = createParamList((yyvsp[0].node), NULL);  /* Single parameter */
    }
#line 1975 "parser.tab.c"
    break;

  case 34: /* param_list: param_list ',' param  */
#line 420 "parser.y"
                           { 
        (yyval.node) = createParamList((yyvsp[-2].node), (yyvsp[0].node));  /* Multiple parameters */
    }
#line 1983 "parser.tab.c"
    break;

  case 35: /* param: INT ID  */
#line 427 "parser.y"
           { 
        (yyval.node) = createParam((yyvsp[0].str), TYPE_INT);  /* Integer parameter */
        free((yyvsp[0].str));
    }
#line 1992 "parser.tab.c"
    break;

  case 36: /* param: FLOAT ID  */
#line 431 "parser.y"
               { 
        (yyval.node) = createParam((yyvsp[0].str), TYPE_FLOAT);  /* Float parameter */
        free((yyvsp[0].str));
    }
#line 2001 "parser.tab.c"
    break;

  case 37: /* param: INT ID '[' ']'  */
#line 435 "parser.y"
                     {
        (yyval.node) = createArrayParam((yyvsp[-2].str), TYPE_INT);  /* Integer array parameter */
        free((yyvsp[-2].str));
    }
#line 2010 "parser.tab.c"
    break;

  case 38: /* param: FLOAT ID '[' ']'  */
#line 439 "parser.y"
                       {
        (yyval.node) = createArrayParam((yyvsp[-2].str), TYPE_FLOAT);  /* Float array parameter */
        free((yyvsp[-2].str));
    }
#line 2019 "parser.tab.c"
    break;

  case 39: /* param: STRUCT ID '*' ID  */
#line 443 "parser.y"
                       {
        if (!lookupStruct((yyvsp[-2].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s' in parameter\n", (yyvsp[-2].str));
            fprintf(stderr, "💡 Suggestion: define 'struct %s' before using it as a parameter type\n\n", (yyvsp[-2].str));
            semantic_error_count++;
        }
        (yyval.node) = createStructParam((yyvsp[0].str), (yyvsp[-2].str), 1);
        free((yyvsp[-2].str));
        free((yyvsp[0].str));
    }
#line 2035 "parser.tab.c"
    break;

  case 40: /* param: STRUCT ID ID  */
#line 454 "parser.y"
                   {
        if (!lookupStruct((yyvsp[-1].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s' in parameter\n", (yyvsp[-1].str));
            fprintf(stderr, "💡 Suggestion: define 'struct %s' before using it as a parameter type\n\n", (yyvsp[-1].str));
            semantic_error_count++;
        }
        (yyval.node) = createStructParam((yyvsp[0].str), (yyvsp[-1].str), 0);
        free((yyvsp[-1].str));
        free((yyvsp[0].str));
    }
#line 2051 "parser.tab.c"
    break;

  case 41: /* stmt_list: stmt  */
#line 469 "parser.y"
         { 
        (yyval.node) = (yyvsp[0].node);  /* Single statement */
    }
#line 2059 "parser.tab.c"
    break;

  case 42: /* stmt_list: stmt_list stmt  */
#line 472 "parser.y"
                     { 
        (yyval.node) = createStmtList((yyvsp[-1].node), (yyvsp[0].node));  /* Multiple statements */
    }
#line 2067 "parser.tab.c"
    break;

  case 43: /* stmt_list_opt: %empty  */
#line 478 "parser.y"
                { (yyval.node) = NULL; }
#line 2073 "parser.tab.c"
    break;

  case 44: /* stmt_list_opt: stmt_list  */
#line 479 "parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 2079 "parser.tab.c"
    break;

  case 54: /* stmt: func_call ';'  */
#line 493 "parser.y"
                    {
        /* Bare function call as statement (e.g., fillNumbers(arr);) */
        (yyval.node) = (yyvsp[-1].node);
    }
#line 2088 "parser.tab.c"
    break;

  case 55: /* stmt: '{' stmt_list '}'  */
#line 497 "parser.y"
                        {
        /* Nested block statement */
        (yyval.node) = (yyvsp[-1].node);
    }
#line 2097 "parser.tab.c"
    break;

  case 56: /* stmt: error ';'  */
#line 501 "parser.y"
                { yyerrok; }
#line 2103 "parser.tab.c"
    break;

  case 57: /* stmt: error  */
#line 502 "parser.y"
            { yyerrok; }
#line 2109 "parser.tab.c"
    break;

  case 58: /* decl: INT ID ';'  */
#line 507 "parser.y"
               { 
        addVar((yyvsp[-1].str), TYPE_INT);                    
        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_INT);  
        free((yyvsp[-1].str));  
        printSymTab();          
    }
#line 2120 "parser.tab.c"
    break;

  case 59: /* decl: FLOAT ID ';'  */
#line 513 "parser.y"
                   { 
        addVar((yyvsp[-1].str), TYPE_FLOAT); 
        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_FLOAT); 
        free((yyvsp[-1].str));                       
        printSymTab();          
    }
#line 2131 "parser.tab.c"
    break;

  case 60: /* decl: STRUCT ID ID ';'  */
#line 519 "parser.y"
                       {
        StructType* st = lookupStruct((yyvsp[-2].str));
        if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s'\n", (yyvsp[-2].str));
            fprintf(stderr, "💡 Suggestion: define it first with: struct %s { ... };\n\n", (yyvsp[-2].str));
            semantic_error_count++;
        } else {
            addStructVar((yyvsp[-1].str), (yyvsp[-2].str));
        }

        (yyval.node) = createStructDecl((yyvsp[-1].str), (yyvsp[-2].str));
        free((yyvsp[-2].str));
        free((yyvsp[-1].str));
        printSymTab();
    }
#line 2152 "parser.tab.c"
    break;

  case 61: /* decl: STRUCT ID '*' ID ';'  */
#line 535 "parser.y"
                           {
        StructType* st = lookupStruct((yyvsp[-3].str));
        if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s'\n", (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestion: define it first with: struct %s { ... };\n\n", (yyvsp[-3].str));
            semantic_error_count++;
        } else {
            addStructPtrVar((yyvsp[-1].str), (yyvsp[-3].str));
        }

        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_STRUCT_PTR);
        (yyval.node)->data.var.struct_name = strdup((yyvsp[-3].str));
        free((yyvsp[-3].str));
        free((yyvsp[-1].str));
        printSymTab();
    }
#line 2174 "parser.tab.c"
    break;

  case 62: /* decl: INT ID '[' NUM ']' ';'  */
#line 552 "parser.y"
                             {
        /* Array declaration with size validation */
        int size = (int)(yyvsp[-2].num);
        if (size == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Array '%s' cannot have zero size\n", (yyvsp[-4].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Array size must be at least 1\n");
            fprintf(stderr, "   • Example: int %s[10];\n\n", (yyvsp[-4].str));
            semantic_error_count++;
        } else {
            addArrayVar((yyvsp[-4].str), TYPE_INT, size); /* Add INT array to symbol table */
        }
        (yyval.node) = createArrayDecl((yyvsp[-4].str), TYPE_INT, size); /* Create array declaration node */
        free((yyvsp[-4].str));                       /* Free the identifier string */
        printSymTab();          /* Print symbol table for verification */
    }
#line 2196 "parser.tab.c"
    break;

  case 63: /* decl: FLOAT ID '[' NUM ']' ';'  */
#line 569 "parser.y"
                               {
        /* Array declaration with size validation */
        int size = (int)(yyvsp[-2].num);
        if (size == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Array '%s' cannot have zero size\n", (yyvsp[-4].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Array size must be at least 1\n");
            fprintf(stderr, "   • Example: float %s[10];\n\n", (yyvsp[-4].str));
            semantic_error_count++;
        } else {
            addArrayVar((yyvsp[-4].str), TYPE_FLOAT, size); /* Add FLOAT array to symbol table */
        }
        (yyval.node) = createArrayDecl((yyvsp[-4].str), TYPE_FLOAT, size); /* Create array declaration node */
        free((yyvsp[-4].str));                       /* Free the identifier string */
        printSymTab();          /* Print symbol table for verification */
    }
#line 2218 "parser.tab.c"
    break;

  case 64: /* decl: INT ID '[' '-' NUM ']' ';'  */
#line 586 "parser.y"
                                 {
        /* ERROR: Negative array size */
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have negative size (-%d)\n", (yyvsp[-5].str), (int)(yyvsp[-2].num));
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Array size must be a positive integer\n");
        fprintf(stderr, "   • Example: int %s[%d];\n\n", (yyvsp[-5].str), (int)(yyvsp[-2].num));
        semantic_error_count++;
        (yyval.node) = createArrayDecl((yyvsp[-5].str), TYPE_INT, 1); /* Dummy node for recovery */
        free((yyvsp[-5].str));
    }
#line 2234 "parser.tab.c"
    break;

  case 65: /* decl: FLOAT ID '[' '-' NUM ']' ';'  */
#line 597 "parser.y"
                                   {
        /* ERROR: Negative array size */
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have negative size (-%d)\n", (yyvsp[-5].str), (int)(yyvsp[-2].num));
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Array size must be a positive integer\n");
        fprintf(stderr, "   • Example: float %s[%d];\n\n", (yyvsp[-5].str), (int)(yyvsp[-2].num));
        semantic_error_count++;
        (yyval.node) = createArrayDecl((yyvsp[-5].str), TYPE_FLOAT, 1); /* Dummy node for recovery */
        free((yyvsp[-5].str));
    }
#line 2250 "parser.tab.c"
    break;

  case 66: /* assign: ID '=' expr ';'  */
#line 612 "parser.y"
                    { 
        /* Check if left-side variable is declared */
        if (!isVarDeclared((yyvsp[-3].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the variable first: int %s;\n", (yyvsp[-3].str));
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        } else if (isArrayVar((yyvsp[-3].str))) {
            fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
            fprintf(stderr, "   '%s' is an array but assigned without subscript\n", (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Use array indexing: %s[index] = value;\n", (yyvsp[-3].str));
            fprintf(stderr, "   • Access specific element: %s[0] = value;\n\n", (yyvsp[-3].str));
            semantic_error_count++;
        }
        /* Check if right side is a bare array name (array used as scalar) */
        if ((yyvsp[-1].node) && (yyvsp[-1].node)->type == NODE_VAR && isArrayVar((yyvsp[-1].node)->data.var.name)) {
            fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
            fprintf(stderr, "   '%s' is an array but used without subscript [index]\n", (yyvsp[-1].node)->data.var.name);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Use array indexing: %s[index]\n", (yyvsp[-1].node)->data.var.name);
            fprintf(stderr, "   • Access a specific element: %s[0]\n\n", (yyvsp[-1].node)->data.var.name);
            semantic_error_count++;
        }

        if (isVarDeclared((yyvsp[-3].str))) {
            VarType lhs = getVarType((yyvsp[-3].str));
            VarType rhs = inferExprType((yyvsp[-1].node));
            if (lhs == TYPE_STRUCT || rhs == TYPE_STRUCT) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Cannot assign struct values directly\n");
                fprintf(stderr, "💡 Suggestion: assign individual fields (e.g., p.x = value)\n\n");
                semantic_error_count++;
            } else if (lhs == TYPE_STRUCT_PTR && rhs != TYPE_STRUCT_PTR) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Pointer assignment requires an address value\n");
                fprintf(stderr, "💡 Suggestion: use '&var' or another struct pointer expression\n\n");
                semantic_error_count++;
            } else if (lhs != TYPE_STRUCT_PTR && rhs == TYPE_STRUCT_PTR) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Cannot assign an address value to a non-pointer variable\n");
                fprintf(stderr, "💡 Suggestion: assign pointer values only to 'struct T*' variables\n\n");
                semantic_error_count++;
            } else if (lhs != rhs && rhs != TYPE_VOID) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Type mismatch in assignment to '%s'\n", (yyvsp[-3].str));
                fprintf(stderr, "💡 Suggestion: assign a value with matching scalar type\n\n");
                semantic_error_count++;
            }
        }

        (yyval.node) = createAssign((yyvsp[-3].str), (yyvsp[-1].node));  
        free((yyvsp[-3].str));                   
    }
#line 2311 "parser.tab.c"
    break;

  case 67: /* assign: ID '[' expr ']' '=' expr ';'  */
#line 668 "parser.y"
                                   {
        /* Check if variable is declared and is an array */
        if (!isVarDeclared((yyvsp[-6].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-6].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the array first: int %s[size];\n", (yyvsp[-6].str));
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        } else if (!isArrayVar((yyvsp[-6].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not an array but used with subscript\n", (yyvsp[-6].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Assign without subscript: %s = value;\n", (yyvsp[-6].str));
            fprintf(stderr, "   • Redeclare as array: int %s[size];\n\n", (yyvsp[-6].str));
            semantic_error_count++;
        }
        /* Array element assignment with bounds checking */
        // Check if index is constant and in bounds
        if ((yyvsp[-4].node)->type == NODE_NUM) {
            int index = (yyvsp[-4].node)->data.num;
            if (isArrayVar((yyvsp[-6].str))) {
                int size = getArraySize((yyvsp[-6].str));
                if (index < 0) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is negative (out of bounds)\n", (yyvsp[-6].str), index);
                    fprintf(stderr, "💡 Suggestion: Array indices must be >= 0\n\n");
                    semantic_error_count++;
                } else if (index >= size) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is out of bounds (size is %d, valid range [0..%d])\n",
                           (yyvsp[-6].str), index, size, size - 1);
                    fprintf(stderr, "💡 Suggestion: Use an index between 0 and %d\n\n", size - 1);
                    semantic_error_count++;
                }
            }
        }

        if (isVarDeclared((yyvsp[-6].str)) && !isArrayVar((yyvsp[-6].str))) {
            VarType lhs = getVarType((yyvsp[-6].str));
            VarType rhs = inferExprType((yyvsp[-1].node));
            if (lhs != rhs && rhs != TYPE_VOID) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Type mismatch in array assignment for '%s[index]'\n", (yyvsp[-6].str));
                fprintf(stderr, "💡 Suggestion: assign a value with matching element type\n\n");
                semantic_error_count++;
            }
        }

        (yyval.node) = createArrayAssign((yyvsp[-6].str), (yyvsp[-4].node), (yyvsp[-1].node)); /* $1 = ID, $3 = index expr, $6 = value expr */
        free((yyvsp[-6].str));                           /* Free the identifier string */
    }
#line 2368 "parser.tab.c"
    break;

  case 68: /* assign: ID DOT ID '=' expr ';'  */
#line 720 "parser.y"
                             {
        ASTNode* base = createVar((yyvsp[-5].str));
        StructType* st = getVarStructType((yyvsp[-5].str));

        if (!isVarDeclared((yyvsp[-5].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-5].str));
            fprintf(stderr, "💡 Suggestion: declare it before member assignment\n\n");
            semantic_error_count++;
        } else if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not a struct\n", (yyvsp[-5].str));
            fprintf(stderr, "💡 Suggestion: only struct variables support dot access\n\n");
            semantic_error_count++;
        } else if (getStructFieldOffset(st, (yyvsp[-3].str)) < 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct '%s' has no field named '%s'\n", st->name, (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestion: use one of the defined fields in struct '%s'\n\n", st->name);
            semantic_error_count++;
        }

        if (inferExprType((yyvsp[-1].node)) == TYPE_STRUCT) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Cannot assign a struct value to field '%s.%s'\n", (yyvsp[-5].str), (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestion: fields currently store scalar values (int/float)\n\n");
            semantic_error_count++;
        }

        (yyval.node) = createMemberAssign(base, (yyvsp[-3].str), (yyvsp[-1].node));
        free((yyvsp[-5].str));
        free((yyvsp[-3].str));
    }
#line 2405 "parser.tab.c"
    break;

  case 69: /* expr: NUM  */
#line 756 "parser.y"
        { 
        (yyval.node) = createNum((yyvsp[0].num));  
    }
#line 2413 "parser.tab.c"
    break;

  case 70: /* expr: FLT  */
#line 759 "parser.y"
          { 
        (yyval.node) = createFlt((yyvsp[0].num));  
    }
#line 2421 "parser.tab.c"
    break;

  case 71: /* expr: STRING  */
#line 762 "parser.y"
             {
        (yyval.node) = createStr((yyvsp[0].str));
        free((yyvsp[0].str));
    }
#line 2430 "parser.tab.c"
    break;

  case 72: /* expr: ID  */
#line 766 "parser.y"
         { 
        /* Check if variable is declared */
        if (!isVarDeclared((yyvsp[0].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[0].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the variable first: int %s;\n", (yyvsp[0].str));
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createVar((yyvsp[0].str));
        free((yyvsp[0].str));
    }
#line 2448 "parser.tab.c"
    break;

  case 73: /* expr: AMP ID  */
#line 779 "parser.y"
             {
        if (!isVarDeclared((yyvsp[0].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[0].str));
            fprintf(stderr, "💡 Suggestion: declare it before taking its address\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createAddrOf(createVar((yyvsp[0].str)));
        free((yyvsp[0].str));
    }
#line 2463 "parser.tab.c"
    break;

  case 74: /* expr: func_call  */
#line 789 "parser.y"
                { 
        (yyval.node) = (yyvsp[0].node);  
    }
#line 2471 "parser.tab.c"
    break;

  case 75: /* expr: expr '+' expr  */
#line 792 "parser.y"
                    { 
        if (inferExprType((yyvsp[-2].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT ||
            inferExprType((yyvsp[-2].node)) == TYPE_STRUCT_PTR || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '+'\n");
            fprintf(stderr, "💡 Suggestion: use struct fields (e.g., p.x + p.y)\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp('+', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 2486 "parser.tab.c"
    break;

  case 76: /* expr: expr '-' expr  */
#line 802 "parser.y"
                    { 
        if (inferExprType((yyvsp[-2].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT ||
            inferExprType((yyvsp[-2].node)) == TYPE_STRUCT_PTR || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '-'\n");
            fprintf(stderr, "💡 Suggestion: use struct fields (e.g., p.x - p.y)\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp('-', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 2501 "parser.tab.c"
    break;

  case 77: /* expr: expr '*' expr  */
#line 812 "parser.y"
                    { 
        if (inferExprType((yyvsp[-2].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT ||
            inferExprType((yyvsp[-2].node)) == TYPE_STRUCT_PTR || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '*'\n");
            fprintf(stderr, "💡 Suggestion: multiply scalar fields instead\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp('*', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 2516 "parser.tab.c"
    break;

  case 78: /* expr: expr '/' expr  */
#line 822 "parser.y"
                    { 
        if (inferExprType((yyvsp[-2].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT ||
            inferExprType((yyvsp[-2].node)) == TYPE_STRUCT_PTR || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '/'\n");
            fprintf(stderr, "💡 Suggestion: divide scalar fields instead\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp('/', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 2531 "parser.tab.c"
    break;

  case 79: /* expr: expr EQ expr  */
#line 832 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_EQ, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2539 "parser.tab.c"
    break;

  case 80: /* expr: expr NE expr  */
#line 835 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_NE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2547 "parser.tab.c"
    break;

  case 81: /* expr: expr LT expr  */
#line 838 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_LT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2555 "parser.tab.c"
    break;

  case 82: /* expr: expr GT expr  */
#line 841 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_GT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2563 "parser.tab.c"
    break;

  case 83: /* expr: expr LE expr  */
#line 844 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_LE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2571 "parser.tab.c"
    break;

  case 84: /* expr: expr GE expr  */
#line 847 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_GE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2579 "parser.tab.c"
    break;

  case 85: /* expr: expr AND expr  */
#line 850 "parser.y"
                    {
        if (inferExprType((yyvsp[-2].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT ||
            inferExprType((yyvsp[-2].node)) == TYPE_STRUCT_PTR || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '&&'\n");
            fprintf(stderr, "💡 Suggestion: compare scalar fields and combine those results\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp(OP_AND, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2594 "parser.tab.c"
    break;

  case 86: /* expr: expr OR expr  */
#line 860 "parser.y"
                   {
        if (inferExprType((yyvsp[-2].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT ||
            inferExprType((yyvsp[-2].node)) == TYPE_STRUCT_PTR || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '||'\n");
            fprintf(stderr, "💡 Suggestion: compare scalar fields and combine those results\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp(OP_OR, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2609 "parser.tab.c"
    break;

  case 87: /* expr: NOT expr  */
#line 870 "parser.y"
                         {
        if (inferExprType((yyvsp[0].node)) == TYPE_STRUCT || inferExprType((yyvsp[0].node)) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '!'\n");
            fprintf(stderr, "💡 Suggestion: negate a scalar comparison result instead\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBinOp(OP_NOT, (yyvsp[0].node), NULL);
    }
#line 2623 "parser.tab.c"
    break;

  case 88: /* expr: '(' expr ')'  */
#line 879 "parser.y"
                   {
        (yyval.node) = (yyvsp[-1].node);
    }
#line 2631 "parser.tab.c"
    break;

  case 89: /* expr: ID '[' expr ']'  */
#line 882 "parser.y"
                      {
        /* Check if variable is declared and is an array */
        if (!isVarDeclared((yyvsp[-3].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the array first: int %s[size];\n", (yyvsp[-3].str));
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        } else if (!isArrayVar((yyvsp[-3].str))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not an array but used with subscript\n", (yyvsp[-3].str));
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Remove the subscript if '%s' is a scalar variable\n", (yyvsp[-3].str));
            fprintf(stderr, "   • Redeclare as array: int %s[size];\n\n", (yyvsp[-3].str));
            semantic_error_count++;
        }
        /* Array element access with bounds checking */
        // Check if index is constant and in bounds
        if ((yyvsp[-1].node)->type == NODE_NUM) {
            int index = (yyvsp[-1].node)->data.num;
            if (isArrayVar((yyvsp[-3].str))) {
                int size = getArraySize((yyvsp[-3].str));
                if (index < 0) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is negative (out of bounds)\n", (yyvsp[-3].str), index);
                    fprintf(stderr, "💡 Suggestion: Array indices must be >= 0\n\n");
                    semantic_error_count++;
                } else if (index >= size) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is out of bounds (size is %d, valid range [0..%d])\n",
                           (yyvsp[-3].str), index, size, size - 1);
                    fprintf(stderr, "💡 Suggestion: Use an index between 0 and %d\n\n", size - 1);
                    semantic_error_count++;
                }
            }
        }
        (yyval.node) = createArrayAccess((yyvsp[-3].str), (yyvsp[-1].node));  /* $1 = ID, $3 = index expression */
        free((yyvsp[-3].str));                        /* Free the identifier string */
    }
#line 2676 "parser.tab.c"
    break;

  case 90: /* expr: expr DOT ID  */
#line 922 "parser.y"
                  {
        StructType* st = getMemberBaseStruct((yyvsp[-2].node));
        if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Dot access requires a struct variable on the left side\n");
            fprintf(stderr, "💡 Suggestion: use '<structVar>.field'\n\n");
            semantic_error_count++;
        } else if (getStructFieldOffset(st, (yyvsp[0].str)) < 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct '%s' has no field named '%s'\n", st->name, (yyvsp[0].str));
            fprintf(stderr, "💡 Suggestion: use a valid field defined in the struct\n\n");
            semantic_error_count++;
        }

        (yyval.node) = createMemberAccess((yyvsp[-2].node), (yyvsp[0].str));
        free((yyvsp[0].str));
    }
#line 2698 "parser.tab.c"
    break;

  case 91: /* print_stmt: PRINT '(' expr ')' ';'  */
#line 943 "parser.y"
                           { 
        (yyval.node) = createPrint((yyvsp[-2].node));  
    }
#line 2706 "parser.tab.c"
    break;

  case 92: /* return_stmt: RETURN expr ';'  */
#line 950 "parser.y"
                    { 
        (yyval.node) = createReturn((yyvsp[-1].node));  
    }
#line 2714 "parser.tab.c"
    break;

  case 93: /* return_stmt: RETURN ';'  */
#line 953 "parser.y"
                 { 
        (yyval.node) = createReturn(NULL);  
    }
#line 2722 "parser.tab.c"
    break;

  case 94: /* func_call: ID '(' arg_list ')'  */
#line 960 "parser.y"
                        { 
        (yyval.node) = createFuncCall((yyvsp[-3].str), (yyvsp[-1].node));  
        free((yyvsp[-3].str));
    }
#line 2731 "parser.tab.c"
    break;

  case 95: /* func_call: ID '(' ')'  */
#line 964 "parser.y"
                 { 
        (yyval.node) = createFuncCall((yyvsp[-2].str), NULL);  
        free((yyvsp[-2].str));
    }
#line 2740 "parser.tab.c"
    break;

  case 96: /* arg_list: expr  */
#line 972 "parser.y"
         { 
        (yyval.node) = createArgList((yyvsp[0].node), NULL);  /* Single argument */
    }
#line 2748 "parser.tab.c"
    break;

  case 97: /* arg_list: arg_list ',' expr  */
#line 975 "parser.y"
                        { 
        (yyval.node) = createArgList((yyvsp[0].node), (yyvsp[-2].node));  /* Multiple arguments - new arg with link to previous list */
    }
#line 2756 "parser.tab.c"
    break;

  case 98: /* $@11: %empty  */
#line 982 "parser.y"
                       { enterBreakContext(); }
#line 2762 "parser.tab.c"
    break;

  case 99: /* while_stmt: WHILE '(' expr ')' $@11 '{' stmt_list '}'  */
#line 982 "parser.y"
                                                                  {
        checkWhileLoop((yyvsp[-5].node));  /* Semantic check for infinite/dead loops */
        exitBreakContext();
        (yyval.node) = createWhile((yyvsp[-5].node), (yyvsp[-1].node));
    }
#line 2772 "parser.tab.c"
    break;

  case 100: /* $@12: %empty  */
#line 994 "parser.y"
                                                     { enterBreakContext(); }
#line 2778 "parser.tab.c"
    break;

  case 101: /* for_stmt: FOR '(' for_init ';' for_cond ';' for_update ')' $@12 stmt  */
#line 994 "parser.y"
                                                                                   {
        checkForLoop((yyvsp[-5].node));  /* Semantic check for infinite/dead for loops */
        exitBreakContext();
        (yyval.node) = createFor((yyvsp[-7].node), (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node));
    }
#line 2788 "parser.tab.c"
    break;

  case 102: /* if_stmt: IF '(' expr ')' stmt  */
#line 1009 "parser.y"
                                               {
        checkIfCondition((yyvsp[-2].node));          /* warn on constant condition */
        (yyval.node) = createIf((yyvsp[-2].node), (yyvsp[0].node), NULL);   /* if without else */
    }
#line 2797 "parser.tab.c"
    break;

  case 103: /* if_stmt: IF '(' expr ')' stmt ELSE stmt  */
#line 1013 "parser.y"
                                     {
        checkIfCondition((yyvsp[-4].node));          /* warn on constant condition */
        (yyval.node) = createIf((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));     /* if with else */
    }
#line 2806 "parser.tab.c"
    break;

  case 104: /* $@13: %empty  */
#line 1020 "parser.y"
                        { enterBreakContext(); }
#line 2812 "parser.tab.c"
    break;

  case 105: /* switch_stmt: SWITCH '(' expr ')' $@13 '{' case_clause_list_opt '}'  */
#line 1020 "parser.y"
                                                                              {
        if (!isIntegralExpr((yyvsp[-5].node))) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   switch controlling expression must be integral (int)\n");
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Use an int expression in switch(...)\n");
            fprintf(stderr, "   • Convert float expressions before switching\n\n");
            semantic_error_count++;
        }

        validateSwitchCases((yyvsp[-1].node));
        exitBreakContext();
        (yyval.node) = createSwitch((yyvsp[-5].node), (yyvsp[-1].node));
    }
#line 2831 "parser.tab.c"
    break;

  case 106: /* case_clause_list_opt: %empty  */
#line 1037 "parser.y"
                { (yyval.node) = NULL; }
#line 2837 "parser.tab.c"
    break;

  case 107: /* case_clause_list_opt: case_clause_list  */
#line 1038 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 2843 "parser.tab.c"
    break;

  case 108: /* case_clause_list: case_clause  */
#line 1042 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 2849 "parser.tab.c"
    break;

  case 109: /* case_clause_list: case_clause_list case_clause  */
#line 1043 "parser.y"
                                   {
        (yyval.node) = appendCase((yyvsp[-1].node), (yyvsp[0].node));
    }
#line 2857 "parser.tab.c"
    break;

  case 110: /* case_clause: CASE case_value ':' stmt_list_opt  */
#line 1049 "parser.y"
                                      {
        (yyval.node) = createCase((int)(yyvsp[-2].num), 0, (yyvsp[0].node));
    }
#line 2865 "parser.tab.c"
    break;

  case 111: /* case_clause: DEFAULT ':' stmt_list_opt  */
#line 1052 "parser.y"
                                {
        (yyval.node) = createCase(0, 1, (yyvsp[0].node));
    }
#line 2873 "parser.tab.c"
    break;

  case 112: /* case_value: NUM  */
#line 1058 "parser.y"
             { (yyval.num) = (yyvsp[0].num); }
#line 2879 "parser.tab.c"
    break;

  case 113: /* case_value: '-' NUM  */
#line 1059 "parser.y"
              { (yyval.num) = -(yyvsp[0].num); }
#line 2885 "parser.tab.c"
    break;

  case 114: /* break_stmt: BREAK ';'  */
#line 1063 "parser.y"
              {
        if (break_context_depth == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   'break' is only valid inside a loop or switch\n");
            fprintf(stderr, "💡 Suggestion: Place break inside while/for/switch blocks\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createBreak();
    }
#line 2899 "parser.tab.c"
    break;

  case 115: /* for_init: %empty  */
#line 1076 "parser.y"
                { (yyval.node) = NULL; }
#line 2905 "parser.tab.c"
    break;

  case 116: /* for_init: INT ID '=' expr  */
#line 1077 "parser.y"
                      {
        /* Inline declaration: for (int i = 0; ...) */
        addVar((yyvsp[-2].str), TYPE_INT);
        printSymTab();
        (yyval.node) = createStmtList(createDecl((yyvsp[-2].str), TYPE_INT), createAssign((yyvsp[-2].str), (yyvsp[0].node)));
        free((yyvsp[-2].str));
    }
#line 2917 "parser.tab.c"
    break;

  case 117: /* for_init: FLOAT ID '=' expr  */
#line 1084 "parser.y"
                        {
        /* Inline declaration: for (float x = 0.0; ...) */
        addVar((yyvsp[-2].str), TYPE_FLOAT);
        printSymTab();
        (yyval.node) = createStmtList(createDecl((yyvsp[-2].str), TYPE_FLOAT), createAssign((yyvsp[-2].str), (yyvsp[0].node)));
        free((yyvsp[-2].str));
    }
#line 2929 "parser.tab.c"
    break;

  case 118: /* for_init: ID '=' expr  */
#line 1091 "parser.y"
                  {
        if (!isVarDeclared((yyvsp[-2].str))) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-2].str));
            fprintf(stderr, "\U0001f4a1 Suggestions:\n");
            fprintf(stderr, "   \u2022 Declare the variable first: int %s;\n", (yyvsp[-2].str));
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createAssign((yyvsp[-2].str), (yyvsp[0].node));
        free((yyvsp[-2].str));
    }
#line 2946 "parser.tab.c"
    break;

  case 119: /* for_init: ID '[' expr ']' '=' expr  */
#line 1103 "parser.y"
                               {
        if (!isVarDeclared((yyvsp[-5].str))) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-5].str));
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createArrayAssign((yyvsp[-5].str), (yyvsp[-3].node), (yyvsp[0].node));
        free((yyvsp[-5].str));
    }
#line 2961 "parser.tab.c"
    break;

  case 120: /* for_cond: %empty  */
#line 1117 "parser.y"
                { (yyval.node) = NULL; }
#line 2967 "parser.tab.c"
    break;

  case 121: /* for_cond: expr  */
#line 1118 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 2973 "parser.tab.c"
    break;

  case 122: /* for_update: %empty  */
#line 1123 "parser.y"
                { (yyval.node) = NULL; }
#line 2979 "parser.tab.c"
    break;

  case 123: /* for_update: ID '=' expr  */
#line 1124 "parser.y"
                  {
        if (!isVarDeclared((yyvsp[-2].str))) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-2].str));
            fprintf(stderr, "\U0001f4a1 Suggestions:\n");
            fprintf(stderr, "   \u2022 Declare the variable first: int %s;\n", (yyvsp[-2].str));
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createAssign((yyvsp[-2].str), (yyvsp[0].node));
        free((yyvsp[-2].str));
    }
#line 2996 "parser.tab.c"
    break;

  case 124: /* for_update: ID '[' expr ']' '=' expr  */
#line 1136 "parser.y"
                               {
        if (!isVarDeclared((yyvsp[-5].str))) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", (yyvsp[-5].str));
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        (yyval.node) = createArrayAssign((yyvsp[-5].str), (yyvsp[-3].node), (yyvsp[0].node));
        free((yyvsp[-5].str));
    }
#line 3011 "parser.tab.c"
    break;


#line 3015 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1148 "parser.y"


/* Maximum number of displayed errors before the compiler gives up */
#define MAX_ERRORS 30

/* Track internal yyerror calls to detect infinite loops */
static int yyerror_total_calls = 0;
#define MAX_YYERROR_CALLS 200

/* ERROR HANDLING - Enhanced error reporting with context */
void yyerror(const char* s) {
    /* Detect infinite error recovery loops */
    yyerror_total_calls++;
    if (yyerror_total_calls > MAX_YYERROR_CALLS) {
        int total = syntax_error_count + semantic_error_count;
        fprintf(stderr, "\n🛑 Too many cascading errors. Stopping compilation.\n\n");
        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                   ❌ COMPILATION FAILED                      ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║                      ERROR SUMMARY                          ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        if (syntax_error_count > 0)
            printf("║  Syntax errors:   %3d                                       ║\n", syntax_error_count);
        if (semantic_error_count > 0)
            printf("║  Semantic errors:  %3d                                      ║\n", semantic_error_count);
        printf("║  ─────────────────────                                      ║\n");
        printf("║  Total errors:    %3d                                       ║\n", total);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  Review the detailed error messages above (on stderr).      ║\n");
        printf("║  Each error includes 💡 suggestions for how to fix it.      ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        exit(1);
    }
    
    /* Suppress duplicate errors on the same line to reduce noise */
    static int last_error_line = -1;
    if (yyline == last_error_line) {
        /* Already reported an error on this line - suppress to avoid spam */
        return;
    }
    last_error_line = yyline;
    
    /* Only count errors that are actually displayed */
    syntax_error_count++;
    
    /* Stop after too many displayed errors */
    if (syntax_error_count > MAX_ERRORS) {
        int total = syntax_error_count + semantic_error_count;
        fprintf(stderr, "\n🛑 Too many errors (%d+). Stopping compilation.\n\n", MAX_ERRORS);
        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                   ❌ COMPILATION FAILED                      ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║                      ERROR SUMMARY                          ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        if (syntax_error_count > 0)
            printf("║  Syntax errors:   %3d                                       ║\n", syntax_error_count);
        if (semantic_error_count > 0)
            printf("║  Semantic errors:  %3d                                      ║\n", semantic_error_count);
        printf("║  ─────────────────────                                      ║\n");
        printf("║  Total errors:    %3d                                       ║\n", total);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  Review the detailed error messages above (on stderr).      ║\n");
        printf("║  Each error includes 💡 suggestions for how to fix it.      ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        exit(1);
    }
    
    fprintf(stderr, "\n❌ Syntax Error at line %d, column %d:\n", yyline, yycolumn);
    
    // Show the problematic line with pointer
    fprintf(stderr, "   %d | ", yyline);
    
    // Provide context-specific suggestions based on the error message
    if (strstr(s, "syntax error")) {
        fprintf(stderr, "Unexpected token or incomplete statement\n");
        fprintf(stderr, "💡 Common fixes:\n");
        fprintf(stderr, "   • Add missing semicolon ';' at the end of the previous statement\n");
        fprintf(stderr, "   • Check for unmatched braces '{' or '}'\n");
        fprintf(stderr, "   • Check for unmatched parentheses '(' or ')'\n");
        fprintf(stderr, "   • Verify variable/function declarations are correct\n");
        fprintf(stderr, "   • Remove any stray characters or typos\n");
    } else if (strstr(s, "unexpected")) {
        fprintf(stderr, "Unexpected token in expression\n");
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Check operator precedence and parentheses\n");
        fprintf(stderr, "   • Ensure expressions are complete (e.g., a + b, not a +)\n");
    } else {
        fprintf(stderr, "%s\n", s);
    }
    
    fprintf(stderr, "\n");
}
