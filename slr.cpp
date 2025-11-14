#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <set>
using namespace std;
#define MAX 100
#define STATES 20
#define SYMBOLS 10

struct Production
{
    char lhs;
    string rhs;
};

struct Item
{
    char lhs;
    string rhs;
    int dot_position;
};

struct State
{
    vector<Item> items;
};

struct Stack
{
    vector<int> items;
    void push(int x) { items.push_back(x); }
    void pop()
    {
        if (!items.empty())
            items.pop_back();
    }
    int top() { return items.empty() ? -1 : items.back(); }
    bool empty() { return items.empty(); }
};

vector<Production> grammar;
vector<State> states;
vector<char> terminals;
vector<char> non_terminals;
string action[STATES][SYMBOLS];
int goto_table[STATES][SYMBOLS];
map<char, set<char>> first_sets;
map<char, set<char>> follow_sets;

bool item_exists(State &state, Item item)
{
    for (auto &it : state.items)
    {
        if (it.lhs == item.lhs && it.rhs == item.rhs && it.dot_position == item.dot_position)
        {
            return true;
        }
    }
    return false;
}
void add_item(State &state, Item item)
{
    if (!item_exists(state, item))
    {
        state.items.push_back(item);
    }
}
int symbol_index(char sym, vector<char> &arr)
{
    for (int i = 0; i < arr.size(); i++)
    {
        if (arr[i] == sym)
            return i;
    }
    return -1;
}
bool is_terminal(char sym)
{
    return symbol_index(sym, terminals) != -1;
}
bool is_non_terminal(char sym)
{
    return symbol_index(sym, non_terminals) != -1;
}
bool contains_char(vector<char> &vec, char c)
{
    for (char ch : vec)
    {
        if (ch == c)
            return true;
    }
    return false;
}

bool can_derive_epsilon(const string &str)
{
    if (str.empty())
        return true;
    for (char c : str)
    {
        if (is_terminal(c))
            return false;
        if (first_sets[c].find('#') == first_sets[c].end())
            return false;
    }
    return true;
}

set<char> compute_first_of_string(const string &str)
{
    set<char> result;
    if (str.empty())
    {
        result.insert('#');
        return result;
    }
    for (int i = 0; i < str.length(); i++)
    {
        char symbol = str[i];
        if (is_terminal(symbol))
        {
            result.insert(symbol);
            break;
        }
        else
        {

            for (char c : first_sets[symbol])
            {
                if (c != '#')
                {
                    result.insert(c);
                }
            }

            if (first_sets[symbol].find('#') == first_sets[symbol].end())
            {
                break;
            }

            if (i == str.length() - 1)
            {
                result.insert('#');
            }
        }
    }
    return result;
}

void compute_first_sets()
{
    cout << "Computing FIRST sets...\n";

    first_sets.clear();
    for (char nt : non_terminals)
    {
        first_sets[nt] = set<char>();
    }
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const Production &prod : grammar)
        {
            if (prod.lhs == 'Q')
                continue;
            set<char> old_first = first_sets[prod.lhs];
            if (prod.rhs.empty())
            {

                first_sets[prod.lhs].insert('#');
            }
            else
            {

                bool all_have_epsilon = true;
                for (int i = 0; i < prod.rhs.length(); i++)
                {
                    char symbol = prod.rhs[i];
                    if (is_terminal(symbol))
                    {
                        first_sets[prod.lhs].insert(symbol);
                        all_have_epsilon = false;
                        break;
                    }
                    else
                    {

                        for (char c : first_sets[symbol])
                        {
                            if (c != '#')
                            {
                                first_sets[prod.lhs].insert(c);
                            }
                        }

                        if (first_sets[symbol].find('#') == first_sets[symbol].end())
                        {
                            all_have_epsilon = false;
                            break;
                        }
                    }
                }

                if (all_have_epsilon)
                {
                    first_sets[prod.lhs].insert('#');
                }
            }

            if (first_sets[prod.lhs] != old_first)
            {
                changed = true;
            }
        }
    }
}
void compute_follow_sets()
{
    cout << "Computing FOLLOW sets...\n";

    follow_sets.clear();
    for (char nt : non_terminals)
    {
        if (nt != 'Q')
        {
            follow_sets[nt] = set<char>();
        }
    }

    follow_sets['S'].insert('$');
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const Production &prod : grammar)
        {
            if (prod.lhs == 'Q')
                continue;
            for (int i = 0; i < prod.rhs.length(); i++)
            {
                char symbol = prod.rhs[i];
                if (is_non_terminal(symbol) && symbol != 'Q')
                {
                    set<char> old_follow = follow_sets[symbol];

                    string beta = prod.rhs.substr(i + 1);

                    set<char> first_beta = compute_first_of_string(beta);
                    for (char c : first_beta)
                    {
                        if (c != '#')
                        {
                            follow_sets[symbol].insert(c);
                        }
                    }

                    if (can_derive_epsilon(beta))
                    {
                        for (char c : follow_sets[prod.lhs])
                        {
                            follow_sets[symbol].insert(c);
                        }
                    }

                    if (follow_sets[symbol] != old_follow)
                    {
                        changed = true;
                    }
                }
            }
        }
    }
}
void compute_first_follow_sets()
{
    compute_first_sets();
    compute_follow_sets();
    cout << "FIRST and FOLLOW computation completed.\n";
}
void print_first_follow()
{
    cout << "\nFIRST sets:\n";
    for (char nt : non_terminals)
    {
        if (nt == 'Q')
            continue;
        cout << "FIRST(" << nt << ") = { ";
        for (char c : first_sets[nt])
        {
            if (c == '#')
            {
                cout << "ε ";
            }
            else
            {
                cout << c << " ";
            }
        }
        cout << "}\n";
    }
    cout << "\nFOLLOW sets:\n";
    for (char nt : non_terminals)
    {
        if (nt == 'Q')
            continue;
        cout << "FOLLOW(" << nt << ") = { ";
        for (char c : follow_sets[nt])
        {
            cout << c << " ";
        }
        cout << "}\n";
    }
}

void print_state(int index)
{
    cout << "State " << index << ":\n";
    for (auto &it : states[index].items)
    {
        if (it.lhs == 'Q')
            cout << " S' -> ";
        else
            cout << " " << it.lhs << " -> ";
        for (int j = 0; j < it.rhs.size(); j++)
        {
            if (j == it.dot_position)
                cout << ".";
            cout << it.rhs[j];
        }
        if (it.dot_position == it.rhs.size())
            cout << ".";
        cout << "\n";
    }
    cout << "\n";
}

void closure(State &state)
{
    bool added;
    do
    {
        added = false;
        for (int i = 0; i < state.items.size(); i++)
        {
            Item it = state.items[i];
            if (it.dot_position < it.rhs.size())
            {
                char next_symbol = it.rhs[it.dot_position];
                if (symbol_index(next_symbol, non_terminals) != -1)
                {
                    for (auto &prod : grammar)
                    {
                        if (prod.lhs == next_symbol)
                        {
                            Item new_item;
                            new_item.lhs = prod.lhs;
                            new_item.rhs = prod.rhs;
                            new_item.dot_position = 0;
                            if (!item_exists(state, new_item))
                            {
                                add_item(state, new_item);
                                added = true;
                            }
                        }
                    }
                }
            }
        }
    } while (added);
}
State goto_func(State &state, char symbol)
{
    State new_state;
    for (auto &it : state.items)
    {
        if (it.dot_position < it.rhs.size() && it.rhs[it.dot_position] == symbol)
        {
            Item moved_item = it;
            moved_item.dot_position++;
            add_item(new_state, moved_item);
        }
    }
    closure(new_state);
    return new_state;
}
bool states_equal(State &s1, State &s2)
{
    if (s1.items.size() != s2.items.size())
        return false;
    for (auto &it1 : s1.items)
    {
        bool found = false;
        for (auto &it2 : s2.items)
        {
            if (it1.lhs == it2.lhs && it1.rhs == it2.rhs && it1.dot_position == it2.dot_position)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}
int state_index(State &new_state)
{
    for (int i = 0; i < states.size(); i++)
    {
        if (states_equal(states[i], new_state))
        {
            return i;
        }
    }
    return -1;
}

void build_states()
{
    states.clear();
    State s0;
    Item start_item;
    start_item.lhs = 'Q';
    start_item.rhs = "S";
    start_item.dot_position = 0;
    add_item(s0, start_item);
    closure(s0);
    states.push_back(s0);
    cout << "\nDFA of Item Sets (Transitions):\n";
    for (int front = 0; front < states.size(); front++)
    {
        State curr = states[front];
        vector<char> symbols;
        for (auto &it : curr.items)
        {
            if (it.dot_position < it.rhs.size())
            {
                char sym = it.rhs[it.dot_position];
                if (symbol_index(sym, symbols) == -1)
                {
                    symbols.push_back(sym);
                }
            }
        }
        for (auto sym : symbols)
        {
            State new_state = goto_func(curr, sym);
            int idx = state_index(new_state);
            if (idx == -1)
            {
                idx = states.size();
                states.push_back(new_state);
            }
            if (sym == 'Q')
                cout << "I" << front << " --S'--> I" << idx << "\n";
            else
                cout << "I" << front << " --" << sym << " --> I " << idx << "\n ";
            if (symbol_index(sym, terminals) != -1)
            {
                int t_idx = symbol_index(sym, terminals);
                action[front][t_idx] = "s" + to_string(idx);
            }
            else
            {
                int nt_idx = symbol_index(sym, non_terminals);
                goto_table[front][nt_idx] = idx;
            }
        }
    }
}

void build_slr_parsing_table()
{

    for (int i = 0; i < STATES; i++)
    {
        for (int j = 0; j < SYMBOLS; j++)
        {
            action[i][j] = "error";
            goto_table[i][j] = -1;
        }
    }

    compute_first_follow_sets();

    build_states();

    for (int i = 0; i < states.size(); i++)
    {
        for (auto &it : states[i].items)
        {
            if (it.dot_position == it.rhs.size())
            {

                if (it.lhs == 'Q' && it.rhs == "S")
                {

                    int idx = symbol_index('$', terminals);
                    action[i][idx] = "acc";
                }
                else
                {

                    int prod_idx = -1;
                    for (int k = 0; k < grammar.size(); k++)
                    {
                        if (grammar[k].lhs == it.lhs && grammar[k].rhs == it.rhs)
                        {
                            prod_idx = k;
                            break;
                        }
                    }
                    if (prod_idx != -1)
                    {

                        for (char follow_sym : follow_sets[it.lhs])
                        {
                            int t_idx = symbol_index(follow_sym, terminals);
                            if (t_idx != -1)
                            {
                                if (action[i][t_idx] != "error")
                                {
                                    cout << "SLR Conflict detected in state " << i
                                         << " for symbol " << follow_sym << endl;
                                }
                                action[i][t_idx] = "r" + to_string(prod_idx);
                            }
                        }
                    }
                }
            }
        }
    }
}
void print_parsing_table()
{
    cout << "\nSLR ACTION and GOTO Table:\n";
    cout << "State\t";
    for (auto t : terminals)
        cout << t << "\t";
    for (auto nt : non_terminals)
    {
        if (nt != 'Q')
            cout << nt << "\t";
    }
    cout << "\n";
    for (int i = 0; i < states.size(); i++)
    {
        cout << i << "\t";
        for (int j = 0; j < terminals.size(); j++)
        {
            cout << action[i][j] << "\t";
        }
        for (int j = 0; j < non_terminals.size(); j++)
        {
            if (non_terminals[j] != 'Q')
            {
                if (goto_table[i][j] != -1)
                    cout << goto_table[i][j] << "\t";
                else
                    cout << "-\t ";
            }
        }
        cout << "\n";
    }
}

void parse_input(string input_str)
{
    Stack state_stack, symbol_stack;
    state_stack.push(0);
    symbol_stack.push('$');
    input_str += "$";
    int ip = 0;
    cout << "\nSLR Parsing Trace:\n";
    cout << "Stack\t\tInput\t\tAction\n";
    while (true)
    {
        int state = state_stack.top();
        char lookahead = input_str[ip];
        int term_idx = symbol_index(lookahead, terminals);
        cout << "[";
        for (int i = 0; i < state_stack.items.size(); i++)
            cout << state_stack.items[i] << " ";
        cout << "]\t\t" << input_str.substr(ip) << "\t\t";
        if (term_idx == -1 || action[state][term_idx] == "error")
        {
            cout << "Error\n";
            break;
        }
        if (action[state][term_idx] == "acc")
        {
            cout << "Accept\n";
            break;
        }
        if (action[state][term_idx][0] == 's')
        {
            int next_state = stoi(action[state][term_idx].substr(1));
            cout << "Shift " << lookahead << "\n";
            state_stack.push(next_state);
            symbol_stack.push(lookahead);
            ip++;
        }
        else if (action[state][term_idx][0] == 'r')
        {
            int prod_idx = stoi(action[state][term_idx].substr(1));
            Production p = grammar[prod_idx];
            cout << "Reduce by " << p.lhs << " -> " << p.rhs << "\n";
            int rhs_len = p.rhs.size();
            for (int i = 0; i < rhs_len; i++)
            {
                state_stack.pop();
                symbol_stack.pop();
            }
            state = state_stack.top();
            int nt_idx = symbol_index(p.lhs, non_terminals);
            symbol_stack.push(p.lhs);
            state_stack.push(goto_table[state][nt_idx]);
        }
    }
}
int main()
{

    Production p1 = {'S', "CC"};
    grammar.push_back(p1);
    Production p2 = {'C', "cC"};
    grammar.push_back(p2);
    Production p3 = {'C', "d"};
    grammar.push_back(p3);

    Production p4 = {'Q', "S"};
    grammar.push_back(p4);

    terminals.push_back('c');
    terminals.push_back('d');
    terminals.push_back('$');
    non_terminals.push_back('S');
    non_terminals.push_back('C');
    non_terminals.push_back('Q');
    cout << "Grammar Rules:\n";
    cout << "S -> CC\n";
    cout << "C -> cC\n";
    cout << "C -> d\n";

    build_slr_parsing_table();

    print_first_follow();

    cout << "\nCanonical Collection of LR(0) Items:\n";
    for (int i = 0; i < states.size(); i++)
    {
        print_state(i);
    }

    print_parsing_table();

    string input_str;
    cout << "\nEnter input string (e.g. ccdd): ";
    cin >> input_str;

    parse_input(input_str);
    return 0;
}