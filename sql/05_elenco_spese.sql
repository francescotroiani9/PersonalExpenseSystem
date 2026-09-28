SELECT s.data, c.nome, s.importo_centesimi, s.descrizione
FROM spese AS s
JOIN categorie AS c ON c.id = s.categoria_id
ORDER BY s.data ASC, s.id ASC;
