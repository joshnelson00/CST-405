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
int syntax_error_count = 0;    /* Counter for syntax errors */
extern int semantic_error_count; /* Counter for semantic errors (in symtab.c) */

#line 128 "parser.tab.c"

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
  YYSYMBOL_EQ = 16,                        /* EQ  */
  YYSYMBOL_NE = 17,                        /* NE  */
  YYSYMBOL_LT = 18,                        /* LT  */
  YYSYMBOL_GT = 19,                        /* GT  */
  YYSYMBOL_LE = 20,                        /* LE  */
  YYSYMBOL_GE = 21,                        /* GE  */
  YYSYMBOL_LOWER_THAN_ELSE = 22,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_23_ = 23,                       /* '+'  */
  YYSYMBOL_24_ = 24,                       /* '-'  */
  YYSYMBOL_25_ = 25,                       /* '*'  */
  YYSYMBOL_26_ = 26,                       /* '/'  */
  YYSYMBOL_27_ = 27,                       /* '('  */
  YYSYMBOL_28_ = 28,                       /* ')'  */
  YYSYMBOL_29_ = 29,                       /* '{'  */
  YYSYMBOL_30_ = 30,                       /* '}'  */
  YYSYMBOL_31_ = 31,                       /* '['  */
  YYSYMBOL_32_ = 32,                       /* ']'  */
  YYSYMBOL_33_ = 33,                       /* ','  */
  YYSYMBOL_34_ = 34,                       /* ';'  */
  YYSYMBOL_35_ = 35,                       /* '='  */
  YYSYMBOL_YYACCEPT = 36,                  /* $accept  */
  YYSYMBOL_program = 37,                   /* program  */
  YYSYMBOL_func_list = 38,                 /* func_list  */
  YYSYMBOL_func = 39,                      /* func  */
  YYSYMBOL_40_1 = 40,                      /* $@1  */
  YYSYMBOL_41_2 = 41,                      /* $@2  */
  YYSYMBOL_42_3 = 42,                      /* $@3  */
  YYSYMBOL_43_4 = 43,                      /* $@4  */
  YYSYMBOL_44_5 = 44,                      /* $@5  */
  YYSYMBOL_45_6 = 45,                      /* $@6  */
  YYSYMBOL_46_7 = 46,                      /* $@7  */
  YYSYMBOL_47_8 = 47,                      /* $@8  */
  YYSYMBOL_48_9 = 48,                      /* $@9  */
  YYSYMBOL_49_10 = 49,                     /* $@10  */
  YYSYMBOL_param_list = 50,                /* param_list  */
  YYSYMBOL_param = 51,                     /* param  */
  YYSYMBOL_stmt_list = 52,                 /* stmt_list  */
  YYSYMBOL_stmt = 53,                      /* stmt  */
  YYSYMBOL_decl = 54,                      /* decl  */
  YYSYMBOL_assign = 55,                    /* assign  */
  YYSYMBOL_expr = 56,                      /* expr  */
  YYSYMBOL_print_stmt = 57,                /* print_stmt  */
  YYSYMBOL_return_stmt = 58,               /* return_stmt  */
  YYSYMBOL_func_call = 59,                 /* func_call  */
  YYSYMBOL_arg_list = 60,                  /* arg_list  */
  YYSYMBOL_while_stmt = 61,                /* while_stmt  */
  YYSYMBOL_for_stmt = 62,                  /* for_stmt  */
  YYSYMBOL_if_stmt = 63,                   /* if_stmt  */
  YYSYMBOL_for_init = 64,                  /* for_init  */
  YYSYMBOL_for_cond = 65,                  /* for_cond  */
  YYSYMBOL_for_update = 66                 /* for_update  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  14
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   603

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  31
/* YYNRULES -- Number of rules.  */
#define YYNRULES  91
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  235

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   277


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
      27,    28,    25,    23,    33,    24,     2,    26,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    34,
       2,    35,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    31,     2,    32,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    29,     2,    30,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    99,    99,   103,   108,   109,   116,   116,   122,   122,
     128,   128,   134,   134,   140,   140,   146,   146,   152,   152,
     159,   159,   166,   166,   173,   173,   180,   185,   188,   195,
     199,   203,   207,   215,   218,   225,   226,   227,   228,   229,
     230,   231,   232,   236,   240,   241,   246,   252,   258,   275,
     292,   303,   318,   347,   391,   394,   397,   401,   416,   419,
     422,   425,   428,   431,   434,   437,   440,   443,   446,   449,
     452,   496,   503,   506,   513,   517,   525,   528,   535,   546,
     559,   563,   571,   572,   579,   586,   598,   612,   613,   618,
     619,   631
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
  "IF", "ELSE", "EQ", "NE", "LT", "GT", "LE", "GE", "LOWER_THAN_ELSE",
  "'+'", "'-'", "'*'", "'/'", "'('", "')'", "'{'", "'}'", "'['", "']'",
  "','", "';'", "'='", "$accept", "program", "func_list", "func", "$@1",
  "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10",
  "param_list", "param", "stmt_list", "stmt", "decl", "assign", "expr",
  "print_stmt", "return_stmt", "func_call", "arg_list", "while_stmt",
  "for_stmt", "if_stmt", "for_init", "for_cond", "for_update", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-69)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-5)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      90,   -24,    -2,    -1,     3,    35,    16,    12,   -69,    47,
      52,    66,    70,    85,   -69,   -69,   -69,    45,    81,    49,
     147,    59,   153,   154,   -69,   -26,   -69,   150,   -69,    30,
     152,   -69,    48,   145,   155,   151,   -69,    37,    82,   162,
     -69,    88,   163,   -69,   161,   166,   430,   196,   -69,   -69,
      73,   430,   203,   -69,   112,   430,   204,   -69,   -69,   200,
     107,   176,   178,   157,    21,   160,   168,   209,   430,   189,
     -69,   -69,   -69,   -69,   -69,   205,   -69,   -69,   -69,   430,
     218,   -69,   216,   430,   219,   -69,   230,   430,   -69,   127,
     158,   158,    69,   135,   158,   -69,   -69,   120,   -69,   158,
     -69,   431,   -69,   158,    54,   158,   256,   -69,   -69,   -69,
     270,   430,   220,   -69,   296,   430,   221,   -69,   310,   -69,
     577,   116,   188,   450,     7,   -69,    46,   -69,   525,   158,
     538,   158,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   -69,   551,    78,   246,   247,   222,   564,   -69,   -69,
     336,   430,   -69,   350,   430,   -69,   -69,   158,   223,   -69,
     233,   250,   240,   251,   228,   474,   -69,   149,   149,   149,
     149,   149,   149,   110,   110,   -69,   -69,   226,   158,   158,
     238,   239,   158,   430,   -69,   376,   -69,   390,   577,   158,
     242,   255,   245,   257,   -69,   -69,   430,   491,   577,   158,
     158,   577,   254,   275,   -69,   -69,   462,   -69,   258,   -69,
     259,   416,   260,   577,   577,   286,   430,   -69,   -69,   -69,
     -69,   158,   122,   266,   -69,   577,   158,   158,   430,   508,
     577,   -69,   261,   158,   577
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     0,     0,     0,     1,     3,     5,     0,     0,     0,
       0,     0,     0,     0,     8,     0,    27,     0,    12,     0,
       0,    16,     0,    29,    30,     0,     6,     0,     0,     0,
      10,     0,     0,    14,     0,     0,     0,     0,    28,    20,
       0,     0,     0,    24,     0,     0,     0,    31,    32,    45,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      33,    35,    36,    37,    38,     0,    39,    40,    41,     0,
       0,    18,     0,     0,     0,    22,     0,     0,    44,     0,
       0,     0,     0,     0,     0,    54,    55,    57,    56,     0,
      73,     0,    58,     0,    82,     0,     0,     9,    34,    42,
       0,     0,     0,    13,     0,     0,     0,    17,     0,    75,
      76,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,     0,     0,     0,     0,     0,     0,    43,     7,
       0,     0,    11,     0,     0,    15,    74,     0,     0,    52,
       0,     0,     0,     0,     0,     0,    69,    63,    64,    65,
      66,    67,    68,    59,    60,    61,    62,     0,     0,     0,
       0,     0,    87,     0,    21,     0,    25,     0,    77,     0,
       0,     0,     0,     0,    71,    70,     0,     0,    85,     0,
       0,    88,     0,    80,    19,    23,     0,    48,     0,    49,
       0,     0,     0,    83,    84,    89,     0,    53,    50,    51,
      78,     0,     0,     0,    81,    86,     0,     0,     0,     0,
      90,    79,     0,     0,    91
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -69,   -69,   291,   -69,   -69,   -69,   -69,   -69,   -69,   -69,
     -69,   -69,   -69,   -69,    13,   265,   -40,   -68,   -69,   -69,
     -11,   -69,   -69,   -46,   -69,   -69,   -69,   -69,   -69,   -69,
     -69
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     5,     6,     7,    47,    35,    52,    39,    56,    42,
     112,    80,   116,    84,    25,    26,    69,    70,    71,    72,
     101,    73,    74,   102,   121,    76,    77,    78,   146,   202,
     223
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,   108,    36,     9,    11,    75,     8,    37,    13,    75,
     160,    82,    -4,     1,   108,    86,    -2,    15,   108,     2,
       3,     4,    75,    75,    95,    96,    97,    98,   106,    10,
      12,   161,    29,    75,    32,    14,    75,    75,   108,   110,
      75,    75,   108,   114,    22,    23,   108,   118,    99,   162,
     108,    50,    22,    23,    54,   100,    22,    23,    40,   143,
      75,   144,   145,    37,    75,    75,    22,    23,    75,    75,
     163,   150,    75,    24,    17,   153,    43,    28,   120,   122,
     123,    37,   108,   128,    18,   108,    27,    31,   130,    22,
      23,     1,   142,    19,   147,    22,    23,     2,     3,     4,
     124,    81,    20,   125,    75,    75,    37,    75,    75,   178,
      49,   185,    21,   179,   187,   203,    53,   108,   165,   108,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
      95,    96,    97,    98,    89,   139,   140,    75,    90,    75,
      85,    75,    91,   108,   156,    37,   188,    89,   224,   157,
      75,   129,    30,   226,    99,   119,   211,   227,    33,    34,
     231,    95,    96,    97,    98,    75,   126,   197,   198,   127,
      75,   201,   137,   138,   139,   140,    44,    38,   206,    41,
      46,    92,    75,    93,    94,    99,    45,   103,   213,   214,
      59,    51,    55,    57,    60,   104,    61,    62,    58,    63,
      64,    65,    66,    67,   131,   132,   133,   134,   135,   136,
     225,   137,   138,   139,   140,   229,   230,    59,    68,   107,
     158,    60,   234,    61,    62,    79,    63,    64,    65,    66,
      67,    59,    83,    87,    88,    60,   105,    61,    62,   109,
      63,    64,    65,    66,    67,    68,   113,   111,   115,   151,
     154,   180,   181,   191,   193,   196,   182,    59,   189,    68,
     117,    60,   194,    61,    62,   190,    63,    64,    65,    66,
      67,    59,   192,   199,   200,    60,   207,    61,    62,   209,
      63,    64,    65,    66,    67,    68,   148,   208,   215,   210,
     216,   222,   218,   219,   228,   221,   233,    59,    16,    68,
     149,    60,    48,    61,    62,     0,    63,    64,    65,    66,
      67,    59,     0,     0,     0,    60,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,   152,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,     0,    68,
     155,    60,     0,    61,    62,     0,    63,    64,    65,    66,
      67,    59,     0,     0,     0,    60,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,   184,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,     0,    68,
     186,    60,     0,    61,    62,     0,    63,    64,    65,    66,
      67,    59,     0,     0,     0,    60,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,   204,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,     0,    68,
     205,    60,     0,    61,    62,     0,    63,    64,    65,    66,
      67,    59,     0,     0,     0,    60,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,   220,   131,   132,   133,
     134,   135,   136,     0,   137,   138,   139,   140,     0,    68,
       0,     0,     0,     0,     0,   141,   131,   132,   133,   134,
     135,   136,     0,   137,   138,   139,   140,     0,   131,   132,
     133,   134,   135,   136,   159,   137,   138,   139,   140,     0,
     131,   132,   133,   134,   135,   136,   217,   137,   138,   139,
     140,     0,     0,     0,     0,     0,   195,   131,   132,   133,
     134,   135,   136,     0,   137,   138,   139,   140,     0,     0,
       0,     0,     0,   212,   131,   132,   133,   134,   135,   136,
       0,   137,   138,   139,   140,     0,     0,     0,     0,     0,
     232,   131,   132,   133,   134,   135,   136,     0,   137,   138,
     139,   140,     0,   164,   131,   132,   133,   134,   135,   136,
       0,   137,   138,   139,   140,     0,   166,   131,   132,   133,
     134,   135,   136,     0,   137,   138,   139,   140,     0,   177,
     131,   132,   133,   134,   135,   136,     0,   137,   138,   139,
     140,     0,   183,   131,   132,   133,   134,   135,   136,     0,
     137,   138,   139,   140
};

static const yytype_int16 yycheck[] =
{
      46,    69,    28,     5,     5,    51,    30,    33,     5,    55,
       3,    51,     0,     1,    82,    55,     0,     1,    86,     7,
       8,     9,    68,    69,     3,     4,     5,     6,    68,    31,
      31,    24,    19,    79,    21,     0,    82,    83,   106,    79,
      86,    87,   110,    83,     7,     8,   114,    87,    27,     3,
     118,    38,     7,     8,    41,    34,     7,     8,    28,     5,
     106,     7,     8,    33,   110,   111,     7,     8,   114,   115,
      24,   111,   118,    28,    27,   115,    28,    28,    89,    90,
      91,    33,   150,    94,    32,   153,     5,    28,    99,     7,
       8,     1,   103,    27,   105,     7,     8,     7,     8,     9,
      31,    28,    32,    34,   150,   151,    33,   153,   154,    31,
      28,   151,    27,    35,   154,   183,    28,   185,   129,   187,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
       3,     4,     5,     6,    27,    25,    26,   183,    31,   185,
      28,   187,    35,   211,    28,    33,   157,    27,   216,    33,
     196,    31,     5,    31,    27,    28,   196,    35,     5,     5,
     228,     3,     4,     5,     6,   211,    31,   178,   179,    34,
     216,   182,    23,    24,    25,    26,    31,    27,   189,    27,
      29,     5,   228,     5,    27,    27,    31,    27,   199,   200,
       1,    29,    29,    32,     5,    27,     7,     8,    32,    10,
      11,    12,    13,    14,    16,    17,    18,    19,    20,    21,
     221,    23,    24,    25,    26,   226,   227,     1,    29,    30,
      32,     5,   233,     7,     8,    29,    10,    11,    12,    13,
      14,     1,    29,    29,    34,     5,    27,     7,     8,    34,
      10,    11,    12,    13,    14,    29,    30,    29,    29,    29,
      29,     5,     5,     3,     3,    29,    34,     1,    35,    29,
      30,     5,    34,     7,     8,    32,    10,    11,    12,    13,
      14,     1,    32,    35,    35,     5,    34,     7,     8,    34,
      10,    11,    12,    13,    14,    29,    30,    32,    34,    32,
      15,     5,    34,    34,    28,    35,    35,     1,     7,    29,
      30,     5,    37,     7,     8,    -1,    10,    11,    12,    13,
      14,     1,    -1,    -1,    -1,     5,    -1,     7,     8,    -1,
      10,    11,    12,    13,    14,    29,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    29,
      30,     5,    -1,     7,     8,    -1,    10,    11,    12,    13,
      14,     1,    -1,    -1,    -1,     5,    -1,     7,     8,    -1,
      10,    11,    12,    13,    14,    29,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    29,
      30,     5,    -1,     7,     8,    -1,    10,    11,    12,    13,
      14,     1,    -1,    -1,    -1,     5,    -1,     7,     8,    -1,
      10,    11,    12,    13,    14,    29,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    29,
      30,     5,    -1,     7,     8,    -1,    10,    11,    12,    13,
      14,     1,    -1,    -1,    -1,     5,    -1,     7,     8,    -1,
      10,    11,    12,    13,    14,    29,    30,    16,    17,    18,
      19,    20,    21,    -1,    23,    24,    25,    26,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    34,    16,    17,    18,    19,
      20,    21,    -1,    23,    24,    25,    26,    -1,    16,    17,
      18,    19,    20,    21,    34,    23,    24,    25,    26,    -1,
      16,    17,    18,    19,    20,    21,    34,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    32,    16,    17,    18,
      19,    20,    21,    -1,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    32,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      32,    16,    17,    18,    19,    20,    21,    -1,    23,    24,
      25,    26,    -1,    28,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    26,    -1,    28,    16,    17,    18,
      19,    20,    21,    -1,    23,    24,    25,    26,    -1,    28,
      16,    17,    18,    19,    20,    21,    -1,    23,    24,    25,
      26,    -1,    28,    16,    17,    18,    19,    20,    21,    -1,
      23,    24,    25,    26
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     7,     8,     9,    37,    38,    39,    30,     5,
      31,     5,    31,     5,     0,     1,    38,    27,    32,    27,
      32,    27,     7,     8,    28,    50,    51,     5,    28,    50,
       5,    28,    50,     5,     5,    41,    28,    33,    27,    43,
      28,    27,    45,    28,    31,    31,    29,    40,    51,    28,
      50,    29,    42,    28,    50,    29,    44,    32,    32,     1,
       5,     7,     8,    10,    11,    12,    13,    14,    29,    52,
      53,    54,    55,    57,    58,    59,    61,    62,    63,    29,
      47,    28,    52,    29,    49,    28,    52,    29,    34,    27,
      31,    35,     5,     5,    27,     3,     4,     5,     6,    27,
      34,    56,    59,    27,    27,    27,    52,    30,    53,    34,
      52,    29,    46,    30,    52,    29,    48,    30,    52,    28,
      56,    60,    56,    56,    31,    34,    31,    34,    56,    31,
      56,    16,    17,    18,    19,    20,    21,    23,    24,    25,
      26,    34,    56,     5,     7,     8,    64,    56,    30,    30,
      52,    29,    30,    52,    29,    30,    28,    33,    32,    34,
       3,    24,     3,    24,    28,    56,    28,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    28,    31,    35,
       5,     5,    34,    28,    30,    52,    30,    52,    56,    35,
      32,     3,    32,     3,    34,    32,    29,    56,    56,    35,
      35,    56,    65,    53,    30,    30,    56,    34,    32,    34,
      32,    52,    32,    56,    56,    34,    15,    34,    34,    34,
      30,    35,     5,    66,    53,    56,    31,    35,    28,    56,
      56,    53,    32,    35,    56
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    36,    37,    37,    38,    38,    40,    39,    41,    39,
      42,    39,    43,    39,    44,    39,    45,    39,    46,    39,
      47,    39,    48,    39,    49,    39,    39,    50,    50,    51,
      51,    51,    51,    52,    52,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    54,    54,    54,    54,
      54,    54,    55,    55,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    57,    58,    58,    59,    59,    60,    60,    61,    62,
      63,    63,    64,    64,    64,    64,    64,    65,    65,    66,
      66,    66
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     0,     9,     0,     8,
       0,     9,     0,     8,     0,     9,     0,     8,     0,    11,
       0,    10,     0,    11,     0,    10,     2,     1,     3,     2,
       2,     4,     4,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     1,     3,     3,     6,     6,
       7,     7,     4,     7,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     5,     3,     2,     4,     3,     1,     3,     7,     9,
       5,     7,     0,     4,     4,     3,     6,     0,     1,     0,
       3,     6
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
  case 2: /* program: func_list  */
#line 99 "parser.y"
              { 
        /* Action: Save the function list as our AST root */
        root = (yyvsp[0].node);  /* $1 refers to the first symbol (func_list) */
    }
#line 1401 "parser.tab.c"
    break;

  case 3: /* program: func_list error  */
#line 103 "parser.y"
                               { root = (yyvsp[-1].node); yyerrok; }
#line 1407 "parser.tab.c"
    break;

  case 4: /* func_list: func  */
#line 108 "parser.y"
         { (yyval.node) = (yyvsp[0].node); }
#line 1413 "parser.tab.c"
    break;

  case 5: /* func_list: func func_list  */
#line 109 "parser.y"
                   { 
        (yyvsp[-1].node)->data.func.next = (yyvsp[0].node);
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1422 "parser.tab.c"
    break;

  case 6: /* $@1: %empty  */
#line 116 "parser.y"
                              { prepareFunctionScope((yyvsp[-3].str), TYPE_INT); addParamsToScope((yyvsp[-1].node)); }
#line 1428 "parser.tab.c"
    break;

  case 7: /* func: INT ID '(' param_list ')' $@1 '{' stmt_list '}'  */
#line 116 "parser.y"
                                                                                                              { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_INT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Function with return type and parameters */
        addFunction((yyvsp[-7].str), TYPE_INT, (yyval.node));         /* Update with AST */
        free((yyvsp[-7].str));
    }
#line 1439 "parser.tab.c"
    break;

  case 8: /* $@2: %empty  */
#line 122 "parser.y"
                     { prepareFunctionScope((yyvsp[-2].str), TYPE_INT); }
#line 1445 "parser.tab.c"
    break;

  case 9: /* func: INT ID '(' ')' $@2 '{' stmt_list '}'  */
#line 122 "parser.y"
                                                                               { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_INT, NULL, (yyvsp[-1].node));  /* Function with no parameters */
        addFunction((yyvsp[-6].str), TYPE_INT, (yyval.node));            /* Update with AST */
        free((yyvsp[-6].str));
    }
#line 1456 "parser.tab.c"
    break;

  case 10: /* $@3: %empty  */
#line 128 "parser.y"
                                  { prepareFunctionScope((yyvsp[-3].str), TYPE_FLOAT); addParamsToScope((yyvsp[-1].node)); }
#line 1462 "parser.tab.c"
    break;

  case 11: /* func: FLOAT ID '(' param_list ')' $@3 '{' stmt_list '}'  */
#line 128 "parser.y"
                                                                                                                    { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_FLOAT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Float function with parameters */
        addFunction((yyvsp[-7].str), TYPE_FLOAT, (yyval.node));           /* Update with AST */
        free((yyvsp[-7].str));
    }
#line 1473 "parser.tab.c"
    break;

  case 12: /* $@4: %empty  */
#line 134 "parser.y"
                       { prepareFunctionScope((yyvsp[-2].str), TYPE_FLOAT); }
#line 1479 "parser.tab.c"
    break;

  case 13: /* func: FLOAT ID '(' ')' $@4 '{' stmt_list '}'  */
#line 134 "parser.y"
                                                                                   { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_FLOAT, NULL, (yyvsp[-1].node));  /* Float function with no parameters */
        addFunction((yyvsp[-6].str), TYPE_FLOAT, (yyval.node));            /* Update with AST */
        free((yyvsp[-6].str));
    }
#line 1490 "parser.tab.c"
    break;

  case 14: /* $@5: %empty  */
#line 140 "parser.y"
                                 { prepareFunctionScope((yyvsp[-3].str), TYPE_VOID); addParamsToScope((yyvsp[-1].node)); }
#line 1496 "parser.tab.c"
    break;

  case 15: /* func: VOID ID '(' param_list ')' $@5 '{' stmt_list '}'  */
#line 140 "parser.y"
                                                                                                                  { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_VOID, (yyvsp[-5].node), (yyvsp[-1].node));  /* Void function with parameters */
        addFunction((yyvsp[-7].str), TYPE_VOID, (yyval.node));           /* Update with AST */
        free((yyvsp[-7].str));
    }
#line 1507 "parser.tab.c"
    break;

  case 16: /* $@6: %empty  */
#line 146 "parser.y"
                      { prepareFunctionScope((yyvsp[-2].str), TYPE_VOID); }
#line 1513 "parser.tab.c"
    break;

  case 17: /* func: VOID ID '(' ')' $@6 '{' stmt_list '}'  */
#line 146 "parser.y"
                                                                                 { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_VOID, NULL, (yyvsp[-1].node));  /* Void function with no parameters */
        addFunction((yyvsp[-6].str), TYPE_VOID, (yyval.node));            /* Update with AST */
        free((yyvsp[-6].str));
    }
#line 1524 "parser.tab.c"
    break;

  case 18: /* $@7: %empty  */
#line 152 "parser.y"
                                        { prepareFunctionScope((yyvsp[-3].str), TYPE_INT); addParamsToScope((yyvsp[-1].node)); }
#line 1530 "parser.tab.c"
    break;

  case 19: /* func: INT '[' ']' ID '(' param_list ')' $@7 '{' stmt_list '}'  */
#line 152 "parser.y"
                                                                                                                        { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_INT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Function returning int array with parameters */
        (yyval.node)->data.func.return_type = TYPE_INT;
        addFunction((yyvsp[-7].str), TYPE_INT, (yyval.node));
        free((yyvsp[-7].str));
    }
#line 1542 "parser.tab.c"
    break;

  case 20: /* $@8: %empty  */
#line 159 "parser.y"
                             { prepareFunctionScope((yyvsp[-2].str), TYPE_INT); }
#line 1548 "parser.tab.c"
    break;

  case 21: /* func: INT '[' ']' ID '(' ')' $@8 '{' stmt_list '}'  */
#line 159 "parser.y"
                                                                                       { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_INT, NULL, (yyvsp[-1].node));  /* Function returning int array, no parameters */
        (yyval.node)->data.func.return_type = TYPE_INT;
        addFunction((yyvsp[-6].str), TYPE_INT, (yyval.node));
        free((yyvsp[-6].str));
    }
#line 1560 "parser.tab.c"
    break;

  case 22: /* $@9: %empty  */
#line 166 "parser.y"
                                          { prepareFunctionScope((yyvsp[-3].str), TYPE_FLOAT); addParamsToScope((yyvsp[-1].node)); }
#line 1566 "parser.tab.c"
    break;

  case 23: /* func: FLOAT '[' ']' ID '(' param_list ')' $@9 '{' stmt_list '}'  */
#line 166 "parser.y"
                                                                                                                            { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-7].str), TYPE_FLOAT, (yyvsp[-5].node), (yyvsp[-1].node));  /* Function returning float array with parameters */
        (yyval.node)->data.func.return_type = TYPE_FLOAT;
        addFunction((yyvsp[-7].str), TYPE_FLOAT, (yyval.node));
        free((yyvsp[-7].str));
    }
#line 1578 "parser.tab.c"
    break;

  case 24: /* $@10: %empty  */
#line 173 "parser.y"
                               { prepareFunctionScope((yyvsp[-2].str), TYPE_FLOAT); }
#line 1584 "parser.tab.c"
    break;

  case 25: /* func: FLOAT '[' ']' ID '(' ')' $@10 '{' stmt_list '}'  */
#line 173 "parser.y"
                                                                                           { 
        exitFunction();
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_FLOAT, NULL, (yyvsp[-1].node));  /* Function returning float array, no parameters */
        (yyval.node)->data.func.return_type = TYPE_FLOAT;
        addFunction((yyvsp[-6].str), TYPE_FLOAT, (yyval.node));
        free((yyvsp[-6].str));
    }
#line 1596 "parser.tab.c"
    break;

  case 26: /* func: error '}'  */
#line 180 "parser.y"
                { yyerrok; }
#line 1602 "parser.tab.c"
    break;

  case 27: /* param_list: param  */
#line 185 "parser.y"
          { 
        (yyval.node) = createParamList((yyvsp[0].node), NULL);  /* Single parameter */
    }
#line 1610 "parser.tab.c"
    break;

  case 28: /* param_list: param_list ',' param  */
#line 188 "parser.y"
                           { 
        (yyval.node) = createParamList((yyvsp[-2].node), (yyvsp[0].node));  /* Multiple parameters */
    }
#line 1618 "parser.tab.c"
    break;

  case 29: /* param: INT ID  */
#line 195 "parser.y"
           { 
        (yyval.node) = createParam((yyvsp[0].str), TYPE_INT);  /* Integer parameter */
        free((yyvsp[0].str));
    }
#line 1627 "parser.tab.c"
    break;

  case 30: /* param: FLOAT ID  */
#line 199 "parser.y"
               { 
        (yyval.node) = createParam((yyvsp[0].str), TYPE_FLOAT);  /* Float parameter */
        free((yyvsp[0].str));
    }
#line 1636 "parser.tab.c"
    break;

  case 31: /* param: INT ID '[' ']'  */
#line 203 "parser.y"
                     {
        (yyval.node) = createArrayParam((yyvsp[-2].str), TYPE_INT);  /* Integer array parameter */
        free((yyvsp[-2].str));
    }
#line 1645 "parser.tab.c"
    break;

  case 32: /* param: FLOAT ID '[' ']'  */
#line 207 "parser.y"
                       {
        (yyval.node) = createArrayParam((yyvsp[-2].str), TYPE_FLOAT);  /* Float array parameter */
        free((yyvsp[-2].str));
    }
#line 1654 "parser.tab.c"
    break;

  case 33: /* stmt_list: stmt  */
#line 215 "parser.y"
         { 
        (yyval.node) = (yyvsp[0].node);  /* Single statement */
    }
#line 1662 "parser.tab.c"
    break;

  case 34: /* stmt_list: stmt_list stmt  */
#line 218 "parser.y"
                     { 
        (yyval.node) = createStmtList((yyvsp[-1].node), (yyvsp[0].node));  /* Multiple statements */
    }
#line 1670 "parser.tab.c"
    break;

  case 42: /* stmt: func_call ';'  */
#line 232 "parser.y"
                    {
        /* Bare function call as statement (e.g., fillNumbers(arr);) */
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1679 "parser.tab.c"
    break;

  case 43: /* stmt: '{' stmt_list '}'  */
#line 236 "parser.y"
                        {
        /* Nested block statement */
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1688 "parser.tab.c"
    break;

  case 44: /* stmt: error ';'  */
#line 240 "parser.y"
                { yyerrok; }
#line 1694 "parser.tab.c"
    break;

  case 45: /* stmt: error  */
#line 241 "parser.y"
            { yyerrok; }
#line 1700 "parser.tab.c"
    break;

  case 46: /* decl: INT ID ';'  */
#line 246 "parser.y"
               { 
        addVar((yyvsp[-1].str), TYPE_INT);                    
        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_INT);  
        free((yyvsp[-1].str));  
        printSymTab();          
    }
#line 1711 "parser.tab.c"
    break;

  case 47: /* decl: FLOAT ID ';'  */
#line 252 "parser.y"
                   { 
        addVar((yyvsp[-1].str), TYPE_FLOAT); 
        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_FLOAT); 
        free((yyvsp[-1].str));                       
        printSymTab();          
    }
#line 1722 "parser.tab.c"
    break;

  case 48: /* decl: INT ID '[' NUM ']' ';'  */
#line 258 "parser.y"
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
#line 1744 "parser.tab.c"
    break;

  case 49: /* decl: FLOAT ID '[' NUM ']' ';'  */
#line 275 "parser.y"
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
#line 1766 "parser.tab.c"
    break;

  case 50: /* decl: INT ID '[' '-' NUM ']' ';'  */
#line 292 "parser.y"
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
#line 1782 "parser.tab.c"
    break;

  case 51: /* decl: FLOAT ID '[' '-' NUM ']' ';'  */
#line 303 "parser.y"
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
#line 1798 "parser.tab.c"
    break;

  case 52: /* assign: ID '=' expr ';'  */
#line 318 "parser.y"
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
        (yyval.node) = createAssign((yyvsp[-3].str), (yyvsp[-1].node));  
        free((yyvsp[-3].str));                   
    }
#line 1832 "parser.tab.c"
    break;

  case 53: /* assign: ID '[' expr ']' '=' expr ';'  */
#line 347 "parser.y"
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
        (yyval.node) = createArrayAssign((yyvsp[-6].str), (yyvsp[-4].node), (yyvsp[-1].node)); /* $1 = ID, $3 = index expr, $6 = value expr */
        free((yyvsp[-6].str));                           /* Free the identifier string */
    }
#line 1877 "parser.tab.c"
    break;

  case 54: /* expr: NUM  */
#line 391 "parser.y"
        { 
        (yyval.node) = createNum((yyvsp[0].num));  
    }
#line 1885 "parser.tab.c"
    break;

  case 55: /* expr: FLT  */
#line 394 "parser.y"
          { 
        (yyval.node) = createFlt((yyvsp[0].num));  
    }
#line 1893 "parser.tab.c"
    break;

  case 56: /* expr: STRING  */
#line 397 "parser.y"
             {
        (yyval.node) = createStr((yyvsp[0].str));
        free((yyvsp[0].str));
    }
#line 1902 "parser.tab.c"
    break;

  case 57: /* expr: ID  */
#line 401 "parser.y"
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
        /* Note: array-as-scalar check is done in the assign rule context,
         * since bare array names are valid as function arguments */
        (yyval.node) = createVar((yyvsp[0].str));  
        free((yyvsp[0].str));            
    }
#line 1922 "parser.tab.c"
    break;

  case 58: /* expr: func_call  */
#line 416 "parser.y"
                { 
        (yyval.node) = (yyvsp[0].node);  
    }
#line 1930 "parser.tab.c"
    break;

  case 59: /* expr: expr '+' expr  */
#line 419 "parser.y"
                    { 
        (yyval.node) = createBinOp('+', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1938 "parser.tab.c"
    break;

  case 60: /* expr: expr '-' expr  */
#line 422 "parser.y"
                    { 
        (yyval.node) = createBinOp('-', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1946 "parser.tab.c"
    break;

  case 61: /* expr: expr '*' expr  */
#line 425 "parser.y"
                    { 
        (yyval.node) = createBinOp('*', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1954 "parser.tab.c"
    break;

  case 62: /* expr: expr '/' expr  */
#line 428 "parser.y"
                    { 
        (yyval.node) = createBinOp('/', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1962 "parser.tab.c"
    break;

  case 63: /* expr: expr EQ expr  */
#line 431 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_EQ, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 1970 "parser.tab.c"
    break;

  case 64: /* expr: expr NE expr  */
#line 434 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_NE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 1978 "parser.tab.c"
    break;

  case 65: /* expr: expr LT expr  */
#line 437 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_LT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 1986 "parser.tab.c"
    break;

  case 66: /* expr: expr GT expr  */
#line 440 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_GT, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 1994 "parser.tab.c"
    break;

  case 67: /* expr: expr LE expr  */
#line 443 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_LE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2002 "parser.tab.c"
    break;

  case 68: /* expr: expr GE expr  */
#line 446 "parser.y"
                   {
        (yyval.node) = createBinOp(OP_GE, (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2010 "parser.tab.c"
    break;

  case 69: /* expr: '(' expr ')'  */
#line 449 "parser.y"
                   {
        (yyval.node) = (yyvsp[-1].node);
    }
#line 2018 "parser.tab.c"
    break;

  case 70: /* expr: ID '[' expr ']'  */
#line 452 "parser.y"
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
#line 2063 "parser.tab.c"
    break;

  case 71: /* print_stmt: PRINT '(' expr ')' ';'  */
#line 496 "parser.y"
                           { 
        (yyval.node) = createPrint((yyvsp[-2].node));  
    }
#line 2071 "parser.tab.c"
    break;

  case 72: /* return_stmt: RETURN expr ';'  */
#line 503 "parser.y"
                    { 
        (yyval.node) = createReturn((yyvsp[-1].node));  
    }
#line 2079 "parser.tab.c"
    break;

  case 73: /* return_stmt: RETURN ';'  */
#line 506 "parser.y"
                 { 
        (yyval.node) = createReturn(NULL);  
    }
#line 2087 "parser.tab.c"
    break;

  case 74: /* func_call: ID '(' arg_list ')'  */
#line 513 "parser.y"
                        { 
        (yyval.node) = createFuncCall((yyvsp[-3].str), (yyvsp[-1].node));  
        free((yyvsp[-3].str));
    }
#line 2096 "parser.tab.c"
    break;

  case 75: /* func_call: ID '(' ')'  */
#line 517 "parser.y"
                 { 
        (yyval.node) = createFuncCall((yyvsp[-2].str), NULL);  
        free((yyvsp[-2].str));
    }
#line 2105 "parser.tab.c"
    break;

  case 76: /* arg_list: expr  */
#line 525 "parser.y"
         { 
        (yyval.node) = createArgList((yyvsp[0].node), NULL);  /* Single argument */
    }
#line 2113 "parser.tab.c"
    break;

  case 77: /* arg_list: arg_list ',' expr  */
#line 528 "parser.y"
                        { 
        (yyval.node) = createArgList((yyvsp[0].node), (yyvsp[-2].node));  /* Multiple arguments - new arg with link to previous list */
    }
#line 2121 "parser.tab.c"
    break;

  case 78: /* while_stmt: WHILE '(' expr ')' '{' stmt_list '}'  */
#line 535 "parser.y"
                                         {
        checkWhileLoop((yyvsp[-4].node));  /* Semantic check for infinite/dead loops */
        (yyval.node) = createWhile((yyvsp[-4].node), (yyvsp[-1].node));
    }
#line 2130 "parser.tab.c"
    break;

  case 79: /* for_stmt: FOR '(' for_init ';' for_cond ';' for_update ')' stmt  */
#line 546 "parser.y"
                                                          {
        (yyval.node) = createFor((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));
    }
#line 2138 "parser.tab.c"
    break;

  case 80: /* if_stmt: IF '(' expr ')' stmt  */
#line 559 "parser.y"
                                               {
        checkIfCondition((yyvsp[-2].node));          /* warn on constant condition */
        (yyval.node) = createIf((yyvsp[-2].node), (yyvsp[0].node), NULL);   /* if without else */
    }
#line 2147 "parser.tab.c"
    break;

  case 81: /* if_stmt: IF '(' expr ')' stmt ELSE stmt  */
#line 563 "parser.y"
                                     {
        checkIfCondition((yyvsp[-4].node));          /* warn on constant condition */
        (yyval.node) = createIf((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));     /* if with else */
    }
#line 2156 "parser.tab.c"
    break;

  case 82: /* for_init: %empty  */
#line 571 "parser.y"
                { (yyval.node) = NULL; }
#line 2162 "parser.tab.c"
    break;

  case 83: /* for_init: INT ID '=' expr  */
#line 572 "parser.y"
                      {
        /* Inline declaration: for (int i = 0; ...) */
        addVar((yyvsp[-2].str), TYPE_INT);
        printSymTab();
        (yyval.node) = createStmtList(createDecl((yyvsp[-2].str), TYPE_INT), createAssign((yyvsp[-2].str), (yyvsp[0].node)));
        free((yyvsp[-2].str));
    }
#line 2174 "parser.tab.c"
    break;

  case 84: /* for_init: FLOAT ID '=' expr  */
#line 579 "parser.y"
                        {
        /* Inline declaration: for (float x = 0.0; ...) */
        addVar((yyvsp[-2].str), TYPE_FLOAT);
        printSymTab();
        (yyval.node) = createStmtList(createDecl((yyvsp[-2].str), TYPE_FLOAT), createAssign((yyvsp[-2].str), (yyvsp[0].node)));
        free((yyvsp[-2].str));
    }
#line 2186 "parser.tab.c"
    break;

  case 85: /* for_init: ID '=' expr  */
#line 586 "parser.y"
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
#line 2203 "parser.tab.c"
    break;

  case 86: /* for_init: ID '[' expr ']' '=' expr  */
#line 598 "parser.y"
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
#line 2218 "parser.tab.c"
    break;

  case 87: /* for_cond: %empty  */
#line 612 "parser.y"
                { (yyval.node) = NULL; }
#line 2224 "parser.tab.c"
    break;

  case 88: /* for_cond: expr  */
#line 613 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 2230 "parser.tab.c"
    break;

  case 89: /* for_update: %empty  */
#line 618 "parser.y"
                { (yyval.node) = NULL; }
#line 2236 "parser.tab.c"
    break;

  case 90: /* for_update: ID '=' expr  */
#line 619 "parser.y"
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
#line 2253 "parser.tab.c"
    break;

  case 91: /* for_update: ID '[' expr ']' '=' expr  */
#line 631 "parser.y"
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
#line 2268 "parser.tab.c"
    break;


#line 2272 "parser.tab.c"

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

#line 643 "parser.y"


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
