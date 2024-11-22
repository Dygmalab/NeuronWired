
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __DL_ASSERT_H_
#define __DL_ASSERT_H_

#include "types.h"

#ifndef DEBUG
#define STOP_IF_ERR( err, msg )
#define ASSERT_DYGMA( cond, msg )
#else
#define STOP_IF_ERR( err, msg ) if ( ( err ) == RESULT_ERR ) for ( ; ; );
//#define ASSERT_DYGMA( cond, msg ) if ( !( cond ) ) for ( ; ; );
#define ASSERT_DYGMA( cond, msg ) if ( !( cond ) )                                \
                                  {                                               \
                                      debug_error( __FILE__, __LINE__, msg );     \
                                      for ( ; ; );                                \
                                  }
#endif

#define EXIT_IF_ERR( err, msg ) if ( ( err ) == RESULT_ERR ) goto _EXIT;
//#define EXIT_IF_ERR( err, msg ) if ( ( err ) == RESULT_ERR ){ DEBUG_PRINTF(msg); DEBUG_PRINTF("\r\n"); goto _EXIT; }
#define EXIT_IF_OK( res )       if ( ( res ) == RESULT_OK ) goto _EXIT;
#define EXIT_IF_NOK( res )      if ( ( res ) != RESULT_OK ) goto _EXIT;

#endif /* __DL_ASSERT_H_ */
