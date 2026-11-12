#pragma once

#if defined(HAVE_READLINE)
#if defined(HAVE_EDITLINE_READLINE)
#include <editline/readline.h>
#else
#include <readline/history.h>
#include <readline/readline.h>
#endif
#endif
