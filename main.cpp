/*
nasm -felf64 test.asm && ld test.o -o test && ./test
rm -fr build
cmake -S . -B build/
cmake --build build/
build/zune test.zu:
	nasm -felf64 test.asm -o test.o
	gcc -lc -z execstack out.o -o out // OR
	ld out.o -e main -lc -o out
./out
echo $?
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "util.h"
#include "tokenizer.h"
#include "parser.h"
#include "assemble.h"

// TODO: Implement loops
int main(int argc, char* argv[]) {
	bool print_tokens = false, print_tree = false, print_asm = false;
	string input_filename, asm_filename = "./out.asm", bin_filename;
	
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <input_file.zu>" << endl
			<< "Full params: " << argv[0] << " <input_file.zu> -t -p -a -asm=asm_file.asm -bin=exe_file" << endl
			<< "  -t: print token list on screen" << endl
			<< "  -p: print parse tree on screen" << endl
			<< "  -a: print generated assembly on screen" << endl
			<< "  -asm=asm_file.asm: specify output assembly file name" << endl
			<< "  -bin=exe_file: compile to specified binary after assembly" << endl
			<< "Defaults:" << endl
			<< "  assembly: " << asm_filename << endl
			<< "  binary:   (not compiled if -bin is not specified)" << endl
			<< "Pass all flags separately, i.e. `-t -p -a` and not `-tpa`" << endl;
		exit(-1);
	}
	else
		input_filename = argv[1];

	for (int i=2; i<argc; i++) {
		string arg(argv[i]);
		if ("-t"s == arg)
			print_tokens = true;
		else if ("-p"s == arg)
			print_tree = true;
		else if ("-a"s == arg)
			print_asm = true;
		else if (arg.length() > 5 && arg.rfind("-asm="s, 0) == 0)
			asm_filename = arg.substr(5);
		else if (arg.length() > 5 && arg.rfind("-bin="s, 0) == 0)
			bin_filename = arg.substr(5);
	}

	try {
		string contents;
		{
			fstream input(input_filename, ios::in);
			stringstream contents_stream;
			contents_stream << input.rdbuf();
			contents = contents_stream.str();
		}
		
		//cout << "Input text:" << endl << contents << endl << endl;
		
		// 1) lexer
		Tokenizer tk{contents};
		vector<Token> tokens = tk.tokenize();
		if (print_tokens) {
			cout << "Tokens: " << tokens.size() << endl;
			int i = 0;
			for (Token t : tokens)
				cout << "  " << i++ << ": " << t.str() << endl;
			cout << endl;
		}

		// 2) parsing
		Parser parser{tokens};
		ProgNode parse_root = parser.parse();
		if (print_tree)
			cout << "Parse Tree: " << endl << parse_root.detailed() << endl << endl;

		// 3) interpretation
		// create assembly file
		Assembler assembler{parse_root};
		string asm_str = assembler.assemble();
		{
			fstream file(asm_filename, ios::out);
			file << asm_str;
		}
		cout << "Assembly file written to " << asm_filename;
		if (print_asm)
			cout << ":\n" << asm_str;
		cout << endl;
		
		if (bin_filename.length() > 0) {
			string obj_file = s_replace(asm_filename, ".asm"s, ".o"s);
			string nasm_cmd = "nasm -felf64 "s + asm_filename + " -o "s + obj_file;
			string link_cmd = "gcc -lc -z execstack "s + obj_file + " -o "s + bin_filename;
			//string link_cmd = "ld " + obj_file + " -e main -lc -o "s + bin_filename;
			// run nasm on it
			cout << "Running nasm on " << asm_filename << ":\n" << nasm_cmd << endl;
			system(nasm_cmd.c_str());
			cout << "Running link on " << obj_file << ":\n" << link_cmd  << endl;
			system(link_cmd.c_str());
			
			// done
			cout << "Input file '" << input_filename << "' compiled to executable '"
				<< bin_filename << "'." << endl;
		}
	}
	catch (exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
	
	exit(EXIT_SUCCESS);
}
