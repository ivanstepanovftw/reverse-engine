#!/usr/bin/env python
import master as RE
import ctypes as c
import struct


target = "FAKEMEM"
VALID_ADDRESS = 0x7f851df8c010
search_for = "0"

# print("#"*70)
# print("RE:", dir(RE))
# print("RE.Handle:", dir(RE.Handle))
# print("RE.Handler.read:", dir(RE.Handle.read))
# print("#"*70)
# print(help(RE))

# print(RE.execute("ls -la | grep '.py' > `pwd`/asd.txt "), end="")
# print(RE.execute("ls -1 `pwd`/asd.txt"), end="")
# print(RE.execute("rm -f `pwd`/asd.txt"), end="")

# for p in RE.getProcesses():
#     print(p)

# print("mem total:", RE.get_mem_total())
# print("mem free:", RE.get_mem_free())

handle = RE.Handle(target)
handle.update_regions()
if not handle.is_good():
    print("Not good")
    exit(1)


c_buf1 = (c.c_char * 20)()
sz = handle.read(VALID_ADDRESS, c.addressof(c_buf1), 20)
print("char *; read(3): {}, sz: {}".format([hex(ord(i)) for i in c_buf1], sz))
print("---")

m1 = RE.mem64_t()
sz = handle.read(VALID_ADDRESS, RE.addressof(m1), 8)
print("mem64_t; read(3): {}, sz: {}".format(bytes(m1), sz))
print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
ideal_len = 2
bb = handle.read(VALID_ADDRESS, ideal_len)  # lets read 'short' integer (return 'bytes' type)
if len(bb) != ideal_len:  # possibly end of process' map
    print("Got", len(bb), "instead", ideal_len)
    if len(bb) == 0:  # segfault
        print("Cannot read")
ii = struct.unpack("<H", bb)[0]  # 'bytes' type to 'short' integer
c_ii = c.c_uint16(ii)  # convert to ctypes integer
print("py_bytes     ; read(2): {}, sz: {}".format(bb, len(bb)))
print("py_integer   ; read(2): {}, sz: {}".format(ii, 2))
print("ctypes_uint16; read(2): {}, sz: {}".format(c_ii.value, c.sizeof(c_ii)))
print("---")
print("mem64_t", m1)
print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
# todo[critical]: writing to process example
print("No Errors")
