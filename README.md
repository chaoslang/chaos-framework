QuickStart:

```console
make
make examples
./chaosc <source.ch> <output> <backend>
```

## Build your own C-like dialect

Parser files are separated by responsibility:

- `chaos_parser.h`: AST + parser core data structures and tiny helpers.
- `chaos_default_parser.h`: default parsing behavior (starter implementation).
- `template_parser.h`: **dialect surface** where you customize syntax.

### Customize syntax in `template_parser.h`

Every `parse_*` function is defined in `template_parser.h` and currently
forwards to a default `chaos_parse_*` helper.

To change syntax, edit only the functions you care about, for example:

- `parse_struct` for struct declarations
- `parse_function` for function declarations/signatures
- `parse_enum` for enum declarations
- `parse_statement` if you add/remove statement forms
- `parse_expression` if you change operator grammar

Lexer, semantic analysis, and backends stay unchanged.
