# Antitotem — linha de base de CPU do core

Data: 2026-08-17. Esta medição é uma linha de base para regressão, não uma
meta de produto e não substitui teste de deadline/xrun com dispositivo real e
placa de áudio. Segue o mesmo método (mediana de janelas, checksum finito
obrigatório) já usado em
[`../AQUORBIUM/CPU_BASELINE.md`](../AQUORBIUM/CPU_BASELINE.md), adaptado ao
que o Antitotem de fato tem hoje: um motor (`DualObjectEngine`, PRINCIPAL +
CLONE já somados) em vez de um banco polifônico de vozes externas.

## Origem: por que esta linha de base existe

Criada ao investigar a diluição de PAN dos osciladores (ver
[`docs/TAREFAS.md`](docs/TAREFAS.md), 17 ago. 2026). A correção exigiu que
REVERB/PHASER/FLANGER/RESONATOR (o bus ESPAÇO) passassem a rodar em estéreo
de verdade — duas instâncias, uma por canal — em vez de processar só a média
mono `filtered` como faziam antes. O autor perguntou se isso "exige mais do
hardware" e "pode travar"; esta linha de base responde com números medidos em
vez de estimativa.

## Método

- build `Release` (`-DCMAKE_BUILD_TYPE=Release`), GCC 13.3.0, `-O3` no alvo do
  benchmark;
- Intel Core i5-6500, 4 núcleos, 3,20 GHz (mesma máquina do baseline do
  AQUORBIUM);
- `DualObjectEngine` com os dois objetos (PRINCIPAL/CLONE) totalmente
  patchados: 3 osciladores ativos por objeto, VCF com CV, ADSR não-instantâneo,
  RING/NOISE/S&H ativos, REVERB/PHASER/FLANGER/RESONATOR todos com envio > 0,
  os 4 canais do mixer ligados, conexão entre objetos ativa nas 4 rotas — o
  pior caso realista, não um patch ocioso/bypassado;
- taxas de 44,1, 48 e 96 kHz; blocos de 32, 128, 256 e 512 amostras;
- aquecimento de 250 ms antes de cronometrar;
- mediana de cinco janelas de 2 s por caso, três rodadas sequenciais;
- checksum finito obrigatório (impede medir um caminho de render vazio ou
  otimizado por engano).

`cpu_percent` é o tempo gasto para produzir a janela de áudio como fração
dela — uma medida de render offline num único caminho, não o percentual de
CPU do sistema operacional.

## Antes/depois do bus ESPAÇO em estéreo (+ correção de estabilidade do VCF)

Comparação direta entre o commit anterior a esta sessão (`5b38bf6`, ESPAÇO
mono) e o estado atual (ESPAÇO estéreo + `CmosVcf` corrigido — ver
`docs/TAREFAS.md`), mesma máquina, mesmo binário de benchmark, três rodadas
cada:

| Taxa | Bloco | CPU antes (mediano) | CPU depois (mediano) | Tempo real antes | Tempo real depois |
|---:|---:|---:|---:|---:|---:|
| 44,1 kHz | 128 | 8,74% | 10,42% | 11,44× | 9,60× |
| 48 kHz | 128 | 9,49% | 11,33% | 10,51× | 8,82× |
| 96 kHz | 128 | 18,86% | 22,73% | 5,30× | 4,40× |
| 96 kHz | 512 | 18,90% | 22,64% | 5,29× | 4,41× |

**Leitura:** o acréscimo é de ~1,7 a ~3,9 pontos percentuais conforme a taxa —
não "dobra" o motor inteiro, porque só 4 módulos leves (delay de poucos taps
com interpolação + `tanh`, sem FFT/convolução) passaram a rodar duas vezes;
osciladores, VCF (que já era estéreo), ADSR, mixer, leveler e proteção de
saída continuam com o mesmo custo de antes. Mesmo no pior caso medido
(96 kHz/bloco 512, testado com **dois objetos completos e totalmente
patchados simultâneos**), o render ainda roda a 4,41× mais rápido que tempo
real — ampla folga abaixo do limite de 1×. Em 44,1/48 kHz, a folga passa de
8-9×. **Não trava e não deveria produzir xrun** em hardware de desktop/laptop
comparável a este; a categoria de erro que de fato causa travamento em áudio
(alocação, lock, I/O dentro do callback) não foi introduzida por esta mudança
— continua proibida pelo `AGENTS.md` do RASGO.

## Casos representativos (estado atual, três rodadas)

| Taxa | Bloco | CPU da janela | Vezes mais rápido que tempo real |
|---:|---:|---:|---:|
| 44,1 kHz | 32 | 10,36% | 9,65× |
| 44,1 kHz | 128 | 10,42% | 9,60× |
| 44,1 kHz | 256 | 10,45% | 9,56× |
| 44,1 kHz | 512 | 10,46% | 9,56× |
| 48 kHz | 128 | 11,33-11,49% | 8,70-8,86× |
| 96 kHz | 128 | 22,45-22,90% | 4,37-4,45× |
| 96 kHz | 512 | 22,42-22,65% | 4,41-4,46× |

Dispersão entre rodadas de ±0,3 a ±0,5 ponto percentual, consistente com
escalonamento do sistema operacional — a mesma fonte de ruído já registrada
no baseline do AQUORBIUM.

## Limitações

- Mede um único caminho de render offline, não o percentual global do SO nem
  o comportamento sob concorrência real (Spotify/navegador/outro software
  disputando a mesma CPU, cenário já relatado ao vivo em `docs/TAREFAS.md`);
- não testa deadline real de callback de áudio (JACK/ALSA/CoreAudio) nem
  xruns em hardware de áudio real — só compara tempo de CPU relativo;
  máster, oversampling ou um terceiro/quarto objeto exigirão nova linha de
  base;
- uma única máquina; comparações futuras devem usar o mesmo modelo de CPU,
  compilador e tipo de build para permanecerem comparáveis.

## Reproduzir

```sh
cd ANTITOTEM
cmake -S . -B build-release \
  -DANTITOTEM_BUILD_APP=OFF \
  -DANTITOTEM_BUILD_TESTS=OFF \
  -DANTITOTEM_BUILD_TOOLS=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target antitotem_cpu_benchmark
./build-release/antitotem_cpu_benchmark
```

A ferramenta (`tools/CpuBenchmark.cpp`) imprime CSV com todos os casos.
Comparações futuras a este baseline devem rodar pelo menos três vezes e
comparar medianas, como feito acima — um único render isolado carrega ruído
de escalonamento demais para ser conclusivo sozinho.
