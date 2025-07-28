#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <ctime>
#include <array>
#include <cmath>
#include <memory>
#include <sstream>
#include <map>
#include <regex>
#include <climits>
#include <numeric>
#include <mysqlx/xdevapi.h>
#include<pybind11/pybind11.h>


using namespace std;
namespace py = pybind11;

map<int, array<int, 4>> preferences;
vector<int> members(48);
unordered_map<int, unordered_set<string>> match_history;
unordered_map<int, int> roles, teams;
unordered_map<int, string>id_db;
unordered_map<string, int>reverse_id;



double match_penalty(int m) {
    double penalty = 0;
    int t = teams[m];

    for(int k = 0; k < 4; k++){
        int c = members[t * 4 + k];
        if(c != m) {
            if(match_history.find(m) != match_history.end() &&
               match_history[m].find(id_db[c]) != match_history[m].end()) {
                penalty++;
            }
        }
    }
    return penalty;
}

double calculate_penalty(int i, int j, bool flag) {
    int m1 = members[i];
    int m2 = members[j];
    double old_rank = preferences[m1][roles[m1]] + preferences[m2][roles[m2]];
    double new_rank = preferences[m1][roles[m2]] + preferences[m2][roles[m1]];
    double delta_rank = new_rank - old_rank;

    if(flag) return delta_rank;

    return match_penalty(m1) + match_penalty(m2) + delta_rank;
}

string trim(const string& str) {
    string s = regex_replace(str, regex("^\\s+"), "");
    s = regex_replace(s, regex("\\s+$"), "");
    return s;
}

void scramble_db(string host, string user, string password) { 
    mysqlx::Session connect(host, 33060, user, password);

    connect.sql("DROP TABLE IF EXISTS hics.result;").execute();

    connect.sql(
        "CREATE TABLE IF NOT EXISTS hics.result ("
        "Team INT NOT NULL,"
        "Members VARCHAR(255),"
        "Roles VARCHAR(255));").execute();
    
    //Maps role numbers to roles
    map<int, string> roles_db = {
        {0, "Situation Analyst"},
        {1, "Solutions"},
        {2, "Kabya\\'s Lapdog"},
        {3, "Financials"}
    };

    string insertTable;
    stringstream it;
    it <<  "INSERT INTO hics.result (Team, Members, Roles) VALUES ";
    bool first = true;
    for(int i = 0; i < 12; i++){
        for (int j = 0; j < 4; j++) {
            if(!first) it << ", ";
            int m = members[i * 4 + j];
            it << "(" << i+1 << ", '" << id_db[m] << "', '" << roles_db[roles[m]] << "')";
            first = false;
        }
    }
    it << ";";

    insertTable = it.str();

    connect.sql(insertTable).execute();
    
    return;

}

void updateDB(string host, string user, string password) { 
    mysqlx::Session connect(host, 33060, user, password);

    connect.sql("DROP TABLE IF EXISTS hics.past_history;").execute();

    connect.sql(
        "CREATE TABLE IF NOT EXISTS hics.past_history ("
        "id INT NOT NULL,"
        "past_match VARCHAR(255) );"
    ).execute();

    mysqlx::Schema db = connect.getSchema("hics");
    mysqlx::Table res = db.getTable("result");

    mysqlx::RowResult rows_res = connect.sql(
        "SELECT a.id, b.Team, b.Members "
        "FROM hics.initial a, hics.result b "
        "WHERE a.Members = b.Members;").execute();

    string insertTable;
    stringstream it;
    it <<  "INSERT INTO hics.past_history (id, past_match) VALUES ";
    bool first = true;
    
    vector<mysqlx::Row> row_vector;
    for (auto row: rows_res) row_vector.push_back(row);

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = j + 1; k < 4; k++) {
                if (!first) it << ", ";
                it << "(" << row_vector[i * 4 + j][0] << ", '" << row_vector[i * 4 + k][2] << "'), (" << row_vector[i * 4 + k][0] << ", '" << row_vector[i * 4 + j][2] << "')";
                first = false;
            }
        }
    }

    it << ";";

    insertTable = it.str();

    connect.sql(insertTable).execute();

    return;
}

void scramble(string host, string user, string password) {

    preferences.clear();
    match_history.clear();
    roles.clear();
    teams.clear();
    id_db.clear();
    reverse_id.clear();

    iota(members.begin(), members.end(), 0);

    ifstream file("../cache/cached_data.csv");

    for(string input; getline(file, input);) {
        stringstream ss(input);

        string id, name, match;
        getline(ss, id, ',');

        getline(ss, name, ',');
        name = trim(name);
        if(name.empty()) continue;

        int idi = stoi(id) - 1; //0-index
        id_db[idi] = name;
        reverse_id[name] = idi;


        // Ranked preferences of positions
        array<int, 4> ranks;
        for(int i = 0; i < 4; i++) {
            string temp;
            if(!(getline(ss, temp, ','))) continue;
            temp = trim(temp);
            ranks[i] = stoi(temp);
        }

        while(getline(ss, match, ',')) {
            match = trim(match);
            if(!match.size()) break;
            match_history[idi].insert(match);
        }

        // Store the name and preferences in the database
        preferences[idi] = ranks;
    }

    file.close();

    //Randomise the names
    random_device rd;
    mt19937 g(rd());
    shuffle(members.begin(), members.end(), g);

    for(int i = 0; i < 48; i++) {
        teams[members[i]] = i / 4;
    }

    for(int i = 0; i < 12; i++){
        // Brute force all permutations to find the best assignment
        vector<int> perm = {0, 1, 2, 3};
        vector<int> best_roles(4);
        int best_sum = INT_MAX;
        do {
            int sum = 0;
            for (int j = 0; j < 4; j++) {
                sum += preferences[members[i * 4 + j]][perm[j]];
            }
            if (sum < best_sum) {
                best_sum = sum;
                best_roles = perm;
            }
        } while (next_permutation(perm.begin(), perm.end()));

        for(int j = 0; j < 4; j++) {
            roles[members[i * 4 + j]] = best_roles[j];
        }
    }

    // Simulated annealing parameters
    double init_temp = 1000.0;
    double cooling_rate = 0.995;
    int iterations = 200000; //adjust as necessary

    // Swap members at random
    uniform_int_distribution<int> member_dist(0, 47);
    double T = init_temp;

    for(int itr = 0; itr < iterations; itr++) {
        int m1 = member_dist(g);
        int m2 = member_dist(g);
        while (m1 == m2) m2 = member_dist(g);

        bool teams_flag = (teams[members[m1]] == teams[members[m2]]);

        double delta = calculate_penalty(m1, m2, teams_flag);

        uniform_real_distribution<double> prob_dist(0.0, 1.0);
        double random = prob_dist(g);

        if (delta < 0 || (delta > 0 && exp(-delta / T) > random)) {
            swap(members[m1], members[m2]);

            //swap roles
            swap(roles[members[m1]], roles[members[m2]]);

            if(teams_flag) continue;

            //swap teams
            swap(teams[members[m1]], teams[members[m2]]);
        }

        T *= cooling_rate;
        //can add break condition if T < min_temp and unchanging penalty
    }

    scramble_db(host, user, password);
}

PYBIND11_MODULE(cpp, m) {
    m.def("scramble", &scramble, py::arg("host"), py::arg("user"), py::arg("password"));

    m.def("updateDB", &updateDB, py::arg("host"), py::arg("user"), py::arg("password"));
}


