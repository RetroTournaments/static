Introduction
------------

Static is the open-source technology stack for simultaneous time attack tournaments.
As seen on [youtube.com/flibidydibidy](https://www.youtube.com/@flibidydibidy) at several events.
Or at [GDQ 2025](https://www.youtube.com/watch?v=G0ONr2lloM0)


Build Instructions
------------------

```
git clone --recurse-submodules https://github.com/RetroTournaments/static.git
cd static
git lfs pull
mkdir build && cd build
cmake -G Ninja ..
ninja
```

Then you should have `build/src/static/static` which is the executable, consider adding this to your path.
First initialize the smb database:

```
static smb db init /path/to/smb.rom
```
