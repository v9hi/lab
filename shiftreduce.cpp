#include <iostream>
#include <vector>
#include <string>
using namespace std;
struct R
{
    string l, r;
};
bool endsWith(const string &st, const string &suf)
{
    if (st.size() < suf.size())
        return false;
    return st.substr(st.size() - suf.size()) == suf;
}
int main()
{
    int n;
    cout << "Enter number of grammar rules: ";
    cin >> n;
    cin.ignore();
    vector<R> rs;
    cout << "Enter grammar rules (Format: E->E+T|T, no spaces):\n";
    for (int i = 0; i < n; i++)
    {
        string in;
        getline(cin, in);
        size_t ar = in.find("->");
        string l = in.substr(0, ar);
        string rp = in.substr(ar + 2);
        size_t pos = 0;
        while ((pos = rp.find("|")) != string::npos)
        {
            rs.push_back({l, rp.substr(0, pos)});
            rp.erase(0, pos + 1);
        }
        rs.push_back({l, rp});
    }
    string in_raw;
    cout << "\nEnter input string to parse (e.g., id+id*id$, end with $): ";
    cin >> in_raw;
    string in_str;
    size_t p_id = 0;
    while ((p_id = in_raw.find("id")) != string::npos)
    {
        in_str += in_raw.substr(0, p_id);
        in_str += 'i';
        in_raw.erase(0, p_id + 2);
    }
    in_str += in_raw;
    string st;
    cout << "\nStep-by-step parsing process:\n";
    cout << "Stack\t\tInput Buﬀer\t\tAction\n";
    cout << "-------------------------------------------------------\n";
    while (true)
    {
        cout << st << "\t\t" << in_str << "\t\t";
        if (st == rs[0].l && in_str == "$")
        {
            cout << "Accept\n";
            break;
        }
        bool red = false;
        for (auto &r : rs)
        {
            string rr = r.r;
            if (rr == "id")
                rr = "i";
            if (endsWith(st, rr))
            {
                st.erase(st.size() - rr.size());
                st += r.l;
                cout << "Reduce by " << r.l << "->" << r.r << "\n";
                red = true;
                break;
            }
        }
        if (red)
            continue;
        if (!in_str.empty())
        {
            st.push_back(in_str[0]);
            in_str.erase(0, 1);
            cout << "Shift\n";
        }

        else
        {
            cout << "Error\n";
            break;
        }
    }
    return 0;
}
