struct Color = {
  r: u8,
  g: u8,
  b: u8,
  a: u8
}

fn main(): i32 {
  var lib: ptr = @dlopen("libraylib.so");

  var fn_init:         ptr = @dlsym(lib, "InitWindow");
  var fn_set_fps:      ptr = @dlsym(lib, "SetTargetFPS");
  var fn_should_close: ptr = @dlsym(lib, "WindowShouldClose");
  var fn_begin:        ptr = @dlsym(lib, "BeginDrawing");
  var fn_end:          ptr = @dlsym(lib, "EndDrawing");
  var fn_clear:        ptr = @dlsym(lib, "ClearBackground");
  var fn_text:         ptr = @dlsym(lib, "DrawText");
  var fn_close:        ptr = @dlsym(lib, "CloseWindow");

  @ffi_call(fn_init, 800, 450, "Chaos Raylib");
  @ffi_call(fn_set_fps, 60);

  var bg: Color;
  bg.r = 245;
  bg.g = 245;
  bg.b = 245;
  bg.a = 255;

  var black: Color;
  black.r = 0;
  black.g = 0;
  black.b = 0;
  black.a = 255;

  var should_close: i32 = @cast(i32, @ffi_call(fn_should_close));
  while (should_close == 0) {
    @ffi_call(fn_begin);
    @ffi_call(fn_clear, bg);
    @ffi_call(fn_text, "Hello from Chaos!", 190, 200, 40, black);
    @ffi_call(fn_end);
    should_close = @cast(i32, @ffi_call(fn_should_close));
  }

  @ffi_call(fn_close);
  return 0;
}
