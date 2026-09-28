-- SQLite 3.24 o successivo. Il database e' il file aperto dal client.
PRAGMA foreign_keys = ON;
BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS categorie (
    id INTEGER PRIMARY KEY,
    nome TEXT NOT NULL COLLATE NOCASE UNIQUE,
    CHECK (typeof(nome) = 'text' AND length(nome) BETWEEN 1 AND 80
           AND nome = trim(nome))
);

CREATE TABLE IF NOT EXISTS spese (
    id INTEGER PRIMARY KEY,
    data TEXT NOT NULL,
    importo_centesimi INTEGER NOT NULL,
    categoria_id INTEGER NOT NULL,
    descrizione TEXT NOT NULL DEFAULT '',
    FOREIGN KEY (categoria_id) REFERENCES categorie(id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    CHECK (typeof(importo_centesimi) = 'integer'
           AND importo_centesimi BETWEEN 1 AND 1000000000),
    CHECK (length(descrizione) <= 200),
    CHECK (
        data GLOB '[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]'
        AND CAST(substr(data, 1, 4) AS INTEGER) BETWEEN 1 AND 9999
        AND CAST(substr(data, 6, 2) AS INTEGER) BETWEEN 1 AND 12
        AND CAST(substr(data, 9, 2) AS INTEGER) BETWEEN 1 AND
        CASE CAST(substr(data, 6, 2) AS INTEGER)
            WHEN 2 THEN 28 + (
                CAST(substr(data, 1, 4) AS INTEGER) % 400 = 0
                OR (CAST(substr(data, 1, 4) AS INTEGER) % 4 = 0
                    AND CAST(substr(data, 1, 4) AS INTEGER) % 100 <> 0))
            WHEN 4 THEN 30 WHEN 6 THEN 30
            WHEN 9 THEN 30 WHEN 11 THEN 30 ELSE 31
        END
    )
);

CREATE TABLE IF NOT EXISTS budget (
    id INTEGER PRIMARY KEY,
    mese TEXT NOT NULL,
    categoria_id INTEGER NOT NULL,
    importo_centesimi INTEGER NOT NULL,
    FOREIGN KEY (categoria_id) REFERENCES categorie(id)
        ON UPDATE CASCADE ON DELETE RESTRICT,
    UNIQUE (mese, categoria_id),
    CHECK (typeof(importo_centesimi) = 'integer'
           AND importo_centesimi BETWEEN 1 AND 1000000000),
    CHECK (mese GLOB '[0-9][0-9][0-9][0-9]-[0-9][0-9]'
           AND CAST(substr(mese, 1, 4) AS INTEGER) BETWEEN 1 AND 9999
           AND CAST(substr(mese, 6, 2) AS INTEGER) BETWEEN 1 AND 12)
);

CREATE INDEX IF NOT EXISTS idx_spese_categoria_data
    ON spese(categoria_id, data);
COMMIT;
