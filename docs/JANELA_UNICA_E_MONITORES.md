# Janela única (PRINCIPAL/CLONE) e modos de monitor

Especificação combinada em conversa com o autor em 12–13 ago. 2026,
implementada em 13–14 ago. 2026: auditoria concluída (com a correção do
MASTER que ela revelou), cabeçalho único, toggle de corpo PRINCIPAL/CLONE,
ÓRBITA/PÊNDULO e modo dual monitor, todos no ar. Ver "Ainda pendente" no
fim do documento para o que resta (repaginação fina de colunas).

## Auditoria: comum vs. único por objeto

Verificado item a item no código-fonte (`SimpleSequencer.h`/`.cpp`,
`DualObjectEngine.h`/`.cpp`, `CmosVoice.h`), não só por inspeção da UI.

### Genuinamente único no motor inteiro (vai para o cabeçalho)

| Item | Confirmado por |
|---|---|
| PLAY / STOP / RESET | `DualObjectEngine::setRunning()` já liga os dois objetos juntos; RESET já chama `reset()` dos dois |
| REC (+ REC TIMERS) | `WavRecorder::write()` recebe o buffer pós-`dualEngine.render()`, já somado |
| Osciloscópio + GANHO Y | `stereoScope.push()` recebe o mesmo buffer pós-`dualEngine.render()` |
| **MASTER** | **Corrigido nesta sessão** — estava errado, chamava `sequencer.setMasterGain()` (por-objeto, só afetava o PRINCIPAL). `DualObjectEngine` tem seu próprio `OutputStage` final, aplicado à soma já misturada dos dois objetos, mas nunca teve `setMasterGain` exposto (ficava fixo em 0.72 interno). Adicionado `DualObjectEngine::setMasterGain()`, MASTER religado para ele. |
| TUTORIAL / SOBRE / idioma | chrome do app, não é estado do motor |
| título / rodapé (versão, créditos) | idem |

### Genuinamente por-objeto (fica no corpo, já duplicado ou a duplicar)

Cada `SimpleSequencer` (`sequencer`/`fifth`) tem seu próprio estado — nenhum
destes é compartilhado no motor, mesmo quando ainda não foi duplicado na UI:

| Item | Status na UI hoje |
|---|---|
| ENERGIA | já duplicado (`voice.setEnergy()` é por-voz) — **não é** único, ao contrário do que eu havia dito antes de conferir o código |
| CLOCK / PULSO / MÉTRICA / PERCURSO | já duplicado |
| FIM DO LOOP | já duplicado (implementado nesta sessão) |
| PORTAS DE FEEDBACK + FB GAIN | já duplicado |
| DERIVA | já duplicado (implementado nesta sessão); passa pra coluna esquerda nas duas visualizações |
| Osciladores (FREQ/MIX/FORMA/EIXO X/Y/Z) | já duplicado |
| VCF (FREQ/RES/CV + modo) | já duplicado |
| ADSR | já duplicado |
| MODULAÇÃO (LFO/RING/NOISE MIX) + FORMA LFO | já duplicado |
| ESPAÇO/FASE (REVERB/PHASER/FLANGER) + ROTAS ATIVAS (9, incl. RES MIX/ALTURA/CORPO) | já duplicado |
| Mixer (FILTRO/RING/RUÍDO/ESPAÇO) + MEMÓRIA MIX | já duplicado, memórias separadas por objeto |
| NOISE (seletor + S&H) | já duplicado |
| Núcleo (40106/8038/4069UB) | já duplicado |
| 16 steps (CV/AMP/FX/MUTE) | já duplicado |
| PULSO / POROSA / HETERÓDINA / RND 16 (variações) | **não duplicado** — só existe pro PRINCIPAL, mexe em estado por-objeto (steps, razões, rotas). Pendente. |
| ÓRBITA / PÊNDULO (novos) | a implementar já duplicados desde o início, nas duas visualizações |

### Nem comum nem por-objeto: descreve a relação entre os dois

| Item | Observação |
|---|---|
| CONEXÃO ENTRE OBJETOS | já confirmado: zerado por padrão, zero efeito antes do primeiro uso, não precisa de estado "desligado" visual separado. Fica fixo acima do LOG, não entra no cabeçalho nem no corpo alternável. |

### Precisa de decisão pequena, ainda em aberto

- **`flow`** (linha de status hoje usada por REC e por mensagens de fase do
  DERIVA): REC vira single/cabeçalho, DERIVA é por-objeto — os dois
  destinos não combinam mais num único label solto. Recomendação: dobrar
  `flow` dentro do LOG (que já recebe as mesmas mensagens via `appendLog`
  de qualquer forma) em vez de manter os dois em paralelo. Não decidido
  ainda, só uma recomendação registrada aqui.

## Motivação

Hoje o CLONE é uma `DocumentWindow` própria e sempre separada
(`ObjectFiveWindow`), aberta por cima da janela principal, na mesma tela.
O autor quer dois modos de operação, escolhidos pelo próprio usuário:

- **Single monitor:** tudo numa única janela, PRINCIPAL e CLONE alternando no
  mesmo espaço.
- **Dual monitor(s):** CLONE abre numa janela própria, no segundo monitor,
  visível ao mesmo tempo que o PRINCIPAL.

Consultado o documento-base
[`RASGO_DOCUMENTATION/design/INTERFACES_E_LAYOUTS.md`](../../RASGO_DOCUMENTATION/design/INTERFACES_E_LAYOUTS.md)
(seção 5.1, "Contrato responsivo comum RASGO"): a política padrão do RASGO é
sempre um único monitor, nunca abrir no retângulo virtual que soma vários
monitores. O modo dual monitor aqui é uma **divergência deliberada e
documentada**, permitida pela própria seção 6 do documento-base
("divergências artísticas documentadas localmente, sem enfraquecer segurança
de bounds, DPI ou acessibilidade"). Esta divergência precisa ser registrada
também em `docs/design.md`-equivalente do Antitotem (hoje `DESIGN.md`) quando
a implementação avançar.

## Decisão técnica central: manter a duplicidade de componentes

`MainComponent` e `ObjectFiveComponent` continuam sendo duas árvores de
componentes JUCE **totalmente separadas e independentes**, cada uma com seu
próprio conjunto de osciladores, VCF, ADSR, sequenciador, portas de feedback,
mixer, modulação, efeitos, DERIVA, CLOCK/PULSO/MÉTRICA/PERCURSO, FIM DO LOOP —
exatamente como já está implementado hoje, sem unificar num único conjunto de
widgets "redirecionável" entre `sequencer`/`fifth`.

**Motivo:** um componente JUCE só pode pertencer a uma janela por vez. O modo
dual monitor exige PRINCIPAL e CLONE renderizando *ao mesmo tempo*, em janelas
diferentes, cada um respondendo de verdade ao toque — isso é impossível com um
único conjunto de widgets compartilhado. Manter a duplicação (já implementada)
resolve os dois modos com o mesmo código: no single monitor, as duas árvores
existem na mesma janela e alternam visibilidade; no dual monitor, cada árvore
vai para sua própria janela/monitor.

Essa duplicação já mapeia razoavelmente bem o que é de fato único por objeto
(cada `SimpleSequencer` — `sequencer`/`fifth` — tem estado próprio: steps,
osciladores, rotas de feedback, clock) vs. o que é single-instance no motor
inteiro (a saída final já somada em `DualObjectEngine::render()`, antes do
`OutputStage` único): MASTER, REC/WavRecorder, PLAY/STOP/RESET, osciloscópio.
Isso já vivia só no `MainComponent` mesmo antes desta conversa — não por
acaso, mas porque tecnicamente é single-instance no motor.

## Modo single monitor

### Cabeçalho (único, fixo, sempre visível)

Contém apenas o que já é single-instance no motor (não precisa duplicar nem
redirecionar nada):

- PLAY / STOP / RESET
- REC + REC TIMERS — **nova posição:** ao lado direito do slider do
  osciloscópio (GANHO Y); o osciloscópio fica mais estreito para abrir espaço
- MASTER
- Osciloscópio + GANHO Y
- Toggle **single/dual monitor(s)** (nome exato dos dois estados a definir —
  conferir se é "monitor" ou "monitores" no plural)
- Toggle **CLONE/PRINCIPAL** (alterna o corpo, ver abaixo)
- TUTORIAL / SOBRE / seletor de idioma (já existem, permanecem)

Os dois toggles ficam dentro do cabeçalho, junto com o resto — não soltos em
outro lugar da aba.

### Canto inferior direito (único, fixo)

- LOG, como já está hoje, mas passa a refletir ações dos dois objetos
  (PRINCIPAL e CLONE), não só do PRINCIPAL.
- **Acima do LOG:** CONEXÃO ENTRE OBJETOS (PRINCIPAL→CLONE, CLONE→PRINCIPAL,
  AUX→PRINCIPAL, AUX→CLONE, ROTA PRINCIPAL→CLONE, ROTA CLONE→PRINCIPAL) — não
  entra no cabeçalho (ficaria cheio demais); ganha esse espaço fixo próprio,
  sempre visível independente do toggle CLONE/PRINCIPAL, já que descreve a
  *relação* entre os dois objetos, não pertence só a um deles.
  - Confirmado nesta conversa: os sliders de CONEXÃO ENTRE OBJETOS já
    começam em `0.0f` tanto na UI quanto no `DualObjectEngine` (não existe
    JIT/lazy gotcha nisso) — **não precisam de um estado visual "desligado"
    separado**, o zero já é o desligado. Candidatos naturais ao destaque
    âmbar de "knob silencioso" (mesma convenção de RING/NOISE/REVERB), se
    fizer sentido mais adiante.

### Corpo (abaixo do cabeçalho, alterna com o toggle CLONE/PRINCIPAL)

- Mostra/esconde a árvore já existente de cada objeto (`MainComponent`'s own
  vs `ObjectFiveComponent`'s own) — **não** redireciona um conjunto único de
  widgets.
- **Geometria idêntica nos dois estados:** mesmas posições, mesmos tamanhos,
  mesmos espaçamentos, mesma ergonomia — a troca é só de conteúdo/valor/cor,
  nunca de layout. As duas árvores recebem os mesmos bounds; só uma fica
  visível por vez.
- **Mudança de cor ao alternar:** um esquema de cor diferente para PRINCIPAL
  vs. CLONE, como reforço visual didático de qual estágio está sendo editado
  (já que os controles se parecem, a cor vira o sinal principal de "onde você
  está").

### Coluna esquerda (nas duas visualizações, principal e clone)

- **DERIVA migra:** sai da fileira de variação no cabeçalho (onde hoje fica
  ao lado de CLONE/PULSO/POROSA/HETERÓDINA/RND16) e passa para a coluna
  esquerda, no mesmo formato que já existe no CLONE — slider de profundidade
  em cima, botão embaixo — tanto na visualização do PRINCIPAL quanto na do
  CLONE (que já está assim).

### Fileira de variação (hoje no cabeçalho, grid 2×3)

Com CLONE e DERIVA saindo dali, sobram 4 dos 6 espaços (PULSO, POROSA,
HETERÓDINA, RND16). Decidido preencher os 2 espaços livres com dois modelos
novos, em vez de deixar vazio:

- **ÓRBITA** — primeira variação a de fato ligar EIXO Y (proximidade) e EIXO Z
  (órbita) nos 5 osciladores. Regime lento, respirante, espacial — oposto do
  PULSO. Clock lento, energia moderada, feedback leve via DIRETO, ruído
  desligado, reverb suave.
- **PÊNDULO** — primeira variação a colocar o comb/resonador (RES MIX/ALTURA/
  CORPO) como protagonista, não acessório. Caráter percussivo/ressoante.
  RES MIX alto, RES ALTURA média-aguda, RES CORPO longo, feedback via PULSO,
  reverb/phaser/flanger mínimos. Nome ecoa a direção de scanner *pendulum*
  já existente no motor (`ScannerDirection::pendulum`) — cogitar setar
  PERCURSO para pendulum neste preset, para reforçar a coerência.

Parâmetros exatos de ÓRBITA/PÊNDULO ainda por fechar em detalhe fino na hora
da implementação (valores acima são a direção, não a especificação final).

### Duplicação pendente: variações e RND 16 para o CLONE

PULSO, POROSA, HETERÓDINA, RND16 (e os dois novos, ÓRBITA/PÊNDULO) hoje só
existem para `sequencer` (PRINCIPAL) — nunca foram duplicados para `fifth`
(CLONE), mesmo mexendo em estado por-objeto (steps, razões de osciladores,
rotas de feedback, ruído) que já se encaixa na mesma categoria de DERIVA e
FIM DO LOOP (ambos já duplicados nesta sessão). Precisam de uma versão
operando sobre `fifth`.

## Modo dual monitor(s) — opt-in, só quando 2+ monitores existem

- Toggle no cabeçalho (ver acima), só visível/habilitado quando
  `juce::Desktop::getInstance().getDisplays()` reporta mais de um display.
- Com o toggle ligado, o botão/toggle CLONE passa a abrir (ou trazer à
  frente) uma janela própria posicionada no **segundo monitor**, preenchendo
  a área útil real *daquele* monitor (`display->userBounds`, não uma escala
  do monitor 1) — seguindo o mesmo padrão que `MainWindow` já usa para se
  posicionar no monitor primário.
- Resoluções diferentes entre os dois monitores: o CLONE já tem o mecanismo
  de fallback (`ObjectFiveWindow::applyContentForCurrentSize()`, que já
  alterna entre painel direto e `ObjectFiveViewport` com rolagem conforme o
  tamanho real da janela) — não precisa de lógica nova, só garantir que ele
  reage ao tamanho real do segundo monitor, que pode ser menor que o
  primeiro.
- Persistência da escolha (single/dual) via `juce::ApplicationProperties`
  (infraestrutura já existe no código, `applicationProperties` em
  `MainWindow`), nunca em estado de projeto musical.

## Concluído em 13–14 ago. 2026

- **Cabeçalho único**: CLONE, toggle de monitor, MASTER, PLAY/STOP/RESET/REC
  + REC TIMERS todos migraram para o cabeçalho, nas duas abas.
- **Toggle de corpo PRINCIPAL/CLONE**: `setShowingCloneBody()` em
  `MainComponent` — CLONE alterna para dentro da mesma janela (mesma
  geometria que PRINCIPAL usa, só visibilidade muda) quando o modo dual
  monitor não está ativo; abre `ObjectFiveWindow` no segundo monitor quando
  está. A cor da borda da janela sinaliza qual corpo está ativo (âmbar/mais
  grossa para CLONE, marrom fina para PRINCIPAL).
- **Repaginação de colunas esquerda e direita**: concluída nas duas abas,
  incluindo a extração de duas funções livres compartilhadas
  (`layoutRailsBand()`, `layoutTransportColumn()`) usadas por
  `MainComponent` e `ObjectFiveComponent` — elimina a classe de bug onde as
  duas abas divergiam por cópias ajustadas manualmente em separado (vários
  casos reais encontrados e corrigidos nesta rodada: grade PORTAS DE
  FEEDBACK, altura do botão DERIVA, rótulos do ADSR e de PULSO/MÉTRICA/
  PERCURSO).
- **CONEXÃO ENTRE OBJETOS**: migrado de CLONE para `MainComponent`, fixo
  acima do LOG, nunca duplicado na janela do CLONE nem no segundo monitor.
- Nome dos dois estados do toggle de monitor: "1 MONITOR" / "2 MONITORES".
- Parâmetros finais de ÓRBITA e PÊNDULO — implementados e testados.

## Ainda pendente / não decidido

- Dois bugs reais encontrados e corrigidos durante o teste ao vivo do
  toggle de corpo (documentados no commit `fc607a5`): botões SOM/SEQUÊNCIA/
  MIX voltando a aparecer a cada troca para PRINCIPAL, e um flash de um
  frame no lançamento — ambos corrigidos, mas vale reconfirmar após
  qualquer mudança futura em `setShowingCloneBody()`/`setPage()`.

## Explicitamente descartado por ora (não implementar agora)

Cogitado e revertido nesta mesma conversa — mantido aqui só como registro,
não como pendência ativa:

- CLONE deixar de estar ativo/audível desde o primeiro PLAY, passando a só
  iniciar (osciladores e motor) no primeiro clique do botão/toggle CLONE.
- STOP e/ou RESET específicos do CLONE, independentes do PRINCIPAL (hoje
  PLAY/STOP/RESET controlam os dois objetos juntos via `DualObjectEngine`,
  e cada `SimpleSequencer` já tem `running`/`reset()` próprios internamente,
  então a decoupling seria tecnicamente simples, mas foi adiada).

Se algum dia isso for retomado: notar que antes de reverter, confirmamos que
os osciladores do CLONE já tocam desde o primeiro PLAY mesmo sem o
`ObjectFiveWindow`/`ObjectFiveComponent` nunca terem sido construídos —
`DualObjectEngine::render()` roda `fifth.renderSample()` incondicionalmente.
O que de fato fica inerte até o primeiro clique em CLONE é só a conexão/
roteamento entre os dois objetos (ver seção sobre CONEXÃO ENTRE OBJETOS
acima), não a voz do CLONE em si.
