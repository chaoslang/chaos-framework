mod string;

fn string.length(self: string): u64 {
  return self.len;
}

fn string.is_empty(self: string): bool {
  return self.len == 0;
}

fn string.at(self: string, i: u64): u8 {
  var ptr: *u8 = self.data;
  return ptr[i];
}

fn string.eq(a: string, b: string): bool {
  if (a.len != b.len) {
    return 0;
  }
  var pa: *u8 = a.data;
  var pb: *u8 = b.data;
  var i: u64 = 0;
  while (i < a.len) {
    if (pa[i] != pb[i]) {
      return 0;
    }
    i += 1;
  }
  return 1;
}

fn string.starts_with(self: string, prefix: string): bool {
  if (prefix.len > self.len) {
    return 0;
  }
  var ps: *u8 = self.data;
  var pp: *u8 = prefix.data;
  var i: u64 = 0;
  while (i < prefix.len) {
    if (ps[i] != pp[i]) {
      return 0;
    }
    i += 1;
  }
  return 1;
}

fn string.contains(self: string, c: u8): bool {
  var ptr: *u8 = self.data;
  var i: u64 = 0;
  while (i < self.len) {
    if (ptr[i] == c) {
      return 1;
    }
    i += 1;
  }
  return 0;
}
