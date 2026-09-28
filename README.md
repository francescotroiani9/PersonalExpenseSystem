# Sistema di gestione delle spese personali e del budget

Progetto console per un singolo utente, realizzato in **C++17 e SQLite** in
conformità alla traccia `ELABORATO_DEL_2026_01_21_15_49_27.pdf`.
I menu utilizzano `cout`, `getline(cin, ...)`, `switch` e cicli `while`.

**Repository GitHub personale: [INSERIRE QUI IL LINK COMPLETO].**
Lo stesso link deve essere inserito nel PDF prima della consegna: la traccia
considera incompleto il PDF privo del link. Nessun repository è stato pubblicato.

## Contenuto del repository

```text
PersonalExpenseSystem/
  src/main.cpp
  sql/01_schema.sql
  sql/02_dati_esempio.sql
  sql/03_totali_categoria.sql
  sql/04_spese_budget.sql
  sql/05_elenco_spese.sql
  demo/demo_video.mp4
  README.md
```

Il repository contiene solo sorgente, SQL, demo e README. Il PDF, il database
di esempio e la copia modificabile dell'elaborato sono consegnati separatamente.
Non caricare su GitHub eseguibili, database personali o strumenti temporanei.

## Requisiti

- Compilatore con supporto C++17: GCC, Clang o MinGW-w64.
- SQLite **3.24.0 o successivo**: intestazione `sqlite3.h` e libreria `sqlite3`.
- Terminale UTF-8 e permesso di scrittura nella cartella del database.
- Il client `sqlite3` è opzionale: il programma crea da solo le tabelle.
- Librerie standard: algorithm, cctype, fstream, iomanip, iostream, sstream,
  stdexcept, string. Unica dipendenza esterna: API C di SQLite.

## Compilazione ed esecuzione

Aprire un terminale **nella cartella PersonalExpenseSystem** estratta dallo ZIP.
La sottocartella `sql` deve rimanere disponibile durante l'esecuzione.

### Linux (Debian/Ubuntu)

```sh
sudo apt update
sudo apt install g++ libsqlite3-dev sqlite3
g++ -std=c++17 -Wall -Wextra -Wpedantic src/main.cpp -lsqlite3 -o spese
./spese
```

### macOS

Installare gli strumenti da riga di comando Apple, se mancanti, e attendere
il completamento dell'installazione. Poi compilare:

```sh
xcode-select --install
clang++ -std=c++17 -Wall -Wextra -Wpedantic src/main.cpp -lsqlite3 -o spese
./spese
```

Se gli strumenti Apple sono già installati, omettere il primo comando.

### Windows (terminale MSYS2 UCRT64)

Installare MSYS2 e aprire il terminale **UCRT64**, quindi:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sqlite3
g++ -std=c++17 -Wall -Wextra -Wpedantic src/main.cpp -lsqlite3 -o spese.exe
./spese.exe
```

Usare il terminale UCRT64 anche per avviare il programma, così le librerie sono
reperibili. Linux e Windows sono istruzioni di portabilità; il collaudo effettivo
è stato svolto su macOS arm64.

### Database vuoto, esempi e riavvio

```sh
./spese
./spese --db esempio.db --demo
./spese --db esempio.db
./spese --help
```

Il primo comando crea `spese.db` se assente. Il secondo crea `esempio.db` e
carica quattro categorie, sei spese e quattro budget **solo se tutte le tabelle
sono vuote**. Ripetere `--demo` su un database con dati non li duplica né li
sovrascrive. Il terzo comando riapre i dati già salvati.

Per avviare da un'altra cartella, specificare il percorso degli SQL:

```sh
./spese --db /percorso/dati/spese.db --sql-dir /percorso/PersonalExpenseSystem/sql
```

Sostituire i percorsi con quelli reali; la cartella del database deve esistere.
Per un database già compilato di esempio, copiare `spese_esempio.db` dalla
consegna nella cartella del progetto e avviare:

```sh
./spese --db spese_esempio.db
```

### Creazione manuale facoltativa con SQL

Su un nuovo file, dal terminale nella cartella del progetto:

```sh
sqlite3 manuale.db < sql/01_schema.sql
sqlite3 manuale.db < sql/02_dati_esempio.sql
./spese --db manuale.db
```

SQLite crea il database aprendo il file: non usa `CREATE DATABASE`.
I dati di esempio vanno eseguiti una sola volta su tabelle vuote. Gli script
attivano `PRAGMA foreign_keys = ON`: in SQLite va attivato per ogni connessione.
L'applicazione lo fa automaticamente. Non usare un database di un altro progetto.

## Funzioni e utilizzo

Menu principale:

1. **Gestione Categorie**: inserisce un nome non vuoto e non duplicato.
2. **Inserisci Spesa**: richiede data `YYYY-MM-DD`, importo, categoria esistente
   e descrizione facoltativa (Invio per lasciarla vuota).
3. **Definisci Budget Mensile**: richiede `YYYY-MM`, categoria e importo; aggiorna
   il budget se la coppia mese/categoria esiste già.
4. **Visualizza Report**: apre il sottomenu.
5. **Esci**: chiude il programma. I dati restano nel file SQLite.

Sottomenu report: 1 totale per categoria su tutte le date; 2 confronto spese/budget
del mese richiesto; 3 elenco completo ordinato per data crescente e ID a parità di
data; 4 ritorno al menu principale. Le scelte errate mostrano
`Scelta non valida. Riprovare.` e consentono un nuovo tentativo.

Le validazioni fallite non salvano dati; tornare al modulo per ripetere
l'inserimento. EOF chiude senza salvare un inserimento incompleto. I problemi
SQL durante un'operazione sono segnalati e si ritorna al menu principale.

## Regole dei dati

- Importi memorizzati in **centesimi interi**, senza errori da virgola mobile.
  Si accettano `12`, `12.3`, `12.34`, `12,34`; niente separatori delle migliaia.
  Intervallo per spesa/budget: 0,01-10.000.000,00 euro, massimo due decimali.
- Date reali del calendario gregoriano, anni 0001-9999, inclusi i bisestili.
- Le categorie sono uniche con confronto `NOCASE` di SQLite: ignora le
  maiuscole/minuscole ASCII, non normalizza tutte le lettere Unicode accentate.
- L'interfaccia elimina spazi esterni; nomi fino a 80 byte UTF-8 e descrizioni
  fino a 200 byte, senza caratteri di controllo. Il database impone limiti di
  80 e 200 caratteri; l'interfaccia applica quindi un limite più restrittivo
  per stringhe multibyte. Descrizione facoltativa salvata come stringa vuota.
- Le query parametrizzate consentono apostrofi senza concatenare input nell'SQL.
- Chiavi esterne con `ON DELETE RESTRICT`: le categorie utilizzate non sono
  cancellabili via SQL. La traccia non richiede funzioni di modifica/cancellazione
  delle spese o delle categorie, quindi il menu non le aggiunge.

## Schema logico

```text
CATEGORIE(id PK, nome NOT NULL UNIQUE NOCASE)
    1 ---- 0..N SPESE
    1 ---- 0..N BUDGET

SPESE(id PK, data NOT NULL, importo_centesimi NOT NULL,
      categoria_id NOT NULL FK -> CATEGORIE.id,
      descrizione NOT NULL DEFAULT '')

BUDGET(id PK, mese NOT NULL, categoria_id NOT NULL FK -> CATEGORIE.id,
       importo_centesimi NOT NULL, UNIQUE(mese, categoria_id))
```

Ogni spesa e budget appartiene a una sola categoria. Le categorie possono
esistere senza spese e senza budget. I `CHECK` completi sono in `01_schema.sql`.

## Risultati attesi sui dati di esempio

Report 1: Alimentari 365.50 EUR; Salute 0.00; Svago 25.00; Trasporti 120.00.
Il totale Alimentari comprende gennaio e febbraio.

Report 2 per `2026-01`:

| Categoria | Budget EUR | Speso EUR | Stato |
|---|---:|---:|---|
| Alimentari | 300.00 | 320.50 | SUPERAMENTO BUDGET |
| Salute | 100.00 | 0.00 | ENTRO IL BUDGET |
| Svago | non definito | 25.00 | BUDGET NON DEFINITO |
| Trasporti | 120.00 | 120.00 | BUDGET RAGGIUNTO |

Il report 2 usa tutte le categorie, anche senza spese. Un budget assente non
equivale a zero. Lo stato viene deciso in C++ tramite `if / else`, dopo
l'aggregazione SQL. Le tre query dei report sono nei file SQL 03, 04 e 05 e
sono lette direttamente dall'applicazione. In 04 il parametro `?1` riceve il mese.

## Demo video

`demo/demo_video.mp4` mostra l'esecuzione reale del programma compilato in un
terminale: avvio con esempi, errore di scelta, inserimento della categoria
Libri, spesa di 35.50 EUR, budget 30.00 EUR, tutti i report, aggiornamento del
budget a 50.00 EUR e uscita. Il video riproduce l'output e gli input acquisiti
da una sessione pseudo-terminale, con titoli esplicativi; non è una simulazione
delle risposte del programma. Non contiene audio.

## Verifiche eseguite

Compilazione effettiva su macOS arm64 con il compilatore C++/Clang di Zig 0.13.0,
SDK macOS 14.5 e libreria SQLite di sistema; C++17, `-Wall -Wextra -Wpedantic -Werror`:
nessun avviso. Gli strumenti temporanei non sono dipendenze del progetto.

**47 controlli automatici superati** su SQL ed eseguibile: integrità database,
chiavi esterne, vincoli, totali/report, aggiornamento senza duplicati, input
invalidi, date bisestili e non valide, importi limite, apostrofi, descrizione
vuota, database vuoto, protezione degli esempi, EOF e persistenza dopo riavvio.
La suite SQL è stata eseguita anche tramite SQLite 3.53.1 di Python, come
strumento esterno di verifica. L'applicazione consegnata è interamente C++.

## Consegna ufficiale

1. Pubblicare il contenuto di questa cartella nel proprio repository GitHub.
2. Inserire il link completo nel README e nel campo dedicato del PDF.
3. Verificare che sorgente, SQL, video e README siano visibili al docente.
4. Consegnare **un solo PDF**, come richiesto dalla traccia. Gli altri file
   allegati servono a preparare il repository e a eseguire il progetto.
