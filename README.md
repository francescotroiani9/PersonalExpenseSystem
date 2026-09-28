# Gestione delle spese personali e del budget

Programma da console in C++ con database SQLite. Permette di registrare le
spese, dividerle in categorie e confrontarle con un budget mensile.

Repository GitHub: **https://github.com/francescotroiani9/PersonalExpenseSystem**

## File del progetto

- `src/main.cpp`: codice del programma.
- `sql/01_schema.sql`: creazione delle tabelle e dei vincoli.
- `sql/02_dati_esempio.sql`: dati di esempio.
- `sql/03_totali_categoria.sql`, `04_spese_budget.sql`, `05_elenco_spese.sql`:
  query dei tre report.
- `demo/demo_video.mp4`: video dimostrativo.

## Requisiti

- Compilatore C++17, come GCC o Clang.
- SQLite 3.24 o successivo, con il file `sqlite3.h` e la libreria `sqlite3`.
- Terminale UTF-8.

Il codice usa le librerie standard `algorithm`, `cctype`, `fstream`, `iomanip`,
`iostream`, `sstream`, `stdexcept` e `string`.

## Compilazione e avvio

Aprire il terminale nella cartella `PersonalExpenseSystem`. La cartella `sql`
deve restare presente: il programma la usa per creare le tabelle e leggere
le query dei report.

### Linux (Debian/Ubuntu)

Installare i requisiti, se mancanti, poi compilare e avviare:

```sh
sudo apt update
sudo apt install g++ libsqlite3-dev
g++ -std=c++17 src/main.cpp -lsqlite3 -o spese
./spese
```

### macOS

Se mancano gli strumenti di sviluppo, eseguire `xcode-select --install`
e attendere la fine dell'installazione. Poi:

```sh
clang++ -std=c++17 src/main.cpp -lsqlite3 -o spese
./spese
```

### Windows con MSYS2

Aprire il terminale **MSYS2 UCRT64**, installare i requisiti e compilare:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sqlite3
g++ -std=c++17 src/main.cpp -lsqlite3 -o spese.exe
./spese.exe
```

Usare lo stesso terminale anche per gli avvii successivi.

## Dati di esempio

Il programma crea automaticamente il database `spese.db` al primo avvio.
Per provare il programma con dati già inseriti, usare:

```sh
./spese --db esempio.db --demo
```

Gli esempi vengono caricati solo se il database è vuoto. Per riaprire gli stessi
dati in seguito:

```sh
./spese --db esempio.db
```

Su Windows sostituire `./spese` con `./spese.exe`.

## Utilizzo

Il menu principale contiene cinque opzioni:

1. **Gestione Categorie**: inserisce una categoria con nome non vuoto e non duplicato.
2. **Inserisci Spesa**: richiede data, importo, categoria e descrizione facoltativa.
3. **Definisci Budget Mensile**: inserisce o aggiorna il limite per mese e categoria.
4. **Visualizza Report**: mostra totali per categoria, confronto mensile con il budget
   oppure elenco delle spese ordinate per data.
5. **Esci**: chiude il programma. I dati restano salvati nel database.

Le date devono avere formato `YYYY-MM-DD`, per esempio `2026-01-15`.
Per il budget si usa `YYYY-MM`, per esempio `2026-01`.
Gli importi devono essere positivi, con al massimo due decimali: `12.50` o `12,50`.
La categoria deve essere creata prima di inserire una spesa o un budget.

Per una prova, avviare con i dati di esempio e scegliere il report mensile
per `2026-01`: Alimentari mostra 320,50 euro spesi su un budget di 300 euro,
quindi segnala il superamento del budget.

## Demo

Il video `demo/demo_video.mp4` mostra l'avvio, l'inserimento di una categoria
e di una spesa, la definizione e l'aggiornamento del budget e i report.
