# Créditos, fontes e licenças — Antitotem — Objeto Sonoro

O protótipo nasceu do núcleo do Objeto Sonoro 5. O nome geral identifica o
campo modular atual; não renomeia nem reatribui os objetos históricos.

Antitotem 2013 foi um projeto de instalação sonora interativa de Lúcio
Araújo, selecionado pelo edital n° 090/12 (Pesquisa e Produção em Arte
Digital) da Fundação Cultural de Curitiba, com apoio do Fundo Municipal de
Cultura: cinco objetos sonoros distintos (Objeto Sonoro 1 a 5) instalados em
diversos locais da cidade de Curitiba, culminando em exposição e oficina.
Mais em [antitotem.arquiviagem.net](https://antitotem.arquiviagem.net/).

## Tipografia e continuidade de arquivo

- **Fonte de interface:** o aplicativo segue a política já validada no Rasgo
  Synth e solicita a família genérica `Monospace` ao fontconfig (DejaVu Sans
  Mono nesta máquina). O código converte explicitamente cada texto UTF-8 antes
  de entregá-lo ao JUCE; acentos não dependem de uma conversão ASCII implícita.
  IBM Plex Mono e sua SIL Open Font License 1.1 permanecem em `assets/fonts/`
  como referência de arquivo, mas não são requeridos em tempo de execução.

## Pista técnica a verificar

- ICL8038, *Precision Waveform Generator / Voltage Controlled Oscillator*:
  referência para senoide, triangular, quadrada, rampa e pulso simultâneos.
  Fonte: [datasheet Renesas/legado Intersil](https://www.renesas.com/en/document/dst/icl8038-datasheet).
  A recordação autoral “8083” é tratada como provável referência ao ICL8038;
  não é prova de presença no Objeto Sonoro 5 histórico.

## Referência de filtro CMOS

- Phil Baljeu / Ruin Electronics, “CMOS VCF”, 19 nov. 2012: esquema visual
  compartilhado pela autoria em `cmos_vcf_philipbaljeu.jpg`. Referência para o
  princípio de filtro com inversores 4069 em rede RC e CV; nenhum esquema,
  valor, layout ou código foi copiado. Licença de reutilização: a verificar.

## Referências de memória/atraso

## Referências Yusynth: envelope e modulação lenta

- Yves Usson / Yusynth, `vc_lfo/ADSR-V2-sch.gif`, compartilhado pela autoria:
  referência para separar gate condicionado, ataque, decay, sustain, release e
  saídas normal/invertida. O `CONTOUR · ADSR` do Antitotem continua uma
  implementação digital própria; não reutiliza circuito, valores, diagrama ou
  layout. Licença da imagem: a verificar antes de qualquer reutilização.

- Yves Usson / Yusynth, `vc_lfo/VCLFO1-sch.gif` e `vc_lfo/VCLFO2-sch.gif`:
  referência para LFO com range, entrada de CV, formas simultâneas
  seno/triângulo/retangular/serra, sync e largura de pulso. O projeto adotará
  esses princípios como vocabulário de interface e comportamento, nunca como
  reprodução do esquema. Licença das imagens: a verificar.

- Esquemas `cmos4006_reverb.png` e `cmos4006.gif`, compartilhados pela autoria:
  uso de CD4006, clock CMOS e TL074 como linha de atraso/reverberação. Autor e
  licença não identificados nas imagens: referência funcional apenas, sem cópia
  de esquema, layout ou valores.

- Esquema `cmos13600.gif`, compartilhado pela autoria: filtro OTA de quatro
  células em torno de LM13600. Autor e licença não identificados na imagem;
  referência de princípio somente. A ficha do sucessor LM13700 está em
  [TI](https://www.ti.com/product/LM13700).

- PHOBoSapiens Inc., *Jabberwock*, rev. 1.0, 19 maio 2012, esquema
  `cmos_phobos_4015.gif`: referência para memória/realimentação com 4015;
  licença de reutilização a verificar.

- CTM Berlin, *XOR Ring Mod*, 2010, esquema `cmos_xormodulator4070.jpg`:
  referência para modulação lógica por 4070. O desenho exibe símbolo Creative
  Commons, mas a licença exata e a fonte devem ser verificadas antes de usar
  qualquer esquema ou arte.

- Roman Sowa, *ringmodulator*, 2000/2002, esquema `ring_modulator/m_rm.gif`:
  referência para multiplicação analógica balanceada por AD633. A relação com
  a fonte comunitária está em [Electro-Music](https://electro-music.com/wiki/pmwiki.php?n=Schematics.BallancedModulatorAkaRingmodulator);
  nenhum esquema, valor, layout ou código foi reutilizado. Licença a verificar.

- Ian Fritz, *Double-Well Chaos*, 2007, esquema `doublewellchaos_ianfritz.jpg`:
  referência para caos analógico contínuo. Licença de reutilização a verificar.

- Richard Brewster, *MOTM-Compatible Mixer-Comparator*, CGS23 PCB, novembro de
  2002, `mix-comp-2002.pdf`: referência para mistura, inversão e comparação;
  nenhum desenho, PCB, valor ou código foi reutilizado. Licença a verificar.

- “phono”, *Simple DIY Eurorack Polarizing mixer*, 31 dez. 2010, PDF de mesmo
  nome: referência para mixer ± de quatro entradas. O desenho também menciona
  Synovatron DIY e `dinsync.info`; autoria/licença de reutilização a verificar.

- Ray Wilson / Music From Outer Space LLC, *9V Battery Powered Sine, Square and
  Triangle Wave Function Generator*, copyright 2013/14, esquema
  `revised-battery-function-generator-nov-2013.gif`: referência para a
  separação de núcleo triangular, quadrada e shaping senoide por OTA. Nenhuma
  cópia de esquema, valores, layout ou imagem; licença a verificar.

- Kevin B. / synthgeek, *Semi-Random Source*, rev. 1.0, 20 nov. 2010, esquema
  `semirandom13.jpg`: referência para quatro 40106, escada R–2R e glide. O
  desenho menciona proximidade não intencional com Psycho LFO de Ken Stone;
  nenhum esquema, valor ou layout foi reutilizado. Licença a verificar.

- Gakken, *SX-150* (manual); diagrama anotado `SX150_SchemV3.PNG` declara
  valores adicionados por Sam Hoshuyama. Referência de arquitetura tátil/híbrida
  somente; autoria e licença da versão anotada a verificar.

- Esquema `t_cmos_random_stepwave_lfo_test_circuit_174.gif`, autoria e licença
  não identificadas: referência para registrador 4006 com realimentação XOR e
  soma resistiva de taps. Nenhum desenho, valor ou layout foi reutilizado.

- Christopher Macdonald, *ADSR Envelope Generator*, esquema
  `Dual_ADSR_muff_menciona_problema.gif`: referência para gate, latch 4093,
  chaves 4066, TL084 e diodos em um envelope ADSR. A imagem indica que há um
  problema mencionado no título do arquivo: topologia, autoria completa,
  versão e licença exigem verificação antes de qualquer uso físico. O
  `CONTOUR · ADSR` deste protótipo é uma implementação digital original, sem
  copiar esquema, valores ou layout.

- J. Jacky, *ADSR*, 1980, `J-Jacky-ADSR-1980.pdf`: material de acervo para
  pesquisa comparativa. O PDF é escaneado e não contém metadados/texto
  extraível; autoria, licença e condições de reutilização permanecem `a
  verificar`. Nenhuma parte foi reproduzida no código.

- Nathan Snyder / Owyhee Sound, *Atari Punk Console with 8 Step Sequencer*,
  4 nov. 2009, `sequencer_apc.png`: referência para articulação visível entre
  clock 555, 4017, oito controles, diodos e dois osciladores 555. O próprio
  desenho credita César Verela e membros de Electro-Music. Licença a verificar;
  nenhum esquema, valor, layout ou imagem será reutilizado.

- Esquema `atari_punk_console_schem_556.JPG`, autoria/licença a verificar:
  referência mínima para a relação interna de dois temporizadores do 556 e sua
  saída. É estudado como princípio de `PULSO DUPLO`, não como circuito copiado.

- *lunchbeat-PCB-schematics.pdf*, KiCad/Eeschema, 6 nov. 2013: esquema de uma
  placa com ATmega, 74x595, botões, potenciômetros e DAC. O PDF não revela
  autoria ou licença reutilizável; é referência de legibilidade de superfície
  física e separação entre edição, play/stop e som, sem uso de firmware, PCB ou
  diagrama no Antitotem.

- `buffer_j201.jpg`, autoria/licença a verificar: buffer J201 com entrada de
  alta impedância, capacitor de acoplamento e ponto de saída. Referência para
  isolamento/carga e coloração de conexão; não é circuito copiado.

- René Schmitz, *VCO 4069-1*, set. 2001, `4069/4069vco1.png`: VCO analógico
  com inversores 4069, integrador, Schmitt, conversor exponencial e modulação
  de largura de pulso. Referência de princípios para voz contínua e `PULSO
  DUPLO`; licença a verificar e nenhuma topologia, valor ou layout será
  reutilizado.

- National Semiconductor, *LM158/LM258/LM358/LM2904 Low Power Dual
  Operational Amplifiers*, PDF `158/LM158.pdf`: datasheet primário da família
  de dois amplificadores operacionais compensados internamente, aptos a fonte
  simples. Base técnica para um futuro módulo `PONTE · ANALÓGICA`; não é
  evidência de presença histórica em um Objeto específico.

- Esquema `556/DELAY.GIF`, autoria/licença a verificar: dois temporizadores do
  556 e SCRs como referência de tempo, retenção e duas saídas. Será pesquisado
  como possível `TEMPO · RETIDO`, sem reprodução de topologia ou valores.

- Esquema `741/C04-009_mixer741.gif`, autoria/licença a verificar: somador
  inversor de três entradas em torno de 741, com capacitor de acoplamento na
  saída. Referência de leitura para `MIX ±4` e `PONTE · ANALÓGICA`; nenhum
  desenho, valor ou layout será reutilizado.

- Esquema `4017/4046_4017.jpeg`, atribuído visualmente a SeekIC.com mas sem
  autoria/licença verificadas: cadeia 555 → 4046 → 4017. Referência para a
  ideia de aproximação/PLL antes da seleção de etapas; não será reproduzido.

- Bill Bowden, desenho `4017/4017-18stageledsequencer.gif`, 23 jun. 2005:
  dois 4017, 555, diodos e 18 LEDs/etapas. Referência para extensão e retorno
  por diodos em sequências; licença a verificar, sem cópia de esquema, valores
  ou layout.

- R. M. Marston, figura 10.23, *Simple 1 kHz–90 kHz frequency synthesizer*,
  `4017/marston_4046multiplier_818.jpg`: 4046B e 4017B para seleção de razão
  de frequência. Origem bibliográfica e licença completas a verificar antes de
  reutilização; referência conceitual apenas.

- Daniel Vera, *Simple 8 step sequencer for Sound Lab*, desenhos 001/002,
  3 ago. 2005, `4017/seq8.gif` e `4017/seq2.1.gif`: 74HC4017, duas 74HC4016,
  555, oito potenciômetros, gate, diodos, passo manual e reset em etapa.
  A autoria está no carimbo; licença de reutilização a verificar. Referência de
  princípio somente, sem cópia de esquema, valores ou layout.

- `4046/t_pitch_tracker_174.png`, *4046 Pitch Tracker*, crédito no próprio
  desenho a Nick Collins, *Handmade Electronic Music*, interpretado para uso
  Lunetta; atualização indicada em 10 dez. 2012. Autoria completa/licença da
  imagem a verificar. Referência para extração lenta de trajetória de altura,
  sem reutilizar esquema, valores ou layout.

- `4046/4046-vco.png`, autoria/licença a verificar: 4046 usado como VCO com
  múltiplas entradas resistivas, controle de frequência e slide. Referência de
  princípio para soma de CVs no tempo, não para reprodução do circuito.

- `4046/4046_lunetta_wet_mud_filter.jpg`, *Lunetta Wet Mud Filter*,
  autoria/licença a verificar: uso mínimo de 4046, potenciômetro e capacitores,
  apresentado pela imagem como filtro. Referência para resposta lenta e
  carregada por RC; a função elétrica precisa não será inferida além do que o
  desenho mostra e nenhum circuito será reproduzido.

- `4049/4049trisin_442_182.jpg`, *TSP – simple multi shape lfo*, autoria e
  licença a verificar: inversores 4049B, rede RC e saídas triangular/senoidal.
  Referência para formas lentas correlacionadas; não será reproduzido como
  circuito, valores ou layout.

- `4093/Alarme_com_sirene_F3.jpg`, *Diagrama elétrico do Alarme com Sirene*,
  autoria/licença a verificar: 555, portas Schmitt 4093, rede RC e buzzer.
  Referência para encadeamento de limiares e pulsos; não será reproduzido como
  circuito, valores ou layout.

- Wolfgang Wieser, *Funktionsgenerator mit ICL8038*, revisão 3, 8 set. 2004,
  `8038/oscillator.png`: gerador de funções com ICL8038, seleção de faixa,
  ajuste fino, saídas seno/triângulo/quadrada e estágio de saída. A autoria e a
  data estão no desenho; licença de reutilização a verificar. Referência de
  princípios, sem cópia de esquema, valores ou layout.

- `8038/cir_msr015.gif`, autoria/licença a verificar: ICL8038PCD, LF351,
  seleção de triângulo/senoide/quadrada, faixa/frequência, níveis alto/baixo e
  alimentação. Referência comparativa; nenhum esquema, valor ou layout será
  reutilizado.

- `8038/th_audio_gen_schem_1.pdf`, PDF escaneado criado em 15 ago. 2007,
  autoria e licença não identificadas. Não há texto extraível além de metadata
  técnica; é mantido apenas como fonte a verificar, sem qualquer reutilização.

- `8038/waveform-generator-with-icl8038.jpg`, *Function Generator*,
  autoria/licença e publicação de origem a verificar: ICL8038CPD, LF351, quatro
  faixas de frequência, formas seno/triângulo/quadrada, níveis e impedância de
  saída declarados. Referência de especificação conceitual; nenhum circuito,
  valor ou layout será reutilizado.

- Série de esquemas `40106/02_arp.gif`, `04_seq.gif`, `05_env_gen.gif` e
  `06_seq_enve.gif`, autoria/licença a verificar: variações 40106, 4040 e
  4051 com rede de resistores. Referências para diferenças entre endereçamento
  por osciladores, divisão, envelope em degraus e dupla leitura de tensão; não
  serão reproduzidas como circuito, valores ou layout.

- `40106/08_custom.gif`, autoria/licença a verificar: 40106, 4040, 4051 e uma
  rede de potenciômetros organizada em duas fileiras. Referência para etapas
  compostas e leitura de redes de controle; não será reproduzido como circuito,
  valores ou layout.

- Beavis Audio Research, *Heterodyne Space Explorer* e *Heterodyne Peyote
  Space Explorer*, rev. 1.0, mar. 2010, esquemas `Heterodyne-*.png`: quatro ou
  três osciladores 40106, mistura por diodos e, na variante Peyote, 4040/4051 e
  gerador de ruído. A autoria e URL constam nas imagens; licença de reutilização
  a verificar. Referências de princípio, sem cópia de esquema, valores ou layout.

- `40106/lfo.png`, `Schematic-40106-Simple-LFO-LED-Indicator.png` e
  `Schematic-40106-Simple-Oscillator-Adjustable-Duty-Cycle.png`, autoria e
  licença a verificar: variações 40106 para LFO, indicação e duty cycle.
  Referências de princípio, sem reprodução de circuito, valores ou layout.

- Rich Decibels, *Sinister Tone Generator*,
  `40106/rich_decibels_sinister_tone_generator.png` (e arquivo de acervo
  `sinister_tone_generator.jpg`): pares de osciladores 40106, diodos, chaves e
  estágio de saída. A autoria é indicada pelo nome do arquivo; licença a
  verificar. Referência para encontro de pares e mistura por limiar, sem cópia.

- `40106/Schematic-40106-Dual-Oscillators-With-Mixer.png`,
  `Schematic-40106-Simple-Dual-Oscillator.png` e
  `Schematic-40106-Simple-LFO.png`, autoria/licença a verificar: duas
  topologias de oscilador duplo (mistura por diodos ou acoplamento por diodo) e
  LFO simples. Referências de princípio, sem reprodução de circuito, valores ou
  layout.

- Lúcio Araújo, *List of the CMOS 4000 series*, `listadecmos.pdf`, 10 ago.
  2026: inventário de famílias CMOS como mapa de pesquisa interna. A lista não
  substitui datasheets de fabricante; qualquer função ou implementação futura
  deve ser novamente confrontada com fonte primária e disponibilidade real.

- `fuzz/fuzztone_schematic.gif`, autoria/licença a verificar: referência de
  estágio de ganho, limiar por diodos e ajuste tonal. Apenas princípio de
  saturação será estudado; nenhum circuito, valor ou layout será reutilizado.

- Univox, *Super-Fuzz*, `fuzz/uvspfuzz.gif`, autoria original e licença do
  desenho a verificar: referência de retificação/oitava e seleção tonal. Pode
  orientar a ideia autoral `OITAVA FANTASMA`, sem reprodução do circuito.

- ZVex / Gottfried Divos, *Super Hard On (SHO)*, `fuzz/zvex_sho.gif`, copyright
  2005 indicado na própria imagem: referência de ganho FET e controle
  "Crackle". Material não livre ou sem licença de reutilização confirmada será
  tratado exclusivamente no nível de conceito; não será copiado.

- `lfo/lfo.gif`, autoria/licença a verificar: referência de LFO com rampas,
  triângulo e quadrada. Inspira uma família de gestos de modulação, sem cópia.

- Figura 3.133, `mixer/4053.jpeg`, autoria do livro/licença a verificar:
  referência de seleção temporal de entradas com CD4053, não de mixer somador.

- `mixer/6ipmix.gif`, autoria/licença a verificar: referência de
  condicionamento de múltiplas entradas e soma ativa. Inspira somente a
  separação entre ganho por canal e orçamento de saída.

- madbeanpedals, *Sea Urchin (d.b.d.)*, 2011, `pedais/deep blue delay
  original.gif`: referência de cadeia PT2399, filtro de retorno e controles de
  delay/feedback/mix. O aviso de copyright está na imagem; nenhum esquema,
  valor, layout ou arte será reutilizado.

- `pedais/Deep Blue Delay sch.pdf`, autoria/licença a verificar: referência
  adicional de pedal de delay; estudada apenas como princípio de memória
  filtrada e repetição controlada.

- Francisco Peña, *Rebote Delay 2.5*, rev. 3, 31 maio 2006,
  `tonepad/tonepad_rebotedelay25.pdf`: referência de PT2399 e repetição
  filtrada; o PDF declara 10–580 ms e deterioração nas repetições longas.
  O material é estudado somente como princípio, sem reprodução de circuito,
  valores ou layout.

- Scott Stites / KS Synthesis, *4006 Randomizer Core*, rev. 1.0, 2004,
  `random/shift_register_sequencer.jpg`: referência de registro de
  deslocamento, escolha random/loop e CV ponderada; princípio para futura
  `MEMÓRIA DESLIZANTE`, sem cópia.

- Roman Sowa, *Ringmodulator*, 2002, `ring_modulator/m_rm.gif`: referência de
  multiplicação analógica por AD633 e estágios de buffer. Inspira somente a
  distinção conceitual de ring modulation; não há reprodução de circuito.

- `sample_hold/s_a_h.gif`, autoria/licença a verificar: referência de
  sample-and-hold com chave eletrônica, capacitor e buffers. Inspira o modelo
  de retenção de valor e não uma reprodução de circuito.

- HEXINVERTER.NET, *sympleSEQ v2.0 Dual Euro*,
  `sequencer/sympleSEQ_dualeuro_MkII_sch_LOGIC_pg1.png`: referência de lógica
  de sequenciador 4017/4093 e interface de duas camadas; licença a verificar.
  Nenhuma lógica, circuito ou layout será copiado.

- Ray Wilson, *16 Step Analog Sequencer Analog Board*, copyright 2005,
  `sequencer/SEQ16_analogboardschematicpg1.gif`: referência de varredura de 16
  controles por CD4067 e múltiplos CVs. Inspira apenas expansão conceitual de
  gestos por etapa, sem reprodução.

- `sequencer/sequncer-counter.jpg`, autoria/licença a verificar: referência de
  contador 4520, comparação 4063 e multiplexação 4067; inspira somente a
  distinção entre comprimento, retorno e endereçamento de sequência.

- Grey Area Media inc., *16 step analog sequencer*, protótipo ©2010,
  `sequencer/4796997608_1453a07f03_o.jpg`: referência de cadeia 4015 e etapas
  com habilitação/CV. Inspira a ideia de passo como porta, sem cópia de desenho,
  circuito ou layout.

- `sequencer/APS.png`, autoria/licença a verificar: referência de 555/556,
  contador 4017, oito etapas, diodos e LEDs. Inspira a legibilidade performativa
  de cada step; nenhum circuito, valor ou layout será reutilizado.

- `theremin/etherwave.gif`, provavelmente relacionado ao Etherwave,
  autoria/versão/licença do desenho a verificar: referência de heteródina,
  detector e VCA por proximidade. Inspira apenas o conceito autoral de gesto
  contínuo sem teclado; nenhum circuito, layout ou imagem será reutilizado.

- `theremin/4046_Based_Theremin.jpg`, autoria/licença a verificar: referência
  de 4069, 4046 VCO/PLL e perturbação por proximidade. Inspira os conceitos
  `APROXIMAÇÃO`, `CAPTURA` e `FUGA`, sem cópia de circuito ou layout.

- Francisco Peña, *Offboard Wiring*, rev. 2, 2005,
  `tonepad/tonepad_offboardwiring.pdf`: referência de bypass, aterramento em
  estrela e fiação externa. O PDF restringe o uso a termos próprios; é lido
  apenas como contexto, sem reutilização de desenho ou layout.

- Francisco Peña, *Romavia*, rev. 1, 2007,
  `tonepad/tonepad_octaviarm.pdf`: referência de octave-up fuzz. O PDF declara
  todos os direitos reservados; inspira somente a distinção de oitava/retificação
  já documentada em `OITAVA FANTASMA`.

- Francisco Peña, *Austremolo*, rev. 1, 2009,
  `tonepad/tonepad_eatremolo.pdf`: referência de tremolo e prevenção de
  vazamento de LFO no áudio. Material estudado apenas como princípio técnico.

- Francisco Peña, *A/B Selector*, rev. 1, 2006,
  `tonepad/tonepad_abselector.pdf`: referência de seleção de dois caminhos e
  indicação de estado; não autoriza cópia.

- `transmissorfm/3m1wtx01.gif`, marcado como excitador FM 88–108 MHz/1 W,
  autoria/licença a verificar: explicitamente excluído de implementação por
  segurança e regulação de radiofrequência; não será reproduzido nem adaptado.

- `transmissorfm/esquema.pdf`, uma página, sem título/autoria legíveis nos
  metadados: material de radiofrequência catalogado exclusivamente como acervo
  excluído de implementação. Não será analisado para construção, emissão ou
  adaptação de transmissor.

- `transmissorfm/lowpass.pdf`, uma página, sem título/autoria legíveis nos
  metadados: filtro de RF catalogado como acervo excluído de implementação. A
  pesquisa de filtragem do Antitotem permanece restrita ao domínio de áudio.

- René Schmitz / Ryan Williams, versão CMOS de filtro MS-20, 19 nov. 2005,
  `vcf/cmosed_ms20_filter_11-18-2005.pdf`: referência de 4069UB, LM13700 e
  rede de diodos/LEDs; estudada como princípio, sem cópia.

- René Schmitz, *VCO 4069-1*, set. 2001, `vco/4069vco1.png`: referência de
  VCO contínuo, integrador e PWM. Inspira a distinção de VCO/VCF, sem réplica.

- `vcf/supercem3328/super-cem3328-rev2-schematics.jpg`, título/ano indicados
  na imagem, autoria/licença a verificar: referência de VCF dedicado e CVs
  independentes para corte/ressonância; sem reutilização de circuito.

- `vcf/filtro_ca3046.jpg`, autoria/licença a verificar: referência de núcleo
  CA3046, pares casados e diodos; apenas contexto de materialidade discreta.

- littleBits Electronics, *LB_BIT_i32_filter*, `vcf/littlebitsfilter.jpg`:
  a imagem declara CC BY-SA 3.0/Open Hardware. Ainda assim o Antitotem não
  reutiliza o esquema; a referência é creditada por transparência.

- RSF Kobol, *Expander VCO Wave Shaper*, redesenhado por Mark Verbos em 2000,
  `vco_waveshaper/kobolws.pdf`: referência de morph triângulo/serra/quadrada/
  pulso e CV de forma. O próprio documento registra incerteza sobre valores;
  será tratado somente como princípio, sem cópia.

Este arquivo separa proveniência, influência e dependência. Nenhuma referência
abaixo transfere autoria para este código nem autoriza reproduzir um esquema,
imagem ou layout alheio.

| Elemento | Papel no protótipo | Crédito / situação |
|---|---|---|
| Website Antitotem, Objeto Sonoro 5 | identificação histórica da obra | Lúcio Araújo; [antitotem.arquiviagem.net](https://antitotem.arquiviagem.net/) lista 40106, 4017, 4051, 4040 e 386 |
| Website Antitotem, Objeto Sonoro 1 | obra irmã e diálogo de circuito | Lúcio Araújo; [antitotem.arquiviagem.net](https://antitotem.arquiviagem.net/) lista 40106, 4040, 4051, 567 e 386 |
| `eme.pdf`, p. 9 | referência funcional para a cadeia 40106 → 4040 → 4051 | caderno “Eletrônica Musical Experimental”, autoria de Lúcio Araújo; mantido no acervo pessoal, não incluído neste repositório |
| `eme.pdf`, p. 7 | contexto histórico do APC e do sequenciador 555/4017 | a própria página credita Nathan Snyder / Owyhee Sound; não há cópia do desenho, imagem ou valores para o código |
| Forrest Mims III, *Stepped Tone Generator* | princípio histórico do Atari Punk Console (astável + monoestável) | referência conceitual; nenhum esquema, imagem ou texto de publicação é reproduzido |
| Aquorbium | pesquisa interna de técnicas de oscilador | `BiomaMorphOscillator` e `BiomaPulsar` foram estudados; `CmosVoice` é implementação específica, não dependência nem cópia de módulo |
| JUCE | framework de app, GUI e áudio | dependência externa, dual-licenciada AGPLv3 ou licença comercial; consultar a versão efetivamente usada antes de distribuir binários |
| JUCE `WavAudioFormat` / `ThreadedWriter` | gravação REC em WAV PCM 24-bit | dependência do framework; escrita em thread e metadata implementadas no protótipo |

## Estado de licença do protótipo

O código novo é licenciado sob **GNU AGPLv3** (ver `ANTITOTEM/LICENSE`),
decisão registrada em 11 ago. 2026 — compatível com a dependência obrigatória
do framework JUCE (dual-licenciado AGPLv3 ou comercial; este projeto não usa
licença comercial da JUCE). Os esquemas, imagens e PDFs de pesquisa citados
nesta página continuam com seus próprios direitos e não são licenciados por
este código; nenhum deles é distribuído junto do repositório.

## Regra de pesquisa

1. localizar fonte primária, autoria e licença antes de incorporar material;
2. quando a fonte não for livre ou estiver sem licença clara, estudar somente o
   conceito e produzir uma implementação/autoria próprias;
3. registrar a diferença entre referência e resultado;
4. verificar novamente antes de publicar, distribuir binários ou reutilizar em
   outro instrumento.

## Informação histórica pendente

O autor recorda que o Objeto Sonoro 1 tinha menos potenciômetros e ligações
distintas. Registrar aqui não transforma a lembrança em especificação: falta
conferir as fotografias e qualquer esquema preservado antes de modelar essa
variante digitalmente.
