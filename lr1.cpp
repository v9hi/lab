#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

#define MAX 100
#define STATES 20
#define SYMBOLS 10

// Grammar rules
struct Production {
    char lhs;
    string rhs;
};

// Item structure
struct Item {
    char lhs;
    string rhs;
    int dot_position;
};

// State structure
struct State {
    vector<Item> items;
};

// Stack structure for parsing
struct Stack {
    vector<int> items;
    void push(int x) { items.push_back(x); }
    void pop() { if (!items.empty()) items.pop_back(); }
    int top() { return items.empty() ? -1 : items.back(); }
    bool empty() { return items.empty(); }
};

// Global variables
vector<Production> grammar;
vector<State> states;
vector<char> terminals;
vector<char> non_terminals;
string action[STATES][SYMBOLS];
int goto_table[STATES][SYMBOLS];

// ---------- Utility functions ----------
bool item_exists(State &state, Item item) {
    for (auto &it : state.items) {
        if (it.lhs == item.lhs && it.rhs == item.rhs && it.dot_position == item.dot_position) {
            return true;
        }
    }
    return false;
}

void add_item(State &state, Item item) {
    if (!item_exists(state, item)) {
        state.items.push_back(item);
    }
}

int symbol_index(char sym, vector<char> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] == sym) return i;
    }
    return -1;
}

// ---------- Printing ----------
void print_state(int index) {
    cout << "State " << index << ":\n";
    for (auto &it : states[index].items) {
        if (it.lhs == 'Q')
            cout << "  S' -> ";
        else
            cout << "  " << it.lhs << " -> ";
        for (int j = 0; j < it.rhs.size(); j++) {
            if (j == it.dot_position) cout << ".";
            cout << it.rhs[j];
        }
        if (it.dot_position == it.rhs.size()) cout << ".";
        cout << "\n";
    }
    cout << "\n";
}

// ---------- Closure & GOTO ----------
void closure(State &state) {
    bool added;
    do {
        added = false;
        for (int i = 0; i < state.items.size(); i++) {
            Item it = state.items[i];
            if (it.dot_position < it.rhs.size()) {
                char next_symbol = it.rhs[it.dot_position];
                if (symbol_index(next_symbol, non_terminals) != -1) {
                    for (auto &prod : grammar) {
                        if (prod.lhs == next_symbol) {
                            Item new_item{ prod.lhs, prod.rhs, 0 };
                            if (!item_exists(state, new_item)) {
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

State goto_func(State &state, char symbol) {
    State new_state;
    for (auto &it : state.items) {
        if (it.dot_position < it.rhs.size() && it.rhs[it.dot_position] == symbol) {
            Item moved_item = it;
            moved_item.dot_position++;
            add_item(new_state, moved_item);
        }
    }
    closure(new_state);
    return new_state;
}

bool states_equal(State &s1, State &s2) {
    if (s1.items.size() != s2.items.size()) return false;
    for (auto &it1 : s1.items) {
        bool found = false;
        for (auto &it2 : s2.items) {
            if (it1.lhs == it2.lhs && it1.rhs == it2.rhs && it1.dot_position == it2.dot_position) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

int state_index(State &new_state) {
    for (int i = 0; i < states.size(); i++) {
        if (states_equal(states[i], new_state)) {
            return i;
        }
    }
    return -1;
}

// ---------- Build states ----------
void build_states() {
    states.clear();
    State s0;
    Item start_item{ 'Q', "S", 0 };
    add_item(s0, start_item);
    closure(s0);
    states.push_back(s0);
    cout << "\nDFA of Item Sets (Transitions):\n";

    for (int front = 0; front < states.size(); front++) {
        State curr = states[front];
        vector<char> symbols;
        for (auto &it : curr.items) {
            if (it.dot_position < it.rhs.size()) {
                char sym = it.rhs[it.dot_position];
                if (symbol_index(sym, symbols) == -1) {
                    symbols.push_back(sym);
                }
            }
        }

        for (auto sym : symbols) {
            State new_state = goto_func(curr, sym);
            int idx = state_index(new_state);
            if (idx == -1) {
                idx = states.size();
                states.push_back(new_state);
            }
            if (sym == 'Q')
                cout << "I" << front << " --S'--> I" << idx << "\n";
            else
                cout << "I" << front << " --" << sym << "--> I" << idx << "\n";

            if (symbol_index(sym, terminals) != -1) {
                int t_idx = symbol_index(sym, terminals);
                action[front][t_idx] = "s" + to_string(idx);
            } else {
                int nt_idx = symbol_index(sym, non_terminals);
                goto_table[front][nt_idx] = idx;
            }
        }
    }
}

// ---------- Parsing Table ----------
void build_parsing_table() {
    for (int i = 0; i < STATES; i++) {
        for (int j = 0; j < SYMBOLS; j++) {
            action[i][j] = "error";
            goto_table[i][j] = -1;
        }
    }
    build_states();
    for (int i = 0; i < states.size(); i++) {
        for (auto &it : states[i].items) {
            if (it.dot_position == it.rhs.size()) {
                if (it.lhs == 'Q' && it.rhs == "S") {
                    int idx = symbol_index('$', terminals);
                    action[i][idx] = "acc";
                } else {
                    for (int t = 0; t < terminals.size(); t++) {
                        int prod_idx = -1;
                        for (int k = 0; k < grammar.size(); k++) {
                            if (grammar[k].lhs == it.lhs && grammar[k].rhs == it.rhs) {
                                prod_idx = k;
                                break;
                            }
                        }
                        if (prod_idx != -1) {
                            action[i][t] = "r" + to_string(prod_idx);
                        }
                    }
                }
            }
        }
    }
}

void print_parsing_table() {
    cout << "\nACTION and GOTO Table:\n";
    cout << "State\t";
    for (auto t : terminals) cout << t << "\t";
    for (auto nt : non_terminals) cout << nt << "\t";
    cout << "\n";

    for (int i = 0; i < states.size(); i++) {
        cout << i << "\t";
        for (int j = 0; j < terminals.size(); j++) {
            cout << action[i][j] << "\t";
        }
        for (int j = 0; j < non_terminals.size(); j++) {
            if (goto_table[i][j] != -1)
                cout << goto_table[i][j] << "\t";
            else
                cout << "-\t";
        }
        cout << "\n";
    }
}

// ---------- Parsing ----------
void parse_input(string input_str) {
    Stack state_stack, symbol_stack;
    state_stack.push(0);
    symbol_stack.push('$');
    input_str += "$";
    int ip = 0;

    cout << "\nParsing Trace:\n";
    cout << "Stack\t\tInput\t\tAction\n";
    while (true) {
        int state = state_stack.top();
        char lookahead = input_str[ip];
        int term_idx = symbol_index(lookahead, terminals);

        cout << "[";
        for (int i = 0; i < state_stack.items.size(); i++) cout << state_stack.items[i] << " ";
        cout << "]\t\t" << input_str.substr(ip) << "\t\t";

        if (term_idx == -1 || action[state][term_idx] == "error") {
            cout << "Error\n";
            break;
        }
        if (action[state][term_idx] == "acc") {
            cout << "Accept\n";
            break;
        }
        if (action[state][term_idx][0] == 's') {
            int next_state = stoi(action[state][term_idx].substr(1));
            cout << "Shift " << lookahead << "\n";
            state_stack.push(next_state);
            symbol_stack.push(lookahead);
            ip++;
        } else if (action[state][term_idx][0] == 'r') {
            int prod_idx = stoi(action[state][term_idx].substr(1));
            Production p = grammar[prod_idx];
            cout << "Reduce by " << p.lhs << " -> " << p.rhs << "\n";
            int rhs_len = p.rhs.size();
            for (int i = 0; i < rhs_len; i++) {
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

int main() {
    // Define grammar rules (excluding augmented one)
    grammar.push_back({ 'S', "CC" });
    grammar.push_back({ 'C', "cC" });
    grammar.push_back({ 'C', "d" });
    // Augmented production: S' -> S (internally Q -> S)
    grammar.push_back({ 'Q', "S" });

    // Terminals and non-terminals
    terminals = { 'c', 'd', '$' };
    non_terminals = { 'S', 'C', 'Q' };

    // Build parsing table
    build_parsing_table();

    // Print canonical collection
    cout << "\nCanonical Collection of LR(0) Items:\n";
    for (int i = 0; i < states.size(); i++) {
        print_state(i);
    }

    // Print parsing table
    print_parsing_table();

    // Input string
    string input_str;
    cout << "\nEnter input string (e.g. ccdd): ";
    cin >> input_str;

    // Parse the input
    parse_input(input_str);

    return 0;
}
