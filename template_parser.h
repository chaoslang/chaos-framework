#import "./src/chaos_parser.h"


inline Chaos_AST *parse_primary(Chaos_Parser *p);
inline Chaos_AST *parse_postfix(Chaos_Parser *p, Chaos_AST *left);
inline Chaos_AST *parse_factor(Chaos_Parser *p);
inline Chaos_AST *parse_multiplicative(Chaos_Parser *p);
inline Chaos_AST *parse_additive(Chaos_Parser *p);
inline Chaos_AST *parse_comparison(Chaos_Parser *p);
inline Chaos_AST *parse_assignment(Chaos_Parser *p);
inline Chaos_AST *parse_expression(Chaos_Parser *p);
inline Chaos_AST *parse_var_decl(Chaos_Parser *p);
inline Chaos_AST *parse_block(Chaos_Parser *p);
inline Chaos_AST *parse_if(Chaos_Parser *p);
inline Chaos_AST *parse_return(Chaos_Parser *p);
inline Chaos_AST *parse_while(Chaos_Parser *p);
inline Chaos_AST *parse_struct(Chaos_Parser *p);
inline Chaos_AST *parse_function(Chaos_Parser *p);
inline Chaos_AST *parse_import(Chaos_Parser *p);
inline Chaos_AST *parse_mod_decl(Chaos_Parser *p);
inline Chaos_AST *parse_enum(Chaos_Parser *p);
inline Chaos_AST *parse_statement(Chaos_Parser *p);
inline Chaos_AST *parse_program(Chaos_Parser *p);

#import "./src/chaos_default_parser.h"

/*
|==================================================|
| Only Edit the Functions under this!!!!           |
|==================================================|
*/

inline Chaos_AST *parse_primary(Chaos_Parser *p) {
  return chaos_parse_primary(p);
}

inline Chaos_AST *parse_postfix(Chaos_Parser *p, Chaos_AST *left) {
  return chaos_parse_postfix(p, left);
}

inline Chaos_AST *parse_factor(Chaos_Parser *p) {
  return chaos_parse_factor(p);
}

inline Chaos_AST *parse_multiplicative(Chaos_Parser *p) {
  return chaos_parse_multiplicative(p);
}

inline Chaos_AST *parse_additive(Chaos_Parser *p) {
  return chaos_parse_additive(p);
}

inline Chaos_AST *parse_comparison(Chaos_Parser *p) {
  return chaos_parse_comparison(p);
}

inline Chaos_AST *parse_assignment(Chaos_Parser *p) {
  return chaos_parse_assignment(p);
}

inline Chaos_AST *parse_expression(Chaos_Parser *p) {
  return chaos_parse_expression(p);
}

inline Chaos_AST *parse_var_decl(Chaos_Parser *p) {
  return chaos_parse_var_decl(p);
}

inline Chaos_AST *parse_block(Chaos_Parser *p) { return chaos_parse_block(p); }

inline Chaos_AST *parse_if(Chaos_Parser *p) { return chaos_parse_if(p); }

inline Chaos_AST *parse_return(Chaos_Parser *p) {
  return chaos_parse_return(p);
}

inline Chaos_AST *parse_while(Chaos_Parser *p) { return chaos_parse_while(p); }

inline Chaos_AST *parse_struct(Chaos_Parser *p) {
  return chaos_parse_struct(p);
}

inline Chaos_AST *parse_function(Chaos_Parser *p) {
  return chaos_parse_function(p);
}

inline Chaos_AST *parse_import(Chaos_Parser *p) {
  return chaos_parse_import(p);
}

inline Chaos_AST *parse_mod_decl(Chaos_Parser *p) {
  return chaos_parse_mod_decl(p);
}

inline Chaos_AST *parse_enum(Chaos_Parser *p) { return chaos_parse_enum(p); }

inline Chaos_AST *parse_statement(Chaos_Parser *p) {
  return chaos_parse_statement(p);
}

inline Chaos_AST *parse_program(Chaos_Parser *p) {
  return chaos_parse_program(p);
}
