local ffi = require 'ffi'

ffi.cdef[[
      int do_cache_init();
]]

d4lua = ffi.load('./libd4lua.so')

d4lua.do_cache_init();

