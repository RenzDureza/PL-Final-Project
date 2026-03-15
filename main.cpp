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
#include <iomanip>
#include <queue>
using namespace std;

//Regex for Keywords, Integers, Identifiers, Comments
static const vector<pair<regex, string>> patterns = {
	{regex(R"(^\s+\n)"), "Empty Line"},
	{regex(R"(^\s+)"), "Whitespace"},
	{regex(R"(^(var|input|output)\b)"), "Keyword"},
	{regex(R"(^[0-9]+)"), "Integer"},
	{regex(R"(^[_a-zA-Z][_a-zA-Z0-9]*)"), "Identifier"},
	{regex(R"(^//[^\n]*)"), "Comment"},
	// {regex(R"(^[{}\[\](),.:])"), "Punctuation"},
	// {regex(R"(^;)"), "Terminator"},
	// {regex(R"(^[-+*/()])"), "Operator"},
	// {regex(R"(^=)"), "Assignment"},
};

//Data Structures for saving Tokens, Token Lines, and Variables
vector<vector<pair<string, string>>> token_lines;
vector<pair<string, string>> line;
unordered_map<string, int> variable_table;
queue<string> input_queue;
queue<string> output_queue;
queue<pair<string,string>> expression_queue;

//Boolean for when error is found within lexical or syntactic analysis
bool errorFound = false;

//Read Source code from file
string readSourceCode(const string& source_code_filename);

//Display Source Code
void displaySourceCode(const string& source_code);

//Lexing
void tokenizeCode(const string& myCode);
void displayTokens();

//Parsing
bool isInputValid(string input);
bool varExists(string var_name);
int inputExpression(string input);
void parseTokens();

//Execution
void executeCode();

//Operations on Expressions
bool isOperator(char c);
int precedence(char op);
double applyOperation(double a, double b, char op);
double evaluateExpression(const string& expression);

//Display Variables
void displayVariables();

int main() {

	string myCode = readSourceCode("main.txt");

	//Display Source Code
	displaySourceCode(myCode);

	//Lexical Analysis
	tokenizeCode(myCode);

	//Display Tokens
	displayTokens();

	//Parsing
	parseTokens();

	//If no error found
	if (!errorFound) {
		//Execute Code
		executeCode();

		//Display Saved Variables
		displayVariables();
	}
	

	return 0;
}

string readSourceCode(const string& source_code_filename){
	ifstream codeFile(source_code_filename);
	stringstream buffer;
	buffer << codeFile.rdbuf();
	string myCode = buffer.str();
	return myCode;
}

void displaySourceCode(const string& source_code){
	cout << "==========================" << " Source Code:" << endl;
	cout << source_code << endl;
	return;
}

//Lexical Analysis
void tokenizeCode(const string& myCode){
	size_t pos = 0;
	while (pos < myCode.size()) {
		bool matched = false;

		char c = myCode[pos];
		// cout << "char: " << c << " size: " << myCode.size() << " pos: " << pos << endl;
		if (c == '/' && pos+1 < myCode.size() && myCode[pos+1] == '/'){
			size_t start = pos;
			pos += 2;
			while (pos < myCode.size() && myCode[pos+1] != '\n') pos++;
			pos++;
			line.push_back({myCode.substr(start, pos-start), "Comment"});
			token_lines.push_back(line);
			line.clear();
			continue;
		}
		if (c == '\n'){
			if(!line.empty()){
				token_lines.push_back(line);
				line.clear();
			}
			else {
				token_lines.push_back({{"", "Empty Line"}});
			}
			pos++;
			continue;
		}
		switch (c){
			case '+': case '-': case '*': case '/': case '(': case ')':
			line.push_back({string(1, c), "Operator"});
			pos++;
			continue;
			
			case ';':
			line.push_back({";", "Terminator"});
			pos++;
			continue;

			case '=':
			line.push_back({"=", "Assignment"});
			pos++;
			continue;
		}
		
		auto begin = myCode.cbegin() + pos;
		auto end = myCode.cend();
		for (const auto &token : patterns) {
			smatch match;
			if (regex_search(begin, end, match, token.first, regex_constants::match_continuous)) {
				if (match.position() == 0) {
					if (token.second == "Comment"){
						line.push_back({match.str(), token.second});
						token_lines.push_back(line);
						line.clear();
					}
					else if (token.second != "Whitespace") {
						line.push_back({match.str(), token.second});
						if (match.str() == ";") {
							token_lines.push_back(line);
							line.clear();
						}
					}
					else if (token.second == "Empty Line"){
						line.push_back({match.str(), token.second});
						token_lines.push_back(line);
						line.clear();
					}
					pos += match.length();
					matched = true;
					break;
				}
			}
		}
		if (!matched) {
			errorFound = true;
			cerr << "Unknown token starting at: " << myCode.substr(pos, 10)
				 << endl;
			break;
		}
	}
}
void displayTokens(){
	size_t l_count = 1;
	cout << "==========================" << " Lexing.." << endl;
	for (const auto &line : token_lines) {
		if (line[0].second == "Empty Line"){
			l_count++;
			continue;
		}
		cout << "Line " << l_count << ": " << endl;
		for (const auto &pair : line) {
			cout << pair.first <<" - " << pair.second << endl;
		}
		cout << endl;
		l_count++;
	}
	return;
}

//Parsing
bool isInputValid(string input) {
	if (input.empty()) return false;
	//Check only valid characters
	regex valid_expression("^([a-zA-Z][a-zA-Z0-9_]*|\\d+|[+\\-*/()^]|\\s)*$");
	if (!regex_match(input, valid_expression)) return false;

	//Check balance of parentheses
	int parentheses_balance = 0;
	for (char c : input){
		if (c == '(') parentheses_balance++;
		else if (c == ')') parentheses_balance--;
	}
	if (parentheses_balance != 0) return false;
	
	//Check invalid consecutive of operators (++ +- /*)
	for (size_t i = 1; i < input.size(); i++){
		if (string("+-*/^").find(input[i]) != string::npos &&
			string("+-*/^").find(input[i-1]) != string::npos) return false;
	}
	return true;
}
bool varExists(string var_name){
	if (variable_table.find(var_name) != variable_table.end()) return true;
	else return false;
}
void parseTokens(){
	size_t l_count = 1;
	cout << "==========================" << " Parsing.." << endl;
	for (const auto &line : token_lines) {
		auto last = line.size() - 1;
		for (const auto &pair : line) {
		}
		/*
		var
		-- Initializes identifiers as variables
		-- Only accepts identifier names starting with alphabetic characters or underscore(_)
		-- Accepts pattern "var <identifier>;" as variable initialization
		-- Accepts pattern "var <identifier> = <intger;" as variable declaration
		-- Accepts mathematical expressions as declaration for variables
		-- Accepts other variables in declaration only if the variable that will be used was declared first

		Valid identifier names:
		var x;
		var person123;
		var _name;

		Valid var uses:
		var x = 32;
		var x = y + 3 - (5 * 3); (assuming y was declared and has a value)
		*/
		if (line[0].first == "var") {
			//Initialization
			if (line[1].second == "Identifier" && line[2].second == "Terminator") {
				variable_table.insert({line[1].first, NULL});
			}
			//Declaration
			else if (line[1].second == "Identifier" && line[2].second == "Assignment" && line[3].second == "Integer" && line[4].second == "Terminator") {
				variable_table.insert({line[1].first, stoi(line[3].first)});
			}
			//Declaration with expression assignment
			else if (line[1].second == "Identifier" && line[2].second == "Assignment" && line[4].second != "Terminator"){
				string expression;
				for (size_t i = 3; i < line.size() - 1; i++) {
					if (line[i].second == "Integer" || line[i].second == "Operator") {
						expression += line[i].first + " ";
					}
					else if (line[i].second == "Identifier" && varExists(line[i].first)){
						expression += line[i].first + " ";
					}
					else if (line[i].second == "Identifier" && !varExists(line[i].first)){
						cout << "Identifier " << line[i].first << " was not declared first." << endl;
						break;
					}
					else {
						cout << "Invalid use of " << line[i].first << " at Line " << l_count << endl;
					}
				}
				variable_table.insert({line[1].first, NULL});
				expression_queue.push({line[1].first, expression});
			}
			else if (line[last-1].second == "Operator" && line[last].second == "Terminator") {
				cout << "Missing identifier or integer " << line[last-1].second << "at line at Line " << l_count<< endl;
				errorFound = true;
				break;
			}
			else {
				cout << "Out of Scope var use at at Line " << l_count << endl;
				errorFound = true;
				break;
			}
		} 
		/*
		input
		-- Accepts input from the user through the console once code is successfuly parsed
		-- Only accepts if identifier was declared first with var
		-- Only accepts pattern "input <identifier>;" in source code
		-- Console input accepts identifiers, integers, and operations + - / * ^ only
		-- Console accepts parentheses ( ) but must be closed or balanced
		-- Handles expressions with or without spaces
		-- Accepts existing variables for expressions

		Example use:
		var z;
		input z;

		Valid console inputs:
		z: 5
		z: 10 + 5 - (1 + 3)
		z: y + 5 (assuming y exists and has a value prior input)
		*/
		else if (line[0].first == "input") {
			if (line[1].second == "Identifier" && line[2].second == "Terminator") {
				if (varExists(line[1].first)){
					variable_table.insert({line[1].first, NULL});
					input_queue.push(line[1].first);
				}
				else {
					cout << "Identifier " << line[1].first << " was not declared first at Line " << l_count << endl;
					errorFound = true;
					break;
				}		
			}
			else {
				cout << "Out of Scope input use at Line " << l_count << endl;
				errorFound = true;
				break;
			}
		}
		/*
		output
		-- Displays variable and value in console
		-- Only accepts if identifier was declared first with var
		-- Only accepts pattern "output <identifier>;" in source code

		Example use:
		var x = 10;
		output x;

		Console Output:
		x: 12
		*/
		else if (line[0].first == "output") {
			if (line[1].second == "Identifier" && line[2].second == "Terminator") {
				if (varExists(line[1].first)) output_queue.push(line[1].first);
				else {
					cout << "Undefined variable: " << line[1].first << " at Line " << l_count << endl;
					errorFound = true;
					break;
				}
			}
			else {
				cout << "Out of Scope output use" << endl;
				errorFound = true;
				break;
			}
		}
		else if (line[0].second == "Empty Line" || line[0].second == "Comment") {
			l_count++;
			continue;
		}
		else {
			cout << "Invalid use of " << line[0].first << " at Line " << l_count << endl;
			errorFound = true;
			break;
		}
		l_count++;
	}
	return;
}

//Execution of code after parsing
void executeCode(){
	cout << "==========================" << " Executing.." << endl;
	while (!input_queue.empty()){
		string input;
		cout << input_queue.front() << ": ";
		getline(cin, input);

		if (isInputValid(input)) {
			variable_table[input_queue.front()] = evaluateExpression(input);
		}
		else {
			cout << "Invalid expression." << endl;
			errorFound = true;
			return;
		}
		input_queue.pop();
	}
	while (!expression_queue.empty()){
		variable_table[expression_queue.front().first] = evaluateExpression(expression_queue.front().second);
		expression_queue.pop();
	}
	while (!output_queue.empty()){
		cout << output_queue.front() << ": " << variable_table[output_queue.front()] << endl;
		output_queue.pop();
	}
}

//Operations on Expressions
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
	size_t i = 0;
	while (i < expression.size()){
		char c = expression[i];
		if (isspace(c)) {
			i++;
			continue;
		}
		else if (isdigit(c)){
			string num_string;
			while (i < expression.size() && isdigit(expression[i])){
				num_string += expression[i++];
			}
			double num = stod(num_string);
			operands.push(num);
		}
		else if (isalpha(c)){
			string var;
			while (i < expression.size() && (isalnum(expression[i]) || expression[i] == '_')){
				var += expression[i++];
			}
			if (!varExists(var)){
				cout << "Missing identifier " << var << endl;
				errorFound = true;
				return 0;
			}
			operands.push(variable_table[var]);
		}
		else if(isOperator(c)){
			char op = c;
			while (!operators.empty() && precedence(operators.top()) >= precedence(op)){
				double b = operands.top();
				operands.pop();
				double a = operands.top();
				operands.pop();
				char op2 = operators.top();
				operators.pop();
				operands.push(applyOperation(a, b, op2));
			}
			operators.push(op);
			i++;
		}
		else if(c == '(') {
			operators.push('(');
			i++;
		}
		else if(c == ')') {
			char op = c;
			while (!operators.empty() && operators.top() != '('){
				double b = operands.top();
				operands.pop();
				double a = operands.top();
				operands.pop();
				char op2 = operators.top();
				operators.pop();
				operands.push(applyOperation(a, b, op2));
			}
			operators.pop();
			i++;
		}
		else {
			cout << "Invalid character " << c << endl;
			errorFound = true;
			return 0;
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

//Display Variables
void displayVariables(){
	cout << "==========================" << " Variable Table: " << endl;
	for (const auto &var : variable_table) {
	cout << var.first << " - " << var.second << endl;
	}
}