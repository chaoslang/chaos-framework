enum Color = {
  Red,
  Green,
  Blue
}

fn Color.print(self: Color): void {
  var c: Color = self;
  
  if (c == 0) {
    print("Red");
  } else {
    if (c == 1) {
      print("Green");
    } else {
      print("Blue");
    }
  }
}

fn main(): int {

  print("Setting to Green!");
  var c: Color = 1;
  c.print();

  print("Setting to Blue!");
  c = 2;
  c.print();

  return 0;
}
