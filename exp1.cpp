#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <cctype>
#include <regex>

using namespace std;

set<string> keywords;
set<string> operators;
set<char> specialSymbols;

void initKeywords()
{
    const char *kw[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
        "long", "namespace", "register", "return", "short", "signed", "sizeof",
        "static", "struct", "switch", "typedef", "union", "unsigned", "void",
        "volatile", "while", "class", "bool", "private", "protected", "public",
        "this", "true", "false", "virtual", "include", "using", "new", "delete",
        "nullptr", "try", "catch", "throw", "template", "typename", "inline"};
    int n = sizeof(kw) / sizeof(kw[0]);
    for (int i = 0; i < n; ++i)
        keywords.insert(kw[i]);
}

void initOperators()
{
    const char *ops[] = {
        "+", "-", "*", "/", "%", "++", "--", "==", "!=", ">=", "<=", ">", "<",
        "&&", "||", "!", "&", "|", "^", "~", "<<", ">>", "=", "+=", "-=", "*=", "/=",
        "%=", "&=", "|=", "^=", "<<=", ">>=", "->", "."};
    int n = sizeof(ops) / sizeof(ops[0]);
    for (int i = 0; i < n; ++i)
        operators.insert(ops[i]);
}

void initSpecialSymbols()
{
    char syms[] = {'(', ')', '{', '}', '[', ']', ';', ':', ',', '#', '"', '\''};
    int n = sizeof(syms) / sizeof(syms[0]);
    for (int i = 0; i < n; ++i)
        specialSymbols.insert(syms[i]);
}

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

bool isCharLiteral(const string &token)
{
    return regex_match(token, regex("'(\\\\.|[^\\\\'])'"));
}

bool isStringLiteral(const string &token)
{
    return regex_match(token, regex("\"(\\\\.|[^\\\\\"])*\""));
}

void analyze(const string &code)
{
    istringstream stream(code);
    int keywordCount = 0, identifierCount = 0, operatorCount = 0;
    int specialSymbolCount = 0, literalCount = 0, errorCount = 0;

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
            for (int len = 3; len >= 1; --len)
            {
                if (i + len <= word.length())
                {
                    string sub = word.substr(i, len);
                    if (operators.find(sub) != operators.end())
                    {
                        ++operatorCount;
                        i += len;
                        matched = true;
                        break;
                    }
                }
            }
            if (matched)
                continue;

            if (specialSymbols.find(word[i]) != specialSymbols.end())
            {
                if (word[i] == '\'')
                {
                    size_t start = i;
                    size_t end = word.find('\'', start + 1);
                    if (end != string::npos && end > start)
                    {
                        string charLit = word.substr(start, end - start + 1);
                        if (isCharLiteral(charLit))
                        {
                            ++literalCount;
                            i = end + 1;
                            continue;
                        }
                    }
                }
                else if (word[i] == '"')
                {
                    size_t start = i;
                    size_t end = word.find('"', start + 1);
                    while (end != string::npos && word[end - 1] == '\\')
                        end = word.find('"', end + 1);
                    if (end != string::npos && end > start)
                    {
                        string strLit = word.substr(start, end - start + 1);
                        if (isStringLiteral(strLit))
                        {
                            ++literalCount;
                            i = end + 1;
                            continue;
                        }
                    }
                }
                ++specialSymbolCount;
                ++i;
                continue;
            }

            if (isalnum(word[i]) || word[i] == '_')
            {
                string temp = "";
                while (i < word.size() && (isalnum(word[i]) || word[i] == '_'))
                    temp += word[i++];

                if (isIntegerLiteral(temp) || isFloatingLiteral(temp))
                {
                    ++literalCount;
                }
                else if (keywords.find(temp) != keywords.end())
                {
                    ++keywordCount;
                }
                else if (isValidIdentifier(temp))
                {
                    ++identifierCount;
                }
                else
                {
                    cerr << "Lexical Error: Unrecognized token '" << temp << "'\n";
                    ++errorCount;
                }
                continue;
            }

            cerr << "Lexical Error: Unrecognized token '" << word[i] << "'\n";
            ++errorCount;
            ++i;
        }
    }

    cout << "\nTOKENS\n";
    cout << "Keywords        : " << keywordCount << endl;
    cout << "Identifiers     : " << identifierCount << endl;
    cout << "Operators       : " << operatorCount << endl;
    cout << "Special Symbols : " << specialSymbolCount << endl;
    cout << "Literals        : " << literalCount << endl;
    cout << "Errors          : " << errorCount << endl;
}

int main()
{
    initKeywords();
    initOperators();
    initSpecialSymbols();

    string filename;
    cout << "Enter the C++ source filename: ";
    cin >> filename;

    ifstream file(filename.c_str());
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
