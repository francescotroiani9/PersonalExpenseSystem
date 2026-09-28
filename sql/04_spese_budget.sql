-- ?1 e' il mese YYYY-MM, associato dal programma con un parametro.
WITH speso AS (
    SELECT categoria_id, SUM(importo_centesimi) AS totale
    FROM spese WHERE substr(data, 1, 7) = ?1
    GROUP BY categoria_id
)
SELECT c.nome, b.importo_centesimi, COALESCE(s.totale, 0)
FROM categorie AS c
LEFT JOIN budget AS b ON b.categoria_id = c.id AND b.mese = ?1
LEFT JOIN speso AS s ON s.categoria_id = c.id
ORDER BY c.nome COLLATE NOCASE;
