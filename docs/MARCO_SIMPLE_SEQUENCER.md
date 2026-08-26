# Marco — Antitotem - Objeto sonoro 5

- Estado: `prototype`
- Instrumento: Antitotem - Objeto sonoro 5
- Referência de acervo: `../eme.pdf`, página 9, “40106 – 4040 – 4051 Sequencer”
- Critério de pronto deste incremento: uma fatia JUCE/C++ reproduz o ciclo de
  oito endereços, torna a seleção visível e converte cada CV em comportamento
  sonoro audível.

## Decisão de interpretação

O site histórico identifica o Objeto Sonoro 5 com os CIs 40106, 4017, 4051,
4040 e 386. O Objeto Sonoro 1 é uma obra irmã que também reúne 40106, 4040 e
4051, mas com 567 e 386. Eles não são a mesma máquina: diferem nas saídas e
na quantidade/distribuição de potenciômetros. O diagrama alimenta um oscilador
40106, usa o contador binário 4040 para gerar
os três bits de endereço e seleciona uma entre oito tensões por meio do 4051.
O protótipo mantém essa topologia musical:

```text
40106 CLOCK -> 4040 DIVIDER / address -> 4051 SCANNER -> 8 CV -> CMOS VOICE
```

Não se declara simulação de eletrônica de bancada. Clock, divisão, seleção e
CV são comportamentos digitais estáveis; a voz é uma invenção específica do
instrumento, inspirada no caráter de osciladores CMOS. Os oito canais atuais
são a leitura direta do 4051 do esquema, não a afirmação de que o painel físico
do Objeto 5 tinha exatamente oito potenciômetros ou uma única saída.

O objetivo musical não é uma calculadora de circuitos: é preservar uma prática
intuitiva de composição por gesto, escuta e experimentação com o incerto. A
implementação deve proteger a saída e tornar as relações legíveis sem reduzir
a descoberta a valores técnicos predeterminados.

### Expansão pesquisada: 4020, matriz 8×8 e retenção

Um esquema compartilhado pela autoria mostra CD4020 alimentando dois CD4051:
Q6–Q8 endereçam uma dimensão e Q9–Q11 a outra, atravessando uma matriz de 64
ajustes. Outro esquema, de Ray Wilson/MFOS, mostra Sample & Hold por TL084 e
JFETs. Juntos, eles sugerem uma expansão de 64 posições em que o scanner escolhe
uma tensão e um S&H a retém até o próximo trigger. Isso deve entrar como nova
camada explicitamente inspirada nas fontes, nunca como atribuição automática ao
Objeto 5 histórico.

### Nota de memória a verificar

Segundo a autoria, o Objeto Sonoro 1 usava menos potenciômetros e incluía
ligações diferentes das que aparecem no diagrama de oito canais. A quantidade,
os destinos e as conexões exatas estão `a verificar`: não devem ser inferidos
nem reconstruídos como fatos sem confrontar fotografias, esquema original ou
uma nova recordação documentada.

A autoria também recorda experimentações em breadboard nas quais
retroalimentações entre os blocos produziram sonoridades peculiares. Isso é um
comportamento histórico a recuperar: implementar uma matriz de feedback
explícita, limitada e desligável, mas não alegar ainda quais eram as ligações
físicas originais.

As ligações físicas não eram cabos neutros: resistores, diodos, capacitores e
alto-falantes participavam da transformação do sinal. A tradução digital deve
portanto tratar cada rota como uma cadeia de componentes — atenuação/mistura,
retificação/limiar, carga/memória e resposta de saída — em vez de reduzir todo
feedback a um único valor de ganho.

### Pista de oscilação contínua a verificar

A autoria recorda um CI referido como “8083”; a identificação mais provável é
o **ICL8038**, gerador de funções/VCO que disponibiliza simultaneamente senoide,
triangular e quadrada e também permite rampa/pulso por seus ajustes externos.
Ele é uma pista para um futuro núcleo de formas contínuas, não uma afirmação de
que integrava o Objeto Sonoro 5 histórico. Se a peça ou um esquema reaparecer,
a designação deverá ser confrontada com sua inscrição e datasheet antes de ser
incluída na lista de componentes do instrumento.

## Relação com código existente

Foram estudados `AQUORBIUM/src/core/BiomaMorphOscillator.h` e
`BiomaPulsar.h`. Eles demonstram duas decisões úteis: formas descontínuas
precisam de correção anti-aliasing e pulsos precisam de contorno para evitar
cliques. O Antitotem não os importa nem replica sua organização de organismos:
cria `CmosVoice`, com três osciladores audíveis, morph contínuo
senoide/triangular/serra/quadrada, PolyBLEP, cross-modulation e saturação leve,
controlada pela CV selecionada.

A ENERGIA não pretende simular uma fonte real: é uma tensão digital musical que
faz o clock, a afinação relativa e o corpo da voz respirarem juntos. Sua faixa
baixa introduz queda e deriva controladas; sua faixa alta dá presença e mais
atrito nas relações. A proteção de saída permanece posterior a essa matéria.

Os três estudos de núcleo `40106 · limiar/pulso`, `8038 · formas` e
`4069UB · deriva` são feitos com diferença: preservam respectivamente limiar,
geração de formas e instabilidade de ganho como princípios composicionais, mas
não copiam esquemas, código ou arquiteturas internas. O ICL8038 e o HEF4069UB
permanecem referências técnicas creditadas; sua presença no Objeto histórico
continua não confirmada.

Se ambos os projetos confirmarem uma API estável e testes suficientes, a
técnica poderá ser candidata a módulo comum. Antes disso, cada voz permanece
no instrumento que lhe dá sentido.

## Próximo incremento

- validar a GUI JUCE em áudio e visualmente;
- acrescentar patch points explícitos (clock out, CV out e audio out);
- desenhar uma matriz de feedback entre osciladores, scanner e detector 567,
  com limites de ganho, estado visível e elementos de rota (resistor, diodo,
  capacitor e carga/resposta de saída);
- decidir se o próximo módulo explora o APC da página 7 ou o arpejador da
  página 9, sem misturar os dois prematuramente.

MIDI não é tarefa deste marco: a performance nasce da energia autônoma dos
osciladores do Objeto, não de notas externas.

## Evidência desta etapa

- CMake configurado com `RASGO_SYNTH/JUCE-master` e app JUCE compilado;
- `antitotem_simple_sequencer_tests`: aprovado (ciclo de oito endereços,
  limites de CV e sinal não silencioso);
- `OutputStage`: DC blocker por canal e limiter estéreo vinculado com 2 ms de
  antecedência e teto de -1,4 dBFS; é proteção técnica, não efeito criativo;
- smoke test gráfico não executado: `xvfb-run` não está instalado neste
  ambiente; inspeção visual e escuta humana continuam pendentes.
