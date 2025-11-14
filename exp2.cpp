#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <regex>
using namespace std;

unordered_set<string> keywords = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
    "long", "namespace", "register", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while", "class", "bool", "private", "protected", "public",
    "this", "true", "false", "virtual", "include", "using", "new", "delete",
    "nullptr", "try", "catch", "throw", "template", "typename", "inline"};

unordered_set<string> operators = {
    "+", "-", "*", "/", "%", "++", "--", "==", "!=", ">=", "<=", ">", "<",
    "&&", "||", "!", "&", "|", "^", "~", "<<", ">>", "=", "+=", "-=", "*=", "/=",
    "%=", "&=", "|=", "^=", "<<=", ">>=", "->", "."};

unordered_set<char> specialSymbols = {
    '(', ')', '{', '}', '[', ']', ';', ':', ',', '#', '"', '\''};

bool isValidIdentifier(const string &token)
{
    if (token.empty() || isdigit(token[0]))
        return false;
    return regex_match(token, regex("[_a-zA-Z][_a-zA-Z0-9]*"));
}

bool isIntegerLiteral(const string &token)
{
    return regex_match(token, regex("[+-]?[0-9]+"));
}

bool isFloatingLiteral(const string &token)
{
    return regex_match(token, regex("[+-]?([0-9]*\\.[0-9]+|[0-9]+\\.[0-9]*)([eE][+-]?[0-9]+)?"));
}

bool isStringLiteral(const string &token)
{
    return regex_match(token, regex("^\"(\\\\.|[^\"\\\\])*\"$"));
}

void analyze(const string &code)
{
    istringstream stream(code);
    int errorCount = 0;

    string word;
    while (stream >> word)
    {
        size_t i = 0;
        while (i < word.size())
        {
            if (isspace(word[i]))
            {
                ++i;
                continue;
            }

            bool matched = false;

            if (word[i] == '"')
            {
                size_t start = i;
                size_t end = word.find('"', start + 1);
                while (end != string::npos && word[end - 1] == '\\')
                {
                    end = word.find('"', end + 1);
                }
                if (end != string::npos)
                {
                    string strLit = word.substr(start, end - start + 1);
                    if (isStringLiteral(strLit))
                    {
                        cout << "String Literal: " << strLit << endl;
                    }
                    i = end + 1;
                    continue;
                }
                else
                {
                    break;
                }
            }

            for (int len = 3; len >= 1; --len)
            {
                if (i + len <= word.length())
                {
                    string sub = word.substr(i, len);
                    if (operators.count(sub))
                    {
                        cout << "[Operator] " << sub << endl;
                        i += len;
                        matched = true;
                        break;
                    }
                }
            }
            if (matched)
                continue;

            if (specialSymbols.count(word[i]))
            {
                cout << "Special Symbol: " << word[i] << endl;
                ++i;
                continue;
            }

            if (isalnum(word[i]) || word[i] == '_')
            {
                string temp;
                while (i < word.length() && (isalnum(word[i]) || word[i] == '_'))
                {
                    temp += word[i++];
                }

                if (keywords.count(temp))
                {
                    cout << "Keyword: " << temp << endl;
                }
                else if (isIntegerLiteral(temp) || isFloatingLiteral(temp))
                {
                    cout << "Literal: " << temp << endl;
                }
                else if (!isValidIdentifier(temp))
                {
                    cout << "Lexical Error: Invalid identifier '" << temp << "'" << endl;
                    ++errorCount;
                }
                // valid identifier case: no print needed (or you can print if you want)
                continue;
            }

            ++i;
        }
    }

    if (errorCount > 0)
    {
        cout << "\nTotal Invalid Identifiers: " << errorCount << endl;
    }
}

int main()
{
    string filename;
    cout << "Enter the C++ source filename: ";
    cin >> filename;

    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Cannot open file " << filename << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string code = buffer.str();

    analyze(code);
    return 0;
}
