/**************************************************************************

Copyright 2018 Samsung Electronics co., Ltd. All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#include <cmath>
#include <vector>
#include <cstdint>
#include <memory>

#include "ctorrent_protocols.h"
                                                                 
struct hash_data
{
  uint64_t str_offset;
  uint64_t str_size;

  uint64_t start_idx;
  uint64_t base;
};
                                                                 
extern "C" std::unique_ptr<const calc_result> compute( const calc_chunk& co )
{
  const hash_data* _hash_data = static_cast<const hash_data*>( co.get_data() );
  std::vector<uint64_t> coefs( _hash_data->str_size );
  const char* str = static_cast<const char*>( co.get_data() ) + _hash_data->str_offset;
  uint64_t sum = 0;

  auto res = std::make_unique<calc_result>( co );

  res->data = reinterpret_cast<char*>( new uint64_t[1] );
  res->data_size = sizeof(uint64_t);

  coefs[0] = std::pow( _hash_data->base, _hash_data->start_idx );

  for( std::size_t i = 1; i < _hash_data->str_size; i++ )
    coefs[i] = coefs[i - 1] * _hash_data->base;

  for( std::size_t i = 0; i < _hash_data->str_size; i++ )
    sum += str[i] * coefs[i];
                                                                 
  *reinterpret_cast<uint64_t*>(res->data) = sum;

  return res;
}
