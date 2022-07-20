# BSDiff Inplace

This lib is modified from [bsdiff](https://github.com/mendsley/bsdiff), the original version is placed at ./bsdiff.

## Compile

You can run the following command to build this project

``` bash
cd ${path-to-this-project}
cmake -B build .
make -C build
```

You can find the executables in `./build/src/bin`, the libs in `./build/src/lib`, and the headers are placed in ./include

## Demo

Try this demo to trial this lib. 

``` bash
./demo/run.sh
```

## Future Work

I just hardcoded the in-place patching part in the `./src/bin/patch.c`, it's not elegant and people do need write their own patch executable for different platforms. Moreover, now, we still need to load the whole old file into our memory, which is unfriendly for tiny devices, the ultimate reason for me to rewrite bsdiff. <del>I plan to modify the original bsdiff interface later and offer an array-like data I/O to support tiny devices. -- maybe next week :)</del>

It's already implemented, using compressing blocks instead of BZip2, the lib is fastlz based on LZ77.
