# Website — Antitotem — Objeto Sonoro

Site editorial próprio: separado da interface JUCE e do portal RASGO.

## Estado atual (25 ago. 2026)

Primeira página implementada: `index.html` (PT, canônica) + `en.html`/`fr.html`/
`es.html`, com `styles.css` própria (paleta extraída do app real, não a do portal
geral). Escopo deliberadamente reduzido a uma landing page de instalação/
getting-started — requisitos mínimos, instalação (`.deb` e build do zero), status do
projeto — porque o gate de publicação do Antitotem ainda não fechou (ver
`../docs/TAREFAS.md`). O mapa completo abaixo é a direção futura, não o estado atual.

## Mapa completo (direção futura, pós-gate de publicação)

1. Objeto Sonoro: manifesto, materialidade e origem histórica.
2. Campo modular: fontes, portas, mixer, memória e variações autorais.
3. Escuta: takes WAV e configurações.
4. Tutoriais.
5. Pesquisa, créditos e licenças.

## Fontes

`assets/fonts/` self-hospeda DejaVu Sans e DejaVu Sans Mono (regular/bold,
Bitstream Vera License, livre pra embutir), carregadas via `@font-face` em
`styles.css`. Achado ao vivo, 26 ago. 2026: antes só o *nome* da fonte era
citado no CSS, sem arquivo embutido — comum estar instalada no Linux, rara
no Windows/macOS, então o site renderizava com fallback diferente conforme
o sistema do visitante (mesmo bug achado no site do Navalha 2 no mesmo dia).
Regra da família RASGO desde então: sites só usam fontes livres, sempre
embutidas, nunca só citadas pelo nome esperando que estejam instaladas.

## Regras

- Esquemas externos exigem autoria, fonte e licença declaradas.
- Módulos de software são estudos autorais, não reconstruções.
- Objeto Sonoro 5 permanece núcleo histórico.
- PT é fonte canônica; EN/FR/ES já implementados na página atual.
