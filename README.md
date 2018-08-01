# Reverse Engine

Reverse Engine is a reverse engineering tool inspired by Cheat Engine.
You can also create [trainers](./trainer.cc) for any program.

##### Root required!

## License

<a rel="license" href="https://www.gnu.org/copyleft/lesser.html">
  <img alt="LGPLv3" style="border-width:0" src="https://www.gnu.org/graphics/lgplv3-88x31.png"/>
</a> This work is licensed under a <a rel="license" href="https://www.gnu.org/copyleft/lesser.html">GNU Lesser General Public License v3</a>.

## Contents

   * [Features](#features)
   * [Simple trainer showcase](#simple-trainer-showcase)
   * [Contacts](#contacts)

## Features

- [x] Find and attach process by PID or title
- [x] Access target memory space, read and write any value
- [x] Pattern scanner
- [x] Get call address
- [ ] Resolve pointers
- [ ] Value scanner with GUI
- [ ] Pointer scanner
- [ ] Memory view
- [ ] Watchpoints and Breakpoints
- [ ] Code injection

## Simple trainer showcase
*Further information:* [Advanced example of trainer](./trainer.cc) 

```cpp
Handle h("csgo_linux64");
h.update_regions();
Cregion *client = h.get_region_by_name("client_client.so");
uintptr_t glow_pointer_call;
h.find_pattern(&glow_pointer_call,
               client,
               "\xE8\x00\x00\x00\x00\x48\x8b\x10\x48\xc1\xe3\x06\x44",
               "x????xxxxxxxx");
uintptr_t glow_call = h.get_call_address(glow_pointer_call);
uintptr_t glow_array_offset;
h.read(&glow_array_offset, glow_call+0x10, sizeof(uintptr_t));
// etc...
```

## Contacts

- Ivan Stepanov ivanstepanovftw@gmail.com
- Andrea Stacchiotti andreastacchiotti@gmail.com 

