mod raylib;

struct Color = {
  r: u8,
  g: u8,
  b: u8,
  a: u8,
}

var _lib: ptr;
var _fn_init_window: ptr;
var _fn_set_target_fps: ptr;
var _fn_window_should_close: ptr;
var _fn_begin_drawing: ptr;
var _fn_end_drawing: ptr;
var _fn_clear_background: ptr;
var _fn_draw_text: ptr;
var _fn_close_window: ptr;

fn init(): void {
  _lib = @dlopen("libraylib.so");

  _fn_init_window = @dlsym(_lib, "InitWindow");
  _fn_set_target_fps = @dlsym(_lib, "SetTargetFPS");
  _fn_window_should_close = @dlsym(_lib, "WindowShouldClose");
  _fn_begin_drawing = @dlsym(_lib, "BeginDrawing");
  _fn_end_drawing = @dlsym(_lib, "EndDrawing");
  _fn_clear_background = @dlsym(_lib, "ClearBackground");
  _fn_draw_text = @dlsym(_lib, "DrawText");
  _fn_close_window = @dlsym(_lib, "CloseWindow");
}

fn make_color(r: u8, g: u8, b: u8, a: u8): Color {
  var c: Color;
  c.r = r;
  c.g = g;
  c.b = b;
  c.a = a;
  return c;
}

fn light_gray(): Color {
  return make_color(245, 245, 245, 255);
}

fn black(): Color {
  return make_color(0, 0, 0, 255);
}

fn init_window(w: int, h: int, title: str): void {
  @ffi_call(_fn_init_window, w, h, title);
}

fn set_target_fps(fps: int): void {
  @ffi_call(_fn_set_target_fps, fps);
}

fn window_should_close(): bool {
  return @cast(i32, @ffi_call(_fn_window_should_close)) != 0;
}

fn begin_drawing(): void {
  @ffi_call(_fn_begin_drawing);
}

fn end_drawing(): void {
  @ffi_call(_fn_end_drawing);
}

fn clear_background(color: Color): void {
  @ffi_call(_fn_clear_background, color);
}

fn draw_text(text: str, x: int, y: int, size: int, color: Color): void {
  @ffi_call(_fn_draw_text, text, x, y, size, color);
}

fn close_window(): void {
  @ffi_call(_fn_close_window);
}
