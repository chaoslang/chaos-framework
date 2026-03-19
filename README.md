QuickStart:

```console
make
make examples
./chaosc <source.ch> <output> <backend>
```

## Instructions:
- `chaos_parser.h`: core data structures and helpers.
- `chaos_default_parser.h`: default parsing behavior (for inspiration).
- `template_parser.h`: **only file you should touch** where you customize syntax.

### Customize syntax in `template_parser.h` by looking at `chaos_default_parser.h`

Basically just edit only the functions you care about having an alternative syntax for, say:
- `parse_struct()`
- `parse_function()`
- `parse_enum()`
- `parse_statement()`
- `parse_expression()`

# Catch:
- Lexer, semantic analysis, and backend stays unchanged. which means you dont have to care about any of that!
