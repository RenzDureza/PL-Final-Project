#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>
using namespace std;

vector<pair<regex, string>> patterns = {
	{regex(R"(^\b(var|input|output)\b)"), "Keyword"},
	{regex(R"(^[0-9][0-9]*)"), "Integer"},
	{regex(R"(^[_a-zA-Z][_a-zA-Z0-9]*)"), "Identifier"},
	{regex(R"(^//[^\n]*)"), "Comment"},
	{regex(R"(^[{}\[\](),.:])"), "Punctuation"},
	{regex(R"(^[;])"), "Terminator"},
	{regex(R"(^[-+*/=]+)"), "Operator"},
	{regex(R"(^\s+)"), "Whitespace"}};

regex r_initialization("var\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*=\\s*[0-9]");
// regex
// r_assignment("[a-zA-Z_][a-zA-Z0-9_]*\\s+=\\s+([a-zA-Z][a-zA-Z0-9_]*|\\-*""\\d+|\\-*[0-9]+.[0-9]+)");

// struct token_line {
// 	string token_string;
// 	string token_identifier;
// };
//
// token_line line1
// vector<token_line> lines = {}
//
// lines = { line{
// 	{"var", "Keyword"},
// 	{"x" , "Identifier"},
// 	{"=", "Operator"},
// 	{"3", "Integer"}},
// 	{{}
//
// 	}
// }

int main() {
	vector<vector<pair<string, string>>> token_lines;
	vector<pair<string, string>> line;
	ifstream codeFile("main.txt");
	stringstream buffer;
	buffer << codeFile.rdbuf();
	string myCode = buffer.str();

	cout << myCode << endl;

	size_t pos = 0;
	while (pos < myCode.size()) {
		bool matched = false;
		string remaining = myCode.substr(pos);
		for (auto &token : patterns) {
			smatch match;
			if (regex_search(remaining, match, token.first)) {
				if (match.position() == 0) {
					if (token.second != "Whitespace") {
						line.push_back({match.str(), token.second});
						if (match.str() == ";") {
							token_lines.push_back(line);
							line.clear();
						}
					}
					pos += match.length();
					matched = true;
					break;
				}
			}
		}
		if (!matched) {
			cerr << "Unknown token starting at: " << myCode.substr(pos, 10)
				 << endl;
			break;
		}
	}

	size_t l_count = 1;
	for (const auto &line : token_lines) {
		cout << "Line " << l_count << ": " << endl;
		for (const auto &pair : line) {
			cout << pair.first << " - " << pair.second << endl;
			if (line[0].first == "var") {
				if (line[1].second == "Identifier" && line[2].second == "Terminator") { // initialization

				}
			} else if (line[0].first == "input") {
			
			} else if (line[0].first == "output") {
			
			} else {
				cout << "Out of Scope" << endl;
			}
		}
		l_count++;
	}
	return 0;
}
