#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;

vector<string> tokenize(const string &str, char delimiter)
{
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter))
    {
        if (!token.empty())
        {
            tokens.push_back(token);
        }
    }
    return tokens;
}

string trim(const string &str)
{
    const string WHITESPACE = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}

class OperatorPrecedenceParser
{
private:
    map<string, vector<vector<string>>> grammarRules;
    set<string> nonTerminalsSet;
    set<string> terminalsSet;
    string startSymbol;

    map<string, set<string>> leadingSet;
    map<string, set<string>> trailingSet;
    map<string, map<string, char>> precedenceTable;

    void parseGrammar(const string &grammarLine)
    {
        size_t arrowPos = grammarLine.find("->");
        if (arrowPos == string::npos)
        {
            cout << "Invalid grammar format. Use 'NT -> productions'." << endl;
            return;
        }
        string lhs = trim(grammarLine.substr(0, arrowPos));
        string rhs = trim(grammarLine.substr(arrowPos + 2));

        startSymbol = lhs;
        nonTerminalsSet.insert(lhs);

        vector<string> productions = tokenize(rhs, '|');
        for (const auto &prodStr : productions)
        {
            string trimmedProd = trim(prodStr);
            vector<string> prodTokens = tokenize(trimmedProd, ' ');
            grammarRules[lhs].push_back(prodTokens);
            for (const auto &token : prodTokens)
            {
                bool isNonTerminal = true;
                for (char c : token)
                {
                    if (!isupper(c))
                    {
                        isNonTerminal = false;
                        break;
                    }
                }
                if (isNonTerminal && !token.empty())
                {
                    nonTerminalsSet.insert(token);
                }
                else if (!token.empty())
                {
                    terminalsSet.insert(token);
                }
            }
        }
    }

    void computeLeadingTrailingSets()
    {
        // Initialize
        for (const auto &nt : nonTerminalsSet)
        {
            leadingSet[nt] = set<string>();
            trailingSet[nt] = set<string>();
        }

        bool changed = true;
        while (changed)
        {
            changed = false;
            for (const auto &pair : grammarRules)
            {
                string nt = pair.first;
                for (const auto &prod : pair.second)
                {
                    if (!prod.empty())
                    {
                        // LEADING
                        string firstSymbol = prod[0];
                        size_t oldLeadingSize = leadingSet[nt].size();
                        if (terminalsSet.count(firstSymbol))
                        {
                            leadingSet[nt].insert(firstSymbol);
                        }
                        else if (nonTerminalsSet.count(firstSymbol))
                        {
                            leadingSet[nt].insert(leadingSet[firstSymbol].begin(), leadingSet[firstSymbol].end());
                            if (prod.size() > 1 && terminalsSet.count(prod[1]))
                            {
                                leadingSet[nt].insert(prod[1]);
                            }
                        }
                        if (leadingSet[nt].size() > oldLeadingSize)
                            changed = true;
                    }
                    if (!prod.empty())
                    {
                        // TRAILING
                        string lastSymbol = prod.back();
                        size_t oldTrailingSize = trailingSet[nt].size();
                        if (terminalsSet.count(lastSymbol))
                        {
                            trailingSet[nt].insert(lastSymbol);
                        }
                        else if (nonTerminalsSet.count(lastSymbol))
                        {
                            trailingSet[nt].insert(trailingSet[lastSymbol].begin(), trailingSet[lastSymbol].end());
                            if (prod.size() > 1 && terminalsSet.count(prod[prod.size() - 2]))
                            {
                                trailingSet[nt].insert(prod[prod.size() - 2]);
                            }
                        }
                        if (trailingSet[nt].size() > oldTrailingSize)
                            changed = true;
                    }
                }
            }
        }
    }

    void buildPrecedenceTable()
    {
        set<string> allTerminals = terminalsSet;
        allTerminals.insert("$");
        for (const auto &t1 : allTerminals)
        {
            for (const auto &t2 : allTerminals)
            {
                precedenceTable[t1][t2] = ' ';
            }
        }

        for (const auto &pair : grammarRules)
        {
            for (const auto &prod : pair.second)
            {
                for (size_t i = 0; i < prod.size() - 1; ++i)
                {
                    string s1 = prod[i];
                    string s2 = prod[i + 1];
                    if (terminalsSet.count(s1) && terminalsSet.count(s2))
                        precedenceTable[s1][s2] = '=';
                    if (i < prod.size() - 2 && terminalsSet.count(s1) && nonTerminalsSet.count(s2) && terminalsSet.count(prod[i + 2]))
                        precedenceTable[s1][prod[i + 2]] = '=';
                    if (terminalsSet.count(s1) && nonTerminalsSet.count(s2))
                    {
                        for (const auto &term : leadingSet[s2])
                            precedenceTable[s1][term] = '<';
                    }
                    if (nonTerminalsSet.count(s1) && terminalsSet.count(s2))
                    {
                        for (const auto &term : trailingSet[s1])
                            precedenceTable[term][s2] = '>';
                    }
                }
            }
        }

        map<string, int> precedenceMap;
        precedenceMap["+"] = 1;
        precedenceMap["-"] = 1;
        precedenceMap["*"] = 2;
        precedenceMap["/"] = 2;
        vector<string> operators = {"+", "-", "*", "/"};

        for (const auto &op1 : operators)
        {
            for (const auto &op2 : operators)
            {
                precedenceTable[op1][op2] = (precedenceMap[op1] >= precedenceMap[op2]) ? '>' : '<';
            }
        }

        for (const auto &op : operators)
        {
            precedenceTable[op]["("] = '<';
            precedenceTable[op]["id"] = '<';
            precedenceTable[")"][op] = '>';
            precedenceTable["id"][op] = '>';
        }

        precedenceTable["("]["("] = '<';
        precedenceTable["("]["id"] = '<';
        precedenceTable[")"][")"] = '>';
        precedenceTable["id"][")"] = '>';
        precedenceTable["("][")"] = '=';

        for (const auto &term : leadingSet[startSymbol])
            precedenceTable["$"][term] = '<';
        for (const auto &term : trailingSet[startSymbol])
            precedenceTable[term]["$"] = '>';

        for (const auto &op : operators)
        {
            precedenceTable[")"][op] = '>';
            precedenceTable[op][")"] = '>';
            precedenceTable["$"][")"] = ' ';
            precedenceTable["("]["$"] = ' ';
        }
    }

    void printLeadingTrailingSets()
    {

        cout << left << setw(15) << "NonTerminal" << setw(30) << "LEADING" << setw(30) << "TRAILING" << endl;
        cout << string(75, '-') << endl;
        for (const auto &nt : nonTerminalsSet)
        {
            cout << left << setw(15) << nt;
            string leadStr = "{ ";
            for (const auto &term : leadingSet[nt])
                leadStr += term + ", ";
            if (leadStr.length() > 2)
            {
                leadStr.pop_back();
                leadStr.pop_back();
            }
            leadStr += " }";
            cout << left << setw(30) << leadStr;
            string trailStr = "{ ";
            for (const auto &term : trailingSet[nt])
                trailStr += term + ", ";
            if (trailStr.length() > 2)
            {
                trailStr.pop_back();
                trailStr.pop_back();
            }
            trailStr += " }";
            cout << left << setw(30) << trailStr << endl;
        }
    }

    void printPrecedenceTable()
    {
        cout << "\nOperator Precedence Table:\n";
        vector<string> termList = {"id", "+", "-", "*", "/", "(", ")", "$"};
        cout << setw(6) << " ";
        for (const auto &term : termList)
        {
            cout << setw(6) << term;
        }
        cout << endl
             << string(6 * (termList.size() + 1), '-') << endl;

        for (const auto &rowTerm : termList)
        {
            cout << setw(6) << rowTerm;
            for (const auto &colTerm : termList)
            {
                char rel = ' ';
                if (precedenceTable.count(rowTerm) && precedenceTable.at(rowTerm).count(colTerm))
                {
                    rel = precedenceTable.at(rowTerm).at(colTerm);
                }
                cout << setw(6) << rel;
            }
            cout << endl;
        }
    }

public:
    void setup()
    {
        cout << "Enter the grammar (A-> productions), e.g. E->id + E | id :\n";
        string grammarLine;
        getline(cin, grammarLine);
        parseGrammar(grammarLine);
        computeLeadingTrailingSets();
        buildPrecedenceTable();
        printLeadingTrailingSets();
        printPrecedenceTable();
    }

    void parse(const string &inputStr)
    {
        cout << "\nParsing Input String: \"" << inputStr << "\"\n";
        cout << left << setw(45) << "STACK" << setw(40) << "INPUT" << "ACTION" << endl;
        cout << string(95, '-') << endl;

        vector<string> inputBuffer = tokenize(inputStr, ' ');
        inputBuffer.push_back("$");

        vector<string> stack;
        stack.push_back("$");

        while (true)
        {
            string stackStr;
            for (const auto &s : stack)
                stackStr += s + " ";
            string inputStrRem;
            for (const auto &s : inputBuffer)
                inputStrRem += s + " ";
            cout << left << setw(45) << stackStr << setw(40) << inputStrRem;

            string topTerminal = "$";
            for (int i = (int)stack.size() - 1; i >= 0; --i)
            {
                if (terminalsSet.count(stack[i]) || stack[i] == "$")
                {
                    topTerminal = stack[i];
                    break;
                }
            }
            string currentInput = inputBuffer.front();

            if (topTerminal == "$" && currentInput == "$")
            {
                cout << "Accept" << endl;
                cout << "\nString successfully parsed.\n"
                     << endl;
                return;
            }

            char relation = ' ';
            if (precedenceTable.count(topTerminal) && precedenceTable.at(topTerminal).count(currentInput))
            {
                relation = precedenceTable.at(topTerminal).at(currentInput);
            }

            if (relation == '<' || relation == '=')
            {
                cout << "Shift" << endl;
                stack.push_back(currentInput);
                inputBuffer.erase(inputBuffer.begin());
            }
            else if (relation == '>')
            {
                cout << "Reduce";

                int handleStartIndex = 1;

                int topTerminalIndex = -1;
                for (int i = (int)stack.size() - 1; i >= 0; --i)
                {
                    if (terminalsSet.count(stack[i]))
                    {
                        topTerminalIndex = i;
                        break;
                    }
                }

                if (topTerminalIndex != -1)
                {
                    for (int i = topTerminalIndex; i > 0; --i)
                    {
                        if (terminalsSet.count(stack[i]))
                        {
                            string prevTerminal = "$";
                            int prevTerminalIndex = 0;
                            for (int j = i - 1; j >= 0; --j)
                            {
                                if (terminalsSet.count(stack[j]) || stack[j] == "$")
                                {
                                    prevTerminal = stack[j];
                                    prevTerminalIndex = j;
                                    break;
                                }
                            }
                            if (precedenceTable.at(prevTerminal).at(stack[i]) == '<')
                            {
                                handleStartIndex = prevTerminalIndex + 1;
                                break;
                            }
                        }
                    }
                }

                string handleStr;
                for (size_t i = handleStartIndex; i < stack.size(); ++i)
                    handleStr += stack[i] + " ";
                cout << " (handle: " << handleStr << ")" << endl;

                stack.erase(stack.begin() + handleStartIndex, stack.end());
                stack.push_back(startSymbol);
            }
            else
            {
                cout << "Error" << endl;
                cout << "\nSyntax Error: No relation between stack top terminal '" << topTerminal << "' and input '" << currentInput << "'" << endl;
                cout << "\nString rejected.\n"
                     << endl;
                return;
            }
        }
    }
};

int main()
{
    OperatorPrecedenceParser parser;
    parser.setup();

    while (true)
    {
        cout << "\nEnter an input string to parse (or '0' to exit):" << endl;
        string inputLine;
        getline(cin, inputLine);

        if (inputLine == "0")
        {
            break;
        }
        parser.parse(inputLine);
    }

    return 0;
}
