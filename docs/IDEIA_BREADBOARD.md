# Ideia futura — Antitotem Breadboard Lab

**Status: não implementada.** Isto é uma ideia registrada, anterior ao app
JUCE atual (ver [`README.md`](../README.md) para o que de fato existe e roda
hoje). Só deve ser retomada como projeto próprio, quando fizer sentido —
não é uma revisão nem uma substituição do Antitotem atual.

Proposta de instrumento educativo e performático cuja tela é uma **breadboard
virtual**. A pessoa constrói módulos conectando chips CMOS, componentes e
cabos; em vez de começar por um painel abstrato de síntese, começa pelo gesto
real de prototipar, testar, errar, refazer e descobrir.

O Eurorack entra como gramática complementar de interação: cada módulo pode
ter painel, entradas, saídas, atenuadores e pontos de patch legíveis, enquanto
a breadboard mantém a origem material e a liberdade de prototipagem. Não é uma
emulação de um rack comercial nem uma cópia estética.

Cada componente terá uma ficha de aprendizagem curta, baseada em datasheets e
fontes técnicas verificadas: função, pinos, limites, comportamento musical,
conexões usuais, riscos e pequenos experimentos guiados. Datasheets não serão
reproduzidos sem autorização; a aplicação apontará para as fontes oficiais e
produzirá explicações próprias.

Nota de tensão conceitual a resolver quando essa frente for retomada: o
[`DESIGN.md`](DESIGN.md) (linha ~16) diz que o painel não deve "simular uma
breadboard com precisão" nem ser "reprodução gráfica literal de breadboard"
(linha ~46) — isso foi escrito para orientar o Antitotem atual (painel fixo),
não necessariamente descarta este projeto separado, cujo ponto de partida é
literalmente a metáfora de montagem. Vale esclarecer com o autor, quando for
retomar, se é precisão de simulação elétrica que se quer evitar (equações,
cálculo) ou a metáfora de montagem em si.

## Vocabulário inicial

- **chips:** osciladores/inversores Schmitt CMOS, portas lógicas, contadores,
  multiplexadores, registradores, PLLs e demais circuitos somente após estudo
  individual de datasheet;
- **componentes:** resistores, capacitores, diodos, transistores, potenciômetros,
  chaves, LEDs, alto-falante/saída e alimentação;
- **superfície:** trilhas, barramentos, furos, jumpers e pontos de medição;
- **módulo:** agrupamento salvo de componentes e conexões que pode ser inserido
  na breadboard maior;
- **aprendizado:** desafios e exemplos como "o que muda se este resistor vira
  um diodo?", sempre explicando a consequência elétrica e sonora possível.

## Três modos complementares

1. **Estudo:** conexões respeitam o modelo elétrico que for implementado,
   mostram alimentação, incompatibilidades e leitura guiada de componente.
2. **Performance:** uma abstração musical segura permite tocar, modular e
   recombinar módulos sem exigir que cada gesto seja uma simulação completa de
   bancada.
3. **Laboratório:** permite ligações estranhas, circuit bending digital e
   comportamento incerto sob limites de segurança sonora/computacional. O
   usuário pode congelar/registrar um patch que tenha descoberto por intuição.

## Princípios de implementação

- começar por uma pequena família de chips e componentes bem pesquisados, não
  por uma biblioteca enorme de símbolos;
- separar modelo elétrico, motor sonoro e interface de breadboard para que cada
  um possa amadurecer sem fingir precisão que ainda não existe;
- declarar sempre o nível de fidelidade: conceito musical, comportamento
  aproximado ou simulação elétrica validada;
- preservar conexões, versões de módulo e anotações como parte da composição;
- não tentar substituir um simulador SPICE completo no primeiro marco; o alvo é
  um instrumento de descoberta sonora e aprendizagem situada;
- tratar a documentação e os experimentos de componentes como conteúdo autoral
  e pedagógico, não apenas ajuda de interface.

## Primeiro marco futuro

Construir uma breadboard 2D pequena com alimentação, jumpers, resistor,
capacitor, potenciômetro, LED e um único chip CMOS estudado. Ela deve permitir
montar um oscilador/gerador de pulsos audível, medir alguns pontos e salvar o
patch. A partir daí, cada novo chip entra acompanhado de pesquisa, teste,
modelo, exemplo e gesto performático.

## Acervo e imagem de origem

A fotografia do **SDIY RASGO Breadboard Prototype Synth** pertence ao arquivo
pessoal de Lúcio Araújo. Ela poderá orientar uma futura linguagem visual e
editorial, mas não será copiada para este repositório ou publicada sem decidir
o recorte, os metadados, o crédito e o contexto de uso.
