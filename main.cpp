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
#include <stack>
#include <string>
#include <cctype>
using namespace std;

vector<pair<regex, string>> patterns = {
	{regex(R"(^(var|input|output))"), "Keyword"},
	{regex(R"([0-9][0-9]*)"), "Integer"},
	{regex(R"(^[_a-zA-Z][_a-zA-Z0-9]*)"), "Identifier"},
	{regex(R"(^//[^\n]*)"), "Comment"},
	// {regex(R"(^[{}\[\](),.:])"), "Punctuation"},
	{regex(R"(;)"), "Terminator"},
	{regex(R"([-+*/()])"), "Operator"},
	{regex(R"(=)"), "Assignment"},
	{regex(R"(^\s+)"), "Whitespace"},
};

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

vector<vector<pair<string, string>>> token_lines;
vector<pair<string, string>> line;
unordered_map<string, int> variable_table;

bool isInputValid(string input);
bool varExists(string var_name);

bool isOperator(char c);
int precedence(char op);
double applyOperation(double a, double b, char op);
double evaluateExpression(const string& expression);

int main() {
	ifstream codeFile("main.txt");
	stringstream buffer;
	buffer << codeFile.rdbuf();
	string myCode = buffer.str();

	cout << "==========================" << " Source Code:" << endl;
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
	cout << "==========================" << " Tokenizing.." << endl;
	for (const auto &line : token_lines) {
		cout << "Line " << l_count << ": " << endl;
		for (const auto &pair : line) {
			cout << pair.first << " - " << pair.second << endl;
		}
		cout << endl;
		l_count++;
	}

	l_count = 1;
	cout << "==========================" << " Parsing.." << endl;
	for (const auto &line : token_lines) {
		for (const auto &pair : line) {
		}
		if (line[0].first == "var") {
			if (line[1].second == "Identifier" && line[2].second == "Terminator") { // initialization
				variable_table.insert({line[1].first, NULL});
			}
			else if (line[1].second == "Identifier" && line[2].second == "Assignment" && line[3].second == "Integer" && line[4].second == "Terminator") { // initialization with assignment
				variable_table.insert({line[1].first, stoi(line[3].first)});
			}
			else if (line[1].second == "Identifier" && line[2].second == "Assignment" && line[4].second != "Terminator"){ // initialization with expression assignment
				string expression;
				for (size_t i = 3; i < line.size() - 1; i++) {
					if (line[i].second == "Integer" || line[i].second == "Operator") {
						expression += line[i].first + " ";
					}
					else if (line[i].second == "Identifier" && varExists(line[i].first)){
						expression += to_string(variable_table[line[i].first]) + " ";
					}
					else if (line[i].second == "Identifier" && !varExists(line[i].first)){
						cout << "Identifier " << line[i].first << " was not declared first." << endl;
						break;
					}
					else {
						cout << "Invalid use of " << line[i].first << " at Line " << l_count << endl;
					}
				}
				cout << "Expression: " << expression << endl;
				variable_table.insert({line[1].first, evaluateExpression(expression)});
			}
			else if (line[-2].second == "Operator" && line[-1].second == "Terminator") {
				cout << "Missing identifier or integer at " << line[-2].second << endl;
			}
			else {
				cout << "Out of Scope var use" << endl;
			}
		} 
		else if (line[0].first == "input") { // input from console
			if (line[1].second == "Identifier" && line[2].second == "Terminator") {
				if (varExists(line[1].first)){
					string input;
					cout << line[1].first << ": ";
					cin >> input;

					if (isInputValid) variable_table[line[1].first] = stoi(input);
					else cout << "Invalid input operation" << endl;
				}
				else cout << "Identifier " << line[1].first << " was not declared first." << endl;				
			}
			else {
				cout << "Out of Scope input use" << endl;
			}
		} 
		else if (line[0].first == "output") {
			if (line[1].second == "Identifier" && line[2].second == "Terminator") {
				if (varExists(line[1].first)) cout << line[1].first << ": " << variable_table[line[1].first] << endl;
				else cout << "Undefined variable: " << line[1].first << endl;
			}
			else {
				cout << "Out of Scope output use" << endl;
			}
		} 
		else {
			cout << "Invalid use of " << line[0].first << " at Line " << l_count << endl;
			break;
		}
		l_count++;
	}

	// string abc = "1 + 2 * ( 10 / 5 )";
	// double result = evaluateExpression(abc);
	// cout << "Result: " << result << endl;
	// variable_table.insert({"abc", result});

	cout << "==========================" << " Variable Table: " << endl;
	for (const auto &var : variable_table) {
	cout << var.first << " - " << var.second << endl;
	}

	return 0;
}

//TODO
bool isInputValid(string input) {
	return true;
}

bool varExists(string var_name){
	if (variable_table.find(var_name) != variable_table.end()) return true;
	else return false;
}

bool isOperator(char c){
	return c =='+' || c =='-' || c =='*' || c =='/' || c =='^';
}
int precedence(char op){
	if (op =='+' || op =='-') return 1;
	if (op =='*' || op == '/') return 2;
	if (op == '^') return 3;
	return 0;
}
double applyOperation(double a, double b, char op){
	switch(op){
		case '+': return a + b;
		case '-': return a - b;
		case '*': return a * b;
		case '/': return a / b;
		case '^': return pow(a,b);
		default: return 0;
	}
}
double evaluateExpression(const string& expression){
	stack<char> operators;
	stack<double> operands;
	stringstream ss(expression);

	string token;
	while (getline(ss, token, ' ')){
		if (token.empty()) continue;
		if (isdigit(token[0])){
			double num;
			stringstream(token) >> num;
			operands.push(num);
		}
		else if(isOperator(token[0])){
			char op = token[0];
			while (!operators.empty() && precedence(operators.top()) >= precedence(op)){
				double b = operands.top();
				operands.pop();
				double a = operands.top();
				operands.pop();
				char op2 = operators.top();
				operators.pop();
				operands.push(applyOperation(a, b, op));
			}
			operators.push(op);
		}
		else if(token[0] == '(') {
			operators.push('(');
		}else if(token[0] == ')') {
			while (!operators.empty() && operators.top() != '('){
				double b = operands.top();
				operands.pop();
				double a = operands.top();
				operands.pop();
				char op = operators.top();
				operators.pop();
				operands.push(applyOperation(a, b, op));
			}
			operators.pop();
		}
	}
	while (!operators.empty()){
		double b = operands.top();
		operands.pop();
		double a = operands.top();
		operands.pop();
		char op = operators.top();
		operators.pop();
		operands.push(applyOperation(a, b, op));
	}
	return operands.top();
}