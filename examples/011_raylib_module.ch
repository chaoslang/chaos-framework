import "raylib/raylib";

fn main(): i32 {
  raylib.init();
  raylib.init_window(800, 450, "Chaos Raylib");
  raylib.set_target_fps(60);

  var color: raylib.Color;
  color.set(255, 255, 255, 255);

  var textColor: raylib.Color;
  textColor.set(0, 0, 0, 255);
  
  while (!raylib.window_should_close()) {
    raylib.begin_drawing();
    raylib.clear_background(color);
    raylib.draw_text("Hello from Chaos!", 190, 200, 40, textColor);
    raylib.end_drawing();
  }

  raylib.close_window();
  return 0;
}
