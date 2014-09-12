local ffi = require 'ffi'

ffi.cdef[[
      int do_cache_init(void);
      int do_cache_ref(void);
]]

d4lua = ffi.load('./libd4lua.so')

d4lua.do_cache_init()


miss = d4lua.do_cache_ref()
while miss >= 0 do
   miss = d4lua.do_cache_ref()
end
