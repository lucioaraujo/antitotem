# Tarefas — Antitotem / Objeto Sonoro

**Reorganizado em 24 ago. 2026.** Este arquivo tinha crescido para 8446
linhas de log cronológico contínuo (10-20 ago. 2026), sem distinguir o que
ainda está pendente do que já foi concluído e confirmado. Nada foi apagado —
o log completo, com todo o raciocínio, bugs investigados e citações do
autor, está preservado em
[`TAREFAS_HISTORICO.md`](TAREFAS_HISTORICO.md). Este documento contém só o
que ainda está genuinamente aberto, organizado por tema, com um ponteiro de
volta ao histórico (pelo título da seção — os números de linha mudaram)
para quem quiser o contexto completo por trás de cada item.

Ver também [`docs/TAREFAS_HISTORICO.md`](TAREFAS_HISTORICO.md) para o
registro completo, e [`../RASGO_DOCUMENTATION/PAINEL_MESTRE.md`](../../RASGO_DOCUMENTATION/PAINEL_MESTRE.md)
para o gate de publicação vigente na família RASGO.

## Ponto de retomada — 26 ago. 2026 (fim de sessão — gate de publicação fechado)

**O gate de publicação (seção 1) fechou nesta sessão**: os três itens
(empacotamento `.deb`, usabilidade 1920×1080, avaliação sonora humana) estão
`[x]`. Windows/macOS build+empacotamento já rodam com sucesso via GitHub
Actions (CI real, runners hospedados), mas nunca foram abertos num
Windows/macOS de verdade — o autor decidiu que isso não bloqueia mais a
publicação (ver decisão registrada na seção 1). **Decisão de publicar ainda
não tomada** — é o próximo passo humano, não técnico.

**Concluído nesta sessão** (24-26 ago.), em ordem, resumido (detalhe completo
nas seções 0/1/2 abaixo e nos commits):

1. Reorganização deste arquivo e triagem item a item contra o código atual.
2. Bug real de tempo real no `MidiCapture` (lock bloqueante → `tryEnter()`
   + `reserve()`), validado sob ThreadSanitizer.
3. **Empacotamento `.deb` (Linux)** — gerado, instalado de verdade e
   confirmado abrindo pelo autor.
4. **Site do instrumento** (`ANTITOTEM/website/`) + `INSTALL.md` bilíngue no
   GitHub; logo/favicon redesenhados pelo autor, movidos pra
   `assets/identity/` (compartilhado entre app, `.deb` e site).
5. **Windows/macOS via GitHub Actions**: preparado, `push`ado, e **CI rodou
   com sucesso nas 3 plataformas** (um fix de um `choco install nsis` que
   faltava no runner Windows).
6. **Usabilidade 1920×1080**: confirmada ao vivo pelo autor.
7. **Avaliação sonora humana (4 estudos) — bateria completa, fechada**:
   - Estudo 1 (Pulso/Geada): PASSOU, as 8 subdivisões de MÉTRICA variam.
   - Estudo 2 (Matéria): PASSOU, após investigação funda de ASYMMETRY
     (achado não óbvio: RESONANCE alta reduz o nível de saída do
     `MaterialFilter` em ~13x, "tudo no máximo" é uma das configs mais
     quietas) — autor confirmou diferença real, ainda que sutil por natureza.
   - Estudo 3 (Retorno): PASSOU. Corrigi um erro meu (tinha dito que PORTAS
     DE FEEDBACK precisava do CLONE audível — é interno a um único
     `CmosVoice`, não precisa). Autor testou as 6 rotas, nenhuma incomodou.
   - Estudo 4 (Espaço): PASSOU. **Bug real encontrado e corrigido**: com
     todos os osciladores em MIX=0, o som continuava em 2 dos 3 núcleos
     (40106/schmittPulse e 4069UB/unbufferedDrift) — `CmosVoice::
     tickStereo`'s `shapeCore` lia `oscillatorA`/`B`/`C` brutos, sem
     escalar pelo `oscillatorLevels`. Corrigido, testes/build OK, autor
     confirmou ouvindo. Pan por oscilador, pan dos 4 canais do mixer e os
     4 efeitos (REVERB/PHASER/FLANGER/RESONATOR) todos testados e
     funcionando — a última dúvida ("efeitos sem diferença audível") era
     estado de UI (canal ESPAÇO mudo/solo errado, RES MIX num knob
     separado que começa em 0 por design), não bug.

Commits locais ainda não empurrados (8, todos desta sessão, de
`ccd8f6f` até `6efb3d6`) mais o fix do `CmosVoice` + esta atualização de
docs, ainda por commitar. `origin/main` está 0 à frente, 8 atrás — nada
perdido, só aguardando autorização de push.

**Pendente**: nenhum bloqueador técnico restante no gate. Em aberto (não
bloqueador): seção 5 (EXCITAÇÃO e as pesquisas generativas irmãs) nunca
teve escuta isolada item a item; validação real em hardware Windows/macOS
(fallback já registrado, publica-se sem ela se não houver máquina); demais
itens das seções 2-7 (bugs reais residuais, validações pendentes, ideias
adiadas, exploração futura).

**Próximo passo recomendado**: decisão do autor sobre publicar agora (gate
tecnicamente fechado) ou esperar tentar Windows/macOS em hardware real
primeiro; se publicar, commitar o fix do `CmosVoice` + esta doc, e então
autorizar o push dos 8+ commits pendentes.

**Testes/validações executados nesta sessão**: `antitotem_simple_sequencer_tests`
(CTest) e build da app JUCE, limpos a cada mudança de código (incluindo
depois do fix do `CmosVoice`); suíte completa também sob
AddressSanitizer/UBSan sem achado; ThreadSanitizer no `MidiCapture`;
diagnósticos numéricos isolados (fora do repo, `g++` direto) pra várias
investigações de escuta (ASYMMETRY, FB GAIN, núcleos do oscilador, chain
REVERB/PHASER/FLANGER/RESONATOR) confirmando ou refutando cada achado antes
de reportar ao autor; `.deb` instalado de verdade; Windows/macOS build+
empacotamento verificados via CI real (GitHub Actions), não testados à mão
(sem máquina).

## 0. Confirmado/resolvido na sessão de revisão de 24 ago. 2026

Itens fechados ao longo desta sessão, removidos das listas abaixo — registro
curto pra não perder o rastro de quem confirmou o quê:

- **FORMA LFO ultrapassando o NOISE** — já estava corrigido no código (fix de
  18 ago. nunca tinha sido baixado no log); confirmado lendo `Main.cpp`.
- **LEARN: 16 sliders sem tooltip / MIXER OBJETOS e M1-M4 sem tooltip /
  janela CLONE sem LEARN próprio** — os três já estavam implementados ou
  eram decisão deliberada; confirmado lendo `UiLanguage.h`/`Main.cpp`.
- **TUTORIAL: só FEEDBACK tinha dica/macete** — os 8 capítulos do BÁSICO já
  tinham tip nos 4 idiomas; confirmado lendo `UiLanguage.h`.
- **TUTORIAL: capítulo "como começar/usar as abas" faltando** — escrito e
  implementado nesta sessão (NAVEGAÇÃO, 4 idiomas), navegação confirmada ao
  vivo pelo autor; leitura completa do conteúdo ainda em andamento.
- **Rótulo "INTERMEDIÁRIO" espremido no botão** — trocado por
  MÉDIO/MEDIUM/MOYEN/MEDIO, autor relatou ao vivo, corrigido e recompilado.
- **GROOVE genérico (qualquer subdivisão, não só SWG)** — confirmado ao vivo
  pelo autor.
- **Osciloscópio: teto do range sob patch exagerado** — autor forçou MASTER,
  sliders do mixer e ENERGIA no máximo; gráfico não cortou. Confirmado ao
  vivo.
- **Crash esporádico (17-18 ago.)** — rebaixado (não confirmado como
  corrigido, só sem recorrência relatada); suíte completa rodou limpa sob
  AddressSanitizer/UBSan nesta sessão, reforçando a ausência de corrupção de
  memória óbvia no core. Ver seção 2 abaixo — continua listado, só que como
  baixa prioridade.
- **ALSA underrun com REC ativo/EXCITAÇÃO (24-25 ago.) — escopo específico
  resolvido.** Auditoria do callback de áudio achou uma violação real da
  regra de zero lock/alocação (`MidiCapture`, captura MIDI de 20 ago.: locks
  bloqueantes + `push_back` sem `reserve()` rodando na thread de áudio
  durante REC). Corrigido em dois commits (`aeae302` checkpoint, `941628f`
  redesenho: `tryEnter()` não-bloqueante + reserve). Validado com teste de
  estresse de concorrência sob ThreadSanitizer (3M+ ciclos, zero corrida de
  dados) e confirmado ao vivo: duas tomadas gravadas com REC ativo, sem
  travar. **Escopo importante**: isso resolve especificamente o caminho de
  código do `MidiCapture` (só ativo durante REC) — não é uma alegação de
  "underrun resolvido no geral". Ver próximo item.
- **ALSA underrun recorrente, 25 ago. 2026, sem REC/sem PLAY** — reaberto o
  app, `snd_pcm_recover: underrun occurred` repetido, mesmo com o
  sequenciador parado e REC desligado. Isso descarta tanto `MidiCapture`
  (só roda durante REC) quanto custo de DSP (motor não tem modo ocioso mais
  barato — `render()` roda o DSP completo mesmo parado, confirmado grepando
  `isRunning`/`running_` em `DualObjectEngine.cpp`/`SimpleSequencer.cpp`,
  zero hits — e já sabemos que sobra CPU mesmo sob carga pesada).
  **Hipótese do autor, corroborada em 25 ago.**: `RASGO_SYNTH/
  rasgo-synth-performance` (outro app JUCE de áudio) estava aberto ao mesmo
  tempo — dois processos disputando o mesmo dispositivo de áudio é causa
  clássica de underrun, sem ser bug de nenhum dos dois lados. Reteste com o
  Antitotem sozinho (Performance fechado): gravação normal, sem underrun
  até agora. Não é prova formal/exaustiva (poucas sessões, não um teste
  controlado), mas corrobora a hipótese o suficiente pra rebaixar este item
  — não é tarefa de código do Antitotem, é nota de uso (não rodar dois apps
  de áudio JUCE ao mesmo tempo nesta máquina). Reabrir só se acontecer de
  novo com o Antitotem sozinho.
- **Bug de mute ignorado pela captura MIDI — confirmado corrigido, 25 ago.
  2026.** Autor gravou uma tomada de propósito com EXCITAÇÃO mutada
  (PRINCIPAL/CLONE tocando normal). Parseei o `.mid`
  (`ANTITOTEM_2026-08-25_17-22-19.mid`): PRINCIPAL 197 notas, CLONE 102
  notas, **EXCITAÇÃO 0 notas** — exatamente o esperado. Fix validado com
  teste deliberado, não só ausência de recorrência.
- **Site do instrumento (`ANTITOTEM/website/`) — primeira versão real, 25
  ago. 2026.** Duas páginas, 4 idiomas cada (PT canônico, EN/FR/ES):
  instalação/getting-started (requisitos, `.deb`, build do zero) e
  conceito/manifesto (origem 2012-2013 na Fundação Cultural de Curitiba,
  manifesto verbatim de `antitotem.arquiviagem.net`, características, "o
  que é hoje"). Reaproveita as duas imagens já publicadas no README
  (`antitotem.png`, `antitotem_modo_principal.png`) em vez de gerar
  novas. Identidade visual própria, não a do portal geral. Escopo restante
  do mapa editorial completo (campo modular, escuta, tutoriais, pesquisa)
  fica pra depois do gate de publicação fechar — ver
  `website/README.md`. Nada disso foi publicado/deployado, só criado
  localmente no repositório.

## 1. Gate de publicação (3 de 3 fechados — desbloqueado, 26 ago. 2026)

Registrado desde 15 ago. 2026, confirmado no PAINEL_MESTRE.md: não preparar
release pública enquanto os três itens abaixo não estiverem concluídos.
**Os três fecharam em 26 ago. 2026** — falta só a decisão de publicar
(ver "Ponto de retomada" acima) e, opcionalmente, validação real em
hardware Windows/macOS (não bloqueadora, ver decisão do autor abaixo).

- [x] **Empacotamento `.deb` — primeira versão funcional, 25 ago. 2026.**
  Ferramenta: CPack (CMake nativo), decisão do autor. `antitotem-0.1.0-
  Linux.deb` gerado e inspecionado (`dpkg-deb -I`/`-c`): binário instala em
  `/usr/bin/antitotem` (nome sem espaço, RENAME do target JUCE), `.desktop`
  em `share/applications` (`packaging/linux/antitotem.desktop`, novo),
  ícone SVG em `share/icons/hicolor/scalable/apps`. **Portabilidade
  resolvida no mesmo dia**: achado que o SVG já existe versionado em
  `website/logo_antitotem_novo.svg` (idêntico ao arquivo da máquina do
  autor, confirmado por `diff`) — `ANTITOTEM_LOGO_PATH` passou a apontar
  pra esse caminho relativo ao repo por padrão, em vez do caminho absoluto
  da máquina local; resolve também o logo embutido no app, não só o ícone
  do pacote. Ainda sobrescrevível via `-DANTITOTEM_LOGO_PATH=...`.
  Dependências: `dpkg-shlibdeps` automático
  + `libx11-6/libxext6/libxss1/libgio-2.0-0` adicionadas à mão (JUCE
  carrega essas via `dlopen()`, não aparecem como NEED do ELF — confirmado
  com `strings` no binário — sem declarar isso o pacote instala mas a
  janela nunca abre). Maintainer/contato: `rasgo.instruments@gmail.com`
  (trocado de `lucio.matema@gmail.com`, 26 ago. 2026, pedido do autor —
  também no site e no CMakeLists.txt). Homepage aponta pro repo real
  (`github.com/lucioaraujo/rasgo-instruments`).
  **Ainda falta**: instalar de fato o `.deb` (`dpkg -i`) numa máquina/
  container limpo pra confirmar que abre; `lintian` não estava disponível
  neste ambiente pra checagem automática de qualidade do pacote.
  **Instalado de verdade pelo autor, mesmo dia** (`sudo dpkg -i`, autorizado
  explicitamente) — `dpkg -l antitotem` confirma `ii` (instalado/
  configurado), `/usr/bin/antitotem`, `.desktop` e ícone todos no lugar
  certo com dono `root`. **Confirmado pelo autor: abre normalmente a
  partir da instalação real do pacote.** Linux/.deb fechado como primeira
  versão funcional, incluindo ícone portátil (ver acima).
  **Achado à parte, mesmo dia**: `run_antitotem.sh` (Opção A de uso rápido)
  tem um caminho absoluto do JUCE fixo na máquina do autor
  (`juce_dir`), diferente da lacuna do ícone — não corrigido ainda, listado
  como pendência nova em `INSTALL.md`.
  **Windows/macOS — CI rodou de verdade, 25 ago. 2026, sucesso.** `CPack`
  generalizado no `CMakeLists.txt` (`NSIS` pro Windows, `DragNDrop`/`.dmg`
  pro macOS), `install()` ajustado por plataforma, e workflow criado em
  `.github/workflows/antitotem-package.yml` (matriz ubuntu/windows/macos-
  latest). Push autorizado e feito; workflow disparou nos três runners
  hospedados pelo GitHub (esta máquina de desenvolvimento é Linux sem
  MSVC/Xcode, não podia compilar nenhum dos dois). **Primeira tentativa**
  (run `32886619540`): Linux e macOS geraram artefato de primeira; Windows
  falhou só no passo de empacotamento — `cpack -G NSIS` não achou
  `makensis` no runner (a app em si compilou e linkou limpo, warning
  ignorável de `createWriterFor` deprecated à parte). **Corrigido**:
  `choco install nsis -y` adicionado como passo antes do `cpack` no
  Windows (Chocolatey já vem no runner). **Segunda tentativa** (run
  `32887683743`): **sucesso nas três plataformas** — artefatos
  `antitotem-ubuntu-latest`, `antitotem-windows-latest` e
  `antitotem-macos-latest` publicados. Nenhum dos três instaladores foi
  baixado/instalado/aberto ainda (isso exige alguém com Windows/macOS de
  verdade pra testar — nem o autor nem eu temos essas máquinas). **Autor
  pretende testar em máquinas Windows e Mac reais nos próximos dias**
  (relatado 25 ago. 2026) — reabrir este item quando o resultado chegar.
  **Teste real em Windows, 26 ago. 2026**: autor instalou de verdade —
  app funciona (não testou áudio ainda), mas o instalador registrava o
  app em "Aplicativos instalados" sem nunca criar o atalho do Menu
  Iniciar, então não aparecia na busca do Windows nem em
  `shell:AppsFolder` (autor teve que caçar o `.exe` manualmente em
  `Program Files`). **Causa raiz**: `CPACK_GENERATOR "NSIS"` nunca teve
  `CPACK_PACKAGE_EXECUTABLES` definido — sem essa variável o template NSIS
  do CPack não sabe qual arquivo instalado é um app lançável, então nunca
  gera atalho nenhum (só registra a entrada de desinstalação). O `.exe`
  também não carregava ícone nenhum (nem `ICON_BIG`/`ICON_SMALL` no
  `juce_add_gui_app`, nem ícone do instalador NSIS). **Corrigido**: gerado
  `packaging/windows/antitotem.ico` (16/32/48/256px, a partir do glifo
  `logo_antitotem_favicon.svg`, centralizado em canvas quadrado já que o
  SVG original não é quadrado); `ICON_BIG`/`ICON_SMALL` adicionados ao
  `juce_add_gui_app`; `CPACK_PACKAGE_EXECUTABLES`,
  `CPACK_CREATE_DESKTOP_LINKS`, `CPACK_NSIS_MUI_ICON`/`_UNIICON`/
  `_INSTALLED_ICON_NAME` adicionados ao bloco Windows do CPack. Mesmo bug
  achado e corrigido no Navalha 2 no mesmo dia — virou checklist padrão
  pra qualquer app JUCE novo da família RASGO (ver memória do Claude
  Code). Verificado com build local Linux completo (configure+build+
  `cpack -G DEB` limpos) e CI real nas 3 plataformas (run `32996496450`,
  sucesso) — release `antitotem-v0.1.0` atualizada com os instaladores
  corrigidos. **Ainda falta**: autor confirmar que o atalho aparece de
  verdade com a versão corrigida, e testar áudio.
  **Bug real de macOS achado no mesmo dia (via relato do Codex/FaSol
  sobre o Navalha 2, verificado e confirmado igual aqui)**: `.dmg`
  recusava abrir ("Impossible d'utiliser cette version de l'application...
  avec cette version de macOS"). Causa: `CMAKE_OSX_ARCHITECTURES` e
  `CMAKE_OSX_DEPLOYMENT_TARGET` nunca foram definidos, então o Clang
  usava só a arquitetura nativa do runner (arm64, já que `macos-latest`
  do GitHub é Apple Silicon) e o deployment target da própria versão do
  SDK do runner — confirmado lendo o load command `LC_BUILD_VERSION` do
  binário publicado: `minos 26.0`. **Corrigido**: `CMAKE_OSX_ARCHITECTURES
  "x86_64;arm64"` e `CMAKE_OSX_DEPLOYMENT_TARGET "10.13"` definidos antes
  do `project()`. Diferente do Navalha 2 (que usa `std::filesystem` e
  precisou subir pra 10.15), o Antitotem compilou limpo em 10.13 — sem
  uso de APIs com essa restrição. Verificado com CI real: binário
  Universal 2 confirmado (`file`/`lipo` mostram x86_64 e arm64), `minos
  11.0` (o slice arm64 sobe sozinho pra 11.0, já que Apple Silicon nunca
  rodou versão mais antiga que Big Sur — comportamento esperado, não bug),
  assinatura ad-hoc presente (`codesign -dv`). **Sem assinatura Developer
  ID/notarização** (exigiria conta Apple Developer paga) — Gatekeeper vai
  mostrar aviso de "desenvolvedor não identificado" na primeira abertura,
  contornável com clique direito → Abrir. Adicionada etapa de validação
  automática no CI (`file`/`lipo`/`otool`/`plutil`/`codesign`) já que não
  há máquina Mac disponível pra testar manualmente. Release
  `antitotem-v0.1.0` atualizada com o `.dmg` corrigido. Virou checklist
  padrão da família (memória do Claude Code), junto com o do Windows.
  Reconferir
  a licença efetiva do JUCE (AGPLv3 vs. comercial) no momento de gerar um
  pacote pra distribuição real, não só no código-fonte
  (`CREDITS_AND_SOURCES.md` já avisa isso).
- [x] **Usabilidade em 1920×1080 — confirmado pelo autor, 25 ago. 2026.**
  O teste formal anterior era de 16 ago., antes de toda a expansão de
  layout (MÉTRICA/SUBDIVISÃO, EXCITAÇÃO, LEARN, toggles de participação da
  DERIVA). Autor relatou "já está funcionando em 1920×1080" — fecha o item.
- [x] **Avaliação sonora humana — PASSOU, 26 ago. 2026.** Bateria de
  avaliação musical (4 estudos curtos: pulso, matéria, retorno, espaço)
  prevista desde a "Prioridade imediata" original, nunca executada como
  bateria formal até esta sessão (25-26 ago. 2026). Os 4 estudos fecharam;
  1 bug real foi achado e corrigido no processo (vazamento de sinal bruto
  no `shapeCore` do `CmosVoice`, ver Estudo 4 abaixo). Com isso, os 3
  itens do gate de publicação estão fechados.
  - [x] **Estudo 1 — Pulso/Geada: PASSOU.** Autor testou todas as 8
    subdivisões da fileira MÉTRICA (RET/3:2/5:4/SWG/7:4/9:8/11:8/GLT), não
    só as 4 do critério original — há variação perceptível entre todas.
    Nuance registrada: 5:4, 9:8 e 11:8 soam parecidos entre si (diferença
    sutil, mas presente). Autor decidiu que isso é aceitável como está,
    não é defeito — diferença sutil é esperada nessa faixa de subdivisão,
    sem necessidade de ajuste.
  - [x] **Estudo 2 — Matéria: PASSOU.** Auditoria profunda do
    MATÉRIA (CUTOFF/RESONANCE/DRIVE/ASYMMETRY/MIX="MAT") depois do autor
    relatar não sentir efeito do ASYMMETRY mesmo em vários patches:
    verifiquei DSP (`MaterialFilter.h`, diferença numérica real e grande
    entre asymmetry=0/1 em teste isolado), fiação do slider (`detailControls[12]`
    → `syncDetails()` → `setMaterialFilterAsymmetry`, imediata, sem erro de
    índice) e roteamento do canal MIXER (sinal do MATÉRIA entra no canal
    FILTRO, índice 0) — tudo correto, nenhum bug de código encontrado.
    Achado real e não-óbvio: RESONANCE alta **reduz** o nível de saída do
    MATÉRIA em até 13× (satura tanto dentro da própria realimentação que
    a saída fica mais baixa, não mais dramática) — "tudo no máximo" é uma
    das configurações mais silenciosas, não a mais audível. Recomendação
    corrigida: RESONANCE baixa/moderada (0,2-0,4) com DRIVE alto mantém a
    saída audível e a diferença do ASYMMETRY grande. **Confirmado pelo
    autor, testando no CLONE com troca abrupta 0→1 (não giro gradual)**:
    diferença perceptível, porém muito sutil/quase imperceptível — bate
    com a explicação técnica (distorção assimétrica mexe em harmônicos
    pares, um dos tipos de coloração mais difíceis do ouvido humano
    perceber). Aceito como parâmetro real, mas sutil por natureza, não
    bug.
    **Parte original do critério (ruído colorido/S&H) concluída, mesmo
    dia.** Autor testou as 6 cores do NOISE (BRC/ROS/MAR/AZL/VIO/BIT)
    através do filtro, com o sequenciador rodando normal (som
    naturalmente picotado por passo — característica do instrumento, não
    falha de "continuidade"). Dois achados reais, ambos explicados pelo
    código e aceitos como característica, não bug:
    - **BRC e BIT parecidos** — `NoiseFields.h`: BIT não é uma cor
      filtrada própria, é o mesmo branco reduzido a 1 bit (`state &
      0x10000 ? 1 : -1`) — espectralmente ambos são "brancos", só muda a
      distribuição de amplitude. Só ROS/MAR/AZL/VIO são integrações/
      derivações reais com inclinação espectral diferente.
    - **MAR soa mais "mudo"** — medido: RMS do marrom (0,59) é o **mais
      alto** de todas as cores (branco 0,58, rosa 0,23) — não é amplitude
      baixa. É psicoacústico: energia concentrada em graves, onde o
      ouvido é menos sensível (curvas de igual-sonoridade) — característica
      clássica do ruído marrom, não falta de nível.
    Autor aceitou as duas como características naturais das cores, sem
    necessidade de compensação de sonoridade.
  - [x] **Estudo 3 — Retorno: PASSOU.** Correção registrada: PORTAS
    DE FEEDBACK (FB/DIODE/CAP/PULSE/TRANS/REFLUX) é **interno a um único
    objeto** (`CmosVoice::setFeedbackConnections`, realimenta a própria
    fase do oscilador com a saída anterior dele mesmo) — não precisa do
    CLONE audível, ao contrário do que eu disse por engano numa resposta
    anterior (confundi com `DualObjectEngine::ConnectionRoute`, sistema
    diferente por trás de "CONEXÃO ENTRE OBJETOS"). FB (rota "direct")
    relatado como difícil de perceber — verifiquei: das 6 rotas, é a
    única sem reformatação (as outras retificam/veiram onda quadrada/
    saturam); testei `CmosVoice` isolado, FB GAIN 0→0,72 dá RMS 0,649→0,697
    (~7%, real mas modesto) — bate com ser a mais sutil por natureza.
    **Confirmado pelo autor**: percebe as diferenças entre rotas, algumas
    mais sutis que outras — consistente com o previsto. Slider FB GAIN
    funcionando corretamente. **Autor testou todas as seis rotas**: algumas
    difíceis de identificar (esperado — bate com o previsto pra FB, e
    provavelmente REFLUX/DIODE em níveis baixos), mas nenhuma incomodou —
    sem colapso, sem fadiga, sem saturação desagradável em nenhuma rota.
  - [x] **Estudo 4 — Espaço: PASSOU.**
    **Bug real encontrado e corrigido, 26 ago. 2026**: testando com só o
    PRINCIPAL (sem CLONE, nada mutado), autor zerou o MIX dos 5 osciladores
    e o som continuou. Reproduzi nos 3 modos do seletor CORE (40106/8038/
    4069UB): 8038 (`functionForms`) silenciava corretamente; 40106
    (`schmittPulse`) e 4069UB (`unbufferedDrift`) não — `CmosVoice::
    tickStereo`'s `shapeCore` (linhas ~157-170) lia `oscillatorA`/
    `oscillatorB`/`oscillatorC` **brutos** (antes do ganho por
    `oscillatorLevels[i]`) pra colorir a saída: 40106 somava um pulso fixo
    ±0,36 baseado no sinal de A+B (autor: "soa mais" — presente mesmo com
    MIX=0), 4069UB somava `oscillatorC * asimetria` (autor: "clip grave
    sutil como de mudança de step" — vazamento mais fraco, só de C).
    Corrigido escalando cada leitura bruta pelo `oscillatorLevels` do
    oscilador correspondente (`std::max(levels[0], levels[1])` pro
    pulso 40106, `levels[2]` pro termo de C do 4069UB) — verificado com
    diagnóstico numérico isolado: pico 0,0 exato nos 3 núcleos com MIX=0,
    caráter preservado (picos ~0,86-0,99) com MIX=0,8. Testes do core
    (`ctest`) e build do app OK após a mudança. **Confirmado pelo autor**,
    ouvindo os 3 núcleos com MIX=0: "agora ficou bom".
    **EIXO X (pan por oscilador)**: testado, os 5 osciladores respondem
    corretamente. **Panorâmica dos canais no mixer** (FILTRO/RING/NOISE/
    ESPAÇO): testado, os 4 canais respondem corretamente.
    **Efeitos (REVERB/PHASER/FLANGER/RESONATOR)**: relatado sem diferença
    audível a princípio; verifiquei o chain isolado (`phaser→flanger→
    reverb→resonator`, exatamente como ligado em `SimpleSequencer.cpp:648-
    658`) com um diagnóstico numérico — DSP funcionando e sliders
    corretamente ligados (`Main.cpp:6691-6693`), RMS cai de 0,369 (seco)
    pra 0,114 (tudo no máximo), cada efeito sozinho já muda bastante.
    Causa provável não era bug: (1) os 4 efeitos só chegam ao ouvido pelo
    canal ESPAÇO do mixer (`resonated`) — se ele estiver mudo/sem solo
    correto, nada se ouve; (2) RES MIX (CombResonator) não é um dos 3
    sliders visíveis REVERB/PHASER/FLANGER — mora num knob separado em
    PARÂMETROS/ROTAS ATIVAS (`detailControls[6]`), e começa em 0 por
    design, igual aos outros MIX de efeito. Orientei soloar ESPAÇO e achar
    o RES MIX. **Confirmado pelo autor**: "ok funcionam".

**Decisão do autor sobre Windows/macOS, 25 ago. 2026**: testar nas duas
plataformas continua o plano (ver nota do CI acima), mas não é mais
bloqueador absoluto — se não houver máquina Windows ou macOS disponível
pro autor testar, a publicação segue mesmo assim, sem validação real
nesses ambientes (só o build/CI verificado). Registrado pra não travar o
gate indefinidamente por falta de hardware.

*(Histórico: seções "Prioridade imediata", "Critério de aceitação sonora",
"Registrado em 2026-08-15" — item `.deb`.)*

## 2. Bugs reais, nunca resolvidos

**Verificado em 24 ago. 2026** contra o código-fonte atual (`Main.cpp`
`layoutVoiceArea()`, comentário "Real bug found and fixed"): o item "FORMA
LFO ultrapassa a largura do NOISE" listado numa versão anterior deste
documento já está corrigido — a tentativa registrada no log original
(restringir aos bounds do `NoiseSelector`, sem efeito) foi seguida, no
mesmo dia (18 ago.), por um fix real: `lfoArea` passou a copiar largura e X
de `noiseSelectorBounds` diretamente (`.withY(...).withHeight(...)`, não
mais `.withHeight(energyNoiseArea.getHeight())`), então estruturalmente não
pode mais ultrapassar a largura do NOISE. O log estava incompleto, não o
código. Removido da lista aberta; contexto completo em
`TAREFAS_HISTORICO.md`, seção "Ponto de retomada" e "Registrado em
2026-08-18" adiante.

- [ ] **Crash esporádico do app (17-18 ago.) — provavelmente obsoleto,
  rebaixado.** Nunca foi reproduzido em teste automatizado na época, e o
  autor não relatou recorrência desde então, apesar de bastante trabalho
  depois (DERIVA overhaul, MelodicInterpreter, exportação MIDI/MusicXML).
  Sem evidência de que foi corrigido — só sem recorrência relatada — então
  não fica marcado como resolvido, só deixa de ser tratado como bloqueio
  ativo. Se voltar a acontecer: `ulimit -c` retornava 0 nesse shell mesmo
  com `systemd-coredump` ativo — rodar `ulimit -c unlimited` antes de
  lançar o processo de teste, senão uma reprodução não deixa rastro para
  depurar.
- [ ] **Risco registrado, não confirmado**: EXCITAÇÃO pode continuar gerando
  notas MIDI durante trechos com STOP pressionado — o disparo interno não é
  condicionado a `stimulus.running` dentro de `MelodicInterpreter::tick()`,
  só o ganho de áudio final é. Mesma família do bug já corrigido de mute
  ignorado pela captura MIDI; vale conferir se acontece de fato.
*(Histórico: "Ponto de retomada" no topo do arquivo original; "Correção de
bug: BPM da exportação MIDI errado" / "mute do object mixer ignorado pela
captura MIDI".)*

## 3. Validações pendentes (escuta ou visual — nunca testado ao vivo desde a implementação)

Regra do projeto: não usar entrada sintética de teclado/mouse para testar a
UI — toda confirmação depende do autor abrir o app e relatar. Isto significa
que uma fração grande do trabalho abaixo está implementada, compila limpo e
passa nos testes automatizados, mas nunca foi de fato ouvida ou vista.

- [ ] **LEARN (modo de ajuda contextual) — nunca confirmado visualmente pelo
  autor abrindo o app** (isso continua genuinamente pendente — é o tipo de
  validação que só escuta/visão humana fecha). **Verificado em 24 ago. 2026
  contra o código atual**: os três "gaps" que a versão anterior deste
  documento listava já estão implementados — os 16 sliders de
  `detailControls` têm tooltip real nos 4 idiomas
  (`UiLanguage.h:detailControlTips`); MIXER OBJETOS (volume/mute
  PRINCIPAL/CLONE) e os botões M1-M4 da MEMÓRIA MIX têm `setTooltip()`
  confirmado (`objectMixPrincipal/Clone`, `mixSlotRecall`); e a janela CLONE
  autônoma não ter caixa LEARN própria não é uma lacuna, é decisão
  explícita do autor registrada em comentário ("no segundo monitor não é
  necessário repetir a caixa learn... fica somente na aba principal") — o
  listener de hover/foco funciona nos dois monitores e alimenta a caixa
  única via `onExplain`. O que resta de fato aberto é só a confirmação
  visual em si, não implementação faltando.
- [ ] **TUTORIAL: estrutura de 3 níveis (BÁSICO/INTERMEDIÁRIO/AVANÇADO)** —
  nunca confirmada visualmente pelo autor abrindo o app; isso continua
  pendente. **Verificado em 24 ago. 2026 contra o código**: a régua
  "dica/macete didático" já cobria os 8 capítulos do BÁSICO inteiros (não só
  FEEDBACK) — item também estava desatualizado nesta versão anterior do
  documento.
  **Implementado nesta sessão** o capítulo que faltava, "como começar, como
  usar as abas" — novo capítulo `NAVIGATION`/`NAVEGAÇÃO`/`NAVIGATION`/
  `NAVEGACIÓN`, inserido como primeiro do BÁSICO (agora 9 capítulos,
  `UiLanguage.h`), nos 4 idiomas: explica o toggle CLONE (corpo único +
  Shift+C) vs. janela própria em modo 2 MONITORES, e o alternador
  SOM/SEQUÊNCIA/MIX (só visível abaixo de 1600px, escondido no layout
  unificado 1920×1080). Tip aplicada é a já cogitada pelo autor: começar
  com CLONE mutado para aprender o PRINCIPAL primeiro. `contentsButtons`/
  `levelChapterCount`/`chapterAt` são todos genéricos por `.size()`, sem
  número mágico — nenhuma outra mudança de código foi necessária. Build da
  app (`AntitotemSimpleSequencerApp`) recompilado limpo, CTest 1/1 passou.
  **Confirmado visualmente pelo autor, mesmo dia**: os três botões de nível
  (BÁSICO/MÉDIO/AVANÇADO) e a navegação de capítulos funcionam ao vivo.
  No mesmo teste, autor relatou "INTERMEDIÁRIO" espremido na largura do
  botão — trocado por **MÉDIO/MEDIUM/MOYEN/MEDIO** (mais curto, paralelo
  aos outros dois rótulos) em `Main.cpp` (2 ocorrências do array de
  labels). Build recompilado limpo depois da troca.
  **Leitura em andamento**: autor já leu parte do conteúdo (não tudo),
  sem erros adicionais reportados até agora além do MÉDIO acima; segue
  aberto até a leitura completa, reportando erros conforme aparecerem.
- [ ] **MÉTRICA/SUBDIVISÃO/GROOVE (versão final)** — **GROOVE genérico
  (aplicado a qualquer subdivisão, não só SWG) confirmado ao vivo pelo
  autor em 24 ago. 2026.** Restam sem confirmação: botão SWG fixo dentro
  da fileira dos 8 botões de subdivisão, e as portas de feedback/variação
  encolhidas.
- [ ] **Tooltips localizados (popup)**: confirmação visual do popup de
  tooltip nunca foi possível via automação (JUCE desenha isso numa janela
  X11 separada) — segue precisando do autor olhar direto na tela.
  **REC TIMER confirmado, 25 ago. 2026**: autor gravou as duas tomadas
  usando um botão de duração pré-definida, deixando finalizar sozinho no
  fim do loop. Conferi os arquivos reais em
  `~/Music/Antitotem Objeto Sonoro/` — `.wav`/`.mid`/`.musicxml` salvos no
  diretório certo, durações de 60,3s/60,8s (quantizadas pro fim do loop, o
  comportamento documentado — não corta em 60s exatos). Parte do item
  fechada.
- [x] **Correção de estéreo em REVERB/PHASER/FLANGER/RESONATOR** (canal
  ESPAÇO) e o pan diluído por NOISE — **confirmado ao vivo, 26 ago. 2026**,
  no Estudo 4 (Espaço): autor testou o pan por oscilador (EIXO X), a
  panorâmica dos 4 canais do mixer (FILTRO/RING/NOISE/ESPAÇO) e os efeitos
  do canal ESPAÇO, todos respondendo corretamente — ver seção 1, Estudo 4.
- [ ] **Site `antitotem.arquiviagem.net` (fora deste repositório)**: trocar a
  screenshot antiga pela nova, e levar o fluxograma de sinal + explicação
  das portas de feedback para lá — nenhuma das duas feita (sem
  acesso/instrução de publicação nesta sessão).

*(Histórico: seção "LEARN implementado" e todos os "Ajuste 18 ago." /
"Ajuste 19 ago." seguintes; "MÉTRICA: 8 presets..."; "Osciloscópio
(concluído 2026-08-11)"; "Registrado em 2026-08-10"; "Registrado em
2026-08-15".)*

## 4. Ideias de design/UI explicitamente adiadas

- [ ] **"Dança" dos rails do sequenciador via DERIVA** — ideia do autor:
  DERIVA também mover/reordenar a posição dos próprios steps, não só seus
  valores. Ainda não desenhada tecnicamente (o que exatamente "move"?
  ordem de disparo? posição visual? as duas?).
- [ ] **Oscilação leve do botão PLAY enquanto toca** — implementada duas
  vezes (brilho, depois cor), efeito nunca ficou perceptível na tela, causa
  raiz não encontrada. Autor: "deixa isso pra lá, cansei" — retomar só se
  fizer sentido; antes de tentar de novo, checar com log temporário se o
  estado/fase realmente chega como esperado.
- [ ] **Título dentro do knob, só em alguns knobs específicos** e **knobs
  concêntricos** (ex. VCF com FREQ/RES/CV como arcos um dentro do outro) —
  duas ideias de design explicitamente adiadas como experimento separado.
- [ ] **Traço do anel do CLOCK mais fino que os osciladores** (proporção
  fixa de 8px do próprio JUCE) — autor decidiu "deixar como está por
  agora"; precisaria de um `LookAndFeel` próprio com traço proporcional ao
  raio.
- [ ] **Ativação do meta-sequenciador sem controle de UI** — hoje disputa o
  mesmo knob de DERIVA·PROFUNDIDADE (correção aplicada), mas o autor pediu
  para decidir depois entre um controle real no mixer de objetos ou outro
  caminho.
- [ ] **Ajustar `TimeSignature`/`stepsPerBeat` (compasso real) via UI** —
  hoje só o padrão de fábrica 4/4 é usado; decisão deliberada de não
  arriscar o layout denso sem poder testar visualmente. Sugestão registrada:
  atalho de teclado cíclico (mesmo precedente do Shift+C do CLONE) em vez
  de um botão novo.

*(Histórico: "Registrado em 2026-08-14, não é prioridade imediata";
"Registrado em 2026-08-13" e blocos correlatos de layout ao longo da
sessão de 18-19 ago.; "Nova pesquisa: RASGO Score" / "Exportação MIDI".)*

## 5. EXCITAÇÃO e as cinco pesquisas generativas irmãs

EXCITAÇÃO (terceiro objeto generativo, tipo-theremin) e cinco documentos de
pesquisa irmãos nasceram entre 19-20 ago. 2026: `CRI-MEL-001` (melodia),
`CRI-SEQ-001` (sequenciador), `CRI-ACC-001` (acentuação), `CRI-NOI-001`
(ruído), `CRI-DRF-001` (deriva) — todos em `ANTITOTEM/docs/PESQUISA_*.md` e
registrados em `RASGO_DOCUMENTATION/CRIACAO_PESQUISA_E_INOVACAO.md`. Depois
vieram mais três: `CRI-CMP-001` (compasso real), `CRI-REL-001` (EXCITAÇÃO
como agente relacional/caráter), `CRI-MID-001`/`CRI-SCR-001` (exportação
MIDI/MusicXML e representação musical própria).

**Estado geral**: um volume muito grande de DSP foi implementado, compila
limpo e passa nos testes automatizados — mas a esmagadora maioria nunca foi
confirmada por escuta isolada (só confirmações gerais em lote, tipo "testei,
funciona", sem validar item por item). Cada `PESQUISA_*.md` tem sua própria
seção **"Calibração pendente"** listando toda constante estimada/nunca
medida contra sinal real — é a fonte de verdade para retomar, não recriar
essa lista aqui.

- [ ] **Rodada de escuta dedicada, item por item**, cobrindo pelo menos: os
  eixos de melodia mais recentes (#2/#4 ataque-final, #8 chegada/estrutural,
  #12 microafinação "living pitch", #1/#13 tipos de gesto/articulação,
  caráter contínuo energia/suavidade/brilho de `CRI-REL-001`); a DERIVA
  estendida (atratores, instâncias paralelas A/B/C, modo AUTO, os 16
  toggles de "participação por título"); o Noise Field e o gain-staging
  revisado do NOISE; e a acentuação contínua (Accent Field, herança,
  rotação, fatigue).
- [ ] **Melodia (CRI-MEL-001)** — restam: #14 (densidade → liberdade
  interpretativa, já parcialmente coberto), aprofundar #8 (hierarquia de
  notas — só "ápice" e "chegada" feitos, faltam passagem/tensão/
  repetição/bordadura) e a decisão de fato usar `MelodicInterpreter` como
  camada compartilhável por outros instrumentos RASGO (extraída, mas nunca
  reaproveitada em outro lugar).
- [ ] **Sequenciador (CRI-SEQ-001)** — as "três tarefas grandes" (meta-
  sequenciador, event budget, arquitetura de 5 escalas) têm todas pelo
  menos uma fatia implementada. Resta: event budget **compartilhado entre
  instrumentos** de verdade (hoje é só por objeto, dentro do próprio
  ANTITOTEM), ecologia RASGO-wide, e as 3 transformações motívicas que
  faltam (aumentação/diminuição/fragmentação — rotação/inversão/retrógrado
  já feitas) + identidade/comparação de padrão entre motivos.
- [ ] **Acentuação (CRI-ACC-001)** — implementados só os itens mais baratos
  das 7 famílias/25 técnicas do brief original; o resto nunca abordado.
- [ ] **Ruído (CRI-NOI-001)** — implementados BreathExciter (redesenhado
  como "ping" percussivo por step) e Noise Field; resta a maioria das 30
  técnicas do brief (3 territórios: matéria sonora, modulador tímbrico,
  acontecimento estrutural).
- [ ] **Deriva (CRI-DRF-001) — reflexão de arquitetura em aberto, não
  implementada**: o autor propôs um "cérebro" central que observe os fluxos
  do instrumento e decida variações, em vez de dezenas de blocos com
  constante ajustada à mão um por um. Discussão registrada, sem direção
  escolhida.
- [ ] **CRI-REL-001 (EXCITAÇÃO como agente relacional)** — feitas as fatias
  mais baratas de caráter (6.1), acoplamento intensidade↔timbre (6.2 parte
  barata) e consequência de volta ao Noise Field (6.3). Falta: o eixo de
  ruído respiratório (precisa de fonte de ruído nova, mais caro) e o
  "pipeline relacional geral" mais especulativo do documento.
- [ ] **Auditoria "sem aplicação direta" — getters sem consumidor de UI**:
  `getExcitationMicroState/PhraseState`, `getExcitationCharacterEnergy/
  Softness/Brightness`, `getFormState`, `getFormHistoryAt` — o som já reage
  a esses estados internamente, só não há painel nenhum na UI mostrando o
  valor. Sugestão registrada, não decidida: um pequeno painel de estado,
  baixa prioridade.

*(Histórico completo: da seção "EXCITAÇÃO: terceiro objeto generativo" em
diante, até o fim do arquivo — é a maior e mais recente parte do log.)*

## 6. Exportação MIDI / MusicXML / RASGO Score

- [ ] **Andamento variável — confirmado, 25 ago. 2026.** Parseei os dois
  `.mid` reais das tomadas pós-fix (fora do repo, parser binário manual):
  9 e 3 `tempoMetaEvent` reais em cada uma, com variação real de BPM
  (79-138 e 78-105) — não é mais só harness sintético, é captura real
  durante uso ao vivo. Note on/off perfeitamente balanceados nas 3 trilhas
  das duas tomadas (401/401, 134/134, 19/19 e 304/304, 136/136, 121/121),
  nenhuma nota órfã.
- [ ] **MusicXML nunca aberto num software de notação real** — verificado
  só por parser (`xml.dom.minidom`, bem formado, contagem de notas/
  compassos plausível nas duas tomadas de 25 ago.), nunca visualmente no
  MuseScore ou equivalente.
- [ ] **EXCITAÇÃO uniformiza durações no MusicXML** (achado e documentado,
  não é bug: ela dispara livre, sem grid, ao contrário de PRINCIPAL/CLONE
  que são travados no grid do sequenciador) — autor decidiu manter como
  está por ora ("depois vamos melhorar isso"). Duas direções já registradas
  para quando isso for retomado: grade mais fina só para EXCITAÇÃO, ou
  notação por gesto/frase em vez de por disparo individual.
- [ ] **RASGO Score (CRI-SCR-001)** — representação musical intermediária
  própria de três camadas (SCORE/PERFORMANCE/INTERPRETATION), só
  documento/pesquisa por enquanto. 4 perguntas deixadas em aberto para o
  autor: vale persistir a representação estendida sem um consumidor
  definido ainda? formato JSON ou binário? MusicXML antes ou depois dela?
  existe um caso de uso concreto de "replay"/regeração, ou o valor é só
  documentação/análise?

*(Histórico: "Exportação MIDI e extração de partitura (CRI-MID-001)" até o
final do arquivo.)*

## 7. Exploração futura (projetos separados, nada implementado)

- [ ] **Antitotem Breadboard/Protoboard** — instrumento educativo/
  performático numa breadboard virtual. Escopo nem decidido ainda: projeto
  novo (diretório próprio) ou modo alternativo dentro do Antitotem atual.
  Ver [`docs/IDEIA_BREADBOARD.md`](IDEIA_BREADBOARD.md).
- [ ] **Interface 3D** — Blockout no Blender → protótipo Three.js →
  interface 3D real ligada ao motor via `juce::WebBrowserComponent`. Godot/
  Unity descartados (exigiriam reimplementar o motor de áudio).
- [ ] **Versão web simplificada (vitrine)** — WebAssembly ou porta do DSP
  para Web Audio API/AudioWorklet, subconjunto de controles, interface
  própria para toque.
- [ ] **Tutoriais de escuta dentro do app como um tipo de capítulo à
  parte** — baixa prioridade; 4 dos 5 exercícios originais de
  `docs/TUTORIAIS.md` já foram portados para o nível INTERMEDIÁRIO do
  TUTORIAL, resta avaliar se ainda vale a pena como conceito separado.

*(Histórico: as três seções "Exploração futura" do arquivo original.)*
