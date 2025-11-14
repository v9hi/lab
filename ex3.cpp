#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <map>
#include <vector>
#include <iomanip>
#include <set>

using namespace std;

struct Symbol
{
    string lexeme;
    string tokenType;
    int lineDeclared;
    set<int> lineUsed;
};

regex identifier("^[a-zA-Z_][a-zA-Z0-9_]*$");
regex integerRegex("^[0-9]+$");
regex floatRegex("^[0-9]*\\.[0-9]+$");
regex literalRegex("^\".*\"$");

vector<string> keywords = {
    "int", "float", "char", "string", "return", "void", "if", "else", "while", "for"};

map<string, Symbol> symbolTable;

vector<string> splitTokens(const string &line)
{
    vector<string> tokens;
    string token;
    bool inLiteral = false;
    string literal;

    for (size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];

        if (inLiteral)
        {
            literal += c;
            if (c == '"')
            {
                tokens.push_back(literal);
                inLiteral = false;
                literal.clear();
            }
        }
        else if (c == '"')
        {
            inLiteral = true;
            literal = "\"";
        }
        else if (isspace(c))
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else if (ispunct(c) && c != '_' && c != '.')
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
            string punct(1, c);
            tokens.push_back(punct);
        }
        else
        {
            token += c;
        }
    }

    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

bool isValidIdentifier(const string &token)
{
    if (token.empty())
        return false;
    if (!isalpha(token[0]) && token[0] != '_')
        return false;
    for (size_t i = 1; i < token.length(); ++i)
    {
        if (!isalnum(token[i]) && token[i] != '_')
            return false;
    }
    return true;
}

bool isKeyword(const string &token)
{
    return find(keywords.begin(), keywords.end(), token) != keywords.end();
}

void addToSymbolTable(const string &lexeme, const string &type, int line)
{
    if (symbolTable.find(lexeme) == symbolTable.end())
    {
        symbolTable[lexeme] = {lexeme, type, line, {line}};
    }
    else
    {
        symbolTable[lexeme].lineUsed.insert(line);
    }
}

string removeSingleLineComments(const string &line)
{
    size_t pos = line.find("//");
    if (pos != string::npos)
        return line.substr(0, pos);
    return line;
}

void displaySymbolTable()
{
    const string header1 = "Entry No.";
    const string header2 = "Lexeme (Name/Value)";
    const string header3 = "Token Type";
    const string header4 = "Line No. Declared";
    const string header5 = "Line No. Used";

    int w1 = header1.length(), w2 = header2.length(), w3 = header3.length();
    int w4 = header4.length(), w5 = header5.length();

    vector<Symbol> entries;
    for (auto &entry : symbolTable)
    {
        entries.push_back(entry.second);
        int usedWidth = 0;
        for (int l : entry.second.lineUsed)
            usedWidth += to_string(l).length() + 1;

        w1 = max(w1, 2);
        w2 = max(w2, (int)entry.second.lexeme.length());
        w3 = max(w3, (int)entry.second.tokenType.length());
        w4 = max(w4, (int)to_string(entry.second.lineDeclared).length());
        w5 = max(w5, usedWidth);
    }

    cout << left << setw(w1 + 4) << header1
         << setw(w2 + 4) << header2
         << setw(w3 + 4) << header3
         << setw(w4 + 6) << header4
         << setw(w5 + 6) << header5 << "\n";

    cout << string(w1 + w2 + w3 + w4 + w5 + 24, '-') << "\n";

    int i = 1;
    for (auto &sym : entries)
    {
        cout << left << setw(w1 + 4) << i++
             << setw(w2 + 4) << sym.lexeme
             << setw(w3 + 4) << sym.tokenType
             << setw(w4 + 6) << sym.lineDeclared;

        stringstream ss;
        for (int l : sym.lineUsed)
            ss << l << " ";
        cout << setw(w5 + 6) << ss.str() << "\n";
    }
}

int main()
{
    ifstream file("test.cpp");
    if (!file.is_open())
    {
        cerr << "Failed to open file.\n";
        return 1;
    }

    string line;
    int lineNo = 0;
    bool inMultilineComment = false;

    while (getline(file, line))
    {
        ++lineNo;

        if (inMultilineComment)
        {
            if (line.find("*/") != string::npos)
                inMultilineComment = false;
            continue;
        }

        if (line.find("/*") != string::npos)
        {
            inMultilineComment = true;
            continue;
        }

        line = removeSingleLineComments(line);
        vector<string> tokens = splitTokens(line);

        for (string token : tokens)
        {
            if (token.empty())
                continue;

            if (isKeyword(token))
            {
                addToSymbolTable(token, "Keyword", lineNo);
                continue;
            }
            else if (isValidIdentifier(token))
            {
                addToSymbolTable(token, "Identifier", lineNo);
            }
            else if (regex_match(token, identifier))
            {
                cerr << "Lexical Error at line " << lineNo << ": Invalid identifier name '" << token << "'\n";
            }
            else if (regex_match(token, integerRegex))
            {
                addToSymbolTable(token, "Integer", lineNo);
            }
            else if (regex_match(token, floatRegex))
            {
                addToSymbolTable(token, "Float", lineNo);
            }
            else if (regex_match(token, literalRegex))
            {
                addToSymbolTable(token, "Literal", lineNo);
            }
            else if (token == "=" || token == "+" || token == "-" || token == "*" || token == "/" ||
                     token == "{" || token == "}" || token == "(" || token == ")" ||
                     token == ";" || token == "," || token == ">" || token == "<" ||
                     token == "'" || token == ":" || token == "#")
            {
                addToSymbolTable(token, "Operator/Symbol", lineNo);
            }
            else
            {
                cerr << "Lexical Error at line " << lineNo << ": Unrecognized token '" << token << "'\n";
            }
        }
    }

    displaySymbolTable();
    return 0;
}
