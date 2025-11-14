#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_set>
#include <stack>
#include <algorithm>
#include <cctype>
#include <cstring>
using namespace std;

struct Quadruple
{
    string op, arg1, arg2, result;
};
struct Triple
{
    string op, arg1, arg2;
};

void printQuadruples(const vector<Quadruple> &quads)
{
    cout << "\n=== Quadruples ===\n";
    cout << left << setw(10) << "Op"
         << setw(12) << "Arg1"
         << setw(12) << "Arg2"
         << setw(12) << "Result" << "\n";
    cout << string(46, '-') << "\n";
    for (auto &q : quads)
    {
        cout << left << setw(10) << q.op
             << setw(12) << q.arg1
             << setw(12) << q.arg2
             << setw(12) << q.result << "\n";
    }
}

void printTriples(const vector<Triple> &triples)
{
    cout << "\n=== Triples ===\n";
    cout << left << setw(5) << "Idx"
         << setw(10) << "Op"
         << setw(14) << "Arg1"
         << setw(14) << "Arg2" << "\n";
    cout << string(43, '-') << "\n";
    for (size_t i = 0; i < triples.size(); ++i)
    {
        cout << left << setw(5) << i
             << setw(10) << triples[i].op
             << setw(14) << triples[i].arg1
             << setw(14) << triples[i].arg2 << "\n";
    }
}

// ---------------- Tokenizer ----------------
static inline bool isIdentChar(char c)
{
    return isalnum((unsigned char)c) || c == '_';
}

vector<string> tokenize(const string &s_in)
{
    string s = s_in;
    // replace unicode ≠ with "!=" if present
    for (size_t p = 0; p + 2 <= s.size(); ++p)
    {
        if ((unsigned char)s[p] == 0xC2 || (unsigned char)s[p] == 0xE2)
        { /*skip - crude*/
        }
    }
    // quick replace common unicode '≠' (U+2260) with "!="
    size_t pos;
    while ((pos = s.find("≠")) != string::npos)
        s.replace(pos, 3, "!=");

    vector<string> toks;
    for (size_t i = 0; i < s.size();)
    {
        if (isspace((unsigned char)s[i]))
        {
            ++i;
            continue;
        }
        char c = s[i];

        // multi-char operators and punctuation
        if (c == '&' && i + 1 < s.size() && s[i + 1] == '&')
        {
            toks.push_back("AND");
            i += 2;
            continue;
        }
        if (c == '|' && i + 1 < s.size() && s[i + 1] == '|')
        {
            toks.push_back("OR");
            i += 2;
            continue;
        }
        if (c == '=' && i + 1 < s.size() && s[i + 1] == '=')
        {
            toks.push_back("==");
            i += 2;
            continue;
        }
        if (c == '!' && i + 1 < s.size() && s[i + 1] == '=')
        {
            toks.push_back("!=");
            i += 2;
            continue;
        }
        if (c == '<' && i + 1 < s.size() && s[i + 1] == '=')
        {
            toks.push_back("<=");
            i += 2;
            continue;
        }
        if (c == '>' && i + 1 < s.size() && s[i + 1] == '=')
        {
            toks.push_back(">=");
            i += 2;
            continue;
        }

        // single-char tokens
        if (strchr("()+-*/<>{};:,", c))
        {
            toks.push_back(string(1, c));
            ++i;
            continue;
        }

        if (isalpha((unsigned char)c))
        {
            string id;
            while (i < s.size() && isIdentChar(s[i]))
            {
                id.push_back(s[i]);
                ++i;
            }
            // keywords mapping
            string low = id;
            for (auto &ch : low)
                ch = (char)tolower((unsigned char)ch);
            if (low == "and")
                toks.push_back("AND");
            else if (low == "or")
                toks.push_back("OR");
            else if (low == "then")
                toks.push_back("then");
            else if (low == "else")
                toks.push_back("else");
            else if (low == "if")
                toks.push_back("if");
            else if (low == "while")
                toks.push_back("while");
            else
                toks.push_back(id); // preserve case (A,B vs a,b)
            continue;
        }

        if (isdigit((unsigned char)c))
        {
            string num;
            while (i < s.size() && (isdigit((unsigned char)s[i]) || s[i] == '.'))
            {
                num.push_back(s[i]);
                ++i;
            }
            toks.push_back(num);
            continue;
        }

        // fallback single char token
        toks.push_back(string(1, c));
        ++i;
    }
    return toks;
}

// ---------------- Shunting Yard (infix -> postfix) ----------------
bool isOperatorToken(const string &t)
{
    static const unordered_set<string> ops = {
        "+", "-", "*", "/", "<", ">", "<=", ">=", "==", "!=", "AND", "OR"};
    return ops.count(t) != 0;
}
int prec(const string &op)
{
    if (op == "OR")
        return 1;
    if (op == "AND")
        return 2;
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        return 3;
    if (op == "+" || op == "-")
        return 4;
    if (op == "*" || op == "/")
        return 5;
    return 0;
}
vector<string> infixToPostfix(const vector<string> &tokens)
{
    vector<string> output;
    vector<string> st; // operator stack

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        string tok = tokens[i];
        if (tok == "(")
        {
            st.push_back(tok);
        }
        else if (tok == ")")
        {
            while (!st.empty() && st.back() != "(")
            {
                output.push_back(st.back());
                st.pop_back();
            }
            if (!st.empty() && st.back() == "(")
                st.pop_back(); // pop '('
        }
        else if (isOperatorToken(tok))
        {
            while (!st.empty() && isOperatorToken(st.back()) &&
                   (prec(st.back()) > prec(tok) || (prec(st.back()) == prec(tok))))
            {
                output.push_back(st.back());
                st.pop_back();
            }
            st.push_back(tok);
        }
        else
        {
            // operand
            output.push_back(tok);
        }
    }
    while (!st.empty())
    {
        output.push_back(st.back());
        st.pop_back();
    }
    return output;
}

// ---------------- Generate TAC from postfix, appending to provided vectors ----------------
string generateFromPostfixAndAppend(const vector<string> &postfix,
                                    vector<Quadruple> &quads,
                                    vector<Triple> &triples,
                                    vector<string> &equations, // will append equations like t1 = A + B
                                    int &tcount)
{
    if (postfix.empty())
        return "";

    stack<string> quadStack;   // holds operand names (identifiers or tX)
    stack<string> tripleStack; // holds operand representations (either "a" or "(idx)")

    for (const string &tok : postfix)
    {
        if (!isOperatorToken(tok))
        {
            quadStack.push(tok);
            tripleStack.push(tok);
        }
        else
        {
            // operator
            if (quadStack.size() < 2)
            {
                // malformed expression: return best-effort
                return "";
            }
            string arg2_q = quadStack.top();
            quadStack.pop();
            string arg1_q = quadStack.top();
            quadStack.pop();

            string arg2_tr = tripleStack.top();
            tripleStack.pop();
            string arg1_tr = tripleStack.top();
            tripleStack.pop();

            string temp = "t" + to_string(tcount++);

            // Quadruple
            Quadruple q{tok, arg1_q, arg2_q, temp};
            quads.push_back(q);

            // Triple (index will be current size before push)
            Triple tr{tok, arg1_tr, arg2_tr};
            triples.push_back(tr);
            int trIdx = (int)triples.size() - 1;
            string trRef = "(" + to_string(trIdx) + ")";

            // push results back
            quadStack.push(temp);
            tripleStack.push(trRef);

            // record equation string in readable form (use quad arg names)
            string eq = temp + " = " + arg1_q + " " + tok + " " + arg2_q;
            equations.push_back(eq);
        }
    }

    // final result might be a temp or single operand
    if (!quadStack.empty())
        return quadStack.top();
    return "";
}

// ---------------- High-level processors ----------------

// Process an arithmetic expression (e.g. "(A+B)*(C-D)/(E+F)")
// Appends generated quads/triples/equations to the provided containers.
// Resets/uses tcount passed by caller.
void processArithmeticExpression(const string &exprStr,
                                 vector<Quadruple> &quads,
                                 vector<Triple> &triples,
                                 vector<string> &equations,
                                 int &tcount)
{
    auto toks = tokenize(exprStr);
    // remove stray semicolons or commas if present
    vector<string> exprTokens;
    for (auto &tk : toks)
    {
        if (tk == ";" || tk == ",")
            continue;
        exprTokens.push_back(tk);
    }
    auto postfix = infixToPostfix(exprTokens);
    generateFromPostfixAndAppend(postfix, quads, triples, equations, tcount);
}

// Process an if-statement of form:
// if ( <condition> ) then <stmt> else <stmt>
// where <stmt> are simple assignments (e.g. x = 1) or expressions
void processIfStatement(const string &ifStr,
                        vector<Quadruple> &quads,
                        vector<Triple> &triples,
                        vector<string> &equations,
                        int &tcount)
{
    auto toks = tokenize(ifStr);
    // find '(' after 'if' and matching ')'
    size_t ifPos = 0;
    for (size_t i = 0; i < toks.size(); ++i)
        if (toks[i] == "if")
        {
            ifPos = i;
            break;
        }

    size_t lpar = string::npos;
    for (size_t i = ifPos + 1; i < toks.size(); ++i)
        if (toks[i] == "(")
        {
            lpar = i;
            break;
        }
    if (lpar == string::npos)
        return; // malformed

    int depth = 0;
    size_t rpar = string::npos;
    for (size_t i = lpar; i < toks.size(); ++i)
    {
        if (toks[i] == "(")
            ++depth;
        else if (toks[i] == ")")
        {
            --depth;
            if (depth == 0)
            {
                rpar = i;
                break;
            }
        }
    }
    if (rpar == string::npos)
        return;

    // condition tokens are between lpar+1 ... rpar-1
    vector<string> condTokens(toks.begin() + lpar + 1, toks.begin() + rpar);

    // find 'then' and 'else' positions
    size_t thenPos = string::npos, elsePos = string::npos;
    for (size_t i = rpar + 1; i < toks.size(); ++i)
    {
        if (toks[i] == "then")
        {
            thenPos = i;
            break;
        }
    }
    for (size_t i = (thenPos == string::npos ? rpar + 1 : thenPos + 1); i < toks.size(); ++i)
    {
        if (toks[i] == "else")
        {
            elsePos = i;
            break;
        }
    }
    if (thenPos == string::npos || elsePos == string::npos)
        return;

    // then statement tokens: thenPos+1 .. elsePos-1
    vector<string> thenTokens(toks.begin() + thenPos + 1, toks.begin() + elsePos);
    // else tokens: elsePos+1 .. end
    vector<string> elseTokens(toks.begin() + elsePos + 1, toks.end());

    // Convert condition to postfix and generate TAC (appending to quads/triples)
    auto postfixCond = infixToPostfix(condTokens);
    // capture triple size before condition (so we can reference it)
    size_t beforeCondTripleCount = triples.size();
    generateFromPostfixAndAppend(postfixCond, quads, triples, equations, tcount);
    int condTripleIndex = (int)triples.size() - 1; // last triple index for condition result
    string condTempName;
    if (!quads.empty())
        condTempName = quads.back().result; // last temp produced for condition

    // Create labels L1 (else) and L2 (end)
    static int labelSerial = 1;
    string L1 = "L" + to_string(labelSerial++);
    string L2 = "L" + to_string(labelSerial++);

    // IF_FALSE cond goto L1
    quads.push_back({"IF_FALSE", condTempName, "-", L1});
    triples.push_back({"IF_FALSE", "(" + to_string(condTripleIndex) + ")", L1});

    // THEN part: assume simple assignment(s) separated by ';' or single token set
    // Join thenTokens into statements by ';'
    vector<vector<string>> thenStmts;
    {
        vector<string> cur;
        for (auto &tk : thenTokens)
        {
            if (tk == ";")
            {
                if (!cur.empty())
                {
                    thenStmts.push_back(cur);
                    cur.clear();
                }
            }
            else
                cur.push_back(tk);
        }
        if (!cur.empty())
            thenStmts.push_back(cur);
    }

    // process each then-statement
    for (auto &stmt : thenStmts)
    {
        // look for assignment '='
        auto itEq = find(stmt.begin(), stmt.end(), "=");
        if (itEq != stmt.end())
        {
            string lhs = *(itEq - (itEq == stmt.begin() ? 0 : 1));
            // RHS tokens are after '='
            vector<string> rhsTokens(itEq + 1, stmt.end());
            // generate RHS TAC (if expression), capture last result
            if (!rhsTokens.empty())
            {
                auto postfixRhs = infixToPostfix(rhsTokens);
                // track triple count before RHS
                int beforeRhsTriple = (int)triples.size();
                string rhsTemp;
                if (!postfixRhs.empty())
                {
                    generateFromPostfixAndAppend(postfixRhs, quads, triples, equations, tcount);
                    if (!quads.empty())
                        rhsTemp = quads.back().result;
                }
                // assignment quad
                if (rhsTemp.empty())
                {
                    // simple immediate (e.g., 1)
                    string imm = rhsTokens.size() == 1 ? rhsTokens[0] : "";
                    quads.push_back({"=", imm, "-", lhs});
                    triples.push_back({"=", imm, lhs});
                }
                else
                {
                    quads.push_back({"=", rhsTemp, "-", lhs});
                    // triple: refer to RHS triple index if exists
                    int rhsIdx = (int)triples.size() - 1;
                    triples.push_back({"=", "(" + to_string(rhsIdx) + ")", lhs});
                }
            }
        }
    }

    // GOTO L2
    quads.push_back({"GOTO", "-", "-", L2});
    triples.push_back({"GOTO", "-", L2});

    // Label L1
    quads.push_back({"Label", "-", "-", L1});
    triples.push_back({"Label", "-", L1});

    // ELSE part (same parsing)
    vector<vector<string>> elseStmts;
    {
        vector<string> cur;
        for (auto &tk : elseTokens)
        {
            if (tk == ";")
            {
                if (!cur.empty())
                {
                    elseStmts.push_back(cur);
                    cur.clear();
                }
            }
            else
                cur.push_back(tk);
        }
        if (!cur.empty())
            elseStmts.push_back(cur);
    }
    for (auto &stmt : elseStmts)
    {
        auto itEq = find(stmt.begin(), stmt.end(), "=");
        if (itEq != stmt.end())
        {
            string lhs = *(itEq - (itEq == stmt.begin() ? 0 : 1));
            vector<string> rhsTokens(itEq + 1, stmt.end());
            if (!rhsTokens.empty())
            {
                auto postfixRhs = infixToPostfix(rhsTokens);
                string rhsTemp;
                if (!postfixRhs.empty())
                {
                    generateFromPostfixAndAppend(postfixRhs, quads, triples, equations, tcount);
                    if (!quads.empty())
                        rhsTemp = quads.back().result;
                }
                if (rhsTemp.empty())
                {
                    string imm = rhsTokens.size() == 1 ? rhsTokens[0] : "";
                    quads.push_back({"=", imm, "-", lhs});
                    triples.push_back({"=", imm, lhs});
                }
                else
                {
                    quads.push_back({"=", rhsTemp, "-", lhs});
                    int rhsIdx = (int)triples.size() - 1;
                    triples.push_back({"=", "(" + to_string(rhsIdx) + ")", lhs});
                }
            }
        }
    }

    // Label L2
    quads.push_back({"Label", "-", "-", L2});
    triples.push_back({"Label", "-", L2});
}

// Process a while-statement like: while (i < n) { sum = sum + i; i = i + 1; }
void processWhileStatement(const string &whileStr,
                           vector<Quadruple> &quads,
                           vector<Triple> &triples,
                           vector<string> &equations,
                           int &tcount)
{
    auto toks = tokenize(whileStr);
    // find 'while'
    size_t whilePos = 0;
    for (size_t i = 0; i < toks.size(); ++i)
        if (toks[i] == "while")
        {
            whilePos = i;
            break;
        }

    // find '(' after while and matching ')'
    size_t lpar = string::npos;
    for (size_t i = whilePos + 1; i < toks.size(); ++i)
        if (toks[i] == "(")
        {
            lpar = i;
            break;
        }
    if (lpar == string::npos)
        return;
    int depth = 0;
    size_t rpar = string::npos;
    for (size_t i = lpar; i < toks.size(); ++i)
    {
        if (toks[i] == "(")
            ++depth;
        else if (toks[i] == ")")
        {
            --depth;
            if (depth == 0)
            {
                rpar = i;
                break;
            }
        }
    }
    if (rpar == string::npos)
        return;

    // body between '{' and '}'
    size_t lbrace = string::npos, rbrace = string::npos;
    for (size_t i = rpar + 1; i < toks.size(); ++i)
        if (toks[i] == "{")
        {
            lbrace = i;
            break;
        }
    if (lbrace == string::npos)
        return;
    int bdepth = 0;
    for (size_t i = lbrace; i < toks.size(); ++i)
    {
        if (toks[i] == "{")
            ++bdepth;
        else if (toks[i] == "}")
        { /*we treat char*/
        }
        if (toks[i] == "}")
        {
            --bdepth;
            if (bdepth == 0)
            {
                rbrace = i;
                break;
            }
        }
    }
    // fallback: find last '}'.
    if (rbrace == string::npos)
    {
        for (size_t i = toks.size(); i-- > 0;)
            if (toks[i] == "}")
            {
                rbrace = i;
                break;
            }
    }
    if (rbrace == string::npos)
        return;

    // condition tokens:
    vector<string> condTokens(toks.begin() + lpar + 1, toks.begin() + rpar);
    // body tokens:
    vector<string> bodyTokens(toks.begin() + lbrace + 1, toks.begin() + rbrace);

    // Labels
    static int labelSerial = 1000;
    string L1 = "L" + to_string(labelSerial++);
    string L2 = "L" + to_string(labelSerial++);

    quads.push_back({"Label", "-", "-", L1});
    triples.push_back({"Label", "-", L1});

    auto postfixCond = infixToPostfix(condTokens);
    generateFromPostfixAndAppend(postfixCond, quads, triples, equations, tcount);
    int condIdx = (int)triples.size() - 1;
    string condTemp = (!quads.empty() ? quads.back().result : "");

    quads.push_back({"IF_FALSE", condTemp, "-", L2});
    triples.push_back({"IF_FALSE", "(" + to_string(condIdx) + ")", L2});

    vector<vector<string>> stmts;
    {
        vector<string> cur;
        for (auto &tk : bodyTokens)
        {
            if (tk == ";")
            {
                if (!cur.empty())
                {
                    stmts.push_back(cur);
                    cur.clear();
                }
            }
            else
                cur.push_back(tk);
        }
        if (!cur.empty())
            stmts.push_back(cur);
    }

    for (auto &stmt : stmts)
    {
        auto itEq = find(stmt.begin(), stmt.end(), "=");
        if (itEq != stmt.end())
        {
            string lhs;
            if (itEq != stmt.begin())
                lhs = *(itEq - 1);
            vector<string> rhs(itEq + 1, stmt.end());
            if (!rhs.empty())
            {
                auto postfixRhs = infixToPostfix(rhs);
                string rhsTemp;
                if (!postfixRhs.empty())
                {
                    generateFromPostfixAndAppend(postfixRhs, quads, triples, equations, tcount);
                    if (!quads.empty())
                        rhsTemp = quads.back().result;
                }
                if (rhsTemp.empty())
                {
                    string imm = rhs.size() == 1 ? rhs[0] : "";
                    quads.push_back({"=", imm, "-", lhs});
                    triples.push_back({"=", imm, lhs});
                }
                else
                {
                    quads.push_back({"=", rhsTemp, "-", lhs});
                    int idx = (int)triples.size() - 1;
                    triples.push_back({"=", "(" + to_string(idx) + ")", lhs});
                }
            }
        }
    }

    quads.push_back({"GOTO", "-", "-", L1});
    triples.push_back({"GOTO", "-", L1});

    quads.push_back({"Label", "-", "-", L2});
    triples.push_back({"Label", "-", L2});
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string arith = "(A + B) * (C - D) / (E + F)";
    string booleanIf = "if((a < b) and (c != d)) then x = 1 else x = 0";
    string whl = "while (i < n) { sum = sum + i; i = i + 1; }";

    // ---------- Test Case 1: Arithmetic ----------
    cout << "\n******** Test Case 1: Arithmetic Expression ********\n";
    cout << "Expression: " << arith << "\n";
    vector<Quadruple> quads1;
    vector<Triple> triples1;
    vector<string> eqs1;
    int tcount1 = 1;
    processArithmeticExpression(arith, quads1, triples1, eqs1, tcount1);
    if (!eqs1.empty())
    {
        cout << "\nStep-by-step equations:\n";
        for (auto &e : eqs1)
            cout << e << "\n";
    }
    printQuadruples(quads1);
    printTriples(triples1);

    // ---------- Test Case 2: Boolean If ----------
    cout << "\n******** Test Case 2: Boolean Expression ********\n";
    cout << "Expression: " << booleanIf << "\n";
    vector<Quadruple> quads2;
    vector<Triple> triples2;
    vector<string> eqs2;
    int tcount2 = 1;
    processIfStatement(booleanIf, quads2, triples2, eqs2, tcount2);
    if (!eqs2.empty())
    {
        cout << "\nStep-by-step equations:\n";
        for (auto &e : eqs2)
            cout << e << "\n";
    }
    printQuadruples(quads2);
    printTriples(triples2);

    // ---------- Test Case 3: While Loop ----------
    cout << "\n******** Test Case 3: Loop Expression ********\n";
    cout << "Expression: " << whl << "\n";
    vector<Quadruple> quads3;
    vector<Triple> triples3;
    vector<string> eqs3;
    int tcount3 = 1;
    processWhileStatement(whl, quads3, triples3, eqs3, tcount3);
    if (!eqs3.empty())
    {
        cout << "\nStep-by-step equations:\n";
        for (auto &e : eqs3)
            cout << e << "\n";
    }
    printQuadruples(quads3);
    printTriples(triples3);

    cout << "\n";
    return 0;
}
