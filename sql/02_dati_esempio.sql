-- Eseguire una sola volta, su database privo di dati.
PRAGMA foreign_keys = ON;
BEGIN TRANSACTION;
INSERT INTO categorie (id, nome) VALUES
    (1, 'Alimentari'), (2, 'Trasporti'), (3, 'Svago'), (4, 'Salute');
INSERT INTO spese (data, importo_centesimi, categoria_id, descrizione)
VALUES
    ('2026-01-03', 12050, 1, 'Spesa supermercato'),
    ('2026-01-15', 20000, 1, 'Spesa famiglia'),
    ('2026-01-05',  5000, 2, 'Abbonamento mezzi'),
    ('2026-01-20',  7000, 2, 'Carburante'),
    ('2026-01-21',  2500, 3, 'Cinema'),
    ('2026-02-02',  4500, 1, 'Spesa febbraio');
INSERT INTO budget (mese, categoria_id, importo_centesimi) VALUES
    ('2026-01', 1, 30000), ('2026-01', 2, 12000),
    ('2026-01', 4, 10000), ('2026-02', 1, 20000);
COMMIT;
