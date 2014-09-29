local ffi = require 'ffi'

ffi.cdef[[

      typedef struct {
	 unsigned int	address;
	 char		accesstype;
	 unsigned short	size;		/* of memory referenced, in bytes */
      } d4memref;


      int do_cache_init(int core_id);
      int do_cache_ref(int core_id, d4memref r);
]]

d4lua = ffi.load('./libd4lua.so')

d4lua.do_cache_init(0)

local r = ffi.new("d4memref")

local mem_access_trace = {
   {accesstype=0, address=0xbec2e2ec, size=4, count=d1},
   {accesstype=0, address=0xbec2e2ec, size=4, count=d2},
   {accesstype=0, address=0x04020f1c, size=4, count=d3},
   {accesstype=0, address=0x04020ff4, size=4, count=d4},
   {accesstype=0, address=0x04020f24, size=4, count=d5},
   {accesstype=0, address=0x04020f2c, size=4, count=d6},
   {accesstype=1, address=0xbec2e35c, size=4, count=d7},
   {accesstype=1, address=0xbec2e358, size=4, count=d8},
   {accesstype=1, address=0xbec2e354, size=4, count=d9},
   {accesstype=1, address=0xbec2e350, size=4, count=d10},
   {accesstype=1, address=0xbec2e34c, size=4, count=d11},
   {accesstype=1, address=0xbec2e2ec, size=4, count=d12},
   {accesstype=1, address=0xbec2e310, size=4, count=d13},
   {accesstype=1, address=0x04020e78, size=4, count=d14},
   {accesstype=1, address=0x04020e7c, size=4, count=d15},
   {accesstype=1, address=0x04021544, size=4, count=d16},
   {accesstype=1, address=0x0402153c, size=4, count=d17},
   {accesstype=1, address=0x04021594, size=4, count=d18},
   {accesstype=1, address=0x0402156c, size=4, count=d19},
   {accesstype=0, address=0x04020f34, size=4, count=d20},
   {accesstype=0, address=0x04020f3c, size=4, count=d21},
   {accesstype=0, address=0x04020f44, size=4, count=d22},
   {accesstype=0, address=0x04020f4c, size=4, count=d23},
   {accesstype=1, address=0x04021688, size=4, count=d24},
   {accesstype=1, address=0x04021570, size=4, count=d25},
   {accesstype=1, address=0x04021574, size=4, count=d26},
   {accesstype=1, address=0x04021584, size=4, count=d27},
   {accesstype=0, address=0xbec2e328, size=4, count=d28},
   {accesstype=0, address=0x04018c20, size=4, count=d29},
   {accesstype=0, address=0xbec2e320, size=4, count=d30},
   {accesstype=0, address=0x04000790, size=4, count=d31},
   {accesstype=0, address=0xbec2e318, size=4, count=d32},
   {accesstype=0, address=0xbec2e314, size=4, count=d33},
   {accesstype=0, address=0x0400078c, size=4, count=d34},
   {accesstype=0, address=0xbec2e31c, size=4, count=d35},
   {accesstype=0, address=0x040003a8, size=4, count=d36},
   {accesstype=0, address=0x0402153c, size=4, count=d37},
   {accesstype=0, address=0x040003b0, size=4, count=d38},
   {accesstype=0, address=0xbec2e328, size=4, count=d39},
   {accesstype=0, address=0x04018c20, size=4, count=d40},
   {accesstype=0, address=0xbec2e320, size=4, count=d41},
   {accesstype=0, address=0x04000798, size=4, count=d42},
   {accesstype=0, address=0xbec2e318, size=4, count=d43},
   {accesstype=0, address=0xbec2e314, size=4, count=d44},
   {accesstype=0, address=0x04000794, size=4, count=d45},
   {accesstype=0, address=0xbec2e31c, size=4, count=d46},
   {accesstype=0, address=0x04000348, size=4, count=d47},
   {accesstype=0, address=0x0402153c, size=4, count=d48},
   {accesstype=0, address=0x04000350, size=4, count=d49},
   {accesstype=1, address=0x04021004, size=4, count=d50},
   {accesstype=1, address=0xbec2e328, size=4, count=d51},
   {accesstype=1, address=0x04021008, size=4, count=d52},
   {accesstype=1, address=0xbec2e328, size=4, count=d53},
   {accesstype=0, address=0xbec2e328, size=4, count=d54},
   {accesstype=0, address=0x04018c20, size=4, count=d55},
   {accesstype=0, address=0xbec2e320, size=4, count=d56},
   {accesstype=0, address=0x040007a0, size=4, count=d57},
   {accesstype=0, address=0xbec2e318, size=4, count=d58},
   {accesstype=0, address=0xbec2e314, size=4, count=d59},
   {accesstype=0, address=0x0400079c, size=4, count=d60},
   {accesstype=0, address=0xbec2e31c, size=4, count=d61},
   {accesstype=0, address=0x040003f8, size=4, count=d62},
   {accesstype=0, address=0x0402153c, size=4, count=d63},
   {accesstype=0, address=0x04000400, size=4, count=d64},
   {accesstype=0, address=0xbec2e328, size=4, count=d65},
   {accesstype=0, address=0x04018c20, size=4, count=d66},
   {accesstype=0, address=0xbec2e320, size=4, count=d67},
   {accesstype=0, address=0x040007a8, size=4, count=d68},
   {accesstype=0, address=0xbec2e318, size=4, count=d69},
   {accesstype=0, address=0xbec2e314, size=4, count=d70},
   {accesstype=0, address=0x040007a4, size=4, count=d71},
   {accesstype=0, address=0xbec2e31c, size=4, count=d72},
   {accesstype=0, address=0x04000338, size=4, count=d73},
   {accesstype=0, address=0x0402153c, size=4, count=d74},
   {accesstype=0, address=0x04000340, size=4, count=d75},
   {accesstype=1, address=0x0402100c, size=4, count=d76},
   {accesstype=1, address=0xbec2e328, size=4, count=d77},
   {accesstype=1, address=0x04021010, size=4, count=d78},
   {accesstype=1, address=0xbec2e328, size=4, count=d79},
   {accesstype=0, address=0xbec2e328, size=4, count=d80},
   {accesstype=0, address=0x04018c20, size=4, count=d81},
   {accesstype=0, address=0xbec2e320, size=4, count=d82},
   {accesstype=0, address=0x04020e24, size=4, count=d83},
   {accesstype=0, address=0x04020e20, size=4, count=d84},
   {accesstype=0, address=0xbec2e320, size=4, count=d85},
   {accesstype=0, address=0xbec2e324, size=4, count=d86},
   {accesstype=0, address=0xbec2e32c, size=4, count=d87},
   {accesstype=0, address=0xbec2e320, size=4, count=d88},
   {accesstype=0, address=0xbec2e324, size=4, count=d89},
   {accesstype=0, address=0xbec2e32c, size=4, count=d90},
   {accesstype=0, address=0xbec2e328, size=4, count=d91},
   {accesstype=1, address=0x04021014, size=4, count=d92},
   {accesstype=1, address=0xbec2e328, size=4, count=d93},
   {accesstype=1, address=0xbec2e32c, size=4, count=d94},
   {accesstype=1, address=0xbec2e320, size=4, count=d95},
   {accesstype=1, address=0xbec2e324, size=4, count=d96},
   {accesstype=1, address=0xbec2e328, size=4, count=d97},
   {accesstype=1, address=0xbec2e32c, size=4, count=d98},
   {accesstype=1, address=0xbec2e320, size=4, count=d99},
   {accesstype=1, address=0xbec2e324, size=4, count=d100},
   {accesstype=0, address=0x04020f54, size=4, count=d101},
   {accesstype=0, address=0x04020f5c, size=4, count=d102},
   {accesstype=0, address=0x04020f64, size=4, count=d103},
   {accesstype=0, address=0x04020f6c, size=4, count=d104},
   {accesstype=0, address=0x04020f74, size=4, count=d105},
   {accesstype=0, address=0x04020f7c, size=4, count=d106},
   {accesstype=1, address=0x04021588, size=4, count=d107},
   {accesstype=1, address=0x04021568, size=4, count=d108},
   {accesstype=1, address=0x04021564, size=4, count=d109},
   {accesstype=1, address=0x040215ac, size=4, count=d110},
   {accesstype=1, address=0x040215b8, size=4, count=d111},
   {accesstype=1, address=0x040215a0, size=4, count=d112},
   {accesstype=0, address=0xbec2e320, size=4, count=d113},
   {accesstype=0, address=0xbec2e324, size=4, count=d114},
   {accesstype=0, address=0xbec2e32c, size=4, count=d115},
   {accesstype=0, address=0xbec2e328, size=4, count=d116},
   {accesstype=0, address=0xbec2e320, size=4, count=d117},
   {accesstype=0, address=0xbec2e324, size=4, count=d118},
   {accesstype=0, address=0xbec2e32c, size=4, count=d119},
   {accesstype=0, address=0xbec2e328, size=4, count=d120},
   {accesstype=0, address=0xbec2e320, size=4, count=d121},
   {accesstype=0, address=0xbec2e324, size=4, count=d122},
   {accesstype=0, address=0xbec2e32c, size=4, count=d123},
   {accesstype=0, address=0xbec2e328, size=4, count=d124},
   {accesstype=0, address=0x04021688, size=4, count=d125},
   {accesstype=0, address=0xbec2e2bc, size=4, count=d126},
   {accesstype=0, address=0xbec2e2bc, size=4, count=d127},
   {accesstype=1, address=0xbec2e328, size=4, count=d128},
   {accesstype=1, address=0xbec2e32c, size=4, count=d129},
   {accesstype=1, address=0xbec2e320, size=4, count=d130},
   {accesstype=1, address=0xbec2e324, size=4, count=d131},
   {accesstype=1, address=0xbec2e328, size=4, count=d132},
   {accesstype=1, address=0xbec2e32c, size=4, count=d133},
   {accesstype=1, address=0xbec2e320, size=4, count=d134},
   {accesstype=1, address=0xbec2e324, size=4, count=d135},
   {accesstype=1, address=0xbec2e328, size=4, count=d136},
   {accesstype=1, address=0xbec2e32c, size=4, count=d137},
   {accesstype=1, address=0xbec2e320, size=4, count=d138},
   {accesstype=1, address=0xbec2e324, size=4, count=d139},
   {accesstype=1, address=0x04020e20, size=4, count=d140},
   {accesstype=1, address=0x04020e24, size=4, count=d141},
   {accesstype=1, address=0xbec2e2ec, size=4, count=d142},
   {accesstype=1, address=0xbec2e2dc, size=4, count=d143},
   {accesstype=1, address=0xbec2e2bc, size=4, count=d144},
   {accesstype=0, address=0xbec2e3e0, size=4, count=d145},
   {accesstype=0, address=0xbec2e3e4, size=4, count=d146},
   {accesstype=0, address=0xbec2e3e8, size=4, count=d147},
   {accesstype=0, address=0xbec2e3ec, size=4, count=d148},
   {accesstype=0, address=0xbec2e3f0, size=4, count=d149},
   {accesstype=0, address=0xbec2e3f4, size=4, count=d150},
   {accesstype=0, address=0xbec2e3f8, size=4, count=d151},
   {accesstype=0, address=0xbec2e3fc, size=4, count=d152},
   {accesstype=0, address=0xbec2e400, size=4, count=d153},
   {accesstype=0, address=0xbec2e404, size=4, count=d154},
   {accesstype=0, address=0xbec2e408, size=4, count=d155},
   {accesstype=0, address=0xbec2e40c, size=4, count=d156},
   {accesstype=0, address=0xbec2e410, size=4, count=d157},
   {accesstype=0, address=0xbec2e414, size=4, count=d158},
   {accesstype=0, address=0xbec2e418, size=4, count=d159},
   {accesstype=0, address=0xbec2e3bc, size=4, count=d160},
   {accesstype=0, address=0xbec2ec89, size=4, count=d161},
   {accesstype=0, address=0xbec2e3c0, size=4, count=d162},
   {accesstype=0, address=0xbec2ece4, size=4, count=d163},
   {accesstype=0, address=0xbec2e3c4, size=4, count=d164},
   {accesstype=0, address=0xbec2ecfe, size=4, count=d165},
   {accesstype=0, address=0xbec2e3c8, size=4, count=d166},
   {accesstype=0, address=0xbec2ed2d, size=4, count=d167},
   {accesstype=0, address=0xbec2e3cc, size=4, count=d168},
   {accesstype=0, address=0xbec2ed55, size=4, count=d169},
   {accesstype=0, address=0xbec2e3d0, size=4, count=d170},
}

for k, v in ipairs(mem_access_trace) do
   r.accesstype = v.accesstype
   r.address = v.address
   r.size = v.size
   miss = d4lua.do_cache_ref(0, r)
   if miss < 0 then break end
end

-- miss = d4lua.do_cache_ref()
-- while miss >= 0 do
--    miss = d4lua.do_cache_ref()
-- end
