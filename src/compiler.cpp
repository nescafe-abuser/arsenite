#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include "lexer.h"
#include "parser.hpp"
#include "codegen.hpp"

int main(int argc, char** argv) {
    std::string input_file = "";
    std::string output_file = "a.out";
    bool nocc = false;
    bool emit_ir = false;
    bool output_specified = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
                output_specified = true;
            } else {
                std::cerr << "Error: -o requires a filename." << std::endl;
                return 1;
            }
        } else if (arg == "--nocc") {
            nocc = true;
        } else if (arg == "--ir") {
            emit_ir = true;
        } else {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Usage: " << argv[0] << " <input.at> [-o <out>] [--nocc] [--ir]" << std::endl;
        return 1;
    }

    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << input_file << std::endl;
        return 1;
    }

    std::ostringstream input_ss;
    input_ss << file.rdbuf();

    Lexer l = lexer_lex_file(input_ss.str());
    FunctionDefinition f{};

    if (!parse_function_definition(l, f)) {
        std::cerr << "Error: Parsing failed." << std::endl;
        return 1;
    }

    std::ostringstream ir_ss;
    emit_c_ir(ir_ss, f);
    std::string generated_c = ir_ss.str();

    if (nocc) {
        if (output_specified) {
            std::ofstream out_file(output_file);
            out_file << generated_c;
        } else {
            std::cout << generated_c;
        }
    } else {
        std::string cmd = "clang -x c ";
        
        if (emit_ir) {
            cmd += "-S -emit-llvm ";
            if (!output_specified && output_file == "a.out") {
                output_file = "output.ll";
            }
        }

        cmd += "-o " + output_file + " -";
        
        FILE* clang_pipe = popen(cmd.c_str(), "w");
        if (!clang_pipe) {
            std::cerr << "Error: Failed to invoke clang." << std::endl;
            return 1;
        }

        fwrite(generated_c.c_str(), 1, generated_c.size(), clang_pipe);
        
        int status = pclose(clang_pipe);
        if (status != 0) {
            std::cerr << "Error: clang compilation failed." << std::endl;
            return status;
        }
        
        std::cout << "Successfully generated " << (emit_ir ? "LLVM IR" : "binary") 
                  << " in " << output_file << std::endl;
    }

    return 0;
}
