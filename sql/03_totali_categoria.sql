SELECT c.nome, COALESCE(SUM(s.importo_centesimi), 0) AS totale
FROM categorie AS c
LEFT JOIN spese AS s ON s.categoria_id = c.id
GROUP BY c.id, c.nome
ORDER BY c.nome COLLATE NOCASE;
