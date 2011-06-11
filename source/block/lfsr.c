// Gallois LFSR implementation for FreeRTOS
// 
// Copyright (c) 2009-2010 Samuel Tardieu <sam@rfc1149.net>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
// 
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMUEL TARDIEU
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

#include <lfsr.h>

static unsigned portBASE_TYPE state = 1;

static unsigned portBASE_TYPE
lfsr_bit()
{
  portBASE_TYPE rv = state & 1;
  if (sizeof(portBASE_TYPE) == 32)
    state = (state >> 1) ^ (-(state & 1) & 0xd0000001);
  else
    state = (state >> 1) ^ (-(state & 1) & 0xb400);
  return rv;
}

unsigned portBASE_TYPE
lfsr(unsigned portBASE_TYPE bits)
{
  portBASE_TYPE result = 0;
  for (; bits; --bits)
    result = (result << 1) | lfsr_bit();
  return result;
}

void
lfsr_seed(unsigned portBASE_TYPE n)
{
  state = n;
}
