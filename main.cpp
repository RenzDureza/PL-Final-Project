#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>
#include <unordered_map>
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

bool isInputValid(string input);

int main() {
	vector<vector<pair<string, string>>> token_lines;
	vector<pair<string, string>> line;
	unordered_map<string, int> variable_table;
	ifstream codeFile("main.txt");
	stringstream buffer;
	buffer << codeFile.rdbuf();
	string myCode = buffer.str();

	cout << "Source Code:" << endl;
	cout << "==========================" << endl;
	cout << myCode << endl;
	cout << "==========================" << endl;

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
		}
		if (line[0].first == "var") {
			if (line[1].second == "Identifier" && line[2].second == "Terminator") { // initialization
				variable_table.insert({line[1].first, NULL});
			}
			else if (line[1].second == "Identifier" && line[2].second == "Operator" && line[3].second == "Integer" && line[4].second == "Terminator") { // initialization with assignment
				variable_table.insert({line[1].first, stoi(line[3].first)});
			}
			else {
				cout << "Out of Scope var use" << endl;
			}
		} else if (line[0].first == "input") { // input from console
			if (line[1].second == "Identifier" && line[2].second == "Terminator") {
				string input;
				cout << line[1].first << ": ";
				cin >> input;

				if (isInputValid) variable_table.insert({line[1].first, stoi(input)});
				else cout << "Invalid input operation" << endl;
			}
			else {
				cout << "Out of Scope input use" << endl;
			}
		} else if (line[0].first == "output") {
			if (line[1].second == "Identifier" && line[2].second == "Terminator" && (variable_table.find(line[1].first) != variable_table.end())) {
				cout << line[1].first << ": " << variable_table[line[1].first] << endl;
			}
			else if ((variable_table.find(line[1].first) == variable_table.end())){
				cout << "Undefined variable: " << line[1].first << endl;
			}
			else {
				cout << "Out of Scope output use" << endl;
			}
		} else {
			cout << "Incorrect Keyword at " << line[0].first << endl;
			break;
		}
		l_count++;
		cout << "==========================" << endl;
	}

	cout << "Variable Table: " << endl;
	for (const auto &var : variable_table) {
	cout << var.first << " - " << var.second << endl;
	}

	return 0;
}

//TODO
bool isInputValid(string input) {
	return true;
}