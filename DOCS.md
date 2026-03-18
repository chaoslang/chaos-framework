# Chaos Programming Language
___
## Variables
```chaos
var count: int = 10;
var pi: float = 3.14;
```
___
## Functions
```chaos
fn add(x: int, y: int): int {
  return x + y;
}

fn main(): int {
  return 0;
}
```
___
## Control Flow

- No `else if` or `elif`, did you know that they can be represented with many nested else and ifs? (lazy propaganda to not implement them for now)

```chaos
if (count == 10) {
  print("Ten!");
} else {
  print("Not ten.");
}

var i: int = 0;
while (i < 5) {
  i = i + 1;
}
```
___
## Structs & Methods

- No OOP because inheretance is bad (joke, but OOP is a nightmare with bad programmers. maybe this can help?)

```chaos
struct Point = {
  x: float,
  y: float
}

fn Point.print(self: Point): void {
  print(self.x, self.y);
}

var p: Point;
p.x = 1.0; p.y = 2.0;
p.print();
```
___
## Enums

- They are VERY stupid for now since they dont check if values are from the same enum (kinda like C)

```chaos
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

var c: Color = 1; // Green

c.print();
```

___
## Modules

add.ch
```chaos
mod add;

fn add(x: int, y: int): int {
  return x + y;
}
```

main.ch
```chaos
import "add";

fn main(): int {
  print(add.add());
  return 0;
}
```
