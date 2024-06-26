#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <map>
#include <set>
#include <ctime>

using namespace std;

int maxColour = 0, _time, _eq_cnt, eq_num = 1, ok = 0;
// Стэк покраски
stack <int> painting_stack;

vector <pair <int, int> > help_list_edge;

struct edge
{
    int first;
    int second;
    int l;
    edge(int u, int v, int l)
    {
        this->first = u;
        this->second = v;
        this->l = l;
    }
    bool operator<(const edge& temp) const
    {
        return this->first == temp.first ? this->second < temp.second : this->first < temp.first;
    }

    bool operator==(const edge& temp) const
    {
        return this->first == temp.first && this->second == temp.second && this->l == temp.l;
    }
};

double find_balanced_cycle_time = 0;
clock_t find_balanced_cycle_time_clock_t = 0;

// Список вершин рахделённых на классы эквивалентности, 
// Его эл-ты выглядят след образом: (k, (u, v, l)), где k-класс эквивалентности, (u, v, l) - ребро u<->v веса l
vector <pair <int, edge> > Equal_edges;

set <pair <int, int> > multiple_edges;
set <pair <int, int> > multiple_edges2;

/*void print_vector(vector<int> &v)
{
    cout << "{ ";
    for (int i = 0; i < v.size(); ++i)
    {
        cout << v[i] << " ";
    }
    cout << "}";
}*/

void generate_input(string filename)
{
    string fname = filename;
    string s;
    ifstream fin1(fname);
    ofstream fout;
    fout.open("generate.txt");
    map <string, int> a;
    if (fin1.is_open() && fout.is_open())
    {
        int cur = 1;
        for (int i = 0; i <= 1; ++i)
        {
            getline(fin1, s);
            //s.pop_back();
            int tec = s.find_first_of(':');
            if (s.find_first_of(',', tec + 1) > s.size())
            {
                a[s.substr(tec + 2, s.size())] = cur;
                ++cur;
            }
            else
            {
                a[s.substr(tec + 2, s.find_first_of(',', tec + 1) - tec - 2)] = cur;
                ++cur;
            }
            tec = s.find_first_of(',', tec + 1);
            while (tec < s.size())
            {
                if (s.find_first_of(',', tec + 1) > s.size())
                {
                    a[s.substr(tec + 2)] = cur;
                    ++cur;
                }
                else
                {
                    a[s.substr(tec + 2, s.find_first_of(',', tec + 1) - tec - 2)] = cur;
                    ++cur;
                }
                tec = s.find_first_of(',', tec + 1);
            }
        }
        /*for (auto now : a)
        {
            cout << now.first << " " << now.second << "\n";
        }*/
        fout << cur - 1;
        while (getline(fin1, s))
        {
            int uk1 = s.find_first_of(','), uk2 = s.find_first_of(':');
            fout << "\n" << a[s.substr(0, uk1)] << " " << a[s.substr(uk1 + 2, uk2 - uk1 - 2)] << " " << s.substr(uk2 + 2);
        }
    }
    fin1.close();
    fout.close();
}

class Data
{
public:
    // Кол-во вершин
    int N;
    // colours - массив цветов; tin - время входа; up - время выхода; visited - массив для проверки факта посещения вершины
    // links - массив смены номера вершины, используется для создание подграфа из компоненты рёберной двусвязности
    int* colours, * tin, * up, * visited, * links;

    // adj_matrix - матрица смежности; eq_matrix - матрица эквивалентности
    int** adj_matrix, ** eq_matrix;

    vector <vector <pair <int, int> > > list_connected;

    // Список вершин
    vector <edge> list_edges;

    // Конструктор по данным с файла
    // Файл выглядит следующим образом:
    // N
    // u1 v1 l1
    // u2 v2 l2
    // ...
    // N - кол-во вершин
    // Ребро u_i<->v_i с весом l_i
    Data(string filename)
    {
        int u, v, l;
        ifstream fin(filename);
        if (fin.is_open())
        {
            fin >> N;
            colours = new int[N + 1];
            tin = new int[N + 1];
            up = new int[N + 1];
            visited = new int[N + 1];
            for (int i = 0; i <= this->N; ++i)
            {
                this->colours[i] = this->tin[i] = this->up[i] = this->visited[i] = 0;
            }
            //links = new int[N + 1];
            adj_matrix = new int* [N + 1];
            eq_matrix = new int* [N + 1];
            for (int i = 0; i <= N; ++i)
            {
                adj_matrix[i] = new int[N + 1];
                eq_matrix[i] = new int[N + 1];
                for (int j = 0; j <= N; ++j)
                {
                    adj_matrix[i][j] = -1000;
                    eq_matrix[i][j] = 0;
                }
            }

            list_connected.resize(N + 1);

            while (fin >> u >> v >> l)
            {
                adj_matrix[u][v] = adj_matrix[v][u] = l;
                list_connected[u].push_back(make_pair(v, l));
                list_connected[v].push_back(make_pair(u, l));
                if (find(help_list_edge.begin(), help_list_edge.end(), make_pair(u, v)) != help_list_edge.end() ||
                    find(help_list_edge.begin(), help_list_edge.end(), make_pair(v, u)) != help_list_edge.end())
                {
                    multiple_edges.insert(make_pair(u, v));
                    multiple_edges.insert(make_pair(v, u));
                }
                list_edges.push_back(edge(u, v, l));
                help_list_edge.push_back(make_pair(u, v));
            }
        }
        fin.close();
    }
    // Конструктор для построения подграфа из переданного(temp);
    // colour - номер компоненты рёберной двусвязности, n - кол-во вершин в ней
    Data(Data& temp, int colour, int n)
    {
        int uk_link = 1;
        this->N = n;
        this->colours = new int[N + 1];
        this->tin = new int[N + 1];
        this->up = new int[N + 1];
        this->visited = new int[N + 1];
        for (int i = 0; i <= this->N; ++i)
        {
            this->colours[i] = this->tin[i] = this->up[i] = this->visited[i] = 0;
        }
        this->links = new int[temp.N + 1];
        for (int i = 1; i <= temp.N; ++i)
        {
            this->links[i] = 0;
        }
        for (int i = 1; i <= temp.N; ++i)
        {
            if (temp.colours[i] == colour)
            {
                this->links[i] = uk_link;
                ++uk_link;
            }
        }
        cout << "links:\n";
        for (int i = 1; i <= temp.N; ++i)
        {
            cout << this->links[i] << " ";
        }
        cout << "\n";
        this->adj_matrix = new int* [N + 1];
        this->eq_matrix = new int* [N + 1];
        for (int i = 0; i <= N; ++i)
        {
            this->adj_matrix[i] = new int[N + 1];
            this->eq_matrix[i] = new int[N + 1];
            for (int j = 0; j <= N; ++j)
            {
                this->eq_matrix[i][j] = 0;
                this->adj_matrix[i][j] = -1000;
            }
        }
        for (int i = 0; i < temp.list_edges.size(); ++i)
        {
            if (temp.colours[temp.list_edges[i].first] == colour && temp.colours[temp.list_edges[i].second] == colour)
            {
                this->adj_matrix[this->links[temp.list_edges[i].first]][this->links[temp.list_edges[i].second]] =
                    this->adj_matrix[this->links[temp.list_edges[i].second]][this->links[temp.list_edges[i].first]] =
                    temp.adj_matrix[temp.list_edges[i].first][temp.list_edges[i].second];
                this->list_edges.push_back(edge(this->links[temp.list_edges[i].first], this->links[temp.list_edges[i].second], temp.list_edges[i].l));
            }
        }
        this->list_connected.resize(this->N + 1);
        for (int i = 1; i < temp.list_connected.size(); ++i)
        {
            if (temp.colours[i] == colour)
            {
                for (int j = 0; j < temp.list_connected[i].size(); ++j)
                {
                    if (temp.colours[temp.list_connected[i][j].first] == colour)
                    {
                        this->list_connected[this->links[i]].push_back(make_pair(this->links[temp.list_connected[i][j].first], temp.list_connected[i][j].second));
                    }
                }
            }
        }
        for (auto it = multiple_edges.begin(); it != multiple_edges.end(); ++it)
        {
            if (temp.colours[it->first] == colour && temp.colours[it->second] == colour)
            {
                multiple_edges2.insert(make_pair(this->links[it->first], this->links[it->second]));
            }
        }
    }
};

// Переопределение оператора вывода
ostream& operator<<(ostream& out, Data& data)
{
    out << "N = " << data.N << "; M = " << data.list_edges.size() << "\n";

    out << "colours:\n";
    for (int i = 1; i <= data.N; ++i)
    {
        out << data.colours[i] << " ";
    }
    out << "\n";

    out << "tin:\n";
    for (int i = 1; i <= data.N; ++i)
    {
        out << data.tin[i] << " ";
    }
    out << "\n";

    out << "up:\n";
    for (int i = 1; i <= data.N; ++i)
    {
        out << data.up[i] << " ";
    }
    out << "\n";

    out << "visited:\n";
    for (int i = 1; i <= data.N; ++i)
    {
        out << data.visited[i] << " ";
    }
    out << "\n";

    out << "list_edges:\n";
    for (int i = 0; i < data.list_edges.size(); ++i)
    {
        out << "(" << data.list_edges[i].first << " <-> " << data.list_edges[i].second << ") = " << data.list_edges[i].l << "\n";
    }

    out << "adj_matrix:\n";
    for (int i = 1; i <= data.N; ++i)
    {
        for (int j = 1; j <= data.N; ++j)
        {
            out << data.adj_matrix[i][j] << " ";
        }
        out << "\n";
    }

    out << "eq_matrix:\n";
    for (int i = 1; i <= data.N; ++i)
    {
        for (int j = 1; j <= data.N; ++j)
        {
            out << data.eq_matrix[i][j] << " ";
        }
        out << "\n";
    }

    out << "list_connected:\n";
    for (int i = 1; i < data.list_connected.size(); ++i)
    {
        cout << i << ":\n";
        for (int j = 0; j < data.list_connected[i].size(); ++j)
        {
            cout << "(" << data.list_connected[i][j].first << ", " << data.list_connected[i][j].second << ") ";
        }
        cout << "\n";
    }

    return out;
}

// Функция для покраски компонент рёберной двусвязности(необходимость под вопросом)
void paint(int v, Data& data)
{
    ++maxColour;
    int last = -1;
    while (last != v && !painting_stack.empty())
    {
        data.colours[painting_stack.top()] = maxColour;
        last = painting_stack.top();
        painting_stack.pop();
    }
}

// Поиск в глубину для определения компонент рёберной двусвязности
void dfs1(int v, Data& data, int p = -1)
{
    painting_stack.push(v);
    data.tin[v] = data.up[v] = _time++;
    data.visited[v] = 1;
    for (int i = 0; i < data.list_connected[v].size(); ++i)
    {
        int to = data.list_connected[v][i].first;
        if (to != p)
        {
            if (data.visited[to] != 0)
            {
                data.up[v] = min(data.up[v], data.tin[to]);
            }
            else
            {
                dfs1(to, data, v);
                data.up[v] = min(data.up[v], data.up[to]);
            }
        }
    }
    if (data.tin[v] == data.up[v]) {
        ++maxColour;
        while (!painting_stack.empty()) {
            int u = painting_stack.top();
            painting_stack.pop();
            data.colours[u] = maxColour;
            if (u == v) {
                break;
            }
        }
    }
}

// Обход в глубину для поиска мостов
void dfs2(int v, Data& data, int p = -1)
{
    data.tin[v] = data.up[v] = _time++;
    data.visited[v] = 1;
    for (int i = 1; i <= data.N; ++i)
    {
        if (data.adj_matrix[v][i] != -1000 && i != p)
        {
            if (data.visited[i] != 0)
            {
                data.up[v] = min(data.up[v], data.tin[i]);
            }
            else
            {
                dfs2(i, data, v);
                data.up[v] = min(data.up[v], data.up[i]);
                if (data.up[i] > data.tin[v])
                {
                    if (multiple_edges2.find(make_pair(i, v)) == multiple_edges2.end())
                    {
                        data.eq_matrix[v][i] = data.eq_matrix[i][v] = eq_num;
                        ++_eq_cnt;
                    }
                }
            }
        }
    }
}

// Функция для распределения рёбер по классам эквивалентности
void find_equals(Data& data)
{
    for (int k = 0; k < data.list_edges.size(); ++k)
    {
        if (data.eq_matrix[data.list_edges[k].first][data.list_edges[k].second] == 0)
        {
            // Если дуга кратная, то она не может быть эквивалнетна какой-либо другой
            if (multiple_edges2.find(make_pair(data.list_edges[k].first, data.list_edges[k].second)) != multiple_edges2.end())
            {
                Equal_edges.push_back(make_pair(-eq_num++, edge(data.list_edges[k].first, data.list_edges[k].second, data.list_edges[k].l)));
            }
            else
            {
                // Счётчик эл-тов в классе эквивалентности
                _eq_cnt = 1;
                // Заносим ребро в матрицу эквивалентности
                data.eq_matrix[data.list_edges[k].first][data.list_edges[k].second] =
                    data.eq_matrix[data.list_edges[k].second][data.list_edges[k].first] = eq_num;

                // Запоминаем вес ребра
                int weight = data.list_edges[k].l;
                // Удаляем ребро из матрицы смежности
                data.adj_matrix[data.list_edges[k].first][data.list_edges[k].second] =
                    data.adj_matrix[data.list_edges[k].second][data.list_edges[k].first] = -1000;

                // Поиск мостов
                _time = 0;
                for (int i = 1; i <= data.N; ++i)
                {
                    data.visited[i] = data.tin[i] = data.up[i] = 0;
                }
                for (int i = 1; i <= data.N; ++i)
                {
                    if (!data.visited[i])
                    {
                        dfs2(i, data);
                    }
                }
                // Восстанавливаем ребро
                data.adj_matrix[data.list_edges[k].first][data.list_edges[k].second] =
                    data.adj_matrix[data.list_edges[k].second][data.list_edges[k].first] = weight;

                ++eq_num;
                // Если нет эквивалентных рёбер, то, для удобства сортировки, умножам на -1 номер класса эквивалентности
                if (_eq_cnt == 1)
                {
                    data.eq_matrix[data.list_edges[k].first][data.list_edges[k].second]
                        = data.eq_matrix[data.list_edges[k].second][data.list_edges[k].first] *= -1;
                }
            }
        }
    }

    // По матрице эквивалентности строим список эквивалентности
    for (int i = 1; i <= data.N; ++i)
    {
        for (int j = i + 1; j <= data.N; ++j)
        {
            if (data.eq_matrix[i][j] != 0)
            {
                Equal_edges.push_back(make_pair(data.eq_matrix[i][j], edge(i, j, data.adj_matrix[i][j])));
            }
        }
    }

    sort(Equal_edges.begin(), Equal_edges.end());
    cout << "Equal_edges:\n";
    for (int i = 0; i < Equal_edges.size(); ++i)
    {
        cout << Equal_edges[i].first << " (" << Equal_edges[i].second.first << " <-> " << Equal_edges[i].second.second << ") = " << Equal_edges[i].second.l << "\n";
    }
    cout << "\n";
}

// Обход в глубину для определения цикла, подсчёт происходит на подъёме
bool find_cycle(int v, vector <vector <pair<int, int> > >& spanning_tree_matrix, int& sum_even, int& sum_odd, int end, int step = 1, int p = -1)
{
    for (int i = 0; i < spanning_tree_matrix[v].size(); ++i)
    {
        if (spanning_tree_matrix[v][i].first != p)
        {
            int to = spanning_tree_matrix[v][i].first, len = spanning_tree_matrix[v][i].second;
            if (to == end)
            {
                sum_even += len * abs(2 - step);
                sum_odd += len * abs(1 - step);
                return true;
            }

            bool ret = find_cycle(to, spanning_tree_matrix, sum_even, sum_odd, end, 3 - step, v);
            if (ret)
            {
                sum_even += len * abs(2 - step);
                sum_odd += len * abs(1 - step);
                return true;
            }
        }
    }
    return false;
}

// Функция для подсчёта сбалансированных циклов
int cnt_balanced_cycle(Data& data, vector <edge> spanning_tree)
{
    clock_t start_time = clock();
    int sum_even = 0, sum_odd = 0, ans = 0;
    // Список смежных вершин
    vector <vector <pair<int, int> > > spanning_tree_matrix(data.N + 1);
    vector <edge> copy_list_edges;
    copy(data.list_edges.begin(), data.list_edges.end(), back_inserter(copy_list_edges));
    for (const auto & i : spanning_tree)
    {
        int u = i.first, v = i.second, l = i.l;
        spanning_tree_matrix[u].push_back(make_pair(v, l));
        spanning_tree_matrix[v].push_back(make_pair(u, l));
        auto it = find(copy_list_edges.begin(), copy_list_edges.end(), i);
        if (it != copy_list_edges.end())
        {
            copy_list_edges.erase(it);
        }
    }

    // Замена на binary_search???
    for (int i = 0; i < copy_list_edges.size(); ++i)
    {
        int begin = copy_list_edges[i].first, end = copy_list_edges[i].second, l = copy_list_edges[i].l;
        sum_even = sum_odd = 0;
        find_cycle(begin, spanning_tree_matrix, sum_even, sum_odd, end);
        sum_odd += l;
        //cout << "Even: " << sum_even << "; Odd: " << sum_odd << "\n";

        if (sum_even == sum_odd)
        {
            ans += 1;
        }
    }
    clock_t end_time = clock();
    find_balanced_cycle_time += (double)(end_time - start_time) / CLOCKS_PER_SEC;
    find_balanced_cycle_time_clock_t += end_time - start_time;

    return ans;
}

int max_cnt_balanced_cycle;
multiset <edge> max_spanning_tree;

inline int dsu_get(int v, vector <int>& p) {
    return (v == p[v]) ? v : (p[v] = dsu_get(p[v], p));
}

void dsu_unite(int a, int b, vector <int>& p) {
    a = dsu_get(a, p);
    b = dsu_get(b, p);
    if (rand() & 1)
        swap(a, b);
    if (a != b)
        p[a] = b;
}

// Функция для перебора всех не эквивлентных остовных деревьев, использующая метод ветвей и границ
void branches_boundaries(vector <pair <int, edge> >& _equal_edges, vector <edge>& spanning_tree, vector<int> tree_id, Data& data)
{
    //cout << ok++ << "\n";
    ok++;

    /*cout << "eq_edges: ";
    for (auto x : _equal_edges)
    {
        cout << "(" <<  x.second.first << "<->" << x.second.second << "=" << x.second.l << ") ";
    }
    cout << " sp_tree: ";
    for (auto x : spanning_tree)
    {
        cout << "(" << x.first << "<->" << x.second << "=" << x.l << ") ";
    }
    cout << "\n";*/

    {
        // Запоминаем ребро
        int class_num = _equal_edges.back().first, a = _equal_edges.back().second.first, b = _equal_edges.back().second.second, l = _equal_edges.back().second.l;
        // Удаляем его из списка
        _equal_edges.pop_back();
        // Переходим по правой ветви, но только если ещё остались рёбра
        if (!_equal_edges.empty())
        {
            branches_boundaries(_equal_edges, spanning_tree, tree_id, data);
        }

        // Если можно добавить ребро
        if (dsu_get(a, tree_id) != dsu_get(b, tree_id))
        {
            // Добавляем ребро для перехода по левой ветви
            spanning_tree.push_back(edge(a, b, l));
            dsu_unite(a, b, tree_id);

            // Т.к. мы добавили ребро, могло образоваться остовное дерево
            if (spanning_tree.size() == data.N - 1)
            {
                int count = cnt_balanced_cycle(data, spanning_tree);
                /*cout << "Кол-во сбалансированных циклов: " << count << "\n";
                cout << "spanning_tree(Остовное дерево):\n";
                for (auto i : spanning_tree)
                {
                    cout << "(" << i.first << " <-> " << i.second << ") = " << i.l << "\n";
                }
                cout << "\n";*/
                if (count > max_cnt_balanced_cycle)
                {
                    max_spanning_tree.clear();
                    for (const auto & i : spanning_tree)
                    {
                        max_spanning_tree.insert(i);
                    }
                    max_cnt_balanced_cycle = count;
                }
            }
            else
            {
                // Переходим по левой ветви, где текущее ребро входит в остов
                //cout << "Left\n";
                if (!_equal_edges.empty())
                {
                    branches_boundaries(_equal_edges, spanning_tree, tree_id, data);
                }
            }
            spanning_tree.pop_back();
        }
        _equal_edges.push_back(make_pair(class_num, edge(a, b, l)));
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");

    generate_input("input4.txt");
    //Data phase1("generate.txt");
    Data phase1("input7.txt");
    _time = 0;
    for (int i = 1; i <= phase1.N; ++i)
    {
        if (!phase1.visited[i])
        {
            dfs1(i, phase1);
        }
    }
    //cout << phase1;

    clock_t start_time = clock(); // начальное время

    for (int i = 0; i <= maxColour; ++i) /// !!!
    {
        cout << "Для компоненты рёберной двусвязности №" << i << ":\n";
        int count = 0;
        for (int j = 1; j <= phase1.N; ++j)
        {
            if (phase1.colours[j] == i)
            {
                ++count;
            }
        }
        if (count > 1)
        {
            multiple_edges2.clear();
            Data phase2(phase1, i, count);
            /*cout << "multiple_edges2:\n";
            for (auto i : multiple_edges2)
            {
                cout << i.first << "<->" << i.second << "\n";
            }*/
            cout << "\n";
            find_equals(phase2);
            //cout << phase2;

            vector <edge> spanning_tree;
            vector<int> tree_id(phase2.N + 1);
            for (int i = 0; i < tree_id.size(); ++i)
            {
                tree_id[i] = i;
            }
            max_cnt_balanced_cycle = -1;

            vector <pair <int, edge> > temp;

            while (!Equal_edges.empty() && Equal_edges.back().first > 0)
            {
                int class_num = Equal_edges.back().first, a = Equal_edges.back().second.first, b = Equal_edges.back().second.second, l = Equal_edges.back().second.l;
                temp.emplace_back(class_num, edge(a, b, l));
                Equal_edges.pop_back();
                while (!Equal_edges.empty() && Equal_edges.back().first == class_num)
                {
                    spanning_tree.emplace_back(Equal_edges.back().second.first, Equal_edges.back().second.second, Equal_edges.back().second.l);
                    dsu_unite(Equal_edges.back().second.first, Equal_edges.back().second.second, tree_id);
                    Equal_edges.pop_back();
                }
            }

            for (const auto& x : temp)
            {
                Equal_edges.emplace_back(x);
            }

            cout << "Current spanning tree:\n";
            for (const auto & x : spanning_tree)
            {
                cout << x.first << " " << x.second << " " << x.l << "\n";
            }
            cout << "\n";

            if (spanning_tree.size() == phase2.N - 1)
            {
                int count = cnt_balanced_cycle(phase2, spanning_tree);
                /*cout << "Кол-во сбалансированных циклов: " << count << "\n";
                cout << "spanning_tree(Остовное дерево):\n";
                for (auto i : spanning_tree)
                {
                    cout << "(" << i.first << " <-> " << i.second << ") = " << i.l << "\n";
                }
                cout << "\n";*/
                if (count > max_cnt_balanced_cycle)
                {
                    max_spanning_tree.clear();
                    for (const auto& i : spanning_tree)
                    {
                        max_spanning_tree.insert(i);
                    }
                    max_cnt_balanced_cycle = count;
                }
            }
            else
            {
                branches_boundaries(Equal_edges, spanning_tree, tree_id, phase2);
            }

            cout << "Максимальное кол-во сбалансированных циклов: " << max_cnt_balanced_cycle << "\n";
            cout << "max_spanning_tree(Остовное дерево):\n";
            for (auto& i : max_spanning_tree)
            {
                cout << "(" << i.first << " <-> " << i.second << ") = " << i.l << "\n";
            }
            cout << "\n";
            Equal_edges.clear();
        }
    }

    clock_t end_time = clock();
    double seconds = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    cout << "Общее время работы поиска сбалансированных циклов: " << find_balanced_cycle_time_clock_t << " процессорных тиков\n";
    cout << "Общее время работы поиска сбалансированных циклов: " << find_balanced_cycle_time << " секунд\n";
    cout << "Время работы алгоритма: " << seconds << " секунд\n";

    cout << "Запусков branches_boundaries: " << ok << "\n";
    return 0;
}
/*
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2) (1<->4=2) (1<->3=0) (1<->3=0)  sp_tree:
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2) (1<->4=2) (1<->3=0)  sp_tree:
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2) (1<->4=2)  sp_tree:
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2)  sp_tree:
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree:
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree:
eq_edges: (2<->4=0) (2<->4=0)  sp_tree:
eq_edges: (2<->4=0)  sp_tree:
eq_edges: (2<->4=0)  sp_tree: (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (2<->3=1) (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (2<->3=1) (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2) (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2) (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->4=2) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2) (1<->4=2)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2) (1<->4=2) (1<->3=0)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2) (1<->4=2)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (2<->4=0)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (2<->3=1)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1) (1<->4=2)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0) (2<->3=1)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0) (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)
eq_edges: (2<->4=0)  sp_tree: (1<->3=0) (1<->4=2)

Максимальное кол-во сбалансированных циклов: 3
max_spanning_tree(Остовное дерево):
(1 <-> 4) = 2
(2 <-> 3) = 1
(2 <-> 4) = 0

Общее время работы поиска сбалансированных циклов: 0 процессорных тиков
Общее время работы поиска сбалансированных циклов: 0 секунд
Время работы алгоритма: 0.32 секунд
Запусков branches_boundaries: 80
*/