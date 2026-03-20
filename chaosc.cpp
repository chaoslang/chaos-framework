#include <cstdio>
#include <cstdlib>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <vector>

#define CHAOS_LEXER_IMPLEMENTATION
#include "./src/chaos_backend_c.h"
#include "./src/chaos_backend_js.h"
#import "template_parser.h"

std::string read_file(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    throw std::runtime_error("open failed");

  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  file.seekg(0);

  std::string buffer(size, '\0');
  file.read(buffer.data(), size);
  return buffer;
}

static bool write_file(const std::string &path, const std::string &contents) {
  std::ofstream out(path, std::ios::binary);
  if (!out)
    return false;
  out << contents;
  return out.good();
}

static std::string trim(std::string s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos)
    return "";
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

static std::vector<std::string> parse_imports(const std::string &source) {
  std::vector<std::string> imports;
  std::istringstream in(source);
  std::string line;

  while (std::getline(in, line)) {
    std::string t = trim(line);
    if (t.rfind("import ", 0) != 0)
      continue;

    t = trim(t.substr(7));
    if (!t.empty() && t.back() == ';')
      t.pop_back();
    t = trim(t);

    if (t.size() >= 2 && t.front() == '"' && t.back() == '"')
      t = t.substr(1, t.size() - 2);

    if (!t.empty())
      imports.push_back(t);
  }

  return imports;
}

static bool
resolve_import_order_recursive(const std::filesystem::path &path,
                               std::unordered_set<std::string> &seen,
                               std::vector<std::filesystem::path> &ordered) {
  std::error_code ec;
  auto canonical = std::filesystem::weakly_canonical(path, ec);
  if (ec) {
    std::cerr << "failed to resolve path: " << path << "\n";
    return false;
  }

  std::string key = canonical.string();
  if (seen.find(key) != seen.end())
    return true;

  std::string source;
  try {
    source = read_file(key);
  } catch (const std::exception &) {
    std::cerr << "failed to read module: " << key << "\n";
    return false;
  }

  for (const auto &module : parse_imports(source)) {
    std::filesystem::path module_path = module;
    if (module_path.extension() != ".ch")
      module_path += ".ch";

    std::filesystem::path child = canonical.parent_path() / module_path;
    if (!resolve_import_order_recursive(child, seen, ordered))
      return false;
  }

  seen.insert(key);
  ordered.push_back(canonical);
  return true;
}

static bool resolve_import_order(const std::string &entry_path,
                                 std::vector<std::filesystem::path> &ordered) {
  std::unordered_set<std::string> seen;
  return resolve_import_order_recursive(entry_path, seen, ordered);
}

static bool c_to_exe(const std::string &c_source, const std::string &exe_path) {
  char temp_path[] = "/tmp/chaoscXXXXXX";
  int fd = mkstemp(temp_path);
  if (fd == -1) {
    std::perror("mkstemp");
    return false;
  }
  close(fd);

  std::string base_path = temp_path;
  std::string c_path = base_path + ".c";

  try {
    std::filesystem::rename(base_path, c_path);
  } catch (const std::exception &e) {
    std::cerr << "rename temp file failed: " << e.what() << "\n";
    std::filesystem::remove(base_path);
    return false;
  }

  if (!write_file(c_path, c_source)) {
    std::cerr << "failed to write temp C file\n";
    std::filesystem::remove(c_path);
    return false;
  }

  char exe_buf[4096];
  size_t exe_len = readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
  std::string src_dir;
  if (exe_len > 0) {
    exe_buf[exe_len] = '\0';
    src_dir = std::filesystem::path(exe_buf).parent_path().string() + "/src";
  } else {
    src_dir = "./src";
  }
  std::string cmd = "gcc \"" + c_path + "\" -o \"" + exe_path +
                    "\""
                    " -I\"" +
                    src_dir +
                    "\""
                    " \"" +
                    src_dir +
                    "/libffi.a\""
                    " -ldl";
  int rc = std::system(cmd.c_str());

  std::filesystem::remove(c_path);
  return rc == 0;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::fprintf(stderr,
                 "Usage: %s <source.ch> <output> <backend>\n"
                 "Backends: c, js, exe\n",
                 argv[0]);
    return 1;
  }

  std::vector<std::filesystem::path> files;
  if (!resolve_import_order(argv[1], files))
    return 1;

  Chaos_AST *program = new Chaos_AST();
  program->kind = AST_PROGRAM;

  std::deque<std::string> source_store;
  std::deque<Chaos_Lexer> lexers;

  for (const auto &file : files) {
    source_store.push_back(read_file(file.string()));

    lexers.push_back(Chaos_Lexer{0});
    chaos_lexer_run(&lexers.back(), source_store.back());

    Chaos_Parser parser(&lexers.back().tokens);
    Chaos_AST *ast = parse_program(&parser);

    if (!ast) {
      std::fprintf(stderr, "Failed to parse\n");
      return 1;
    }

    for (Chaos_AST *stmt : ast->block.statements)
      program->block.statements.push_back(stmt);
  }

  IR_Program ir = lower_program(program);
  std::string backend = argv[3];
  std::string out;

  if (backend == "js") {
    JavaScriptBackend js_backend;
    js_backend.codegen(ir);
    out = js_backend.get_output() + "\n";

    if (!write_file(argv[2], out)) {
      std::cerr << "failed to write output file\n";
      return 1;
    }
    return 0;
  }

  if (backend == "c") {
    CBackend c_backend;
    c_backend.codegen(ir);
    out = c_backend.get_output() + "\n";

    if (!write_file(argv[2], out)) {
      std::cerr << "failed to write output file\n";
      return 1;
    }
    return 0;
  }

  if (backend == "exe") {
    CBackend c_backend;
    c_backend.codegen(ir);
    out = c_backend.get_output() + "\n";

    if (!c_to_exe(out, argv[2])) {
      std::cerr << "failed to build executable\n";
      return 1;
    }
    return 0;
  }

  std::fprintf(stderr, "Unknown backend: %s\n", argv[3]);
  return 1;
}
