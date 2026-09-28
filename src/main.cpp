#include <sqlite3.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;
using Money = sqlite3_int64;
constexpr Money MAX_MONEY = 1000000000; // 10.000.000,00 euro
struct EndInput {};

string readFile(const string& path) {
    ifstream file(path);
    if (!file) throw runtime_error("Impossibile leggere: " + path);
    return string(istreambuf_iterator<char>(file), {});
}

class Database {
    sqlite3* handle = nullptr;
public:
    explicit Database(const string& path) {
        if (sqlite3_open(path.c_str(), &handle) != SQLITE_OK) {
            string message = handle ? sqlite3_errmsg(handle) : "Memoria insufficiente";
            sqlite3_close(handle);
            throw runtime_error(message);
        }
        sqlite3_busy_timeout(handle, 3000);
    }
    ~Database() { sqlite3_close(handle); }
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    sqlite3* get() const { return handle; }
    void exec(const string& sql) {
        char* error = nullptr;
        if (sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
            string message = error ? error : sqlite3_errmsg(handle);
            sqlite3_free(error);
            throw runtime_error(message);
        }
    }
};

// RAII: anche in caso di errore ogni statement viene liberato.
class Query {
    sqlite3_stmt* statement = nullptr;
    sqlite3* db;
    void check(int result) {
        if (result != SQLITE_OK) throw runtime_error(sqlite3_errmsg(db));
    }
public:
    Query(Database& database, const string& sql) : db(database.get()) {
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
        if (rc != SQLITE_OK) {
            sqlite3_finalize(statement);
            throw runtime_error(sqlite3_errmsg(db));
        }
    }
    ~Query() { sqlite3_finalize(statement); }
    Query(const Query&) = delete;
    Query& operator=(const Query&) = delete;
    void bind(int index, const string& value) {
        check(sqlite3_bind_text(statement, index, value.c_str(), -1, SQLITE_TRANSIENT));
    }
    void bind(int index, Money value) {
        check(sqlite3_bind_int64(statement, index, value));
    }
    bool next() {
        int rc = sqlite3_step(statement);
        if (rc == SQLITE_ROW) return true;
        if (rc == SQLITE_DONE) return false;
        throw runtime_error(sqlite3_errmsg(db));
    }
    Money number(int col) const { return sqlite3_column_int64(statement, col); }
    bool isNull(int col) const { return sqlite3_column_type(statement, col) == SQLITE_NULL; }
    string text(int col) const {
        const auto* value = sqlite3_column_text(statement, col);
        return value ? reinterpret_cast<const char*>(value) : "";
    }
};

string trim(string s) {
    auto first = s.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

string input(const string& prompt) {
    cout << prompt << flush;
    string s;
    if (!getline(cin, s)) throw EndInput{};
    return trim(s);
}

bool printable(const string& s) {
    return none_of(s.begin(), s.end(), [](unsigned char c) { return c < 32 || c == 127; });
}

int choice() {
    string s = input("Inserisci la tua scelta: ");
    return s.size() == 1 && s[0] >= '0' && s[0] <= '9' ? s[0] - '0' : -1;
}

bool validMonth(const string& s) {
    if (s.size() != 7 || s[4] != '-') return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (i != 4 && (s[i] < '0' || s[i] > '9')) return false;
    int year = stoi(s.substr(0, 4)), month = stoi(s.substr(5, 2));
    return year >= 1 && month >= 1 && month <= 12;
}

bool validDate(const string& s) {
    if (s.size() != 10 || s[7] != '-' || !validMonth(s.substr(0, 7))) return false;
    if (s[8] < '0' || s[8] > '9' || s[9] < '0' || s[9] > '9') return false;
    int year = stoi(s.substr(0, 4)), month = stoi(s.substr(5, 2));
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) days[1] = 29;
    int day = stoi(s.substr(8, 2));
    return day >= 1 && day <= days[month - 1];
}

// Conversione decimale esatta: nessun float/double per il denaro.
bool parseMoney(string s, Money& value) {
    replace(s.begin(), s.end(), ',', '.');
    auto dot = s.find('.');
    string whole = s.substr(0, dot);
    string fraction = dot == string::npos ? "" : s.substr(dot + 1);
    auto digits = [](const string& x) {
        return all_of(x.begin(), x.end(), [](char c) { return c >= '0' && c <= '9'; });
    };
    if (whole.empty() || whole.size() > 8 || !digits(whole) || !digits(fraction)
        || fraction.size() > 2 || (dot != string::npos && fraction.empty())) return false;
    while (fraction.size() < 2) fraction += '0';
    value = stoll(whole) * 100 + stoll(fraction);
    return value > 0 && value <= MAX_MONEY;
}

string euro(Money cents) {
    ostringstream out;
    out << cents / 100 << '.' << setfill('0') << setw(2) << cents % 100;
    return out.str();
}

Money categoryId(Database& db, const string& name) {
    Query q(db, "SELECT id FROM categorie WHERE nome = ?1");
    q.bind(1, name);
    return q.next() ? q.number(0) : 0;
}

void addCategory(Database& db) {
    string name = input("Nome della categoria: ");
    if (name.empty() || name.size() > 80 || !printable(name)) {
        cout << "Errore: nome obbligatorio, massimo 80 byte e nessun carattere di controllo.\n";
        return;
    }
    if (categoryId(db, name)) { cout << "La categoria esiste già.\n"; return; }
    Query q(db, "INSERT INTO categorie(nome) VALUES(?1)");
    q.bind(1, name); q.next();
    cout << "Categoria inserita correttamente.\n";
}

void addExpense(Database& db) {
    string date = input("Data (YYYY-MM-DD): ");
    string amount = input("Importo (euro): ");
    string name = input("Nome della categoria: ");
    string description = input("Descrizione (facoltativa): ");
    Money cents = 0;
    if (!validDate(date)) { cout << "Errore: data non valida.\n"; return; }
    if (!parseMoney(amount, cents)) {
        cout << "Errore: l’importo deve essere maggiore di zero.\n"
             << "Usare al massimo due decimali; massimo 10000000.00 euro.\n";
        return;
    }
    Money id = categoryId(db, name);
    if (!id) { cout << "Errore: la categoria non esiste.\n"; return; }
    if (description.size() > 200 || !printable(description)) {
        cout << "Errore: descrizione troppo lunga (massimo 200 byte) o non valida.\n";
        return;
    }
    Query q(db, "INSERT INTO spese(data, importo_centesimi, categoria_id, descrizione) "
                "VALUES(?1, ?2, ?3, ?4)");
    q.bind(1, date); q.bind(2, cents); q.bind(3, id); q.bind(4, description); q.next();
    cout << "Spesa inserita correttamente.\n";
}

void setBudget(Database& db) {
    string month = input("Mese (YYYY-MM): ");
    string name = input("Nome della categoria: ");
    string amount = input("Importo del budget (euro): ");
    Money cents = 0;
    if (!validMonth(month)) { cout << "Errore: mese non valido.\n"; return; }
    if (!parseMoney(amount, cents)) {
        cout << "Errore: il budget deve essere positivo, con massimo due decimali "
                "e non oltre 10000000.00 euro.\n";
        return;
    }
    Money id = categoryId(db, name);
    if (!id) { cout << "Errore: la categoria non esiste.\n"; return; }
    Query q(db, "INSERT INTO budget(mese, categoria_id, importo_centesimi) "
                "VALUES(?1, ?2, ?3) ON CONFLICT(mese, categoria_id) "
                "DO UPDATE SET importo_centesimi = excluded.importo_centesimi");
    q.bind(1, month); q.bind(2, id); q.bind(3, cents); q.next();
    cout << "Budget mensile salvato correttamente.\n";
}

void reportTotals(Database& db, const string& dir) {
    Query q(db, readFile(dir + "/03_totali_categoria.sql"));
    cout << "Categoria | Totale speso (EUR)\n";
    bool found = false;
    while (q.next()) { found = true; cout << q.text(0) << " | " << euro(q.number(1)) << '\n'; }
    if (!found) cout << "Nessuna categoria presente.\n";
}

void reportBudget(Database& db, const string& dir) {
    string month = input("Mese del report (YYYY-MM): ");
    if (!validMonth(month)) { cout << "Errore: mese non valido.\n"; return; }
    Query q(db, readFile(dir + "/04_spese_budget.sql")); q.bind(1, month);
    cout << "Mese: " << month << '\n';
    bool found = false;
    while (q.next()) {
        found = true;
        cout << "Categoria: " << q.text(0) << "\nBudget: ";
        if (q.isNull(1)) cout << "non definito";
        else cout << euro(q.number(1)) << " EUR";
        cout << "\nSpeso: " << euro(q.number(2)) << " EUR\nStato: ";
        if (q.isNull(1)) cout << "BUDGET NON DEFINITO";
        else if (q.number(2) > q.number(1)) cout << "SUPERAMENTO BUDGET";
        else if (q.number(2) == q.number(1)) cout << "BUDGET RAGGIUNTO";
        else cout << "ENTRO IL BUDGET";
        cout << "\n-------------------------\n";
    }
    if (!found) cout << "Nessuna categoria presente.\n";
}

void reportExpenses(Database& db, const string& dir) {
    Query q(db, readFile(dir + "/05_elenco_spese.sql"));
    cout << "Data | Categoria | Importo (EUR) | Descrizione\n";
    bool found = false;
    while (q.next()) {
        found = true;
        cout << q.text(0) << " | " << q.text(1) << " | "
             << euro(q.number(2)) << " | " << q.text(3) << '\n';
    }
    if (!found) cout << "Nessuna spesa presente.\n";
}

void reports(Database& db, const string& dir) {
    bool running = true;
    while (running) {
        cout << "\nMENU DEI REPORT\n1. Totale spese per categoria\n"
                "2. Spese mensili vs budget\n"
                "3. Elenco completo delle spese ordinate per data\n"
                "4. Ritorna al menu principale\n";
        switch (choice()) {
            case 1: reportTotals(db, dir); break;
            case 2: reportBudget(db, dir); break;
            case 3: reportExpenses(db, dir); break;
            case 4: running = false; break;
            default: cout << "Scelta non valida. Riprovare.\n";
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        string path = "spese.db", dir = "sql";
        bool demo = false;
        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "--demo") demo = true;
            else if ((arg == "--db" || arg == "--sql-dir") && i + 1 < argc) {
                if (arg == "--db") path = argv[++i]; else dir = argv[++i];
            } else if (arg == "--help") {
                cout << "Uso: spese [--db FILE] [--sql-dir CARTELLA] [--demo]\n"
                        "--demo carica esempi solo se il database e' vuoto.\n";
                return 0;
            } else throw runtime_error("Argomento non valido: " + arg);
        }
        if (sqlite3_libversion_number() < 3024000)
            throw runtime_error("Richiesto SQLite 3.24 o successivo");
        Database db(path);
        db.exec("PRAGMA foreign_keys = ON;");
        db.exec(readFile(dir + "/01_schema.sql"));
        if (demo) {
            bool empty;
            {
                Query q(db, "SELECT (SELECT COUNT(*) FROM categorie) + "
                            "(SELECT COUNT(*) FROM spese) + (SELECT COUNT(*) FROM budget)");
                q.next(); empty = q.number(0) == 0;
            }
            if (empty) db.exec(readFile(dir + "/02_dati_esempio.sql"));
            else cout << "Dati di esempio non caricati: database non vuoto.\n";
        }
        cout << "Benvenuto nel sistema di gestione delle spese personali!\n";
        bool running = true;
        while (running) {
            cout << "\n-------------------------\n SISTEMA SPESE PERSONALI\n"
                    "-------------------------\n1. Gestione Categorie\n2. Inserisci Spesa\n"
                    "3. Definisci Budget Mensile\n4. Visualizza Report\n5. Esci\n"
                    "-------------------------\n";
            try {
                switch (choice()) {
                    case 1: addCategory(db); break;
                    case 2: addExpense(db); break;
                    case 3: setBudget(db); break;
                    case 4: reports(db, dir); break;
                    case 5: running = false; break;
                    default: cout << "Scelta non valida. Riprovare.\n";
                }
            } catch (const exception& e) {
                cerr << "Errore operazione: " << e.what() << '\n';
            }
        }
        cout << "Arrivederci!\n";
    } catch (const EndInput&) {
        cout << "\nInput terminato. Chiusura del programma.\n";
    } catch (const exception& e) {
        cerr << "Errore di avvio: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
