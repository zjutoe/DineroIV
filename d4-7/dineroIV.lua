local ffi = require 'ffi'

ffi.cdef[[
      void test(void);
]]

d4lua = ffi.load('./libd4lua.so')

d4lua.test()

