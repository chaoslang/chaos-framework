import "raylib/raylib";

fn main(): i32 {
  raylib.init();
  raylib.init_window(800, 450, "Chaos Raylib");
  raylib.set_target_fps(60);

  var bg = raylib.light_gray();
  var fg = raylib.black();

  while (!raylib.window_should_close()) {
    raylib.begin_drawing();
    raylib.clear_background(bg);
    raylib.draw_text("Hello from Chaos!", 190, 200, 40, fg);
    raylib.end_drawing();
  }

  raylib.close_window();
  return 0;
}
