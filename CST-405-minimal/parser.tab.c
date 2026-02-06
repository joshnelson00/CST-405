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

#line 94 "parser.tab.c"

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
  YYSYMBOL_INT = 6,                        /* INT  */
  YYSYMBOL_FLOAT = 7,                      /* FLOAT  */
  YYSYMBOL_VOID = 8,                       /* VOID  */
  YYSYMBOL_PRINT = 9,                      /* PRINT  */
  YYSYMBOL_RETURN = 10,                    /* RETURN  */
  YYSYMBOL_11_ = 11,                       /* '+'  */
  YYSYMBOL_12_ = 12,                       /* '-'  */
  YYSYMBOL_13_ = 13,                       /* '*'  */
  YYSYMBOL_14_ = 14,                       /* '/'  */
  YYSYMBOL_15_ = 15,                       /* '('  */
  YYSYMBOL_16_ = 16,                       /* ')'  */
  YYSYMBOL_17_ = 17,                       /* '{'  */
  YYSYMBOL_18_ = 18,                       /* '}'  */
  YYSYMBOL_19_ = 19,                       /* '['  */
  YYSYMBOL_20_ = 20,                       /* ']'  */
  YYSYMBOL_21_ = 21,                       /* ','  */
  YYSYMBOL_22_ = 22,                       /* ';'  */
  YYSYMBOL_23_ = 23,                       /* '='  */
  YYSYMBOL_YYACCEPT = 24,                  /* $accept  */
  YYSYMBOL_program = 25,                   /* program  */
  YYSYMBOL_func_list = 26,                 /* func_list  */
  YYSYMBOL_func = 27,                      /* func  */
  YYSYMBOL_param_list = 28,                /* param_list  */
  YYSYMBOL_param = 29,                     /* param  */
  YYSYMBOL_stmt_list = 30,                 /* stmt_list  */
  YYSYMBOL_stmt = 31,                      /* stmt  */
  YYSYMBOL_decl = 32,                      /* decl  */
  YYSYMBOL_assign = 33,                    /* assign  */
  YYSYMBOL_expr = 34,                      /* expr  */
  YYSYMBOL_print_stmt = 35,                /* print_stmt  */
  YYSYMBOL_return_stmt = 36,               /* return_stmt  */
  YYSYMBOL_func_call = 37,                 /* func_call  */
  YYSYMBOL_arg_list = 38                   /* arg_list  */
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
#define YYLAST   263

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  24
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  15
/* YYNRULES -- Number of rules.  */
#define YYNRULES  53
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  148

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   265


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
      15,    16,    13,    11,    21,    12,     2,    14,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    22,
       2,    23,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    19,     2,    20,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    17,     2,    18,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    57,    57,    61,    66,    67,    74,    79,    84,    89,
      94,    99,   104,   110,   116,   122,   128,   133,   136,   143,
     147,   151,   155,   163,   166,   173,   174,   175,   176,   177,
     178,   183,   189,   195,   202,   213,   217,   226,   229,   232,
     236,   239,   242,   245,   248,   251,   254,   263,   270,   273,
     280,   284,   292,   295
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
  "INT", "FLOAT", "VOID", "PRINT", "RETURN", "'+'", "'-'", "'*'", "'/'",
  "'('", "')'", "'{'", "'}'", "'['", "']'", "','", "';'", "'='", "$accept",
  "program", "func_list", "func", "param_list", "param", "stmt_list",
  "stmt", "decl", "assign", "expr", "print_stmt", "return_stmt",
  "func_call", "arg_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-53)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-5)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     217,   -12,    -3,    -2,    31,    26,   162,   214,   -53,    42,
      43,    70,    54,    83,   -53,   -53,   -53,     2,   104,    15,
     150,    40,   159,   165,   175,    23,   -53,   185,   186,    39,
     201,   195,    50,   157,   163,   192,   202,   230,   178,   192,
     213,   180,   192,   223,   222,   224,    37,    -4,   236,   238,
     231,     8,    44,   -53,   -53,   -53,   -53,   -53,   192,   -53,
     228,    56,    77,   192,   232,    59,    98,   192,   -53,   -53,
     -53,   156,   156,    21,   169,   156,   -53,   -53,    13,   156,
     -53,   155,   -53,   -53,   -53,   105,   192,   233,   -53,   112,
     192,   234,   -53,   119,   193,   161,   244,   -53,   245,   -53,
      80,    85,   156,   215,   156,   156,   156,   156,   -53,   -53,
     126,   192,   -53,   133,   192,   -53,   229,   -53,   235,   237,
     239,   -53,   221,   174,   197,   -53,   225,   225,   -53,   -53,
     -53,   140,   -53,   147,   156,   240,   241,   -53,   -53,   156,
     -53,   -53,   -53,   167,   -53,   -53,   221,   -53
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
       0,     0,     0,     0,     1,     3,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,     0,     0,     0,
       0,     0,     0,    19,    20,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,     0,
       0,     0,     0,    23,    25,    26,    27,    28,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,    21,    22,
      29,     0,     0,     0,     0,     0,    37,    38,    39,     0,
      49,     0,    40,     7,    24,     0,     0,     0,     9,     0,
       0,     0,    11,     0,     0,     0,     0,    31,     0,    32,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     6,
       0,     0,     8,     0,     0,    10,     0,    35,     0,     0,
       0,    51,    52,     0,     0,    45,    41,    42,    43,    44,
      13,     0,    15,     0,     0,     0,     0,    47,    50,     0,
      46,    12,    14,     0,    33,    34,    53,    36
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -53,   -53,   246,   -53,   -14,   219,   -38,   -52,   -53,   -53,
     -37,   -53,   -53,   -53,   -53
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     5,     6,     7,    25,    26,    52,    53,    54,    55,
      81,    56,    57,    82,   123
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      84,    62,     9,    11,    66,    29,     8,    32,    22,    23,
      84,    76,    77,    78,    84,    71,    10,    12,    24,    72,
      85,    22,    23,    79,    61,    89,    14,    65,   101,    93,
      80,    28,   102,    84,    94,    95,    13,    84,   100,    36,
      96,    84,   103,    97,    37,    46,    22,    23,   110,    47,
      48,    49,   113,    50,    51,    40,    31,    17,    84,    70,
      37,    84,    83,    18,   122,   124,    43,   126,   127,   128,
     129,    37,    87,   131,    20,    91,   133,    37,    46,    84,
      37,    84,    47,    48,    49,    19,    50,    51,    76,    77,
      78,   104,   105,   106,   107,    88,   120,   143,    21,    46,
      79,   121,   146,    47,    48,    49,    46,    50,    51,    27,
      47,    48,    49,    46,    50,    51,    92,    47,    48,    49,
      46,    50,    51,   109,    47,    48,    49,    46,    50,    51,
     112,    47,    48,    49,    46,    50,    51,   115,    47,    48,
      49,    46,    50,    51,   130,    47,    48,    49,    46,    50,
      51,   132,    47,    48,    49,    30,    50,    51,   141,    76,
      77,    78,    -2,    15,    33,   142,   104,   105,   106,   107,
      34,    79,   104,   105,   106,   107,    44,   108,   104,   105,
     106,   107,    45,   117,    22,    23,    22,    23,    98,   147,
     138,    99,    35,    46,    60,   139,    64,    47,    48,    49,
      38,    50,    51,    39,   104,   105,   106,   107,   104,   105,
     106,   107,    42,   116,    -4,     1,    41,   140,     1,    58,
       2,     3,     4,     2,     3,     4,   104,   105,   106,   107,
      63,   125,   104,   105,   106,   107,    22,    23,   106,   107,
      67,    73,    68,    74,    69,    86,    75,   118,   119,    90,
     111,   114,   134,    16,     0,   135,    59,   136,     0,     0,
       0,   137,   144,   145
};

static const yytype_int16 yycheck[] =
{
      52,    39,     5,     5,    42,    19,    18,    21,     6,     7,
      62,     3,     4,     5,    66,    19,    19,    19,    16,    23,
      58,     6,     7,    15,    38,    63,     0,    41,    15,    67,
      22,    16,    19,    85,    71,    72,     5,    89,    75,    16,
      19,    93,    79,    22,    21,     1,     6,     7,    86,     5,
       6,     7,    90,     9,    10,    16,    16,    15,   110,    22,
      21,   113,    18,    20,   101,   102,    16,   104,   105,   106,
     107,    21,    16,   111,    20,    16,   114,    21,     1,   131,
      21,   133,     5,     6,     7,    15,     9,    10,     3,     4,
       5,    11,    12,    13,    14,    18,    16,   134,    15,     1,
      15,    16,   139,     5,     6,     7,     1,     9,    10,     5,
       5,     6,     7,     1,     9,    10,    18,     5,     6,     7,
       1,     9,    10,    18,     5,     6,     7,     1,     9,    10,
      18,     5,     6,     7,     1,     9,    10,    18,     5,     6,
       7,     1,     9,    10,    18,     5,     6,     7,     1,     9,
      10,    18,     5,     6,     7,     5,     9,    10,    18,     3,
       4,     5,     0,     1,     5,    18,    11,    12,    13,    14,
       5,    15,    11,    12,    13,    14,    19,    22,    11,    12,
      13,    14,    19,    22,     6,     7,     6,     7,    19,    22,
      16,    22,    17,     1,    16,    21,    16,     5,     6,     7,
      15,     9,    10,    17,    11,    12,    13,    14,    11,    12,
      13,    14,    17,    20,     0,     1,    15,    20,     1,    17,
       6,     7,     8,     6,     7,     8,    11,    12,    13,    14,
      17,    16,    11,    12,    13,    14,     6,     7,    13,    14,
      17,     5,    20,     5,    20,    17,    15,     3,     3,    17,
      17,    17,    23,     7,    -1,    20,    37,    20,    -1,    -1,
      -1,    22,    22,    22
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     6,     7,     8,    25,    26,    27,    18,     5,
      19,     5,    19,     5,     0,     1,    26,    15,    20,    15,
      20,    15,     6,     7,    16,    28,    29,     5,    16,    28,
       5,    16,    28,     5,     5,    17,    16,    21,    15,    17,
      16,    15,    17,    16,    19,    19,     1,     5,     6,     7,
       9,    10,    30,    31,    32,    33,    35,    36,    17,    29,
      16,    28,    30,    17,    16,    28,    30,    17,    20,    20,
      22,    19,    23,     5,     5,    15,     3,     4,     5,    15,
      22,    34,    37,    18,    31,    30,    17,    16,    18,    30,
      17,    16,    18,    30,    34,    34,    19,    22,    19,    22,
      34,    15,    19,    34,    11,    12,    13,    14,    22,    18,
      30,    17,    18,    30,    17,    18,    20,    22,     3,     3,
      16,    16,    34,    38,    34,    16,    34,    34,    34,    34,
      18,    30,    18,    30,    23,    20,    20,    22,    16,    21,
      20,    18,    18,    34,    22,    22,    34,    22
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    24,    25,    25,    26,    26,    27,    27,    27,    27,
      27,    27,    27,    27,    27,    27,    27,    28,    28,    29,
      29,    29,    29,    30,    30,    31,    31,    31,    31,    31,
      31,    32,    32,    32,    32,    33,    33,    34,    34,    34,
      34,    34,    34,    34,    34,    34,    34,    35,    36,    36,
      37,    37,    38,    38
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     8,     7,     8,     7,
       8,     7,    10,     9,    10,     9,     2,     1,     3,     2,
       2,     4,     4,     1,     2,     1,     1,     1,     1,     2,
       1,     3,     3,     6,     6,     4,     7,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     4,     5,     3,     2,
       4,     3,     1,     3
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
#line 57 "parser.y"
              { 
        /* Action: Save the function list as our AST root */
        root = (yyvsp[0].node);  /* $1 refers to the first symbol (func_list) */
    }
#line 1224 "parser.tab.c"
    break;

  case 3: /* program: func_list error  */
#line 61 "parser.y"
                               { root = (yyvsp[-1].node); yyerrok; }
#line 1230 "parser.tab.c"
    break;

  case 4: /* func_list: func  */
#line 66 "parser.y"
         { (yyval.node) = (yyvsp[0].node); }
#line 1236 "parser.tab.c"
    break;

  case 5: /* func_list: func func_list  */
#line 67 "parser.y"
                   { 
        (yyvsp[-1].node)->data.func.next = (yyvsp[0].node);
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1245 "parser.tab.c"
    break;

  case 6: /* func: INT ID '(' param_list ')' '{' stmt_list '}'  */
#line 74 "parser.y"
                                                { 
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_INT, (yyvsp[-4].node), (yyvsp[-1].node));  /* Function with return type and parameters */
        addFunction((yyvsp[-6].str), TYPE_INT, (yyval.node));         /* Add to global symbol table */
        free((yyvsp[-6].str));
    }
#line 1255 "parser.tab.c"
    break;

  case 7: /* func: INT ID '(' ')' '{' stmt_list '}'  */
#line 79 "parser.y"
                                       { 
        (yyval.node) = createFunc((yyvsp[-5].str), TYPE_INT, NULL, (yyvsp[-1].node));  /* Function with no parameters */
        addFunction((yyvsp[-5].str), TYPE_INT, (yyval.node));            /* Add to global symbol table */
        free((yyvsp[-5].str));
    }
#line 1265 "parser.tab.c"
    break;

  case 8: /* func: FLOAT ID '(' param_list ')' '{' stmt_list '}'  */
#line 84 "parser.y"
                                                    { 
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_FLOAT, (yyvsp[-4].node), (yyvsp[-1].node));  /* Float function with parameters */
        addFunction((yyvsp[-6].str), TYPE_FLOAT, (yyval.node));           /* Add to global symbol table */
        free((yyvsp[-6].str));
    }
#line 1275 "parser.tab.c"
    break;

  case 9: /* func: FLOAT ID '(' ')' '{' stmt_list '}'  */
#line 89 "parser.y"
                                         { 
        (yyval.node) = createFunc((yyvsp[-5].str), TYPE_FLOAT, NULL, (yyvsp[-1].node));  /* Float function with no parameters */
        addFunction((yyvsp[-5].str), TYPE_FLOAT, (yyval.node));            /* Add to global symbol table */
        free((yyvsp[-5].str));
    }
#line 1285 "parser.tab.c"
    break;

  case 10: /* func: VOID ID '(' param_list ')' '{' stmt_list '}'  */
#line 94 "parser.y"
                                                   { 
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_VOID, (yyvsp[-4].node), (yyvsp[-1].node));  /* Void function with parameters */
        addFunction((yyvsp[-6].str), TYPE_VOID, (yyval.node));           /* Add to global symbol table */
        free((yyvsp[-6].str));
    }
#line 1295 "parser.tab.c"
    break;

  case 11: /* func: VOID ID '(' ')' '{' stmt_list '}'  */
#line 99 "parser.y"
                                        { 
        (yyval.node) = createFunc((yyvsp[-5].str), TYPE_VOID, NULL, (yyvsp[-1].node));  /* Void function with no parameters */
        addFunction((yyvsp[-5].str), TYPE_VOID, (yyval.node));            /* Add to global symbol table */
        free((yyvsp[-5].str));
    }
#line 1305 "parser.tab.c"
    break;

  case 12: /* func: INT '[' ']' ID '(' param_list ')' '{' stmt_list '}'  */
#line 104 "parser.y"
                                                          { 
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_INT, (yyvsp[-4].node), (yyvsp[-1].node));  /* Function returning int array with parameters */
        (yyval.node)->data.func.return_type = TYPE_INT;
        addFunction((yyvsp[-6].str), TYPE_INT, (yyval.node));
        free((yyvsp[-6].str));
    }
#line 1316 "parser.tab.c"
    break;

  case 13: /* func: INT '[' ']' ID '(' ')' '{' stmt_list '}'  */
#line 110 "parser.y"
                                               { 
        (yyval.node) = createFunc((yyvsp[-5].str), TYPE_INT, NULL, (yyvsp[-1].node));  /* Function returning int array, no parameters */
        (yyval.node)->data.func.return_type = TYPE_INT;
        addFunction((yyvsp[-5].str), TYPE_INT, (yyval.node));
        free((yyvsp[-5].str));
    }
#line 1327 "parser.tab.c"
    break;

  case 14: /* func: FLOAT '[' ']' ID '(' param_list ')' '{' stmt_list '}'  */
#line 116 "parser.y"
                                                            { 
        (yyval.node) = createFunc((yyvsp[-6].str), TYPE_FLOAT, (yyvsp[-4].node), (yyvsp[-1].node));  /* Function returning float array with parameters */
        (yyval.node)->data.func.return_type = TYPE_FLOAT;
        addFunction((yyvsp[-6].str), TYPE_FLOAT, (yyval.node));
        free((yyvsp[-6].str));
    }
#line 1338 "parser.tab.c"
    break;

  case 15: /* func: FLOAT '[' ']' ID '(' ')' '{' stmt_list '}'  */
#line 122 "parser.y"
                                                 { 
        (yyval.node) = createFunc((yyvsp[-5].str), TYPE_FLOAT, NULL, (yyvsp[-1].node));  /* Function returning float array, no parameters */
        (yyval.node)->data.func.return_type = TYPE_FLOAT;
        addFunction((yyvsp[-5].str), TYPE_FLOAT, (yyval.node));
        free((yyvsp[-5].str));
    }
#line 1349 "parser.tab.c"
    break;

  case 16: /* func: error '}'  */
#line 128 "parser.y"
                { yyerrok; }
#line 1355 "parser.tab.c"
    break;

  case 17: /* param_list: param  */
#line 133 "parser.y"
          { 
        (yyval.node) = createParamList((yyvsp[0].node), NULL);  /* Single parameter */
    }
#line 1363 "parser.tab.c"
    break;

  case 18: /* param_list: param_list ',' param  */
#line 136 "parser.y"
                           { 
        (yyval.node) = createParamList((yyvsp[-2].node), (yyvsp[0].node));  /* Multiple parameters */
    }
#line 1371 "parser.tab.c"
    break;

  case 19: /* param: INT ID  */
#line 143 "parser.y"
           { 
        (yyval.node) = createParam((yyvsp[0].str), TYPE_INT);  /* Integer parameter */
        free((yyvsp[0].str));
    }
#line 1380 "parser.tab.c"
    break;

  case 20: /* param: FLOAT ID  */
#line 147 "parser.y"
               { 
        (yyval.node) = createParam((yyvsp[0].str), TYPE_FLOAT);  /* Float parameter */
        free((yyvsp[0].str));
    }
#line 1389 "parser.tab.c"
    break;

  case 21: /* param: INT ID '[' ']'  */
#line 151 "parser.y"
                     {
        (yyval.node) = createArrayParam((yyvsp[-2].str), TYPE_INT);  /* Integer array parameter */
        free((yyvsp[-2].str));
    }
#line 1398 "parser.tab.c"
    break;

  case 22: /* param: FLOAT ID '[' ']'  */
#line 155 "parser.y"
                       {
        (yyval.node) = createArrayParam((yyvsp[-2].str), TYPE_FLOAT);  /* Float array parameter */
        free((yyvsp[-2].str));
    }
#line 1407 "parser.tab.c"
    break;

  case 23: /* stmt_list: stmt  */
#line 163 "parser.y"
         { 
        (yyval.node) = (yyvsp[0].node);  /* Single statement */
    }
#line 1415 "parser.tab.c"
    break;

  case 24: /* stmt_list: stmt_list stmt  */
#line 166 "parser.y"
                     { 
        (yyval.node) = createStmtList((yyvsp[-1].node), (yyvsp[0].node));  /* Multiple statements */
    }
#line 1423 "parser.tab.c"
    break;

  case 29: /* stmt: error ';'  */
#line 177 "parser.y"
                { yyerrok; }
#line 1429 "parser.tab.c"
    break;

  case 30: /* stmt: error  */
#line 178 "parser.y"
            { yyerrok; }
#line 1435 "parser.tab.c"
    break;

  case 31: /* decl: INT ID ';'  */
#line 183 "parser.y"
               { 
        addVar((yyvsp[-1].str), TYPE_INT);                    
        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_INT);  
        free((yyvsp[-1].str));  
        printSymTab();          
    }
#line 1446 "parser.tab.c"
    break;

  case 32: /* decl: FLOAT ID ';'  */
#line 189 "parser.y"
                   { 
        addVar((yyvsp[-1].str), TYPE_FLOAT); 
        (yyval.node) = createDecl((yyvsp[-1].str), TYPE_FLOAT); 
        free((yyvsp[-1].str));                       
        printSymTab();          
    }
#line 1457 "parser.tab.c"
    break;

  case 33: /* decl: INT ID '[' NUM ']' ';'  */
#line 195 "parser.y"
                             {
        /* Array declaration */
        addArrayVar((yyvsp[-4].str), TYPE_INT, (int)(yyvsp[-2].num)); /* Add INT array to symbol table */
        (yyval.node) = createArrayDecl((yyvsp[-4].str), TYPE_INT, (int)(yyvsp[-2].num)); /* Create array declaration node */
        free((yyvsp[-4].str));                       /* Free the identifier string */
        printSymTab();          /* Print symbol table for verification */
    }
#line 1469 "parser.tab.c"
    break;

  case 34: /* decl: FLOAT ID '[' NUM ']' ';'  */
#line 202 "parser.y"
                               {
        /* Array declaration */
        addArrayVar((yyvsp[-4].str), TYPE_FLOAT, (int)(yyvsp[-2].num)); /* Add FLOAT array to symbol table */
        (yyval.node) = createArrayDecl((yyvsp[-4].str), TYPE_FLOAT, (int)(yyvsp[-2].num)); /* Create array declaration node */
        free((yyvsp[-4].str));                       /* Free the identifier string */
        printSymTab();          /* Print symbol table for verification */
    }
#line 1481 "parser.tab.c"
    break;

  case 35: /* assign: ID '=' expr ';'  */
#line 213 "parser.y"
                    { 
        (yyval.node) = createAssign((yyvsp[-3].str), (yyvsp[-1].node));  
        free((yyvsp[-3].str));                   
    }
#line 1490 "parser.tab.c"
    break;

  case 36: /* assign: ID '[' expr ']' '=' expr ';'  */
#line 217 "parser.y"
                                   {
        /* Array element assignment */
        (yyval.node) = createArrayAssign((yyvsp[-6].str), (yyvsp[-4].node), (yyvsp[-1].node)); /* $1 = ID, $3 = index expr, $6 = value expr */
        free((yyvsp[-6].str));                           /* Free the identifier string */
    }
#line 1500 "parser.tab.c"
    break;

  case 37: /* expr: NUM  */
#line 226 "parser.y"
        { 
        (yyval.node) = createNum((yyvsp[0].num));  
    }
#line 1508 "parser.tab.c"
    break;

  case 38: /* expr: FLT  */
#line 229 "parser.y"
          { 
        (yyval.node) = createFlt((yyvsp[0].num));  
    }
#line 1516 "parser.tab.c"
    break;

  case 39: /* expr: ID  */
#line 232 "parser.y"
         { 
        (yyval.node) = createVar((yyvsp[0].str));  
        free((yyvsp[0].str));            
    }
#line 1525 "parser.tab.c"
    break;

  case 40: /* expr: func_call  */
#line 236 "parser.y"
                { 
        (yyval.node) = (yyvsp[0].node);  
    }
#line 1533 "parser.tab.c"
    break;

  case 41: /* expr: expr '+' expr  */
#line 239 "parser.y"
                    { 
        (yyval.node) = createBinOp('+', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1541 "parser.tab.c"
    break;

  case 42: /* expr: expr '-' expr  */
#line 242 "parser.y"
                    { 
        (yyval.node) = createBinOp('-', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1549 "parser.tab.c"
    break;

  case 43: /* expr: expr '*' expr  */
#line 245 "parser.y"
                    { 
        (yyval.node) = createBinOp('*', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1557 "parser.tab.c"
    break;

  case 44: /* expr: expr '/' expr  */
#line 248 "parser.y"
                    { 
        (yyval.node) = createBinOp('/', (yyvsp[-2].node), (yyvsp[0].node));  
    }
#line 1565 "parser.tab.c"
    break;

  case 45: /* expr: '(' expr ')'  */
#line 251 "parser.y"
                   {
        (yyval.node) = (yyvsp[-1].node);
    }
#line 1573 "parser.tab.c"
    break;

  case 46: /* expr: ID '[' expr ']'  */
#line 254 "parser.y"
                      {
        /* Array element access */
        (yyval.node) = createArrayAccess((yyvsp[-3].str), (yyvsp[-1].node));  /* $1 = ID, $3 = index expression */
        free((yyvsp[-3].str));                        /* Free the identifier string */
    }
#line 1583 "parser.tab.c"
    break;

  case 47: /* print_stmt: PRINT '(' expr ')' ';'  */
#line 263 "parser.y"
                           { 
        (yyval.node) = createPrint((yyvsp[-2].node));  
    }
#line 1591 "parser.tab.c"
    break;

  case 48: /* return_stmt: RETURN expr ';'  */
#line 270 "parser.y"
                    { 
        (yyval.node) = createReturn((yyvsp[-1].node));  
    }
#line 1599 "parser.tab.c"
    break;

  case 49: /* return_stmt: RETURN ';'  */
#line 273 "parser.y"
                 { 
        (yyval.node) = createReturn(NULL);  
    }
#line 1607 "parser.tab.c"
    break;

  case 50: /* func_call: ID '(' arg_list ')'  */
#line 280 "parser.y"
                        { 
        (yyval.node) = createFuncCall((yyvsp[-3].str), (yyvsp[-1].node));  
        free((yyvsp[-3].str));
    }
#line 1616 "parser.tab.c"
    break;

  case 51: /* func_call: ID '(' ')'  */
#line 284 "parser.y"
                 { 
        (yyval.node) = createFuncCall((yyvsp[-2].str), NULL);  
        free((yyvsp[-2].str));
    }
#line 1625 "parser.tab.c"
    break;

  case 52: /* arg_list: expr  */
#line 292 "parser.y"
         { 
        (yyval.node) = createArgList((yyvsp[0].node), NULL);  /* Single argument */
    }
#line 1633 "parser.tab.c"
    break;

  case 53: /* arg_list: arg_list ',' expr  */
#line 295 "parser.y"
                        { 
        (yyval.node) = createArgList((yyvsp[0].node), (yyvsp[-2].node));  /* Multiple arguments - new arg with link to previous list */
    }
#line 1641 "parser.tab.c"
    break;


#line 1645 "parser.tab.c"

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

#line 300 "parser.y"


/* ERROR HANDLING - Enhanced error reporting with context */
void yyerror(const char* s) {
    fprintf(stderr, "\n❌ Syntax Error at line %d, column %d:\n", yyline, yycolumn);
    
    // Show the problematic line with pointer
    fprintf(stderr, "   %d | ", yyline);
    
    // This is a simplified approach - in a real compiler you'd track the full line
    if (strstr(s, "syntax error")) {
        fprintf(stderr, "Unexpected token or incomplete statement\n");
        fprintf(stderr, "💡 Common fixes:\n");
        fprintf(stderr, "   • Add missing semicolon ';'\n");
        fprintf(stderr, "   • Check for unmatched parentheses\n");
        fprintf(stderr, "   • Verify variable declarations\n");
    } else if (strstr(s, "unexpected")) {
        fprintf(stderr, "Unexpected token in expression\n");
        fprintf(stderr, "💡 Check operator precedence and parentheses\n");
    } else {
        fprintf(stderr, "%s\n", s);
    }
    
    fprintf(stderr, "\n");
}
