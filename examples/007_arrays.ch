fn sum(arr: *i32, len: i32): i32 {
  var total: i32 = 0;
  var i: i32 = 0;
  while (i < len) {
    total += arr[i];
    i += 1;
  }
  return total;
}

fn fill(arr: *i32, len: i32, val: i32): void {
  var i: i32 = 0;
  while (i < len) {
    arr[i] = val;
    i += 1;
  }
}

fn main(): i32 {
  var nums[8]: i32;

  var i: i32 = 0;
  while (i < 8) {
    nums[i] = i * i;
    i += 1;
  }

  i = 0;
  while (i < 8) {
    print(nums[i]);
    i += 1;
  }

  var total: i32 = sum(&nums[0], 8);
  print(total);

  var buf[4]: i32;
  fill(&buf[0], 4, 99);
  print(buf[0], buf[1], buf[2], buf[3]);

  return 0;
}
