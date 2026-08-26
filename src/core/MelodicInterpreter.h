#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace antitotem
{
// MelodicInterpreter - item #19, docs/PESQUISA_MELODIA_GENERATIVA.md
// ("Camada 'Melodic Interpreter' separada e reutilizável [...] só faz
// sentido depois que 2-3 dos itens acima estiverem testados e
// aprovados"), extraída de DualObjectEngine::render() em 20 ago. 2026
// (autor: "item #2/#4, #14 e #19, prossiga") - por essa altura #6, #7,
// #9, #16 e #17 já estavam feitos e com teste de regressão passando (5
// itens, acima do limiar cogitado antes de extrair).
//
// Fronteira deliberada: esta classe SÓ decide música (que pitch soar
// agora, com que ganho, com que dica de timbre, em que estado micro/
// macro) - nenhuma dependência de CmosVoice, JUCE, ou qualquer motor de
// síntese específico. Quem chama (hoje, DualObjectEngine) decide o que
// FAZER com o que ela devolve (hoje, alimenta um CmosVoice.tickStereo);
// um chamador futuro em outro projeto do ecossistema RASGO poderia
// alimentar um oscilador completamente diferente sem que uma linha
// aqui dentro precisasse mudar. Por isso `Stimulus` só pede números/
// bools genéricos (variação de atividade, evento de acento, "clima" de
// instabilidade, amount 0-1) em vez de qualquer referência a
// SimpleSequencer ou first/fifth.
//
// Todo o racional de design original (por que glide contínuo em vez de
// retrigger - RASGO_SYNTH/ThereminVoice.hpp, "a real theremin never
// re-triggers... the glide is ALWAYS on"; por que Narmour; por que
// fraseado é separado de fôlego; por que densidade vira liberdade
// interpretativa e não o oposto; etc.) está preservado nos comentários
// abaixo, só movido de DualObjectEngine.h/.cpp pra cá - nada foi
// redesenhado nesta extração, é uma cópia mecânica de comportamento.
class MelodicInterpreter
{
public:
    // Estados expressivos nomeados (item #17) - ver o racional completo
    // nos comentários de `tick()` abaixo, onde são computados.
    enum class MicroState { Rest, Attack, Connect, Sustain, Release };
    enum class PhraseState { Calm, Ascending, Climax, Release };
    // Tipo de gesto/articulação (itens #1/#13 aprofundados, docs/
    // PESQUISA_MELODIA_GENERATIVA.md), 20 ago. 2026, autor: "precisamos
    // melhorar estacatos e outras formas de articulação e gesto de
    // nota". Legato é o comportamento ORIGINAL/majoritário (glide
    // contínuo + dip proporcional, inalterado); os outros três dão
    // variedade real de caráter sem violar "a real theremin never
    // re-triggers... the glide is ALWAYS on" (ThereminVoice.hpp) - o
    // pitch continua deslizando por baixo o tempo todo, só o ENVELOPE DE
    // GANHO muda de forma, dando a impressão audível de separação/ênfase
    // sem quebrar o mecanismo. Ver o racional completo no roll em
    // tick().
    enum class GestureType { Legato, Staccato, Marcato, Tenuto };

    // O que o intérprete recebe a cada sample - deliberadamente genérico
    // (ver o comentário de classe acima). Quem chama já fez qualquer
    // edge-detection/smoothing específico da SUA própria fonte (ex.:
    // DualObjectEngine ainda mantém seu próprio par de
    // excitationPreviousFirstAccented/FifthAccented pra decidir
    // `accentEvent`) - o que entra aqui já é o sinal cru pronto pra ser
    // escutado, a suavização/decisão musical em cima disso é trabalho
    // do intérprete, não do chamador.
    struct Stimulus
    {
        float rawActivityDelta = 0.0f; // |mudança| instantânea no sinal combinado que esta voz escuta
        float rawLevel = 0.0f;         // |nível| combinado instantâneo (densidade/balanceamento de ganho)
        bool accentEvent = false;      // true só no sample em que um acento estrutural COMEÇA
        float instability = 0.0f;      // 0..1, campo de "clima" compartilhado
        float amount = 0.0f;           // 0..1, sensibilidade/energia geral (o slider EXCITAÇÃO)
        bool running = true;
        bool muted = false;
    };

    // O que o intérprete devolve a cada sample - pronto pra alimentar
    // qualquer oscilador; `gain` já é o produto de TODOS os fatores
    // expressivos (articulação/fôlego/ápice/frase/sustentação/silêncio
    // de fronteira/ataque), o chamador só multiplica pelo frame de
    // áudio bruto.
    struct Voice
    {
        float soundingPitch = 0.5f;
        float gain = 0.0f;
        float timbreBrightness = 0.0f; // 0..3, dica de registro+trajetória (ex.: CmosVoice::setOscillatorShape)
        MicroState micro = MicroState::Rest;
        PhraseState phrase = PhraseState::Calm;
        // Consequência de volta (docs/PESQUISA_ORQUESTRA_RELACIONAL_E_
        // TEMPERAMENTO.md, seção 6.3 - "influenciar e ser influenciado"),
        // 20 ago. 2026. true só no sample em que um disparo real acabou
        // de acontecer - o intérprete não sabe nada de `instabilityField`
        // (fora da fronteira desta classe, ver o comentário de classe
        // acima), só REPORTA o evento; quem chama (DualObjectEngine, que
        // já possui `nudgeInstability()`) decide o que fazer com ele.
        bool justTriggered = false;
    };

    void prepare(double newSampleRate) noexcept;
    [[nodiscard]] Voice tick(const Stimulus& stimulus) noexcept;
    // Leem o estado PERSISTIDO diretamente, não o `Voice` do último
    // `tick()` - quando `stimulus.amount <= 0`, `tick()` devolve um
    // `Voice{}` default (early return, antes do bloco caro que
    // atualizaria micro/phrase) sem tocar nos membros reais abaixo, que
    // continuam guardando o último estado de verdade conhecido (mesmo
    // comportamento do código original antes desta extração).
    [[nodiscard]] MicroState getMicroState() const noexcept { return microState; }
    [[nodiscard]] PhraseState getPhraseState() const noexcept { return phraseState; }
    // Vetor de caráter (docs/PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md,
    // seção 6.1 - "estágios de ânimo, espírito, interpretação,
    // sensibilidade"), 20 ago. 2026, autor: "continuamos as
    // implementações" (seguindo a própria recomendação do documento:
    // "o item de menor risco/maior payoff"). Três eixos CONTÍNUOS, não
    // um enum de "dolce/agitato/etc." travado - nomes tradicionais são
    // só REGIÕES nesse espaço pra documentação (dolce ≈ suavidade
    // alta+energia baixa+brilho baixo), nunca uma categoria exclusiva
    // que o código precisa escolher (permite "70% dolce, 30%
    // misterioso" sem inventar um terceiro nome). Cada eixo deriva
    // lentamente sozinho (ver `tick()`, mesma arquitetura do Noise
    // Field/`instabilityField` em DualObjectEngine - "um campo que
    // deriva e puxa de volta a um repouso" - só que aplicada a
    // CARÁTER, não a instabilidade, e com seed própria pra não mudar o
    // consumo de `seed` nos disparos já calibrados). Mais lento que uma
    // frase (#6) - um "clima" que dura várias frases, a diferença real
    // entre "estado de ânimo" e "posição dentro da frase atual".
    [[nodiscard]] float getCharacterEnergy() const noexcept { return characterEnergy; }
    [[nodiscard]] float getCharacterSoftness() const noexcept { return characterSoftness; }
    [[nodiscard]] float getCharacterBrightness() const noexcept { return characterBrightness; }

private:
    double sampleRate = 44100.0;
    float activity = 0.0f;
    float ensembleLevel = 0.0f;
    int cooldown = 0;
    // Narmour-biased walk (docs/PESQUISA_MELODIA_GENERATIVA.md): a small
    // previous step tends to continue in the same direction (real
    // "process" implication, Narmour 1990); a large one (a leap) tends to
    // reverse ("registral return"). Stored so the NEXT step's direction
    // roll can be biased by what the LAST one actually was.
    float previousDirection = 1.0f, previousMagnitudeSemitones = 1.0f;
    // Memória de gestos (item #16) - buffer circular pequeno dos últimos
    // 4 gestos (direção*magnitude, sinal + tamanho), lido no próximo
    // disparo pra medir monotonia (quantos dos últimos foram na MESMA
    // direção) e reduzir a chance de continuar nessa direção quando já
    // repetiu demais - ver o uso em tick().
    std::array<float, 4> gestureHistory {};
    int gestureHistoryWrite = 0;
    // Note hierarchy, "ápice melódico" slice (item #8) - registerAverage:
    // slow per-trigger follower of the recently-typical register.
    // apexDegree: how far a new target sits from that average, computed
    // BEFORE updating the average so a genuine outlier reads as an
    // outlier against where the register recently WAS.
    float registerAverage = 0.5f;
    float apexDegree = 0.0f;
    // Note hierarchy, segunda categoria além do ápice (item #8, docs/
    // PESQUISA_MELODIA_GENERATIVA.md - "passagem/estrutural/chegada/
    // tensão/resolução/repetição/bordadura"), 20 ago. 2026, autor: "falta
    // algo a implementar na melodia? prossiga com inteligência e
    // perspicácia". apexDegree já mede uma outra dimensão (distância do
    // REGISTRO recente); esta mede proximidade de um acento MÉTRICO
    // estrutural (Accent Field, `Stimulus::accentEvent`) - um eixo
    // diferente, TEMPORAL em vez de espacial. metricGlow salta a 1.0 no
    // instante de um acento e decai em ~90ms (a janela típica entre o
    // acento e o disparo real de EXCITAÇÃO, que depende de `activity`
    // cruzar o threshold, não é instantâneo); capturado no disparo como
    // arrivalStrength, persiste pela vida da nota (mesmo padrão de
    // apexDegree). Uma nota que "aterrissa" perto de um acento real lê
    // como chegada/estrutural (ganho de ataque mais firme, vibrato mais
    // contido - ver tick()); uma nota fora do tempo lê como passagem/
    // bordadura (comportamento normal, sem ênfase extra) - a mesma
    // distinção "tônica/estrutural vs. nota de passagem" da teoria
    // melódica real, derivada de um sinal que o instrumento já escutava
    // (o Accent Field), não uma nova fonte de estímulo.
    float metricGlow = 0.0f;
    float arrivalStrength = 0.0f;
    // Tipo de gesto (ver `GestureType` acima) - sorteado por disparo,
    // enviesado pelo contexto já existente (magnitude pequena/liberdade
    // interpretativa -> mais staccato; chegada métrica/salto grande ->
    // mais marcato; ápice/perto do clímax de frase -> mais tenuto),
    // nunca fixo/escolhido por UI, mesma filosofia "probabilístico, não
    // determinístico" do item #11. `gestureDecay` é o mecanismo real por
    // trás de Staccato/Marcato - some 1.0 no disparo e, SÓ para esses
    // dois tipos, é empurrado por sample rumo a um piso quase-silencioso
    // (ver tick()), soando como uma nota destacada mesmo com o pitch
    // ainda deslizando por baixo. Legato/Tenuto deixam `gestureDecay`
    // travado em 1.0 (sem essa ducagem extra).
    GestureType gestureType = GestureType::Legato;
    float gestureDecay = 1.0f;
    // Fraseado / Phrase Energy (item #6) - phraseEnergy (tenda, sobe até
    // o clímax sorteado e desce depois) e o contourBias derivado dela
    // (puxão direcional real - ver tick()) dão arco a vários eventos,
    // DELIBERADAMENTE separado de breath/#7 (fôlego vaza/recupera
    // continuamente, não dá uma posição narrativa limpa do jeito que uma
    // contagem de notas dá). Comprimento de frase e posição do clímax
    // são sorteados a CADA frase nova, não fixos.
    float phraseEnergy = 0.0f;
    int phraseNoteIndex = 0;
    int phraseLength = 8;
    float phraseClimaxPosition = 0.5f;
    // Final trajectory (item #4) - a última nota de uma frase é conhecida
    // de antemão (lookahead antes de incrementar phraseNoteIndex, ver
    // tick()); enquanto ela soa, vibrato/timbre se acalmam gradualmente
    // rumo ao silêncio real que `restHush` já provoca - duas dimensões
    // diferentes do mesmo gesto de final (volume E caráter).
    bool phraseFinalNote = false;
    // Ataque real (item #2) - punch efêmero (~50ms) a cada disparo,
    // distinto do dip de `articulation` (que reduz ganho em saltos
    // grandes, o lado da "conexão"/#13) - este soma energia no início de
    // TODA nota nova, imitando o ataque percussivo que uma voz que só
    // desliza (nunca re-dispara) normalmente não tem.
    float attackPunch = 0.0f;
    MicroState microState = MicroState::Rest;
    PhraseState phraseState = PhraseState::Calm;
    // Silêncio real de fronteira de frase (efeito audível de #17) -
    // restHush foge de 1.0 rumo a 0 (~0.4s) enquanto phraseBoundaryResting
    // estiver ligado, recupera rumo a 1.0 (~0.15s) assim que a frase
    // seguinte começa - uma entrada suave, não um corte abrupto.
    bool phraseBoundaryResting = false;
    float restHush = 1.0f;
    // Ornamentation, overshoot/approach slice (item #15) - on a trigger,
    // the glide initially heads slightly PAST the real target, then eases
    // back as this decays to 0 - a real scoop/bend arrival.
    float overshoot = 0.0f;
    float pitch = 0.5f, currentPitch = 0.5f;
    float glideCoeff = 0.001f;
    // Articulation dip (item #13, amplitude side of #1's timing-only
    // connection idea) - 1.0 = no dip (true legato, small step); a big
    // leap dips briefly toward a floor and recovers, reading as a small
    // re-attack/breath before the leap.
    float articulation = 1.0f;
    float vibratoPhase = 0.0f;
    // Microafinação, camada "living pitch" (item #12, docs/
    // PESQUISA_MELODIA_GENERATIVA.md - "target pitch/arrival pitch/living
    // pitch [...] instabilidade controlada, não erro aleatório"), 20 ago.
    // 2026. Alvo (`pitch`) e chegada (o próprio overshoot/#15, que já
    // ultrapassa o alvo e assenta) já existiam; faltava a terceira
    // camada: uma micro-instabilidade CONSTANTE, presente mesmo fora de
    // um gesto de vibrato deliberado (#5, que rampeia com a nota e é uma
    // decisão expressiva). Um LFO senoidal separado, bem mais lento
    // (~0.45Hz vs. 6Hz do vibrato) e bem mais raso (~2 cents vs. ~30
    // cents do vibrato em pico) - "controlada" porque é uma curva suave
    // determinística, não um passeio aleatório por sample; nunca reseta
    // de fase no disparo (ao contrário do envelope do vibrato) porque
    // representa a instabilidade do INSTRUMENTO, não um gesto de UMA
    // nota - continua presente inclusive durante Rest/Attack, antes do
    // vibrato ter rampeado.
    float livingPitchPhase = 0.0f;
    float noteAge = 0.0f;
    // Breath (items #6/#7) - 1.0 = full breath. Each trigger costs some;
    // refills on its own over ~2.5s of rest.
    float breath = 1.0f;
    unsigned int seed = 2166136261u;
    // Vetor de caráter (ver o comentário dos getters públicos acima) -
    // repouso em 0.5 (neutro), não 0 nem 1 - um "clima" real também
    // descansa num meio-termo, mesmo espírito do repouso 0.2 (não 0) do
    // Noise Field ("uma weather real raramente fica perfeitamente
    // parada"). `characterSeed` SEPARADA de `seed` (que os disparos já
    // consomem) - o drift de caráter roda todo sample, inclusive fora
    // de disparos; misturar as duas mudaria o consumo de `seed` nos
    // disparos já calibrados, um risco desnecessário pra uma feature
    // nova.
    float characterEnergy = 0.5f;
    float characterSoftness = 0.5f;
    float characterBrightness = 0.5f;
    unsigned int characterSeed = 0x5a17c0deu;
};
} // namespace antitotem
