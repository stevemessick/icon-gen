# Icon Preview Prototype

See the spec here:
https://docs.google.com/document/d/1h4vfkpamhJ-SN-jUESdmOp9MM0MjxMm00rpSeZ3gqZ0

This directory contains patch files for the Dart SDK and Dart-Code,
the VS Code extension. The screen shot in the spec was generated from
the code in these patches.

To apply them, make the working directory the root of the project (.../dart/sdk or .../Dart-Code) then do

`git apply -3 ~/Documents/icon-previews-{dart,vsc}.patch`

The `dart` suffix is for the Dart SDK. The `vsc` one is for Dart-Code.

The patches worked correctly on July 25, 2023 but are probably out-of-date
by now.

In the prototype, the `icon-gen` C program is part of the Dart SDK. It is
in the `ffi/icon-gen` directory. That needs to be moved to the Flutter SDK
and converted to use `skia` instead of `cairo`. Some expremintal code
in `sample.cc` is heading in that direction. The function `raster()` should
write to a PNG file but it hasn't been tested. The `DrawGlyphs()` function
should call it, or just be changed to embed it. A lot of PDF-related code
needs to be deleted. `icon_gen.cc` is the original source for the `icon-gen`
program, which uses `cairo`. All other `*.cc` files can be deleted.
