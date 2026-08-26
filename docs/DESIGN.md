# Design — Antitotem — Objeto Sonoro

**Origem do núcleo:** Objeto Sonoro 5. O nome geral não apaga a obra histórica:
ele permite que o sistema modular cresça sem transformar cada nova relação em
um “novo número”.

## Gesto central

Escutar a posição do scanner circular e alterar uma das oito tensões enquanto
ela define a cor e a altura da voz. O painel deve permitir entender a cadeia
antes de operar o som.

O processo composicional é intuitivo: gesto, escuta e postura diante do incerto
têm prioridade sobre cálculo e programação de valores. O painel deve expor
relações materiais legíveis, mas não exigir que a pessoa resolva equações,
simule uma breadboard com precisão ou navegue por menus para descobrir som.
Limites de ganho, proteção de saída e estados de rota existem para tornar a
experiência segura, não para eliminar suas consequências inesperadas.

Ruptura, saturação, feedback e instabilidade devem poder aparecer como
consequências audíveis de conexões materiais. A interface não deve estetizar o
perigo físico: o risco histórico do circuit bending é traduzido em exploração
computacional contida, não em instruções para danificar componentes.

## Princípio de performance

O núcleo do Objeto Sonoro 5 é uma fonte autônoma: sua origem é a energia própria dos
osciladores, conduzida pelo clock, pelas tensões e pelas ligações. MIDI não
integra este marco nem aparece como pendência de performance. A aplicação pode
futuramente dialogar com outros sistemas, mas isso exigirá uma decisão
conceitual nova; não será introduzido como substituto da lógica interna do
objeto.

## Mapa do painel Eurorack digital

```text
[ 40106 CLOCK ] -> [ 4040 DIVIDER + 4051 SCANNER ] -> [ CV 1 ... CV 8 ] -> [ CMOS VOICE ]
      rate                    endereço/LED                  oito tensões       audio out

                                     -> [ CONTOUR · ADSR ] -> [ 4069 · VCF ]
```

Os oito CVs são verticais, contíguos e têm LEDs de seleção; não representam
notas de uma grade piano-roll. O clock fica separado, à esquerda; a voz fica à
direita como destino do sinal. Esta é uma tradução de leitura do esquema da
página 9, não uma reprodução gráfica literal de breadboard ou de um fabricante
de Eurorack. Antes de afirmar equivalência ao painel físico do Objeto 5, esta
faixa deve ser reconfigurada para sua quantidade real de potenciômetros e suas
saídas específicas.

Uma faixa de conceitos no topo mantém a cadeia legível: `40106` como
limiar/clock, `4040` como divisão/endereço e `4051` como scanner de oito CVs.
`8038*` e `4069UB*` aparecem com asterisco porque são estudos de forma contínua
e deriva implementados no software — referências técnicas, não componentes
confirmados do Objeto Sonoro 5 histórico.

## Fluxo de sinal pós-voz: RING, FILTRO e ESPAÇO como pontos de escuta em série

O mapa acima termina em VCF; o motor atual continua a cadeia bem além disso
(RING, MaterialFilter/MAT, ESPAÇO), e o MIXER (coluna à direita do painel)
não expõe quatro fontes paralelas e independentes - expõe **quatro pontos de
escuta ao longo de uma única cadeia em série**. Cada canal ON/gain do MIXER
controla apenas a contribuição própria daquele ponto à mixagem final; ele não
interrompe o que continua adiante na cadeia:

```text
voz (CMOS VOICE, já com ADSR aplicado)
   │
   + ruído bruto × (NOISE MIX × gain do canal NOISE, só se ele estiver ativo)
   │                                          [interruptor mestre - único caso
   │                                           corrigido em 18 ago. 2026, ver
   │                                           TAREFAS.md; NOISE é a única
   │                                           injeção paralela, não um elo
   │                                           da série]
   ▼
[ RING ] ── canal RING do MIXER (soma + diferencial estéreo) ──► mixagem
   │          só soa se o canal RING estiver ON; a modulação em anel
   │          em si roda sempre, ligado ou não
   ▼
[ 4069 · VCF ] → [ MaterialFilter · MAT ]
   │          ── canal FILTRO do MIXER (soma + diferencial estéreo) ──► mixagem
   │             só soa se o canal FILTRO estiver ON; o sinal filtrado
   │             segue adiante de qualquer forma
   ▼
[ PHASER ] → [ FLANGER ] → [ REVERB ] → [ RESONATOR ]   (ESPAÇO)
   │
   └────── canal ESPAÇO do MIXER (soma + diferencial estéreo) ──► mixagem
              só soa se o canal ESPAÇO estiver ON
```

Consequência prática, direta: **desligar RING no MIXER não remove a
modulação em anel do som** se o canal FILTRO estiver ligado (o filtro
processa o sinal já modulado, não a voz crua) - e **desligar FILTRO no
MIXER não impede o sinal filtrado de chegar na saída** se o canal ESPAÇO
estiver ligado (reverb/phaser/flanger/resonator sempre partem do sinal já
filtrado). Cada canal só controla a própria "escuta" daquele ponto, não um
bloqueio total do que vem antes dele.

Isso é deliberado, não uma falha: é o mesmo princípio de um envio
pré-fader numa mesa real, e permite combinações como "só a reverberação do
filtro, sem o filtro seco por cima" (FILTRO off + ESPAÇO on) ou "o timbre
do RING através do filtro, sem a contribuição estéreo extra do próprio
canal RING" (RING off + FILTRO on) - perder essas combinações custaria mais
do que a clareza ganha ao "consertar". Discutido e decidido em 18 ago. 2026
(ver TAREFAS.md) depois que o autor notou o comportamento testando RING/
FILTRO desligados: "o que sugere?" → mantido como está, documentado aqui e
nos tooltips do próprio botão ON de FILTRO/RING no painel.

## Estados importantes

- `RUN`: scanner avança e exatamente um LED de etapa fica aceso;
- `PLAY` / `STOP`: iniciam ou suspendem o scanner sem alterar as tensões;
- `RESET`: endereço retorna ao canal 0;
- `MASTER`: controla somente o nível final após a proteção técnica;
- `ENERGIA`: uma alimentação digital variável. Em valores baixos desacelera o
  clock, reduz o corpo e introduz pequena deriva relativa entre os três
  osciladores; em valores altos recupera presença, acelera a cadeia e aumenta
  o atrito possível nas rotas. É uma interpretação musical segura, não um
  modelo de alimentação elétrica real nem uma indicação para alterar circuitos
  físicos;
- `silêncio/erro de áudio`: o painel continua informando clock e endereço;
- `CV`: alterar uma etapa muda somente quando o 4051 a seleciona.

## Conexões como matéria sonora

Uma ligação de feedback deve declarar seus elementos: resistor (nível/mistura),
diodo (direção, retificação ou limiar), capacitor (memória, acoplamento ou
slew) e carga/resposta de saída. O painel não deve desenhar uma linha como se
ela fosse neutra: ela precisa informar o que transforma, de onde vem e para
onde retorna.

Na primeira voz digital, quatro sinais de retorno são distintos: direto,
retificado, capacitivo e pulsado por limiar Schmitt. O controle de feedback
deve ser dosável e o master continua posterior a essas relações, como proteção
de saída — não como freio criativo interno.

Cada um dos três osciladores audíveis possui FREQ, MIX e FORMA. FREQ estabelece
uma relação própria de 0,125x a 4x com a frequência definida pela CV; MIX regula
sua presença; FORMA percorre de modo contínuo senoide, triangular, serra e
quadrada, incluindo as regiões entre elas. A CV selecionada e a ENERGIA movem
as relações entre as frequências, de forma que o gesto de formar uma onda nunca
fica isolado da composição sequencial.

## Módulos ativos e módulos em formação

| Módulo | Situação | Relação sonora |
| --- | --- | --- |
| `SCANNER · 8 + 8 CV` | ativo; extensão modular | o núcleo histórico 40106 → 4040 → 4051 lê oito tensões; uma segunda banca virtual explicita a extensão para 16, sem reescrever a história do Objeto 5 |
| `TRÊS OSCILADORES` | ativo; estudo autoral | três frequências, misturas e formas atravessadas pela energia e pela CV |
| `CONTOUR · ADSR` | ativo; estudo autoral | ATT, DEC, SUS e REL modelam a presença da voz e tocam também a CV do filtro de modo sutil |
| `4069 · VCF` | ativo; estudo autoral | LPF/BPF, frequência, ressonância e profundidade de CV; não emula o CI |
| `LFO` / `RING` / `NOISE` | ativos; estudos autorais | LFO percorre lento até audível; ring é multiplicação contínua; ruído branco entra antes do filtro |
| `SAMPLE & HOLD` | ativo; estudo autoral | captura uma fonte de ruído e a mantém em degraus; ruído contínuo pode tornar-se memória rítmica |
| `REVERB` / `PHASER` / `FLANGER` | ativos; estudos autorais | memórias de taps, quatro estágios de fase e atraso curto modulado, todos posteriores ao filtro e anteriores à saída segura |
| `PORTAS` | ativo; infiltração de feedback | chaves combináveis `FB`, `DIODE`, `CAP`, `PULSE`, `TRANS`, `REFLUX` cedem passagem para transformações de retorno |
| `PULSO DUPLO` | em pesquisa | dois temporizadores 555/556 em relação de clock e burst, deslocados do APC para controle e rota |
| `SCANNER DIODO` | em pesquisa | uma leitura de oito posições em que cada percurso declara diodo, limiar e consequência |
| `MIX ±4` | em pesquisa | quatro entradas bipolares, soma, inversão e comparação como possibilidade herdada do campo do Objeto 4 |
| `RING · CONTÍNUO` / `XOR · BORDAS` | em pesquisa | multiplicação analógica e modulação lógica permanecem módulos diferentes |

`CONTOUR · ADSR` usa a linguagem comum de envelope, mas não é uma cópia dos
esquemas 4093/4066/TL084 ou J. Jacky estudados. A cada mudança de etapa aberta,
o contorno é disparado; uma etapa mutada solta a cauda. No `STOP`, a proteção de
saída continua tendo prioridade sobre qualquer resíduo de envelope.

## Dinâmica, efeito e infiltração por etapa

Cada um dos 16 steps possui quatro gestos: `CV`, `AMP`, `FX` e `MUTE`. CV define
a tensão/razão; AMP é dinâmica local, sem alterar a programação das outras
etapas; FX dosa a presença daquela etapa nos módulos globais de reverb, phaser
e flanger; MUTE abre silêncio sem apagar as três relações anteriores.

As `PORTAS` não são efeitos fechados: são passagens para infiltrações de sinais.
`FB` devolve relação direta; `DIODE` impõe direção/retificação; `CAP` retém e
solta parte do movimento; `PULSE` devolve borda/limiar; `TRANS` aplica uma
saturação assimétrica digital inspirada na ideia de estágio transistor; `REFLUX`
devolve a diferença entre sinal recente e memória capacitiva, como uma corrente
que retorna alterada. Mais de uma pode ficar aberta, formando retorno composto.
São estudos originais seguros, não emulações de componentes específicos. O
próximo desenvolvimento é permitir que essas portas recebam também LFO, ruído,
outro scanner ou memória, e não apenas a própria voz.

## Registro de take

`REC` grava o sinal estéreo final — após mixer, efeitos e proteção de saída —
em WAV PCM de 24-bit. A escrita ocorre em thread separada do áudio. Cada take
tem máximo de cinco minutos, evitando gravação involuntária sem limite.

O destino padrão é `Música/Antitotem Objeto Sonoro`; a variável de ambiente
`ANTITOTEM_RECORDINGS_DIR` permite definir outro diretório antes de abrir o
aplicativo. Os WAVs recebem título, artista, comentário de take ao vivo e data
em metadata RIFF/INFO quando suportado pelo leitor. Não há sobrescrita: cada
nome recebe data e hora. O botão REC encerra/fecha o arquivo ao desligar ou ao
atingir o limite.

## Estudos de espírito CMOS

O seletor de núcleo não nomeia clones de CIs. São experimentações autorais que
mantêm relações que importam em cada família e as deslocam para o contexto do
Objeto:

- `40106 · limiar/pulso`: comutação Schmitt, limiar e pulso moldam a mistura;
- `8038 · formas`: três formas contínuas e sua passagem recebem prioridade;
- `4069UB · deriva`: ganho assimétrico e deriva dependente de energia tornam a
  relação menos estável.

Cada estudo usa as formas e rotas do mesmo instrumento; não procura reproduzir
pinagem, tolerâncias, ruído, consumo ou circuito interno dos componentes. A
diferença é indispensável: o 40106 continua clock e matéria pulsada, o possível
8038 torna-se uma abertura formal, e o 4069UB uma postura de instabilidade.

Entre Objetos, as mesmas quatro rotas podem ser combinadas: `DIRECT`, `DIODE`,
`CAP` e `PULSE`. Dois osciladores auxiliares podem entrar como áudio, controle

## Núcleos modulares e proliferação

O Objeto não deve se fixar em um único núcleo trocável. Ele começa com um
**núcleo-base** — `40106 → 4040 → 4051 → voz` — e pode acolher instâncias
adicionais de famílias estudadas. Cada instância é um módulo com entradas e
saídas explícitas de `CLOCK`, `CV`, `ÁUDIO`, `GATE` e `RETORNO`; ela é uma
relação sonora autoral inspirada em um CI, não uma simulação de sua pinagem.

Assim, adicionar um segundo `4069UB` significa criar uma segunda etapa de
deriva/ganho que pode receber áudio da voz, CV do scanner ou retorno; ele não
substitui o primeiro. Um segundo `4040` pode dividir ou deslocar o clock de uma
rota independente, produzindo outra periodicidade de gate/endereço. Um `4006`
acrescentado funciona como linha de memória: empurra estados discretos em
atrasos selecionáveis, podendo alimentar CV, gate, ruído amostrado ou retorno.

O painel futuro deve representar isso como uma pequena malha, e não como um
menu de presets:

```text
BASE: 40106 ──clock──> 4040 ──endereço──> 4051 ──CV──> VOZ
                         │                    │
             + 4040 ──gate/divisão            + 4006 ──memória/CV
                         │                    │
                      + 4069UB ──ganho/deriva ──> VCF / RETORNO
```

Cada inserção terá: tipo de família, quantidade de instâncias, posição na rota,
dosagem e um pequeno conjunto de portas. As combinações podem coexistir, ser
desativadas ou salvas como configurações de objeto. Limites explícitos de ganho,
feedback e saída continuam obrigatórios: proliferar módulos deve ampliar a
escuta, não produzir clipping ou acúmulo fantasma.

## Espacialidade como topologia

Estéreo é apenas a projeção final de uma espacialidade mais ampla. Cada voz ou
módulo pode ocupar uma coordenada `X/Y/Z` dentro da topologia energética:

- `X` abre o campo lateral e, na saída, torna-se distribuição entre L/R;
- `Y` é proximidade/materialidade: peso seco, profundidade de filtro e entrada
  nas memórias de espaço;
- `Z` é órbita/altura: diferença de fase, deriva, circulação ou atravessamento
  de uma rota de retorno.

Esses eixos não pretendem simular uma sala realista. São coordenadas de
composição para que um segundo oscilador, um 4006 ou uma rota de diodos possa
aproximar-se, afastar-se, circular ou infiltrar-se nos demais. O mixer deve
mostrar, por voz, `ganho`, `X/Y/Z`, solo, mute, retorno e presença de sinal; o
osciloscópio estéreo mostra a projeção L/R, enquanto um mapa revela as posições
e as ligações ativas.

Famílias iniciais do acervo de esquemas: CMOS `40106/4093` (limiar e gate),
`4040/4020/4017/4520` (contagem, divisão e endereçamento), `4051/4067/4053`
(scanner e comutação), `4006/4015` (memória e deslocamento), `4070/4077/4011`
(lógica e modulação) e `4046` (VCO/PLL e seguimento); temporizadores
`555/556`; osciladores e OTAs `8038/LM13600/LM13700`; op-amps `741`, `LM358`,
`TL07x`; estágios de potência `386`; além de transistor, JFET/MOSFET, diodo,
capacitor, resistor, BBD/linha de atraso, alto-falante e realimentação física.
O núcleo é definido pela relação que produz — e não pelo encapsulamento. Cada
família ou componente só entra no código quando sua relação sonora e sua
fonte/crédito estiverem documentados.
ou perturbação em cada Objeto. São possibilidades de timbre e conexão, não
presets fechados: a interface final deve permitir ouvir cada combinação antes
de nomeá-la ou estabilizá-la.

## Linguagem

Fundo quase-preto quente, serigrafia clara, vermelho vivo de Antitotem, rosas
de matéria/memória, verde de fluxo e ocre de madeira. A nova paleta nasce do
cabeçalho fotográfico do Antitotem: campo de flores rosadas, folhagem, objeto
de madeira e a marca vermelha. O azul-cobalto dos pequenos knobs físicos entra
somente como acento de seleção de núcleo e controles secundários; não é fundo
nem sinal de alarme. A paleta é funcional: sinal de relógio, seleção e áudio
não dependem apenas de cor; os rótulos mantêm a mesma informação.

### Regra de ocupação

O painel tem prioridade sobre a densidade. Todo módulo reserva, nesta ordem,
uma faixa para título, uma faixa para o gesto e uma faixa para valor/leitura.
Rótulos nunca podem ficar sob sliders, botões ou caixas de texto. Na referência
de 1920×1080, a superfície única usa divisões fixas mínimas; abaixo disso, o
instrumento troca para páginas de trabalho em vez de encolher controles ou
produzir sobreposição.

### Paleta material: Geada + Objeto Sonoro 4

As fotografias autorais de **Geada** (c. 2008) mostram madeira/argila talhada,
metal, sombra profunda e luz quente incidindo sobre poucos controles. A
fotografia do **Objeto Sonoro 4** mostra uma base de madeira e placa perfurada
ocre, componentes pretos, parafusos metálicos e fiação colorida aparente. A
interface não deve transformar essas fotos em decoração ou simular material de
modo literal: ela usa suas relações de contraste para tornar as rotas legíveis.

| Papel | Cor-base | Leitura no objeto e no aplicativo |
|---|---|---|
| vazio / sombra | preto-terra `#110D0E` | campo de escuta e respiro; nunca área a preencher automaticamente |
| suporte / placa | ocre de madeira `#C98A2B` | títulos de núcleo, superfícies de módulo e estado estrutural |
| madeira / contorno | castanho `#9A5A27` | limites, agrupamentos e separações sem caixas pesadas |
| metal / serigrafia | rosa-claro `#FFD4BC` | texto, valores, parafusos conceituais e informação estável |
| fluxo / clock | verde-folhagem `#3E7440` | tempo, endereço, variação contínua e trajetória dos steps |
| memória / retenção | rosa-flor `#E96C9C` | S&H, estado retido, loop e percurso conservado |
| retorno / perigo | vermelho Antitotem `#DD2F37` | feedback, mute, gravação e energia que pode acumular; sempre acompanhado de rótulo |
| controle físico | azul-cobalto `#2477B8` | núcleo, seleção e pequenos knobs; acento restrito inspirado no objeto fotografado |
| áudio / matéria ativa | rosa de voz `#F18AAE` | voz, mistura e zonas em escuta |

As regras são: preto e ocre sustentam; azul, verde, rosa e vermelho aparecem
apenas como sinais; o rosa-claro garante contraste textual. Não há arco-íris
permanente. Uma porta sem seleção permanece escura; ao ser ativada, ganha a
cor da sua função e não apenas uma versão mais brilhante de si mesma.

Essa paleta permite que `RND`, S&H e feedback tenham caráter visual distinto
sem abandonar a noção de gambiarra: fios são relações provisórias, placa é
matéria disponível e o vazio mantém a escuta aberta.

## Deriva, memória e duração de faixa

O modo **DERIVA** não é um aleatorizador de valores independentes. Ao ser
ligado, ele captura o estado presente do objeto — CV, amplitude e envio FX dos
16 steps, a rota de feedback e o ganho de retorno. A cada retorno da frase,
retém essa matéria e a desloca em pequenas proporções. Rotas já atravessadas
podem reaparecer, uma porta pode se abrir ou fechar, e os envios podem se
adensar sem apagar a identidade que antecedeu a mudança. O limite de `FB GAIN`
continua deliberadamente contido para que o retorno seja uma prática de escuta,
não um acidente de saída.

`REC` passa a tratar a gravação como uma faixa temporal, não somente como um
arquivo. O botão **FAIXA** escolhe 1, 3 ou 5 minutos; a duração é sempre um
limite máximo, e parar manualmente continua possível. Com DERIVA ativo, as
proporções temporais nomeiam acontecimentos próprios do Antitotem:

- **germinação** (0–18%): memória capturada, matéria inicial ainda reconhecível;
- **infiltração** (18–46%): a primeira rota retorna e contamina a frase;
- **adensamento** (46–74%): memória e rota são deslocadas novamente;
- **coda** (74–100%): o retorno e o espaço são reduzidos, preservando uma saída
  possível em vez de um corte imposto.

Esses nomes não são uma forma fixa de canção. São um vocabulário de percurso:
uma peça pode começar por qualquer topologia, sustentar um só acontecimento ou
interromper-se antes da coda. A duração organiza a escuta sem normalizar o
incerto.
