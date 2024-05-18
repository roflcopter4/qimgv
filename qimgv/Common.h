#pragma once
#ifndef MKW3FOswbCrnAZjlsqQvpNOJnqADHMTVtSgUEsIrLrbofhZOWrRNmd0jIUCxRC
#define MKW3FOswbCrnAZjlsqQvpNOJnqADHMTVtSgUEsIrLrbofhZOWrRNmd0jIUCxRC //NOLINT(clang-diagnostic-unused-macros)
/***************************************************************************************/


#define ND     [[nodiscard]]
#define QS(s)  QStringLiteral(s)
#define QSV(s) QStringView(u"" s)

#define DELETE_COPY_CONSTRUCTORS(CLASS)       \
    CLASS(CLASS const &)            = delete; \
    CLASS &operator=(CLASS const &) = delete /*NOLINT(bugprone-macro-parentheses)*/

#define DELETE_MOVE_CONSTRUCTORS(CLASS)                                                  \
    CLASS(CLASS &&) noexcept            = delete; /*NOLINT(bugprone-macro-parentheses)*/ \
    CLASS &operator=(CLASS &&) noexcept = delete  /*NOLINT(bugprone-macro-parentheses)*/

#define DELETE_ALL_CONSTRUCTORS(CLASS) \
    DELETE_COPY_CONSTRUCTORS(CLASS);   \
    DELETE_MOVE_CONSTRUCTORS(CLASS)

#define DEFAULT_COPY_CONSTRUCTORS(CLASS)       \
    CLASS(CLASS const &)            = default; \
    CLASS &operator=(CLASS const &) = default /*NOLINT(bugprone-macro-parentheses)*/

#define DEFAULT_MOVE_CONSTRUCTORS(CLASS)                                                  \
    CLASS(CLASS &&) noexcept            = default; /*NOLINT(bugprone-macro-parentheses)*/ \
    CLASS &operator=(CLASS &&) noexcept = default  /*NOLINT(bugprone-macro-parentheses)*/

#define DEFAULT_ALL_CONSTRUCTORS(CLASS) \
    DEFAULT_COPY_CONSTRUCTORS(CLASS);   \
    DEFAULT_MOVE_CONSTRUCTORS(CLASS)


#if defined _MSC_VER
# define PRETTY_FUNCTION_SIG __FUNCSIG__
#elif defined __GNUC__
# define PRETTY_FUNCTION_SIG __PRETTY_FUNCTION__
#else
# define PRETTY_FUNCTION_SIG __func__
#endif


/***************************************************************************************/
#endif