fn make_array(len: i32): *i32 {
  var buf: *i32 = @alloc(i32, len);
  return buf;
}

fn copy_array(dst: *i32, src: *i32, len: i32): void {
  @memcpy(dst, src, len * 4);
}

fn main(): i32 {
  var len: i32 = 5;
  var a: *i32 = make_array(len);

  a[0] = 1;
  a[1] = 2;
  a[2] = 4;
  a[3] = 8;
  a[4] = 16;

  var i: i32 = 0;
  while (i < len) {
    print(a[i]);
    i += 1;
  }

  var b: *i32 = make_array(len);
  copy_array(b, a, len);

  i = 0;
  while (i < len) {
    print(b[i]);
    i += 1;
  }

  var n: i32 = 255;
  var f: f32 = @cast(f32, n);
  var back: i32 = @cast(i32, f);
  print(f);
  print(back);

  @free(a);
  @free(b);
  return 0;
}
