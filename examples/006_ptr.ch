fn swap(a: *i32, b: *i32): void {
  var tmp: i32 = *a;
  *a = *b;
  *b = tmp;
}

fn increment(p: *i32): void {
  *p += 1;
}

fn main(): i32 {
  var x: i32 = 10;
  var y: i32 = 20;

  print(x, y);

  swap(&x, &y);
  print(x, y);

  var px: *i32 = &x;
  increment(px);
  print(x);

  var a: i32 = 100;
  var pa: *i32 = &a;
  var ppa: **i32 = &pa;
  var val: i32 = **ppa;
  print(val);

  return 0;
}
