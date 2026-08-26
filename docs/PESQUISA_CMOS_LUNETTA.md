# Pesquisa — CMOS 4000 e práticas Lunetta

## Escopo e posição

Este é um levantamento da **família musicalmente pertinente** da série CMOS
4000 e da linhagem Lunetta; não pretende listar todos os CIs 4000 já
fabricados. Ele serve ao desenvolvimento do *Antitotem - Objeto sonoro 5* e
separa três camadas:

1. o que o componente efetivamente faz, conferido em datasheet;
2. o que a comunidade Lunetta fez musicalmente com funções lógicas simples;
3. o que Antitotem pode inventar a partir disso, sem reivindicar reconstrução
   do Objeto histórico nem copiar circuitos ou código alheios.

“Lunetta” é o nome posterior de uma linhagem de instrumentos e esculturas
sonoras associados a Stanley Lunetta. A comunidade privilegia blocos lógicos
baratos, entradas e saídas expostas, patching e descoberta por escuta — mais
computador musical temporário do que sintetizador convencional. Uma descrição
contemporânea documenta clocks/oscilladores 40106, contadores, registradores e
XOR como seu vocabulário; o próprio fórum de homenagem registra que Lunetta não
apreciava necessariamente esse nome. [Make](https://makezine.com/article/craft/music/lunetta-cmos-synth/)
[arquivo comunitário](https://electro-music.com/forum/viewtopic.php?t=36130)

Antitotem dialoga com essa abertura, mas sua origem é própria: breadboard,
componentes mediando conexões, escuta intuitiva, incerteza e objetos sonoros.
Não será rotulado como Lunetta nem reduzido a uma estética de lógica digital.

## Fundamento material

CMOS 4000 é lógica de nível de tensão. Uma onda quadrada, um clock, um estado
de contador e uma porta XOR são simultaneamente sinais elétricos e materiais
musicais. Quando interligados, produzem ritmo, subdivisão, batimento, memória,
interferência e rota — não precisam começar como “nota”.

O CD40106B contém seis inversores Schmitt; sua histerese torna possível um
oscilador RC de relaxação. O CD4040B é um contador/divisor binário de 12
estágios. O CD4017B é um contador Johnson com dez saídas decodificadas. Essas
três funções explicam boa parte do vocabulário de pulso, divisão e sequência.
[40106B — TI](https://www.ti.com/lit/ds/symlink/cd40106b.pdf)
[4040B — TI](https://www.ti.com/product/CD4040B)
[4017B — TI](https://www.ti.com/product/CD4017B)

## Famílias úteis como matéria musical

| Família / exemplos | Função técnica | Possibilidade musical | Situação no Antitotem |
| --- | --- | --- | --- |
| Inversores Schmitt: 40106, 4093 | limiar com histerese; 40106 possui seis inversores | clocks, pulsos, osciladores RC, gates e perturbação de limiar | 40106 é central; 4093 é candidato para gates oscilantes |
| Inversores sem buffer: 4069UB; pares MOS acessíveis: 4007UB | inversão/ganho em comportamento menos rigidamente digital; 4007 expõe transistores | oscilação, shaping, limiar, assimetria e deriva | estudo digital `4069UB · deriva`; 4007 é campo futuro |
| Portas: 4001 NOR, 4011 NAND, 4070 XOR, 4077 XNOR | lógica combinatória | ring/mod lógico, intermodulação de pulsos, ritmos e comparações | XOR/XNOR ainda é oportunidade explícita entre os objetos |
| Memória: 4013/4027 flip-flops, 4006/4015/4094 shift registers | armazenamento e deslocamento de bits | alternância, retenção, padrões longos, LFSR/pseudoaleatoriedade | não duplicar o núcleo de randomização já existente no Rasgo; usar como memória de rota |
| Contadores: 4017/4022, 4024/4040, 4029 | estados one-hot, divisão binária ou contagem reversível | passos, sub-oscilações, polirritmia, reinício e saltos | 4040 já organiza o scanner; 4017 merece experimento one-hot próprio |
| Seleção: 4051/4052/4053, 4066 | multiplexação/demultiplexação analógica e chaves bilaterais | scanner de CV, escolha de rota, troca de fonte, memória de conexão | 4051 é central; 4066/4053 podem materializar patch points |
| PLL/VCO: 4046 | VCO linear e comparadores de fase | quase-sincronia, captura, fuga, frequência contra frequência | candidato para relação entre dois Objetos, não para “afinar” tudo |
| Detector PLL: 567 | detector de tom com VCO/PLL (não é CMOS 4000) | eventos quando duas frequências se encontram ou deixam de se encontrar | pertinente ao Objeto 1; função histórica exata desconhecida |
| Temporizadores: 555/556 | 555 combina comparadores, flip-flop e descarga; 556 reúne dois temporizadores | astável, monoestável, envelopes, bursts, APC e relógios que se bloqueiam | página 7 do acervo traz 555/4017; não fundir automaticamente ao esquema da página 9 |
| Amplificador operacional: 741 | bloco analógico de ganho, soma, integração, comparação e buffer (não é CMOS) | transformar pulso em rampa, misturar CV, limitar, devolver feedback ou comparar níveis | listado no Objeto 5; aplicação histórica específica a verificar |
| Amplificador de áudio: 386 | estágio de potência de baixa tensão (não é CMOS) | tornar audível uma mistura já preparada; sua saturação pode ser parte do caráter | listado nos Objetos 1 e 5; preservar como saída, não como oscilador |
| Gerador analógico: ICL8038 (não é CMOS 4000) | VCO/gerador de seno, triângulo e quadrada | uma abertura contínua em diálogo com a matéria de pulso | pista autoral a verificar; não confirmado no Objeto histórico |

O CD4051B seleciona um de oito caminhos analógicos mediante três endereços;
4052/4053 ampliam a ideia para dois 4:1 ou três 2:1. O CD4066B é uma chave
bilateral. Eles não são “som” por si: tornam a escolha de um sinal uma ação
audível. [405xB — TI](https://www.ti.com/lit/ds/symlink/cd4051b-mil.pdf)

### Esquema 4020 + dois 4051: matriz de 64 posições

O esquema fornecido pela autoria em `cmos_sequencer_4051_4020.jpg`, identificado
como “Circuito de salida que emplea un CD4020”, mostra um uso especialmente rico
do contador. Q6, Q7 e Q8 do CD4020 formam os três endereços de um CD4051;
Q9, Q10 e Q11 endereçam o segundo. Os dois chips selecionam, respectivamente,
uma das oito linhas e uma das oito colunas de uma matriz R0–R63. A topologia é
portanto uma memória analógica de **8 × 8 = 64 posições**, e não apenas uma
sequência linear de oito passos.

```text
clock → CD4020 → Q6,Q7,Q8 → 4051 (8 linhas)
                   Q9,Q10,Q11 → 4051 (8 colunas)
entrada X → matriz de 64 ajustes → saída Y
```

A fotografia do esquema confirma contagem, endereçamento duplo e matriz de
ajustes. A função elétrica exata de X, Y e das ligações V/W ainda deve ser
conferida na fonte original antes de uma reprodução física. No software, ela
abre uma expansão legítima: matriz 8×8 de CVs com leitura visual por linha e
coluna, apresentada como estudo inspirado no esquema, não como fato histórico
do Objeto Sonoro 5.

### Sample & Hold como memória de tensão

O segundo esquema compartilhado, “Ray's Single Chip Simple Sample & Hold”, é
creditado no próprio desenho a **Ray Wilson / Music From Outer Space**. Um
TL084 provê os estágios de buffer/controle e dois JFETs MFPF102 formam a chave
analógica: quando chega `TRG`, a tensão de entrada é amostrada; entre triggers,
o capacitor C1 a retém e `CV OUT` permanece aproximadamente estável. O desenho
prevê fontes como ruído, LFO ou envelope.

Em diálogo com a matriz 4020+4051, S&H pode capturar a tensão da posição atual
antes que o contador prossiga: seleção e memória deixam de ser a mesma ação.
É referência técnica creditada, não código ou circuito copiado para Antitotem.

### 4006: registrador como atraso e reverberação clockada

Dois esquemas compartilhados pela autoria (`cmos4006_reverb.png` e
`cmos4006.gif`) usam CD4006 como linha de atraso digital de um bit. Um clock
desloca a amostra de entrada pelos estágios do registrador; taps em diferentes
posições são desacoplados, atenuados e misturados em estágios TL074. O primeiro
desenho também mostra 40106 como fonte de clock. O resultado esperado não é
reverberação transparente: resolução de um bit, taxa de clock e mistura dos
taps deixam atraso granulado, metálico, pulsante e suscetível a variação de
velocidade.

Essa técnica interessa ao Antitotem como memória de sinal e espaço produzido
pela própria lógica. Uma versão digital futura deve preservar a relação entre
clock, comprimento de registro, taps e realimentação, em vez de acrescentar um
reverb genérico. A autoria/licença dos dois esquemas não está identificada na
imagem fornecida; são referências de pesquisa, não material a reproduzir sem
verificação.

### LM13600: filtro OTA de quatro células

O esquema `cmos13600.gif`, compartilhado pela autoria, apresenta um filtro de
quatro células de transcondutância em cascata, com controle comum de frequência
e realimentação de ressonância para a entrada. Diferentemente do VCF 4069, a
frequência é determinada pela corrente de controle dos OTAs: o filtro pode
manter uma relação mais direta entre CV e sua resposta, enquanto as quatro
etapas produzem uma inclinação mais profunda.

O LM13600 não pertence à série CMOS; é um OTA duplo. Seu sucessor disponível,
LM13700, contém dois amplificadores de transcondutância controlados por corrente,
diodos de linearização e buffers. A documentação também informa que os buffers
do LM13700 diferem dos do LM13600, portanto eles não devem ser tratados como
idênticos sem revisão do circuito. [TI LM13700](https://www.ti.com/product/LM13700)

O desenho parece usar quatro células OTA; uma futura reconstrução física deve
confirmar se requer dois encapsulamentos duplos e levantar a autoria/licença da
imagem antes de reutilizar o esquema. No software, ele sugere um segundo módulo
de filtro, `OTA 4P`, separado do `4069 · VCF`: a escolha entre os dois deve ser
uma escolha de matéria sonora, não uma troca cosmética de curva de filtro.

#### Outras famílias de transcondutância a investigar

| Família | Diferença musical/técnica | Lugar possível no Antitotem |
| --- | --- | --- |
| CA3080 / CA3080A | OTA simples, histórico e pouco linearizado; ganho depende diretamente de IABC | estudo de VCA, limiar e filtro mais cru; componente de acervo/mercado legado, não escolha automática de produção |
| NE5517 | dois OTAs controlados por corrente; parente funcional do LM13700 | alternativa para filtro, VCA e relações duplas; exige checagem de disponibilidade e pinagem |
| LM13700 | dois OTAs, diodos de linearização e buffers; fabricação corrente | base mais direta para VCA, S&H, VCF e mixer controlado por CV |
| OPA860 / OPA861 | OTA de largura de banda muito alta, controle externo de corrente e foco em pulsos/controle rápido | transientes, bursts, clocks que entram no domínio de áudio e filtros muito rápidos; não é substituto simples do LM13700 |
| SSI2164 / V2164 / SSM2164 | não é OTA estrito: quatro VCAs de corrente com controle exponencial | mixer de quatro canais, matriz de níveis e rotas dinâmicas, especialmente coerente com o Objeto 4 |

O CA3080 documenta explicitamente uma saída de corrente proporcional à corrente
de polarização e aplicações como ganho, sample-and-hold e multiplexação; é
excelente para compreender a origem do gesto OTA, mas é uma peça histórica.
[CA3080 — datasheet Renesas legado](https://www.digikey.com/en/htmldatasheets/production/71658/0/0/1/ca3080e.html)
O NE5517 é outro OTA duplo de corrente controlada.
[NE5517 — onsemi](https://www.onsemi.com/download/data-sheet/pdf/ne5517-d.pdf)
O OPA860 acrescenta buffer e largura de banda de 80 MHz; sua vocação é outra,
mais rápida e menos imediatamente “synth clássico”.
[OPA860 — TI](https://www.ti.com/product/OPA860)
O SSI2164 fornece quatro VCAs de corrente independentes e deve ser chamado de
VCA, não OTA, para que a documentação não confunda as categorias.
[SSI2164 — Sound Semiconductor](https://www.soundsemiconductor.com/downloads/ssi2164datasheet.pdf)

### Double-Well Chaos: tensão contínua entre dois poços

O esquema `doublewellchaos_ianfritz.jpg` é creditado a **Ian Fritz, 2007**. Dois
integradores produzem os estados X e Y; uma célula não-linear com diodos define
o comportamento de “dois poços”; rate, damping e drive alteram a trajetória.
As saídas X, Y e NL são tensões relacionadas, não três osciladores independentes.
O interesse musical está no sistema alternar permanência, órbita e salto entre
regiões, com a possibilidade de o gesto de drive intervir no próprio regime.

Essa é uma referência para um futuro módulo autoral `DOIS POÇOS`: fonte de CV
contínua para frequência, filtro, rotas ou realimentação. O Rasgo já possui
mapas caóticos próprios; não se deve transportar um deles automaticamente nem
copiar o esquema de Ian Fritz. Autoria e licença de reutilização: a verificar.

### Mixer-Comparator: limiar como acontecimento

O PDF `mix-comp-2002.pdf` foi lido integralmente. A página identifica
**Richard Brewster**, “MOTM-Compatible Mixer-Comparator”, construído sobre PCB
CGS23, novembro de 2002. Com dois TL072, ele oferece mixer de duas entradas,
saída invertida e seção comparadora com BIAS/COMP IN, produzindo MIX, INV e COMP.
O comparador converte uma relação contínua em decisão/evento; não é apenas um
amplificador de saída.

Em conjunto com Double-Well, o mixer pode cruzar X e Y, e o comparador pode
abrir um gate, trocar uma rota de 4051, avançar/reter o clock ou devolver drive.
Essa combinação propõe comportamento emergente por escuta. Para Antitotem, a
implementação deve ser original e nomear claramente entradas, limiar e destino;
o PDF permanece referência creditada, sem cópia de PCB, esquema ou valores.

### Mixer polarizador: soma, subtração e cruzamento por zero

O PDF `Simple DIY Eurorack Polarizing mixer.pdf` foi inspecionado visualmente e
por seus metadados: autor “phono”, título homônimo, 31 dez. 2010. O desenho
identifica também kit Synovatron DIY e `dinsync.info`. Quatro entradas passam
por controles de 100 k e chegam a um somador em amplificador operacional duplo;
uma chave alterna entre mistura normal e polarizadora. No modo polarizador, um
controle central representa zero e permite contribuição positiva ou negativa de
cada entrada.

Esse é um princípio especialmente importante para o mixer de quatro canais
confirmado no Objeto Sonoro 4: além de somar quatro sinais, ele poderia opô-los,
anulá-los ou inverter a direção de uma modulação. Uma futura interpretação
`MIX ±4` deve expor o zero, preservar margem de saída e declarar quando está em
modo normal ou polarizador. O esquema é referência creditada; não reutilizar
PCB, valores, arte ou desenho sem verificar licença.

### Gerador contínuo MFOS: triângulo, quadrada e senoide

O esquema `revised-battery-function-generator-nov-2013.gif` identifica
**Ray Wilson / Music From Outer Space LLC**, copyright 2013/14. Ele articula
TL074 e LM13700 como gerador de funções alimentado por bateria. A leitura de
princípio é: integrador para triângulo, comparador para quadrada e célula OTA
de shaping para senoide, com controles separados de frequência grossa/fina,
faixa, forma e nível.

Essa separação é central para o Antitotem. O módulo futuro `OSC CONTÍNUO` deve
oferecer saídas/formas simultâneas e possibilidade de mistura ou seleção por
rota, em diálogo com os pulsos 40106; não deve ser apenas um seletor cosmético
de waveform. A implementação será autoral e não reutilizará esquema, valores,
placa ou imagens MFOS sem verificar a licença aplicável.

### Semi-random: quatro relógios e escada R–2R

O esquema `semirandom13.jpg` se identifica como **synthgeek, Kevin B.,
“Semi-Random Source”, rev. 1.0, 20 nov. 2010**. Quatro osciladores CD40106 em
taxas próprias entram numa escada R–2R; o somatório ponderado é bufferizado por
LM358 e passa por controle de nível e glide. O texto do próprio desenho observa
proximidade não intencional com o “Psycho LFO” de Ken Stone e que o resultado
vai de modulação lenta a áudio/ruído conforme as taxas sobem.

O ponto conceitual é decisivo: não se trata de aleatoriedade por ruído, LFSR ou
seed. É uma sequência determinística de combinações de quatro estados binários
que se desencontram no tempo; por isso parece semi-aleatória e mantém vínculo
audível com seus clocks. Uma versão Antitotem, `QUATRO RELÓGIOS`, deve expor as
quatro taxas, nível e glide, e poder alimentar CV de filtro, frequência, matriz
4051 ou drive do Double-Well. Não copiar esquema, valores ou layout; licença a
verificar.

### Random stepwave: 4006 + XOR + soma de taps

O esquema `t_cmos_random_stepwave_lfo_test_circuit_174.gif` emprega 4006,
4070 e 40106. Um clock 40106 desloca estados pelo registrador; portas XOR
devolvem combinações de taps à entrada, constituindo uma realimentação de
registro pseudoaleatória. Taps selecionados são somados por resistores de 4k7
na saída e formam uma tensão escalonada. Com clock lento, ela é LFO de degraus;
com clock rápido, torna-se fonte digital áspera de áudio.

Ele é conceitualmente diferente do `QUATRO RELÓGIOS`: o primeiro é memória
binária com realimentação XOR, o segundo é superposição determinística de
osciladores independentes. Para Antitotem, ambos podem coexistir desde que suas
origens permaneçam legíveis no painel. Autor e licença do esquema da imagem: a
verificar; estudar o princípio, não reutilizar desenho ou valores.

### 4015: memória que se reescreve

O esquema `cmos_phobos_4015.gif` traz no próprio desenho o crédito
**PHOBoSapiens Inc., Jabberwock, rev. 1.0, 19 maio 2012**. Ele articula dois
4015, inversores 40106 com redes RC, LM358, chaves de reset/realimentação e
saída por mistura amplificada. A leitura de princípio é uma máquina de estados:
bits deslocados retornam, encontram clocks de velocidades diferentes e são
convertidos novamente em áudio/controle. Não é um sequenciador de notas comum.

Para Antitotem, a diferença produtiva é usar uma memória 4015 para escolher e
reter **rotas** — por exemplo, quais retornos resistor/diodo/capacitor ficam
abertos —, não para importar a topologia, os nomes, a placa ou o som do
Jabberwock. Autoria/licença de reutilização do esquema: a verificar.

### 4070: modulação por diferença lógica

O desenho `cmos_xormodulator4070.jpg` credita “XOR Ring Mod”, CTM Berlin, 2010,
e mostra 4070 (XOR), 4077 (XNOR) e 4011 (NAND). XOR fica alto quando as duas
entradas diferem; em taxa de áudio, duas sequências de pulsos podem produzir
uma modulação binária áspera, aparentada a ring modulation, mas não equivalente
a multiplicação analógica. A inversão/defasagem das relações importa tanto
quanto a frequência de cada entrada.

O módulo autoral correspondente para Antitotem deverá ser `XOR · bordas`, com
duas fontes e uma saída de rota/áudio, opção de inverter uma fonte e nível
protegido. Ele deve ser separado da cross-modulation contínua já presente nos
osciladores. Há um símbolo Creative Commons no desenho; a licença exata e a
fonte devem ser verificadas antes de qualquer reutilização de material visual
ou elétrico.

### Ring mod analógico: AD633, não XOR

O esquema `ring_modulator/m_rm.gif` se identifica como **ringmodulator (c)
Roman Sowa 2000, 2002**. A pesquisa confirma que o CI central é o **AD633**,
multiplicador analógico de quatro quadrantes; OPA482 condicionam modulação e
portadora antes e depois do multiplicador, enquanto P1/P2 ajustam equilíbrio e
P5 o ganho de saída. A fonte comunitária o lista como “Balanced Modulator aka
Ringmodulator by Roman Sowa”.

O resultado é produto contínuo de duas tensões, com somas/diferenças espectrais
quando ambas estão em áudio. Isto não é equivalente ao `XOR · bordas`, em que
a saída depende de diferença entre níveis lógicos. Um futuro `RING · CONTÍNUO`
deve ser módulo separado, com duas entradas, balanceamento e saída protegida;
nenhum esquema, valor ou layout Roman Sowa será copiado. [Referência
comunitária](https://electro-music.com/wiki/pmwiki.php?n=Schematics.BallancedModulatorAkaRingmodulator)

O CD4069UB é documentado como inversor **unbuffered**, e o CD4007UB expõe três
MOS N e três MOS P para formar amplificadores, shapers, detectores de limiar e
osciladores. Isso explica por que são referências mais interessantes para
regiões de ganho e assimetria do que um inversor lógico convencional.
[4069UB — TI](https://www.ti.com/product/CD4069UB)
[4007UB — TI](https://www.ti.com/product/CD4007UB/part-details/CD4007UBPWR)

### 4069 como filtro controlado por tensão

O esquema compartilhado `cmos_vcf_philipbaljeu.jpg` é assinado por **Phil
Baljeu**, *Ruin Electronics*, 19 de novembro de 2012. Ele usa dois inversores
4069N como estágios ativos em uma rede RC, oferecendo entrada de áudio, ganho,
frequência, ressonância, profundidade de CV e seleção LPF/BPF. A anotação do
próprio esquema diz que o inversor pode ser substituído por outro CMOS **não
Schmitt**, convidando à experimentação.

O princípio relevante é usar o inversor polarizado em região analógica como
elemento de ganho dentro de uma rede de filtragem — uma relação bem diferente
do 40106 como gerador de limiar/pulso. Ele é uma referência especialmente
coerente para o filtro confirmado na memória do Objeto Sonoro 4. Ainda assim,
não se deve copiar o circuito, valores, desenho de placa ou sugerir que era a
topologia do filtro histórico: licença e origem do esquema devem ser verificadas
antes de qualquer reutilização física. No software, uma futura interpretação
precisa distinguir `4069UB · deriva` (estudo atual de voz) de `4069 · VCF`
(módulo de filtro ainda não implementado).

O CD4046B combina VCO e comparadores de fase: sua imagem musical não é “um
oscilador afinado”, mas aproximação, captura e escape entre relógios. [4046B —
TI](https://www.ti.com/product/CD4046B)

## A borda híbrida: 555/556, 741 e 386

O campo Antitotem não é purista CMOS. A lógica precisa atravessar tempo,
tensão, soma e escuta. O 555 transforma redes RC em ciclos e eventos; o 556
permite duas dessas células no mesmo invólucro. Eles são fortes para uma camada
de ataques, bursts, clocks interrompidos e para o parentesco histórico do Atari
Punk Console, mas não devem substituir o 40106: as duas topologias têm limiares
e gestos diferentes.

O 741 pode ser lido como ponte: integrador para triangular, somador para CVs e
feedbacks, comparador para eventos e buffer entre domínios. Em uma reconstrução
contemporânea, a escolha do amplificador operacional precisa considerar as
tensões reais e a faixa de sinal do sistema; “741” na lista do Objeto 5 é dado
de acervo, não uma recomendação automática para todo novo módulo.

O 386 deve ficar no fim do fluxo: recebe um sinal condicionado, oferece ganho
para escuta e pode colorir quando levado a saturação, mas não deve ser usado
como proteção contra conexões erradas. A proteção e o controle de nível vêm
antes dele.

```text
555/556 ou 40106 → contador / registro / mux CMOS → 741 (soma, rampa, limiar)
                                                      → condicionamento → 386 → escuta
```

Essa cadeia é uma proposta de leitura e experimentação; não é esquema alegado
do Objeto Sonoro 5.

## Práticas Lunetta que importam

1. **Patching antes de preset.** Saídas de clocks, divisões, bits e portas são
   disponibilizadas como material; o som resulta de conexões temporárias.
2. **Lógica no domínio audível.** Divisores criam sub-oitavas; XOR mistura
   relações temporais; gates não precisam apenas controlar volume.
3. **Sequência como estado.** 4017, registradores e flip-flops formam padrões
   por circulação e memória, não por piano-roll.
4. **Combinação como timbre.** Mistura resistiva preserva relações de amplitude;
   diodos introduzem direção/retificação. São conexões diferentes, não dois
   nomes para “mix”.
5. **Acidente escutável.** A comunidade valoriza resultados que aparecem ao
   mudar rota e velocidade, mas isso não autoriza curto-circuitar saídas nem
   alimentar CIs fora das especificações.

O levantamento da Beavis Audio é especialmente útil para a cadeia 40106–4040–
4051, mix resistivo versus diodo e competição entre osciladores; é referência
de prática DIY, não especificação do Objeto 5. [CMOS Synthesizers — Beavis
Audio](https://beavisaudio.com/projects/cmossynthesizers/)

## Leitura específica dos Objetos Sonoros

O acervo identifica o Objeto 5 com 40106, 4017, 4051, 4040 e 386. O Objeto 1
dialoga com 40106, 4040, 4051, 567 e 386. O esquema de acervo da página 9 de
`eme.pdf` confirma uma cadeia 40106 → 4040 → 4051, mas **não prova** todas as
conexões físicas, número de controles ou o emprego preciso de cada CI em cada
objeto. Essas informações continuam marcadas como “a verificar”.

O Objeto Sonoro 4, segundo a autoria, é um sintetizador mais explicitamente
modular: vários pequenos módulos interconectados formam seu corpo. Ele oferece
uma referência direta para construir patch points e relações entre blocos, mas
não deve apagar a identidade mais concentrada e específica do Objeto 5.
Há, ao menos, um mixer de quatro canais e um filtro confirmados pela memória
autoral. Eles devem entrar como módulos próprios em uma futura versão do Objeto
4; não se deve presumir a topologia do filtro, nem as quatro fontes, antes de
consultar documentação material.

O inventário parcial recordado é: 40106 (limiar/oscilação), 741 (ponte
analógica), 4049 (inversão/buffer), 567 (detecção PLL), 556 (dois
temporizadores), 4017 (sequência one-hot), 4067 e 4051 (seleção analógica),
4040 (divisão binária) e 386 (saída). A presença de “muito mais” é importante:
ela impede que este inventário vire uma ficção de completude ou um esquema
reconstruído sem evidência.

A fotografia pública do Objeto 4 mostra precisamente essa organização por
blocos: placas perfuradas, agrupamentos de controles e CIs, fiação exposta e
uma área de expansão. Ela é evidência visual de modularidade, não mapa elétrico
do instrumento. [Fotografia do Objeto
4](https://antitotem.arquiviagem.net/media/004antitotem_1000_1.jpg)

### SX-150: interface tátil e módulos legíveis

O diagrama `SX150_SchemV3.PNG` declara ter sido recortado do manual do **Gakken
SX-150**, com valores acrescentados por Sam Hoshuyama a partir de esquemas
parciais e inspeção. Ele organiza controlador de painel de carbono/touch,
entrada externa com envelope follower, LFO, VCO com conversor exponencial,
envelope 555, VCF com ressonância, LM386 e alimentação. Não é uma fonte
CMOS/Lunetta estrita, mas uma referência compacta para tornar módulos híbridos
legíveis, corporais e conectáveis.

Para Antitotem, o aprendizado não é copiar a arquitetura SX-150: é preservar a
clareza de trajetos entre toque, controle, oscilação, filtro e escuta. Um futuro
painel deve fazer as rotas aparecerem como gesto, e não escondê-las em menus.
Autoria e licença do diagrama anotado: a verificar.

O que interessa conservar no software e numa futura reconstrução não é uma
lista decorativa de números de CIs, e sim as relações:

```text
limiar/energia → pulso → divisão/estado → seleção → forma/rota → escuta
                    ↑                         ↓                  │
                    └──── retorno por componente material ────────┘
```

## Experimentos autorais propostos

Os módulos abaixo repetem princípios com diferença e não devem ser apresentados
como circuitos históricos de Stanley Lunetta ou de Antitotem.

| Experimento | Princípio de referência | Diferença Antitotem | Resultado a escutar |
| --- | --- | --- | --- |
| `one-hot ferido` | 4017 | uma etapa pode reter, pular ou devolver pulso ao clock | padrões que tropeçam sem virar sequência aleatória genérica |
| `árvore de bordas` | 4040 + XOR | usar bordas de divisões diferentes, e não apenas frequências divididas | polirritmia e timbre de interferência |
| `scanner fantasma` | 4051 | alternar endereço principal e endereço atrasado por capacitor virtual | duas leituras do mesmo conjunto de CVs, uma presente e outra residual |
| `memória que vaza` | 4015/4006 + XOR/XNOR | bits determinam rotas de feedback, não notas | conexões que permanecem, derivam e reaparecem |
| `quase-encontro` | 4046/567 | detector produz eventos de rota quando relações se aproximam, sem “corrigir” a afinação | captura, batimento e fuga audíveis |
| `mistura com matéria` | resistor/diodo/capacitor | cada retorno declara seu elemento e tempo | feedback que muda de caráter, não somente de volume |
| `formas em disputa` | 8038 + 40106 | formas contínuas atravessadas por limiar e divisão lógica | seno/triângulo/serra/quadrada nunca isoladas da sequência |
| `ataque que endereça` | 555/556 + 4017/4051 | um burst não abre somente amplitude: escolhe ou altera um endereço | ataques que reorganizam o scanner |
| `rampa que escuta` | 741 + 40106 | integrar um pulso e devolver apenas sua inclinação ao limiar | glissando e sequência interferem sem virar filtro convencional |
| `saída que responde` | 386 | observar o limite de saída como dado de composição, com proteção anterior | cor e compressão sem sacrificar o sistema a clipping permanente |
| `contorno que abre rota` | ADSR + gate | envelope modula simultaneamente presença, profundidade de filtro e uma escolha de rota — nunca somente volume | ataques que desenham conexões e liberações que deixam vestígio |
| `pulso duplo` | 555/556 / APC | os dois tempos podem ser áudio, clock ou evento de scanner, sem reproduzir o APC | densidade e salto temporal entre sequência e timbre |
| `scanner diodo` | 4017 + oito diodos | cada passo altera o caminho por limiar/retificação, e não somente a altura | sequência com assimetrias e pesos materiais |

## Gate e contorno: ADSR como relação, não como adorno

O esquema compartilhado `Dual_ADSR_muff_menciona_problema.gif` atribui o
*ADSR Envelope Generator* a Christopher Macdonald e mostra 4093, 4066, TL084,
2N3904 e diodos. O próprio nome de arquivo informa que há um problema a ser
investigado; por isso não é base para reprodução física. O PDF
`J-Jacky-ADSR-1980.pdf` também foi guardado como material comparativo, mas é um
scan sem texto/metadata extraível. Ambos permanecem com licença `a verificar`.

O módulo autoral `CONTOUR · ADSR` não tenta simular esses circuitos. Recebe o
gate de uma etapa aberta, gera ATT/DEC/SUS/REL em domínio digital seguro e usa
o contorno para modelar a voz e uma pequena porção da CV do filtro. Isto faz o
envelope entrar no próprio sistema de relações, em vez de virar simplesmente
um VCA decorativo. Um STOP ainda encerra a saída de modo seguro.

## APC, 556 e sequência visível

O diagrama de Nathan Snyder/Owyhee Sound, datado de 4 de novembro de 2009,
mostra um APC com sequenciador de oito passos: clock 555, 4017, oito
potenciômetros, diodos, LEDs e dois 555 como osciladores A/B. O pequeno esquema
`atari_punk_console_schem_556.JPG` apresenta uma variação compacta em 556.

O valor dessa referência não é repetir seu circuito — o Rasgo já possui estudos
próprios de Atari Punk Console. Para Antitotem, ela deixa visível uma ideia
fértil: o relógio, a seleção, o limiar por diodo e a voz podem permanecer
separáveis e recombináveis. `PULSO DUPLO` e `SCANNER DIODO` devem nascer dessa
leitura, com rotas e comportamento próprios, fonte e licença sempre declaradas.

## Superfície digital, buffer e VCO contínuo

O PDF `lunchbeat-PCB-schematics.pdf` é uma página KiCad datada de 6 de novembro
de 2013. Sua leitura mostra ATmega, 74x595, controles de HIHAT/SNARE/KICK/BASS,
PLAY/STOP, edição e DAC. Não foi encontrada autoria ou licença no próprio
arquivo. Ele não orienta a arquitetura de áudio do Antitotem, nem autoriza uso
de firmware ou PCB. Sua contribuição é uma pergunta de interface: como tornar
editável uma relação complexa sem escondê-la em menus? No Antitotem, a resposta
continua sendo módulos nomeados, gestos diretos e relações escutáveis — nunca
uma cópia do instrumento.

O esquema `buffer_j201.jpg` coloca um J201 entre uma entrada de alta impedância
e uma saída acoplada por capacitor. A lição para o campo modular é que um buffer
não é neutro: ele recebe uma carga, preserva ou desloca uma amplitude e decide
quanto uma rota perturba outra. Um futuro módulo autoral `BUFFER · CARGA` deve
oferecer isolamento, saturação leve e acoplamento declarados, sempre com limites
de saída seguros.

Em `4069vco1.png`, René Schmitz (setembro de 2001) apresenta um VCO 4069-1:
integrador, disparo Schmitt, conversor exponencial, saída de serra e modulação
de largura de pulso. Não é o VCF 4069 estudado anteriormente e não deve ser
confundido com o 4069UB de deriva digital já ativo. A diferença abre um futuro
`VCO · CONTÍNUO`: frequência, serra e pulso modulável expostos como relações
independentes, não um emulador da topologia de Schmitz. Autoria é indicada no
desenho; licença de reutilização permanece `a verificar`.

## Ponte analógica e tempo retido

O datasheet primário `LM158.pdf` identifica LM158/258/358/2904 como família de
dois amplificadores operacionais de alto ganho, compensados internamente e
adequados a operação com fonte simples. O LM158 não é outro efeito ou
oscilador: é uma ponte para somar, inverter, comparar, integrar, bufferizar e
condicionar sinais entre módulos lógicos e contínuos. O futuro módulo autoral
`PONTE · ANALÓGICA` deve tornar essas cinco ações escolhíveis e visíveis, com
faixas de sinal e proteção definidas. A ficha é uma fonte técnica, não prova de
que esse CI integrava algum Objeto histórico.

O desenho `DELAY.GIF` apresenta 556, rede RC, duas saídas e SCRs. Sem autoria,
contexto ou licença identificáveis, ele não pode ser reconstruído. Ainda assim,
indica uma questão importante: tempo pode reter um evento e entregá-lo por mais
de uma saída, em vez de ser somente eco. Um possível estudo original
`TEMPO · RETIDO` deverá explorar disparo, guarda e devolução de eventos entre
scanner, contorno e rota, com memória limitada e saída segura.

O diagrama `C04-009_mixer741.gif` mostra um somador inversor de três entradas:
resistores de entrada encontram o 741, a realimentação define o ganho e um
capacitor desacopla a saída. Autoria e licença não estão visíveis, portanto ele
é somente uma referência de princípio. Ele reforça o futuro `MIX ±4`: quatro
entradas de sinal/CV com polaridade explícita, ganho limitado, possibilidade de
comparação e saída protegida. Não deve reproduzir o arranjo, os valores ou a
topologia do desenho, e tampouco afirmar que ele é o mixer histórico do Objeto
4 — a memória autoral confirma o mixer de quatro canais, não seu esquema.

## Quase-encontro e corrente de etapas

O desenho `4046_4017.jpeg` organiza um 555, um 4046 e um 4017. A autoria
visível aponta para SeekIC.com, porém não há licença verificável: ele é apenas
referência de princípio. O 4046 (PLL) aparece entre uma fonte de pulso e a
seleção one-hot. Isso sugere `QUASE-ENCONTRO`: duas frequências não precisam
ser afinadas ou sincronizadas de modo rígido; quando se aproximam, o sistema
pode liberar um evento, alterar uma rota ou permitir temporariamente que um
clock conduza o outro. O estudo Antitotem não copiará a cadeia nem afirmará que
ela existia nos Objetos históricos.

O esquema de Bill Bowden, datado de 23 de junho de 2005, usa 555, dois 4017,
diodos e dezoito LEDs. Ele mostra uma sequência que cresce além dos oito/dez
passos imediatos e em que diodos participam do retorno entre contadores. Daí
surge `CORRENTE DE ETAPAS`: dois scanners podem formar uma trajetória longa,
mas uma escolha de retorno pode encurtá-la, bifurcá-la ou fazê-la reiniciar em
outro ponto. Isso concretiza o “quebra-cabeça mutável”: não uma sequência fixa
com mais LEDs, mas percursos de começo, desenvolvimento e conclusão diferentes.

Na figura 10.23 atribuída a R. M. Marston, `marston_4046multiplier_818.jpg`,
um 4046B e um 4017B aparecem como sintetizador de frequência simples: as
saídas do contador selecionam divisões/razões no laço do PLL, produzindo uma
faixa de saída diferente da referência de entrada. A imagem informa título e
autor, mas não contém dados bibliográficos ou licença suficientes; deve ser
tratada como referência conceitual, não esquema reutilizável.

O deslocamento autoral possível é `RAZÃO ESCOLHIDA`: cada etapa não escolhe uma
nota, mas uma relação temporal para outro módulo. Um scanner pode então passar
por multiplicação, divisão, quase-travamento e fuga. Ligado a `CORRENTE DE
ETAPAS`, isso permite que o percurso seja ao mesmo tempo sequencial e
metamórfico: o passo seguinte pode mudar a própria velocidade com que os
próximos passos serão alcançados.

## Três camadas de um passo: CV, gate e retorno

Os desenhos 001 e 002 de Daniel Vera, datados de 3 de agosto de 2005, chamam
atenção para uma separação valiosa. Um 74HC4017 conduz duas 74HC4016: uma chave
analógica seleciona a tensão de cada potenciômetro para a saída CV, enquanto a
outra participa do percurso de gate/diodos. Há também clock 555, passo manual
e seleção de `reset on step`. A versão `seq8.gif` fixa o retorno após oito
passos; `seq2.1.gif` mostra uma matriz de chaves que permite escolher mais de
um comportamento de gate/reset. A autoria é declarada no carimbo, mas a
licença deve ser verificada; é uma referência de princípio, não de reprodução.

Isto propõe ao Antitotem um módulo autoral `PASSO TRIPLO`. Um passo não é um
único número: ele contém (1) uma CV/razão, (2) uma presença/ausência ou tipo de
gate e (3) uma decisão de retorno. Assim, uma mesma fileira de oito controles
pode realizar um loop curto e denso, atravessar oito posições com silêncios,
ou terminar em uma posição que devolve o percurso a outro começo. É uma forma
precisa de fazer o quebra-cabeça mutável sem esconder a decisão em aleatoriedade
genérica.

## Rastro de altura e VCO de somas

O desenho `t_pitch_tracker_174.png` chama-se *4046 Pitch Tracker* e credita
Nick Collins / *Handmade Electronic Music* como origem de um esquema
interpretado para Lunetta; a imagem traz atualização de 10 de dezembro de
2012. Dois 4049, filtros RC e 4046 aparecem para converter uma relação de
frequência em uma tensão que pode seguir rapidamente ou lentamente. A licença
da imagem e a origem completa devem ser verificadas. O conceito autoral
resultante é `RASTRO DE ALTURA`: não usa uma escala de notas nem tenta afinar o
oscilador; observa a energia periódica de um sinal e deixa uma tensão com inércia
para alterar filtro, rota, contorno ou outro relógio.

Em `4046-vco.png`, também sem autoria/licença identificadas, diferentes entradas
passam por resistores para a entrada de controle do VCO 4046; frequência e
slide estão expostos. A ideia não é reproduzir o desenho, mas propor `VCO DE
SOMAS`: cada módulo pode oferecer uma parcela de tempo (CV de scanner, contorno,
buffer, saída de caos), e a soma limitada dessas parcelas altera uma frequência
com memória. `VCO DE SOMAS` é diferente de `RAZÃO ESCOLHIDA`: o primeiro mistura
influências contínuas; o segundo troca relações discretas entre etapas.

O desenho `4046_lunetta_wet_mud_filter.jpg` se apresenta como *Lunetta Wet Mud
Filter*. Ele traz um 4046, um potenciômetro de 1 k e C1/C2, mas não fornece
autoria, licença ou explicação suficiente para que se determine com segurança
seu comportamento como filtro de áudio, alteração de VCO ou ambos. Essa lacuna
deve ser respeitada: não se deve inventar uma análise de circuito a partir do
título. A imagem vale como convite a `LAMA ÚMIDA`, estudo autoral de resposta
lenta e carregada: uma conexão por resistência/capacitor virtual guarda parte
do movimento e devolve-o como peso, arrasto ou escurecimento. O módulo deverá
declarar claramente sua ação no software, sem alegar ser o circuito Lunetta.

## Formas lentas correlacionadas

`4049trisin_442_182.jpg` se intitula *TSP – simple multi shape lfo* e mostra
4049B, rede RC e saídas identificadas como triangular e senoidal. Autor e
licença não estão claros na imagem, portanto ela fica restrita à pesquisa de
princípio. A potência não está em acrescentar “mais um LFO”, mas em reconhecer
que duas formas de uma mesma trajetória podem ser disponibilizadas em paralelo.

O estudo autoral `TRI-SENO · DESVIO` deve gerar duas leituras correlacionadas de
um mesmo movimento lento: uma pode deslocar frequência/tempo e a outra pode
agir em filtro, contorno, mistura ou retorno. Ao contrário de dois LFOs
aleatórios, elas preservam parentesco; ao contrário de uma fonte única, deixam
o gesto escolher destinos contraditórios. Ele pode passar à faixa audível e
então se tornar uma fonte de interferência entre modulação e timbre. Não é
emulação do 4049 nem do VCO 4069 de René Schmitz, já documentado como outra
referência.

## Limiar que acorda

O desenho `Alarme_com_sirene_F3.jpg` une 555, portas Schmitt 4093, redes RC e
um buzzer para responder a um sensor. Autor e licença não são identificados;
o estudo não reproduzirá seus valores, diagrama ou função de alarme. O princípio
musical está no encadeamento: uma condição atravessa vários limiares e deixa de
ser uma chave binária simples, produzindo pulsos, pausas e acelerações.

`LIMIAR QUE ACORDA` será um módulo autoral que recebe uma fonte — amplitude de
um oscilador, saída de comparador, CV de scanner ou rastro lento — e a compara
com dois limiares ajustáveis. Ao cruzá-los, pode despertar um contorno, liberar
uma rota, alterar retorno ou iniciar um burst temporário. Assim, o 4093 deixa
de significar sirene: torna-se uma forma de fazer o próprio instrumento reagir
àquilo que já está acontecendo dentro dele.

## 8038: formas simultâneas, não seletor de onda

O desenho `8038/oscillator.png`, *Funktionsgenerator mit ICL8038*, é assinado
por Wolfgang Wieser (revisão 3, 8 de setembro de 2004). Ele organiza o ICL8038
com seleção de faixa, ajuste de frequência, modelagem/trim de senoide, saídas
senoidal/triangular/quadrada e condicionamento posterior. A licença de
reutilização não está declarada, logo o esquema não será copiado.

Ele esclarece uma decisão já tomada para o Antitotem: `OSC CONTÍNUO` não deve
ser um botão que troca uma onda por outra. As três formas podem existir ao
mesmo tempo, ter misturas e destinos diferentes, e atravessar limiares,
contornos, scanners e rotas materiais. A atual voz de três osciladores já
introduz essa postura no software; uma versão modular futura deverá separar
`FORMA`, `MISTURA`, `FAIXA` e `DESTINO`, permitindo que uma forma seja áudio e
outra seja controle no mesmo gesto. Isso é uma invenção a partir do princípio,
não uma emulação ou afirmação de que o ICL8038 pertenceu a um Objeto histórico.

## Limites e segurança para hardware

- Use a tensão recomendada no datasheet da família e desacoplamento local em
  cada CI; não use subalimentação como método de dano deliberado.
- Nunca una diretamente duas saídas push-pull. Misture/isolE por resistores,
  diodos, buffers ou chaves apropriadas.
- Entradas CMOS não devem ficar flutuantes e sinais externos precisam respeitar
  os trilhos de alimentação e proteção do CI.
- A saída de lógica não é, por si, uma saída segura para alto-falante, linha ou
  Eurorack: precisa de condicionamento, proteção DC e estágio de saída.
- Em Eurorack, defina níveis, referência de terra e proteção de patch antes de
  abrir rotas. A exploração pode ser radical sem transformar o instrumento em
  uma instrução para tostar componentes.

## Relação com código já existente

O Rasgo já contém reinterpretações próprias de Atari Punk Console, divisor
polirrítmico, randomização/registrador, mistura por diodo, FM cruzada e
realimentação. A contribuição do Objeto 5 não deve ser importar esses blocos
como uma coleção de efeitos: deve concentrar-se em scanner, estado one-hot,
seleção de rota e matéria de conexão entre os dois Objetos.

## Comparações adicionais do 8038

O diagrama `cir_msr015.gif` apresenta outro ICL8038PCD, desta vez junto de um
LF351, chave de forma, seleção de faixa por capacitores, frequência, níveis de
saída alto/baixo e fonte. Autor e licença não estão identificados. Comparado ao
de Wieser, ele torna mais explícita uma separação que interessa ao projeto:
núcleo de forma, condicionamento de nível e alimentação são camadas diferentes.
No Antitotem, `OSC CONTÍNUO` deve manter essa diferença como gesto — uma fonte
pode ser enviada suave a um filtro e forte a um limiar, sem que isso obrigue a
duplicar a forma ou copiar os estágios do circuito de referência.

O arquivo `th_audio_gen_schem_1.pdf` foi conferido: é um scan de uma página,
produzido em 15 de agosto de 2007 pelo PrimoPDF, sem autoria, licença ou texto
extraível que permita atribuição responsável. Ele permanece listado como
material a verificar e não informa nenhuma decisão de implementação até que sua
proveniência possa ser estabelecida.

`waveform-generator-with-icl8038.jpg` é outra página de *Function Generator*
com ICL8038CPD e LF351. Embora a origem e licença não estejam identificadas, a
página declara quatro faixas de frequência, níveis diferentes para senoide,
triângulo e quadrada, impedância de saída e faixas de linearidade/distorção.
Esses dados não serão usados para replicar o circuito, mas acrescentam uma
disciplina ao `OSC CONTÍNUO`: uma forma possui intensidade e destino, não apenas
desenho. No software, áudio, CV lento, entrada de limiar e rota de modulação
devem receber escalas explícitas e limites seguros; uma saída pode ser suave,
forte, bipolar, unipolar ou atenuada antes de encontrar o próximo módulo.

## Quatro leituras de 40106–4040–4051

Os quatro desenhos `02_arp.gif`, `04_seq.gif`, `05_env_gen.gif` e
`06_seq_enve.gif` compartilham 40106, 4040/4051 e redes de resistores, mas não
descrevem a mesma coisa. A autoria e a licença não são identificadas, portanto
nenhum será reproduzido. O que eles oferecem é um mapa de variações:

| Desenho | Leitura segura do princípio | Estudo Antitotem possível |
| --- | --- | --- |
| `02_arp` | três osciladores 40106 parecem chegar diretamente aos endereços A/B/C de um 4051 | `ENDEREÇO VIVO`: a seleção nasce de relações de clocks, não de uma contagem única |
| `04_seq` | 40106 fornece clock e o 4040 entrega divisões para o endereço do 4051 | `SCANNER DIVIDIDO`: a cadeia já ativa no Objeto Sonoro, com divisão como material temporal |
| `05_env_gen` | a leitura 4040/4051 atravessa um controle de nível antes da saída | `CONTORNO EM DEGRAUS`: uma sequência de tensões pode desenhar uma envoltória, não apenas altura |
| `06_seq_enve` | dois 4051 e duas redes resistivas são organizados em uma relação de saída comum | `DUPLA LEITURA`: uma fileira pode escolher razão/CV e outra pode escolher energia, filtro ou retorno |

Os rótulos dos dois últimos são interpretações de software, não afirmações de
que os desenhos implementem exatamente essa função. A descoberta principal é
que endereçamento, tensão e forma temporal podem trocar de papel. Um mesmo
scanner pode, numa montagem, comandar frequência; em outra, desenhar contorno;
em uma terceira, ser perturbado por três relógios que não contam juntos. Essa é
uma base concreta para multiplicar módulos parecidos com diferenças pequenas e
depois cruzá-los no quebra-cabeça mutável.

O desenho `08_custom.gif` mantém o clock 40106, o 4040 e um 4051, mas substitui
a fileira simples de resistores por duas fileiras de potenciômetros. A autoria,
licença e função exata de cada ligação não são dadas; não é correto afirmar que
ele produz duas CVs independentes ou uma soma específica sem analisar o objeto
físico. A leitura válida é estrutural: uma posição de scanner pode depender de
mais de um ponto ajustável.

Daí surge `ETAPA COMPOSTA`. Em vez de tratar um passo como um knob/uma tensão,
o Antitotem pode tratá-lo como pequeno conjunto de tendências: uma para razão
ou altura, outra para densidade, energia, abertura de filtro ou probabilidade
de retorno. A interface precisa manter essas camadas visíveis, para que a
complexidade continue tocável — não uma automação oculta. `ETAPA COMPOSTA`
dialoga diretamente com `PASSO TRIPLO`: CV, gate/contorno e decisão de percurso
podem partir de uma mesma posição, mas não precisam ter o mesmo valor.

## 40106 como constelação de papéis

As imagens do *Heterodyne Space Explorer* e do *Heterodyne Peyote Space
Explorer* são atribuídas na própria arte a Beavis Audio Research (rev. 1.0,
março de 2010). Elas apresentam osciladores 40106 com frequências próximas,
mistura por diodos e controles individuais de volume; a variante Peyote agrega
4040/4051 como gerador de forma de onda definido por usuário e uma fonte de
ruído. A licença de reutilização precisa ser verificada, logo não há cópia de
esquema, valores ou layout. A influência legítima é a heteródina: frequências
vizinhas criam batimento e instabilidade antes de qualquer filtro convencional.

Daí vêm três módulos autorais distintos:

| Estudo | Princípio | Diferença Antitotem |
| --- | --- | --- |
| `BANDO HETERÓDINO` | três ou quatro osciladores próximos | cada oscilador pode entrar como áudio, clock ou fonte de limiar; a mistura declara diodo, capacitor ou buffer |
| `FORMA DE MEMÓRIA` | 4040/4051 como leitura de estados | uma sequência de bits/tensões interfere no bando, não tenta ser a forma de onda do esquema Beavis |
| `RUÍDO QUE ESCOLHE` | ruído soma perturbações à lógica | ruído só altera decisões/rotas dentro de faixa limitada, sem apagar o gesto determinístico |

`Schematic-40106-Simple-Oscillator-Adjustable-Duty-Cycle.png` explicita um
controle de duty e diodo: `PULSO TORTO` pode variar assimetria sem ser apenas
mudança de frequência. Os dois LFOs compartilhados — um com LED/2N3904 e outro
com elemento IRF510 — lembram que indicador e carga podem perturbar/mostrar o
sinal, mas não autorizam deduzir uma função exata sem fonte e licença. O módulo
autoral `LENTO VISÍVEL` deve expor taxa, forma e uma leitura visual de estado;
não é réplica de nenhum deles.

Por fim, `sinister_tone_generator.jpg` parece organizar pares de osciladores
40106, diodos e chaves antes de um condicionamento de saída. Sem autoria ou
licença, a leitura fica no nível de princípio: `PARES QUE SE ENCONTRAM` pode
criar duas heteródinas parcialmente separadas e permitir que um diodo, chave ou
contorno as aproxime. Assim, a mistura não é o fim da cadeia; é o momento em
que dois trajetos antes autônomos passam a modificar um ao outro.

A versão `rich_decibels_sinister_tone_generator.png` permite atribuir o nome
*Sinister Tone Generator* a Rich Decibels, embora a licença ainda precise ser
verificada. As imagens de oscilador duplo esclarecem duas relações que não devem
ser fundidas no software: em `Simple-Dual-Oscillator`, um diodo acopla dois
osciladores antes da saída; em `Dual-Oscillators-With-Mixer`, cada oscilador
passa por diodo e volume antes de se encontrar na saída. `Simple-LFO` deixa o
mesmo núcleo operar em escala lenta.

Assim, o Antitotem deve oferecer `DUAL ACOPLADO` e `DUAL MISTURADO` como
operações separadas. O primeiro permite que a relação de um oscile no outro;
o segundo preserva duas autonomias e decide somente sua proporção. A passagem
entre eles — por chave, contorno, limiar ou scanner — é mais inventiva do que
escolher um preset de "dois osciladores". `LENTO VISÍVEL` pode usar o mesmo
princípio em escalas não audíveis, mantendo uma genealogia clara sem clonar os
esquemas de referência.

## Atlas CMOS: territórios a inventar

O PDF `listadecmos.pdf`, criado por Lúcio Araújo em 10 de agosto de 2026, reúne
famílias 4000/4500 e amplia o campo de pesquisa. Ele é um inventário interno,
não substituto de datasheet de fabricante: nomes, faixas, pinagem, tensões,
variações entre séries e disponibilidade devem ser verificados antes de qualquer
protótipo físico. Também não é uma lista de módulos obrigatórios. O critério é
artístico: só entra uma família que abra uma relação ainda ausente.

| Família da lista | Território ainda pouco explorado | Módulo autoral possível |
| --- | --- | --- |
| 4018 / 4022 | contagem de comprimento configurável ou octal, diferente do ciclo fixo | `CAMINHO ELÁSTICO`: o número de etapas respira sem apagar estados |
| 4029 / 4516 | contagem binária para cima/baixo | `MARCHA REVERSA`: o percurso muda de direção por limiar, contorno ou memória |
| 4052 / 4053 / 4067 | seleção de duas, três ou dezesseis rotas | `ENCRUZILHADA`: uma decisão pode desviar áudio, CV e gate em escalas diferentes |
| 4031 / 4557 | memória deslocável longa ou de comprimento variável | `FITA POROSA`: um gesto persiste, vaza e reaparece com extensão mutável |
| 4098 / 4528 / 4538 | eventos monoestáveis/retriggeráveis | `FAÍSCA COM CAUDA`: um encontro produz duração, não apenas pulso instantâneo |
| 4099 | memória endereçável de oito bits | `DECISÃO GUARDADA`: o scanner escreve ou lê permissões de rota sem virar sequenciador de notas |
| 4089 / 4527 | multiplicação de taxa digital | `DENSIDADE FRACIONADA`: um relógio cria textura rítmica entre divisão e multiplicação simples |
| 4063 / 4585 | comparação digital | `LIMIAR DE DUAS MEMÓRIAS`: duas palavras/estados só alteram rota quando a relação entre elas muda |
| 4541 / 4536 | tempo programável longo | `DURAÇÃO ESTRANHA`: ciclos que sustentam um estado por escalas incompatíveis com o clock rápido |

O atlas mantém dois caminhos simultâneos: multiplicar uma família com pequenas
diferenças e cruzar famílias de papéis distintos. `MARCHA REVERSA` pode ler
`DECISÃO GUARDADA`; `FITA POROSA` pode alimentar `LAMA ÚMIDA`; `ENCRUZILHADA`
pode trocar o destino de `TRI-SENO · DESVIO`. A funcionalidade de cada peça
continua clara, mas o comportamento total aparece na montagem e no tempo.

## Matérias de saturação: diodo, oitava e fissura

O acervo acrescenta três desenhos de pedais/estágios de ganho. Eles não são
especificação do Antitotem e tampouco licença para reproduzi-los. Formam uma
constelação de princípios que pode dialogar com as portas já existentes.

| Arquivo de acervo | Leitura de princípio | Possibilidade autoral no Antitotem |
| --- | --- | --- |
| `fuzz/fuzztone_schematic.gif` | ganho seguido de limiar por diodos e escolha tonal | `FOLD`: curva contínua de dobra com compensação de nível, antes do VCF |
| `fuzz/uvspfuzz.gif` | retificação/duplicação de frequência e filtro notch | `OITAVA FANTASMA`: componente em dobro apenas quando dois osciladores ou uma rota entram em encontro, e não como pedal sempre ligado |
| `fuzz/zvex_sho.gif` | estágio de ganho FET e ajuste de *crackle* | `FISSURA`: ganho instável limitado, associado ao gesto de energia e à rota `TRANS`, sem replicar a topologia ou os valores |

O que interessa aqui é a diferença entre três escutas: **dobrar** a forma,
**extrair** uma relação de oitava por retificação e **forçar** um estágio até a
instabilidade. O software não deve usar clipping duro como atalho. Qualquer
não-linearidade entra antes da cadeia de segurança, recebe suavização/limites,
preserva uma compensação de ganho e passa pelo `OutputStage` final.

`OITAVA FANTASMA` também desloca o efeito de "distorção" para uma relação
musical: o scanner, a proximidade de frequências ou uma etapa marcada podem
permitir a componente, que depois recua. `FISSURA` deve permanecer um campo de
incerteza escutável, não ruído aleatório que torne a saída imprevisível ou
perigosa. A futura interface pode expor essas três matérias como portas de
patch, não como emulações de pedais.

## Movimento e encontro: LFO de formas, seleção e soma

O desenho `lfo/lfo.gif`, sem autoria/licença identificadas no arquivo, reúne
saídas de dente descendente/ascendente, quadrada e triangular, com um estágio
de visualização opcional. Sua lição para o software é separar **a forma do
movimento** da sua taxa: o `LFO` atual pode evoluir de seno/triângulo/quadrada
para uma família que inclua rampas opostas. Isso permite, por exemplo, abrir o
filtro lentamente e fechá-lo de modo abrupto, sem introduzir um segundo
modulador. A tela deverá nomear a forma escolhida e mostrar seu estado sem
fingir ser o circuito analógico.

Em `mixer/4053.jpeg`, a legenda atribui a figura 3.133 a uma fonte ainda não
identificada. O CD4053 comanda a alternância de entradas estéreo por um clock:
isso é **seleção temporal**, não soma. Já `mixer/6ipmix.gif`, também sem
autoria/licença identificada, mostra seis entradas condicionadas por amplificação
e uma soma ativa: isso é **mistura com orçamento de ganho**. São operações que
o Antitotem deve manter distintas.

| Operação autoral | Inspiração de princípio | Regra musical/técnica |
| --- | --- | --- |
| `LFO DE GESTO` | formas múltiplas de `lfo.gif` | uma única fonte produz seno, triângulo, pulso, rampa acima ou rampa abaixo; a troca é suavizada para não criar salto |
| `PORTA 4053` | alternância temporal do 4053 | escolhe qual voz/retorno atravessa por janela de tempo; crossfade curto impede clique |
| `MIX DE SEIS` | condicionamento e soma ativa | canais preservam gain, pan, mute/solo e orçamento de headroom; não se multiplicam sem limite |

`PORTA 4053` é especialmente fértil quando lê o scanner de 16 passos: a mesma
etapa pode decidir **quem** passa, enquanto os controles `AMP` decidem **quanto**
passa. Assim, misturar, mutar e comutar permanecem gestos diferentes. Antes de
qualquer implementação física, autoria, licença, pinagem, alimentação simétrica
e limites de sinal devem ser verificados em datasheet/fonte primária.

## Memória de água: atraso filtrado, não sala simulada

O arquivo `pedais/deep blue delay original.gif` identifica **Sea Urchin (d.b.d.)**,
desenho de **madbeanpedals**, ©2011. O PDF ao lado chama o território de *Deep
Blue Delay*. Ambos devem ser tratados como documentação de terceiros: não há
autorização para replicar esquema, valores, arte ou layout. A leitura relevante
é a cadeia conceitual: pré-condicionamento, memória curta baseada em PT2399,
filtragem no retorno e controle separado de tempo, repetição e mistura.

Em diálogo com o Echo Base anteriormente reunido, o Antitotem pode acrescentar
`MEMÓRIA D'ÁGUA` à família espacial atual:

- **TEMPO** move a distância entre o acontecimento e sua recordação;
- **POROSIDADE** filtra cada retorno e faz a repetição perder bordas;
- **RETORNO** regula quantas recordações sobrevivem, sempre abaixo do limiar
  técnico de crescimento;
- **DERIVA** desloca o tempo discretamente a partir de LFO ou sample-and-hold,
  de modo limitado e suavizado;
- **MISTURA** é independente do reverb material já existente.

Isso não duplica `MaterialReverb`: o reverb atual é rede de várias memórias
curtas simultâneas; `MEMÓRIA D'ÁGUA` será uma linha reconhecível de causa e
efeito. Sua cor poderá vir do VCF e da porta `CAP`, mas sua segurança permanece
depois da experiência: interpolação em mudança de tempo, filtro/DC blocker,
feedback limitado e `OutputStage` sem clip duro.

O PDF `tonepad_rebotedelay25.pdf` identifica **Rebote Delay 2.5**, rev. 3,
31 maio 2006, por **Francisco Peña**. Seu texto declara PT2399, menor atraso de
10 ms, maior de 580 ms e repetições que se tornam “um pouco sujas” nos maiores
tempos, enquanto permanecem limpas nos curtos. Essa observação orienta a
porosidade de `MEMÓRIA D'ÁGUA`: não se trata de defeito a eliminar nem de uma
emulação literal de pedal, mas de uma dimensão contínua onde a memória perde
nitidez conforme ganha distância. O projeto também alerta que diminuir um
resistor pode produzir repetições infinitas; no software essa borda artística
deve ficar sempre antes de um limite estável de feedback e saída.

## Memória deslizante, anel e retenção

O arquivo `random/shift_register_sequencer.jpg` identifica **KS Synthesis**,
**Scott Stites**, *4006 Randomizer Core*, rev. 1.0, 08/2004. Ele reúne registro
de deslocamento CD4006, uma escolha `Random/Loop`, clock condicionado e saídas
ponderadas de CV. A invenção que ele sugere ao Antitotem é `MEMÓRIA DESLIZANTE`:
uma sequência curta de permissões/valores desloca-se a cada clock; a proporção
entre reinserir um estado anterior e introduzir uma perturbação é explícita.
Ela pode escolher retorno, porta, curva do LFO ou uma segunda camada de CV, sem
substituir o scanner de 16 passos já tocável.

O arquivo `ring_modulator/m_rm.gif` credita **Roman Sowa, 2002** e indica o
multiplicador analógico AD633 com buffers. Ele confirma uma distinção essencial:
ring modulation é produto de **carrier × modulation**, não uma porta XOR e nem
um simples volume LFO. O `RING` atual é um estudo digital contínuo dessa ideia;
um desenvolvimento posterior deve poder escolher como modulador o LFO, OSC B,
OSC C ou uma memória retida, sempre com ganho compensado antes do master.

Por fim, `sample_hold/s_a_h.gif`, autoria/licença a verificar, desenha um S&H
mínimo com buffer, chave eletrônica (4066 ou 4053), capacitor e novo buffer. É
uma leitura física clara para o `S&H` já existente: a entrada é observada em
um instante, a memória sustenta aquele valor até o próximo clock e a saída não
deve carregar essa memória. No aplicativo, o botão `S&H` determina se a cor de
ruído escolhida passa continuamente ou como campo de degraus. A taxa de retenção
é módulo futuro próprio; não se deve confundi-la com a taxa do LFO.

## Dezesseis gestos: scanner histórico e expansão paralela

`sequencer/sympleSEQ_dualeuro_MkII_sch_LOGIC_pg1.png` identifica
**HEXINVERTER.NET**, *sympleSEQ v2.0 Dual Euro*, e mostra uma lógica de
sequenciador A com contador 4017, condicionamento de clock/reset/pause, portas
lógicas 4093 e conectores para uma segunda camada. Já
`sequencer/SEQ16_analogboardschematicpg1.gif` identifica **Ray Wilson**, *16
Step Analog Sequencer Analog Board*, copyright 2005, e usa CD4067 para leitura
de controles de um sequenciador de 16 etapas, com mais de uma saída de CV.

O Antitotem não os reproduz e tampouco os projeta retrospectivamente sobre os
Objetos 1 ou 5. Eles esclarecem uma expansão contemporânea do princípio que já
está no instrumento digital:

| Camada | Estado atual | Expansão autoral possível |
| --- | --- | --- |
| `SCANNER` | 16 steps, retorno escolhido, CV/AMP/FX/mute | direção, pausa, avanço manual e memória deslizante como decisões independentes |
| `CV` | uma tensão de altura/timbre por step | três barramentos: `PITCH`, `ROTA` e `MATÉRIA`, cada um podendo aceitar outra fonte |
| `GATE` | mute e contorno por etapa | comprimento, probabilidade de passagem e acento, sem virar uma grade de DAW |
| `DUPLO` | duas fileiras visuais de oito | duas sequências relacionadas: uma lê tempo/gesto, outra lê rota/efeito; elas podem coincidir ou se desencontrar |

O detalhe mais produtivo é que 16 não precisa significar “mais notas”: pode
significar mais formas de articular uma mesma matéria. O próximo módulo de
sequência deve preservar as quatro operações já performáveis (`CV`, `AMP`, `FX`,
`MUTE`) e acrescentar apenas uma nova dimensão por vez, sempre visível e
audível. Conexões, polaridade, faixa de tensão e autoria/licença continuam
assuntos a verificar antes de qualquer transposição ao hardware.

O arquivo `sequencer/sequncer-counter.jpg`, autoria/licença a verificar, junta
dois 4520, um comparador 4063 e um 4067: a comparação parece determinar quando
o contador retorna, enquanto o multiplexador lê uma entre dezesseis fontes. Ele
explicita que **comprimento** de sequência não é o mesmo que o conteúdo dos
steps. Já existe no Antitotem como `RETORNO 1–16`; a próxima diferença será
`DIREÇÃO` (avança, recua, alterna) e depois `ENDEREÇO` (memória/limiar escolhe o
próximo), sem sobrecarregar o primeiro painel.

`sequencer/4796997608_1453a07f03_o.jpg` declara **Grey Area Media inc.**, *16
step analog sequencer*, protótipo ©2010, baseado em quatro 4015. Ali a cadeia de
registros dá uma saída por etapa e as etapas combinam habilitação, LED e valor
de CV por diodos. O princípio autoral que emerge é `PASSO COMO PORTA`: cada
step deve poder abrir uma rota em vez de somente emitir altura. Isso já começa
em `MUTE` e `FX`; depois pode permitir uma escolha entre `OSC`, `ANEL`,
`MEMÓRIA D'ÁGUA` ou `FOLD`, mantendo o estado de cada escolha visível.

`sequencer/APS.png`, autoria e licença a verificar, apresenta um outro núcleo
mais econômico: bateria, 555/556, 4017, oito chaves de step, rede de diodos,
LEDs e controles de valor. A topologia sugere simultaneamente clock, decisão de
etapa e gesto sonoro. Para o Antitotem, ela não pede reprodução: pede que a
interface preserve uma qualidade que o software tende a apagar — **cada etapa
é um lugar físico de escuta**, com uma luz, uma tensão, uma permissão e uma
consequência audível imediata. Por isso os 16 steps do aplicativo permanecem
em duas fileiras de oito e indicam a etapa ativa; a expansão de camadas não
deve transformá-los em uma planilha abstrata.

## Proximidade e heteródina: gesto sem teclado

`theremin/etherwave.gif`, cujo nome aponta para o Etherwave, organiza dois
osciladores RF (um variável e um fixo), detector, VCA e um segundo percurso de
antena para volume. A autoria, versão e licença do desenho devem ser verificadas
antes de qualquer reutilização. O estudo não é uma tentativa de simular antenas
ou copiar a eletrônica: sua contribuição é conceitual.

`PROXIMIDADE / HETERÓDINA` deve tratar duas frequências como uma distância
audível. Quando os osciladores se aproximam, a diferença entra no campo de
batimentos; quando se afastam, ela muda de registro ou vira ritmo. Um segundo
sinal lento regula presença/ganho. No Antitotem isso pode nascer internamente
das relações OSC A/B/C, do LFO e da `MEMÓRIA DESLIZANTE`; no futuro pode aceitar
um sensor externo como fonte de controle, nunca como requisito para tocar.

A regra estética é importante: não converter tudo em pitch cromático. A
heteródina oferece instabilidade, bordas lentas e encontro de frequências; o
VCA oferece distância e aproximação. Ambos entram antes do master protegido e
preservam o princípio de energia autônoma que orienta o instrumento.

`theremin/4046_Based_Theremin.jpg`, autoria/licença a verificar, torna a ponte
mais compacta: um 4069 fornece oscilação e um 4046 introduz VCO/PLL, com uma
antena como perturbação de controle. Para `PROXIMIDADE / HETERÓDINA`, a leitura
é que captura e perda de captura podem ser eventos musicais: dois osciladores
podem tender a uma relação, resistir a ela e escapar. O módulo digital futuro
deve expor `APROXIMAÇÃO`, `CAPTURA` e `FUGA` como relações suaves entre OSC A e
OSC B, em vez de emular o esquema, sua antena ou seu circuito de alimentação.

## Tremor, escolha de caminho e limites de radiofrequência

O PDF `tonepad_eatremolo.pdf` identifica **Austremolo**, rev. 1, 30 abr. 2009,
layout de Francisco Peña para EA Electronics Australia Tremolo. O próprio texto
declara uma alteração de alimentação para impedir vazamento de LFO no caminho de
áudio. Este é um princípio de qualidade diretamente útil: `TREMOLO DE MATÉRIA`
deve variar amplitude com curva contínua, manter a modulação separada do áudio e
atravessar apenas pontos de ganho bem definidos. Não deve inserir clique, offset
DC ou componente periódica indesejada na saída.

`tonepad_abselector.pdf`, **A/B Selector**, Francisco Peña, rev. 1, 22 jul.
2006, descreve uma entrada comum escolhendo duas saídas (ou o inverso), com LEDs
de estado. Ele reforça que um seletor não é mixer: `A/B` deve declarar qual rota
está ativa. No Antitotem, a futura `PORTA 4053` pode usar essa leitura para
mostrar rota A/B, enquanto o `MIX DE SEIS` continua responsável por somar e
orçar ganho. Uma troca digital terá crossfade curto, não chave dura.

`transmissorfm/3m1wtx01.gif` é explicitamente um excitador FM de 88–108 MHz e
1 W, com estágios VCO, driver e PA. Ele fica **fora do escopo de implementação**:
RF nessa faixa envolve riscos de interferência e exigências regulatórias. A
única influência permitida no Antitotem é abstrata e interna ao áudio: separar
oscilação, condicionamento e controle; não produzir, conduzir ou emitir RF.

## Quatro territórios de VCF: escolher contraste, não copiar topologia

O PDF `vcf/cmosed_ms20_filter_11-18-2005.pdf` se identifica como versão CMOS de
filtro MS-20 de **René Schmitz**, esquema por **Ryan Williams**, 19 nov. 2005.
Ele combina 4069UB, LM13700 e diodos/LEDs; as próprias notas ressaltam fonte de
corrente linear, resposta dependente do sinal e ausência de compensação térmica
de alta precisão. `vco/4069vco1.png`, também assinado por René Schmitz (set.
2001), mostra 4069 como VCO contínuo com integrador, corrente exponencial e
saídas serra/pulso. São estudos valiosos, mas não são o atual VCF/VCO do
Antitotem nem autorização para reproduzi-los.

O `SUPER CEM3328 LOW PASS VCF - 2012` expõe entradas separadas de CV e rota de
ressonância em torno de um filtro dedicado; `vcf/filtro_ca3046.jpg` mostra um
núcleo discreto CA3046 e rede de pares casados/diodos; e
`vcf/littlebitsfilter.jpg` declara LittleBits Electronics/Open Hardware e
licença Creative Commons Attribution-ShareAlike 3.0, combinando LM13700,
amplificadores e controle de corte. Cada um corresponde a uma materialidade
distinta; nenhum será transplantado ao código como esquema.

| Território | Qualidade musical a estudar | Papel possível |
| --- | --- | --- |
| `VCF 4069*` atual | limiar, ganho e ressonância porosa | filtro principal, já ativo e estabilizado |
| `OTA 4P` futuro | encadeamento de polos e corte mais organizado | contraste de profundidade, em paralelo ou série com o 4069 |
| `DIODO / CA3046` futuro | compressão assimétrica e limite orgânico | `FOLD` antes do filtro, não “filtro vintage” genérico |
| `CEM` como referência | CV de frequência e ressonância independentes | modelo de interface: separar gesto de corte de gesto de retorno |

A regra para evoluir é dupla: o VCF atual não deve ser substituído por uma
estética de catálogo, e um filtro novo só entra depois de teste espectral,
compensação de ganho, proteção DC e escuta comparativa. Assim a diferença vem
da montagem de matérias, não do nome de um CI.

## Forma contínua: triângulo, serra, quadrada e pulso

`vco_waveshaper/kobolws.pdf` identifica **RSF Kobol Expander VCO Wave Shaper**,
redesenhado por **Mark Verbos** em 19 out. 2000. O texto do documento afirma uma
varredura contínua de triângulo para serra, quadrada e pulso, com CV de
waveshape, modulação de largura de pulso e controle de nível. O redator também
alerta que o esquema original era pouco legível e que alguns valores podem estar
errados; portanto ele é referência conceitual, não base de implementação física.

O Antitotem já oferece `FORMA` contínua nos três osciladores. O próximo passo
não é copiar a cadeia Kobol, mas tornar sua ideia musical mais explícita:
`FORMA` define a matéria, `PULSO` altera a assimetria e `CV DE FORMA` permite que
scanner, S&H, LFO ou proximidade atravessem a transformação. O ganho deve ser
compensado durante o morph, pois a troca de geometria altera energia percebida;
as rotas de `FOLD` e `FISSURA` permanecem distintas da formação da onda.

## Fontes e atribuição

- Datasheets e fichas TI: 40106B, 4040B, 4017B, 405xB, 4069UB, 4007UB e 4046B
  nos links desta página.
- [ICL8038 — datasheet Renesas/legado
  Intersil](https://www.renesas.com/en/document/dst/icl8038-datasheet): fonte
  para as funções do gerador contínuo; não para afirmar presença histórica.
- [Make: Lunetta CMOS Synth](https://makezine.com/article/craft/music/lunetta-cmos-synth/)
  e [Electro-Music, The Lunetta Show](https://electro-music.com/forum/viewtopic.php?t=36130):
  contexto comunitário/histórico, não fontes primárias de especificação.
- [Beavis Audio](https://beavisaudio.com/projects/cmossynthesizers/): prática
  DIY e esquemas de estudo; consultar licença/autor antes de reutilizar qualquer
  desenho ou layout.
- Pesquisa interna anterior: `RASGO_SYNTH/Plano_de_Upgrade_Instrumento_Generativo.md`
  e `RASGO_SYNTH/CREDITS.md`; revisada para evitar reimplementar cegamente o que
  o Rasgo já construiu.
