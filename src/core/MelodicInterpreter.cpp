#include "core/MelodicInterpreter.h"

namespace antitotem
{
void MelodicInterpreter::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    // ~180ms starting glide - recomputed per trigger against the real
    // interval size in tick() (see the "connection varies by interval"
    // comment there); this is just the initial/idle value before any note
    // has ever fired. Same exponential-coefficient formula
    // ThereminVoice::configure() uses in RASGO_SYNTH
    // (1 - exp(-1/(glideSeconds*sampleRate))).
    constexpr double idleGlideSeconds = 0.18;
    glideCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (idleGlideSeconds * sampleRate)));
}

MelodicInterpreter::Voice MelodicInterpreter::tick(const Stimulus& stimulus) noexcept
{
    // Bloco "sempre ligado" - roda todo sample independente de
    // `stimulus.amount`, mesmo motivo do código original antes desta
    // extração: se `ensembleLevel`/`activity`/`breath`/`restHush` só
    // atualizassem enquanto amount>0, reerguer o slider depois de um
    // trecho baixo compararia contra um valor velho/errado, produzindo
    // um disparo espúrio pelo salto artificial em vez de um real.
    ensembleLevel += (stimulus.rawLevel - ensembleLevel) * 0.0006f;
    activity += (stimulus.rawActivityDelta - activity) * 0.02f;
    if (stimulus.accentEvent) activity += 0.03f;
    if (cooldown > 0) --cooldown;
    breath += (1.0f - breath) * static_cast<float>(1.0 / (2.5 * sampleRate));
    // Hierarquia de notas, chegada/estrutural (item #8, ver o comentário
    // do membro `metricGlow` em MelodicInterpreter.h) - decai rápido
    // (~90ms, a janela típica entre um acento e o disparo real, que
    // depende de `activity` cruzar o threshold, não é instantâneo) e
    // salta a 1.0 a cada `accentEvent`. Sempre atualizado, mesmo motivo
    // das outras trilhas "sempre ligadas" acima - um disparo logo depois
    // de reerguer `amount` não deve comparar contra um valor zerado e
    // desatualizado.
    metricGlow -= metricGlow * static_cast<float>(1.0 / (0.09 * sampleRate));
    if (stimulus.accentEvent) metricGlow = 1.0f;
    // Silêncio real de fronteira de frase (item #17) - restHush foge de
    // 1.0 rumo a 0 (~0.4s) enquanto phraseBoundaryResting estiver ligado,
    // recupera rumo a 1.0 (~0.15s) assim que a frase seguinte começa.
    const auto restHushTarget = phraseBoundaryResting ? 0.0f : 1.0f;
    const auto restHushSeconds = phraseBoundaryResting ? 0.4 : 0.15;
    restHush += (restHushTarget - restHush) * static_cast<float>(1.0 / (restHushSeconds * sampleRate));
    // Vetor de caráter (docs/PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md,
    // seção 6.1, ver o comentário dos getters públicos em
    // MelodicInterpreter.h) - três derivas independentes, mesma
    // arquitetura do Noise Field (passo pequeno correlacionado + puxão
    // de volta a um repouso neutro em 0.5), rodando SEMPRE (mesmo com
    // amount=0 ou fora de disparos - é um "clima" do instrumento, não
    // um gesto de nota). `characterSeed` própria, nunca `seed` (que os
    // disparos já consomem de um jeito calibrado).
    characterSeed = characterSeed * 1664525u + 1013904223u;
    const auto energyRoll = static_cast<float>(characterSeed >> 8) / static_cast<float>(1u << 24) * 2.0f - 1.0f;
    characterEnergy += energyRoll * 0.00001f;
    characterEnergy += (0.5f - characterEnergy) * 0.0000004f;
    characterEnergy = std::clamp(characterEnergy, 0.0f, 1.0f);
    characterSeed = characterSeed * 1664525u + 1013904223u;
    const auto softnessRoll = static_cast<float>(characterSeed >> 8) / static_cast<float>(1u << 24) * 2.0f - 1.0f;
    characterSoftness += softnessRoll * 0.00001f;
    characterSoftness += (0.5f - characterSoftness) * 0.0000004f;
    characterSoftness = std::clamp(characterSoftness, 0.0f, 1.0f);
    characterSeed = characterSeed * 1664525u + 1013904223u;
    const auto brightnessRoll = static_cast<float>(characterSeed >> 8) / static_cast<float>(1u << 24) * 2.0f - 1.0f;
    characterBrightness += brightnessRoll * 0.00001f;
    characterBrightness += (0.5f - characterBrightness) * 0.0000004f;
    characterBrightness = std::clamp(characterBrightness, 0.0f, 1.0f);

    // Only the actually expensive part below - the pitch/glide/vibrato
    // math - is skipped at amount == 0 (author reported ALSA underruns
    // after this feature was first added; running the full per-sample
    // decision math while inaudible was pure waste regardless of whether
    // it was the real cause). The caller (DualObjectEngine) separately
    // skips its own CmosVoice::tickStereo() the same way when amount<=0.
    if (stimulus.amount <= 0.0f)
        return {};

    // Multiplicadores do vetor de caráter (seção 6.1) - computados uma
    // vez aqui, usados tanto no bloco de disparo quanto no bloco por
    // sample abaixo. energyMultiplier em 0.7-1.3 (neutro em 1.0 no
    // repouso 0.5) - "mais alto = gestos maiores/mais firmes" (citação
    // da pesquisa). softnessDip/softnessGesture moldam articulação e o
    // roll de GestureType - "mais suave = articulation dip menor, menos
    // marcato".
    const auto characterEnergyMultiplier = 0.7f + characterEnergy * 0.6f;
    // Consequência de volta (seção 6.3) - marcado true só se o bloco de
    // disparo abaixo realmente disparar nesta chamada; lido no fim desta
    // função pra montar `Voice::justTriggered`.
    bool triggeredThisSample = false;
    // threshold shrinks and cooldown shortens as amount rises - both more
    // sensitive AND able to phrase faster ("na escalda do slider
    // gradualmente ele vai se excitando"). instability nudges the
    // threshold down further - more unstable weather means this voice
    // reacts to smaller wobbles, not just a fixed sensitivity from amount
    // alone. Floored well above 0 so it never reaches a literal
    // always-trigger state.
    const auto threshold = std::max(0.002f, 0.05f - stimulus.amount * 0.048f - stimulus.instability * 0.02f);
    if (cooldown == 0 && activity > threshold)
    {
        // Consequência de volta (seção 6.3) - este É o disparo real.
        triggeredThisSample = true;
        // Um disparo real sempre encerra o descanso de fronteira de frase
        // (item #17) - a próxima nota é o começo de uma frase nova.
        phraseBoundaryResting = false;
        // Pitch stays free/chromatic, no scale quantization - but each
        // new pitch is a bounded STEP from the current one ("algo como
        // assoviar uma melodia" - a whistled tune moves mostly by small
        // intervals). Magnitude drawn from a NON-ZERO range (~1-4
        // semitones, CmosVoice::tickStereo's own cv*4.3-octave mapping:
        // 1 semitone = 1/51.6) so a step never lands near zero and reads
        // as the same note again.
        constexpr float semitone = 1.0f / 51.6f;
        seed = seed * 1664525u + 1013904223u;
        const auto directionRoll = static_cast<float>(seed >> 8) / static_cast<float>(1u << 24);
        seed = seed * 1664525u + 1013904223u;
        const auto magnitudeRoll = static_cast<float>(seed >> 8) / static_cast<float>(1u << 24);

        // Fraseado / Phrase Energy (item #6) - progresso por CONTAGEM DE
        // NOTAS (limpo, monotônico dentro da frase), não por fôlego (que
        // vaza/recupera continuamente). Duas formas complementares:
        // phraseEnergy (tenda - sobe até o clímax sorteado, desce depois -
        // ênfase dinâmica) e contourBias (cresce dentro de cada metade -
        // puxão direcional real, explorar subindo/resolver descendo).
        const auto phraseProgress = std::clamp(static_cast<float>(phraseNoteIndex) / static_cast<float>(phraseLength), 0.0f, 1.0f);
        const auto risingHalf = phraseProgress < phraseClimaxPosition;
        phraseEnergy = risingHalf
            ? phraseProgress / phraseClimaxPosition
            : (1.0f - phraseProgress) / (1.0f - phraseClimaxPosition);
        const auto contourBias = risingHalf
            ? phraseProgress / phraseClimaxPosition
            : (phraseProgress - phraseClimaxPosition) / (1.0f - phraseClimaxPosition);
        // Estado de frase nomeado (item #17, macro) - Calm perto do
        // começo/fim, Climax perto do pico sorteado, Ascending/Release
        // conforme o lado.
        phraseState = phraseEnergy < 0.15f ? PhraseState::Calm
                     : phraseEnergy >= 0.75f ? PhraseState::Climax
                     : risingHalf ? PhraseState::Ascending : PhraseState::Release;

        // Hierarquia de notas, chegada/estrutural (item #8, ver o
        // comentário do membro `metricGlow`) - capturado agora, persiste
        // pela vida da nota (mesmo padrão de apexDegree acima).
        arrivalStrength = metricGlow;

        // Memória de gestos (item #16) - quantos dos últimos 4 gestos
        // foram na MESMA direção que a atual, reduzindo continueBias na
        // proporção (até 25 pontos) - empurra pra uma reversão real em
        // vez de um "assoviar" que só sobe (ou só desce) pra sempre.
        unsigned int sameDirectionCount = 0;
        for (const auto pastGesture : gestureHistory)
            if (pastGesture * previousDirection > 0.0f) ++sameDirectionCount;
        const auto monotonyBias = static_cast<float>(sameDirectionCount) / static_cast<float>(gestureHistory.size()) * 0.25f;
        // Densidade -> liberdade interpretativa (item #14) - segunda
        // leitura do mesmo seguidor de nível já usado pra balancear ganho
        // (ensembleGain abaixo), invertido: textura densa/alta = pouca
        // liberdade (disciplinada, segue Narmour de perto); textura rala =
        // liberdade real (desvia mais do esperado).
        const auto interpretiveFreedom = 1.0f - std::clamp(ensembleLevel * 2.0f, 0.0f, 1.0f);
        // Narmour-biased direction (Implication-Realization theory,
        // "process": a small interval implies continuation; "registral
        // return": a leap implies reversal). Phrase arc (item #9) - usa
        // phraseProgress real (frase fresca explora mais, frase madura se
        // compromete mais com a direção atual).
        auto continueBias = std::clamp(0.75f - (previousMagnitudeSemitones - 1.0f) * (0.5f / 3.0f) + phraseProgress * 0.15f - monotonyBias, 0.15f, 0.85f);
        // A própria escolha de direção é onde a liberdade interpretativa
        // age de verdade (item #14) - textura rala deixa continueBias
        // derivar até 35% em direção a uma moeda honesta (0.5).
        continueBias = std::clamp(continueBias + (0.5f - continueBias) * interpretiveFreedom * 0.35f, 0.15f, 0.85f);
        const auto direction = directionRoll < continueBias ? previousDirection : -previousDirection;
        // Saltos maiores perto do clímax da frase (até +50%, item #6) e
        // quando há liberdade interpretativa (até +30%, item #14) - a
        // própria amplitude do passo melódico ganha o arco/contexto, não
        // só decoração de volume/vibrato. * characterEnergyMultiplier
        // (seção 6.1) - o caráter persistente também pesa no tamanho do
        // gesto, eixo mais lento que #6/#14 (que reagem à frase/textura
        // atuais).
        const auto magnitude = (1.0f + magnitudeRoll * 3.0f) * semitone * (1.0f + phraseEnergy * 0.5f) * (1.0f + interpretiveFreedom * 0.3f) * characterEnergyMultiplier;
        previousDirection = direction;
        previousMagnitudeSemitones = magnitude / semitone;
        // Grava este gesto na memória circular (item #16) pro próximo
        // disparo medir monotonia contra ele.
        gestureHistory[static_cast<std::size_t>(gestureHistoryWrite)] = direction * magnitude;
        gestureHistoryWrite = (gestureHistoryWrite + 1) % static_cast<int>(gestureHistory.size());

        // Bounced off the 0/1 edges, not clamped - a plain clamp would let
        // the walk pin itself at a boundary and repeat the exact same
        // pitch.
        auto nextPitch = pitch + direction * magnitude;
        // Contorno de frase real (item #6) - subindo, empurra MAIS longe
        // do registro recente (escalada real); descendo, puxa de VOLTA
        // pra ele (resolução real). Aplicado ANTES do bounce/clamp abaixo.
        nextPitch += risingHalf
            ? (nextPitch - registerAverage) * contourBias * 0.12f
            : (registerAverage - nextPitch) * contourBias * 0.20f;
        if (nextPitch < 0.0f) nextPitch = -nextPitch;
        if (nextPitch > 1.0f) nextPitch = 2.0f - nextPitch;
        pitch = std::clamp(nextPitch, 0.0f, 1.0f);

        // Note hierarchy, "ápice melódico" slice (item #8) - computed
        // BEFORE updating registerAverage, so a genuine outlier still
        // reads as an outlier against where the register recently WAS.
        apexDegree = std::clamp(std::abs(pitch - registerAverage) * 4.0f, 0.0f, 1.0f);
        registerAverage += (pitch - registerAverage) * 0.15f;

        // Ornamentation, overshoot/approach slice (item #15) - scaled by
        // the SAME magnitude/direction already rolled, plus more when the
        // phrase is near its climax (item #6) or there's interpretive
        // freedom to spend (item #14).
        overshoot = direction * magnitude * (0.35f + phraseEnergy * 0.15f + interpretiveFreedom * 0.2f);

        // "é um solista, e uma voz" - phrased, not machine-gunned: ~1.4s
        // cooldown at rest, down to ~0.25s at full amount, stretched
        // further (up to 2.5x) when breath is low (item #7).
        const auto baseCooldown = 60000.0f - stimulus.amount * 49000.0f;
        cooldown = static_cast<int>(baseCooldown * (1.0f + (1.0f - breath) * 1.5f));
        // Each trigger costs some breath (~0.12) - refilled continuously
        // above, so faster triggering than the refill rate is what
        // naturally forces a pause.
        breath = std::clamp(breath - 0.12f, 0.0f, 1.0f);

        // Trajetória de final (item #4) - lookahead ANTES de incrementar:
        // esta nota que acabou de ser decidida é a última da frase se o
        // PRÓXIMO índice já bateria o comprimento sorteado.
        phraseFinalNote = (phraseNoteIndex + 1 >= phraseLength);
        // Fim de frase real (itens #6/#7, "separar a frase") - um
        // descanso GRANDE por cima do stretching normal de fôlego,
        // marcando a fronteira de verdade. A frase seguinte sorteia seu
        // PRÓPRIO comprimento (5-13 notas) e posição de clímax (25%-75%).
        ++phraseNoteIndex;
        if (phraseNoteIndex >= phraseLength)
        {
            cooldown += static_cast<int>(baseCooldown * 2.0f);
            // Silêncio real (item #17) começa AQUI, não só um cooldown
            // mais longo.
            phraseBoundaryResting = true;
            phraseNoteIndex = 0;
            seed = seed * 1664525u + 1013904223u;
            const auto lengthRoll = static_cast<float>(seed >> 8) / static_cast<float>(1u << 24);
            phraseLength = 5 + static_cast<int>(lengthRoll * 9.0f);
            seed = seed * 1664525u + 1013904223u;
            const auto climaxRoll = static_cast<float>(seed >> 8) / static_cast<float>(1u << 24);
            phraseClimaxPosition = 0.25f + climaxRoll * 0.5f;
        }

        // Connection varies by interval size (item #1) - ~90ms at a
        // 1-semitone step up to ~430ms at a 4-semitone leap, recomputed
        // from the SAME magnitude Narmour's bias above already rolled.
        const auto glideSeconds = 0.09 + (static_cast<double>(magnitude) / semitone - 1.0) / 3.0 * 0.34;
        glideCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (glideSeconds * sampleRate)));
        // Vibrato restarts its ramp on every new pitch (item #5) - a
        // fresh note hasn't earned its vibrato yet.
        noteAge = 0.0f;
        // Tipo de gesto (itens #1/#13 aprofundados, ver o comentário do
        // membro `gestureType` em MelodicInterpreter.h) - roll enviesado
        // pelo contexto já calculado acima nesta mesma função: staccato
        // favorecido por passo pequeno + textura rala (mais espaço pra
        // ser articulado); marcato por chegada métrica (#8) + salto
        // grande (ênfase real); tenuto por nota-ápice (#8) ou perto do
        // clímax de frase (#6, um momento importante merece valor
        // pleno). Legato como peso-base fixo, mantendo-o majoritário -
        // "a real theremin never re-triggers" continua sendo o
        // comportamento PADRÃO, os outros três são variedade, não
        // substituição.
        const auto magnitudeFraction = std::clamp(magnitude / (semitone * 4.0f), 0.0f, 1.0f);
        // + (1-characterSoftness)*0.15/0.10 (seção 6.1) - caráter mais
        // "duro" (suavidade baixa) favorece staccato/marcato de verdade,
        // não só o contexto imediato de frase/textura que já pesava
        // aqui.
        const auto staccatoWeight = 0.15f + interpretiveFreedom * 0.25f + (1.0f - magnitudeFraction) * 0.2f + (1.0f - characterSoftness) * 0.10f;
        const auto marcatoWeight = 0.10f + arrivalStrength * 0.35f + magnitudeFraction * 0.15f + (1.0f - characterSoftness) * 0.15f;
        // + characterSoftness*0.10 - caráter mais suave/dócil favorece
        // tenuto (segurar o valor pleno), o oposto do par acima.
        const auto tenutoWeight = 0.10f + apexDegree * 0.25f + phraseEnergy * 0.15f + characterSoftness * 0.10f;
        const auto legatoWeight = 0.65f;
        const auto gestureWeightTotal = legatoWeight + staccatoWeight + marcatoWeight + tenutoWeight;
        seed = seed * 1664525u + 1013904223u;
        const auto gestureRoll = static_cast<float>(seed >> 8) / static_cast<float>(1u << 24) * gestureWeightTotal;
        gestureType = gestureRoll < legatoWeight ? GestureType::Legato
                    : gestureRoll < legatoWeight + staccatoWeight ? GestureType::Staccato
                    : gestureRoll < legatoWeight + staccatoWeight + marcatoWeight ? GestureType::Marcato
                    : GestureType::Tenuto;
        gestureDecay = 1.0f;

        // Ataque real (item #2) - reinicia a 1.0 em TODO disparo, decai
        // sozinho abaixo. + arrivalStrength*0.4f (item #8) - uma nota que
        // aterrissa perto de um acento métrico ganha um ataque MAIS firme
        // (até 40% a mais), lendo como chegada/estrutural em vez de
        // passagem. * characterEnergyMultiplier (seção 6.1) - caráter
        // energético dá ataques mais firmes em TODA nota, não só nas de
        // chegada.
        attackPunch = (1.0f + arrivalStrength * 0.4f) * characterEnergyMultiplier;
        // Articulation dip (item #13, amplitude side) - a 1-semitone step
        // dips to ~0.94 (barely audible - legato), a 4-semitone leap dips
        // to ~0.55 (a real, brief re-attack). * (1 + arrivalStrength*0.3f)
        // (item #8) - notas de chegada re-articulam um pouco mais firme;
        // notas de passagem ficam mais legato/conectadas, sem essa
        // ênfase extra. * (1 + (1-characterSoftness)*0.4f) (seção 6.1) -
        // caráter mais "duro" aprofunda o dip em TODA nota, mesmo eixo
        // que já favorecia staccato/marcato acima, agora na amplitude.
        // Clamped no piso 0.3 - com o salto de magnitude já podendo
        // passar de 4 semitons (itens #6/#14/caráter combinados), a
        // fórmula sem clamp já conseguia ficar negativa num caso
        // extremo antes desta correção.
        articulation = std::clamp(1.0f - (magnitude / semitone - 1.0f) / 3.0f * 0.45f * (1.0f + arrivalStrength * 0.3f) * (1.0f + (1.0f - characterSoftness) * 0.4f), 0.3f, 1.0f);
    }

    // Overshoot decay (item #15, ~110ms) - eases back to 0 a bit faster
    // than most glide times so the "swoop past" resolves well before the
    // next note in typical phrasing.
    overshoot -= overshoot * static_cast<float>(1.0 / (0.11 * sampleRate));
    // Ataque real (item #2) - decaimento rápido (~50ms), mais curto que o
    // overshoot de aproximação (#15, ~110ms) - um punch é um evento
    // pontual no exato início, não um arco de chegada.
    attackPunch -= attackPunch * static_cast<float>(1.0 / (0.05 * sampleRate));
    // Destaque de Staccato/Marcato (itens #1/#13 aprofundados) - só estes
    // dois tipos empurram `gestureDecay` rumo a um piso quase-silencioso
    // (marcato mais rápido/seco, ~100ms; staccato um pouco mais suave,
    // ~160ms); Legato/Tenuto NUNCA entram aqui, ficando travados em 1.0 -
    // o pitch continua deslizando por baixo o tempo todo (nada aqui
    // toca `currentPitch`/`glideCoeff`), só o GANHO cai a quase zero
    // entre o disparo e o próximo, soando como uma nota real e separada
    // sem quebrar "a real theremin never re-triggers... the glide is
    // ALWAYS on" (ThereminVoice.hpp).
    if (gestureType == GestureType::Staccato || gestureType == GestureType::Marcato)
    {
        constexpr float gestureFloor = 0.04f;
        const auto gestureSeconds = gestureType == GestureType::Marcato ? 0.10 : 0.16;
        gestureDecay += (gestureFloor - gestureDecay) * static_cast<float>(1.0 / (gestureSeconds * sampleRate));
    }
    // Glide, never a snap (RASGO_SYNTH's own ThereminVoice.hpp: "a real
    // theremin never re-triggers... the glide is ALWAYS on") - chases
    // pitch + overshoot, not the bare target; with overshoot at 0 (its
    // resting value between ornament decays) this is exactly the pre-#15
    // formula.
    currentPitch += glideCoeff * ((pitch + overshoot) - currentPitch);
    // Recovers back to 1.0 (no dip) over ~35ms, same on every sample
    // regardless of whether a trigger just happened - a leap's own dip
    // fades on its own without a separate timer.
    articulation += (1.0f - articulation) * static_cast<float>(1.0 / (0.035 * sampleRate));
    const auto vibratoTwoPi = 6.283185307f;
    vibratoPhase += 6.0f * vibratoTwoPi / static_cast<float>(sampleRate);
    if (vibratoPhase > vibratoTwoPi) vibratoPhase -= vibratoTwoPi;
    noteAge += 1.0f / static_cast<float>(sampleRate);
    // Estado micro nomeado (item #17) - computado a cada sample a partir
    // de sinais que já existiam: Rest enquanto o cooldown ainda não
    // zerou, Attack logo após o disparo, Connect enquanto o glide ainda
    // não alcançou o alvo, Release numa nota sustentada há muito tempo
    // (mesmo limiar que já ativa sustainSettle), Sustain no meio.
    microState = cooldown > 0 ? MicroState::Rest
               : noteAge < 0.05f ? MicroState::Attack
               : std::abs(pitch - currentPitch) > 0.002f ? MicroState::Connect
               : noteAge > 1.5f ? MicroState::Release
               : MicroState::Sustain;
    // ~350ms ramp - depth is 0 right at the trigger, full depth once the
    // note has been held that long.
    const auto vibratoEnvelope = std::clamp(noteAge / 0.35f, 0.0f, 1.0f);
    // Trajetória de final (item #4) - só age na nota que já se sabe ser a
    // última da frase (phraseFinalNote, decidido no disparo). Cresce ao
    // longo de ~1.2s de vida da nota (mais lento que o silêncio de
    // amplitude de restHush/#17, que já começa a apagar o volume desde o
    // instante do disparo) - o CARÁTER (vibrato/timbre) se acalma um
    // pouco depois do volume já ter começado a ceder, como um gesto de
    // fechamento real.
    const auto finalEase = phraseFinalNote ? std::clamp(noteAge / 1.2f, 0.0f, 1.0f) : 0.0f;
    // * (1 + apexDegree*0.6f) (item #8, ápice): apex notes get up to 60%
    // more vibrato depth. * (1 + phraseEnergy*0.5f) (item #6): a
    // SEPARATE, broader boost from where the phrase itself sits. * (1 -
    // finalEase*0.6f) (item #4): vibrato acalma até 60% rumo ao fim de
    // frase. * (1 - arrivalStrength*0.25f) (item #8, chegada): notas que
    // aterrissam perto de um acento métrico ficam um pouco mais
    // "plantadas" (até -25% de vibrato) - grounded, não flutuante - eixo
    // diferente do ápice (que é sobre REGISTRO, não sobre TEMPO/acento).
    // * (0.7 + characterEnergy*0.6) (seção 6.1) - mesmo multiplicador de
    // energia do disparo, recalculado aqui pro caráter continuar
    // pesando mesmo numa nota já em curso (não só no instante do
    // disparo).
    const auto vibratoDepth = 0.006f * (1.0f + apexDegree * 0.6f) * (1.0f + phraseEnergy * 0.5f) * (1.0f - finalEase * 0.6f) * (1.0f - arrivalStrength * 0.25f) * (0.7f + characterEnergy * 0.6f);
    // Microafinação, "living pitch" (item #12, ver o comentário do membro
    // `livingPitchPhase`) - instabilidade constante, bem mais lenta/rasa
    // que o vibrato, presente inclusive fora do envelope de vibrato
    // (soma direta, sem `vibratoEnvelope`).
    livingPitchPhase += 0.45f * vibratoTwoPi / static_cast<float>(sampleRate);
    if (livingPitchPhase > vibratoTwoPi) livingPitchPhase -= vibratoTwoPi;
    const auto livingPitchDrift = std::sin(livingPitchPhase) * 0.0004f;
    const auto soundingPitch = std::clamp(currentPitch + std::sin(vibratoPhase) * vibratoDepth * vibratoEnvelope + livingPitchDrift, 0.0f, 1.0f);
    // Timbre por registro (item #10) - grave mais escuro/agudo mais
    // brilhante, função contínua do registro (0..3, mesmo range que
    // CmosVoice::setOscillatorShape já clampa). * (1 - finalEase*0.4f)
    // (item #4) - timbre escurece até 40% rumo ao fim de frase, mesma
    // ideia de "fechamento" do vibrato acima, agora no eixo do timbre.
    // gestureEdge (itens #1/#13 aprofundados) - marcato ganha uma aresta
    // de brilho no ataque (reaproveita o mesmo decaimento de
    // `attackPunch`, ~50ms - a mesma ideia de arco/pressão que muda
    // articulação E timbre juntos numa corda/sopro real), staccato uma
    // aresta menor - Legato/Tenuto sem aresta nenhuma (timbre só segue
    // registro/final, como já era).
    const auto gestureEdge = gestureType == GestureType::Marcato ? attackPunch * 0.4f
                            : gestureType == GestureType::Staccato ? attackPunch * 0.15f
                            : 0.0f;
    // characterBrightnessOffset (seção 6.1) - o segundo eixo tímbrico
    // independente do registro que a pesquisa propunha: soma direto
    // (não multiplica), até ±0.6 em torno do neutro (repouso 0.5) - o
    // "camaleão" persistente do instrumento, separado da função de
    // registro (#10) que já existia.
    const auto characterBrightnessOffset = (characterBrightness - 0.5f) * 1.2f;
    // Acoplamento intensidade↔timbre (docs/
    // PESQUISA_ORQUESTRA_RELACIONAL_E_TEMPERAMENTO.md, seção 6.2 - "em
    // cordas e sopros reais, tocar mais forte/rápido não só aumenta
    // volume, aumenta BRILHO... hoje phraseGain/apexGain só mexem em
    // GANHO; um acoplamento real ligaria os mesmos sinais também a
    // timbreBrightness"), 20 ago. 2026. Reaproveita os MESMOS sinais que
    // já escalam ganho (apexDegree/#8, phraseEnergy/#6) - nenhum
    // estímulo novo, só um segundo destino pros dois.
    const auto intensityBrightness = apexDegree * 0.3f + phraseEnergy * 0.25f;
    const auto timbreBrightness = std::clamp(soundingPitch * 3.0f * (1.0f - finalEase * 0.4f) + gestureEdge + characterBrightnessOffset + intensityBrightness, 0.0f, 3.0f);

    // ensembleGain: segue o quão alto PRINCIPAL/CLONE estão tocando -
    // floor evita sumir numa passagem quieta, ceiling evita passar do
    // headroom mesmo numa passagem alta.
    const auto ensembleGain = std::clamp(ensembleLevel * 2.0f, 0.3f, 1.0f);
    // breathGain: diminuendo suave conforme o fôlego cai (itens #6/#7) -
    // só até 25% de dip, nunca silêncio; o esticamento de cooldown acima
    // é o que de fato força a pausa.
    const auto breathGain = 0.75f + 0.25f * breath;
    // Sustain settle (item #3, "diminui") - uma nota sustentada muito
    // tempo sem novo disparo cede até 15% de ganho ao longo de ~3s.
    // Tenuto (itens #1/#13 aprofundados) segura o valor quase pleno
    // (só 5% de cessão, não 15%) - "tenuto" é literalmente "segurado no
    // valor cheio" na notação real, o oposto do settle padrão.
    const auto sustainSettleAmount = gestureType == GestureType::Tenuto ? 0.05f : 0.15f;
    const auto sustainSettle = 1.0f - std::clamp(noteAge / 3.0f, 0.0f, 1.0f) * sustainSettleAmount;
    // apexGain (item #8): notas-ápice ganham até 25% mais presença.
    const auto apexGain = 1.0f + apexDegree * 0.25f;
    // phraseGain (item #6): até 20% mais alto bem no clímax da frase.
    const auto phraseGain = 1.0f + phraseEnergy * 0.2f;
    // attackGain (item #2): punch efêmero de até +18% logo no início de
    // toda nota nova (até +25% em notas de chegada/#8, ver `attackPunch`)
    // - eixo diferente do dip de articulation (que REDUZ em saltos
    // grandes; este SOMA, sempre, no instante do disparo).
    const auto attackGain = 1.0f + attackPunch * 0.18f;
    const auto gain = (stimulus.muted || !stimulus.running) ? 0.0f
        : stimulus.amount * 0.35f * ensembleGain * articulation * breathGain * apexGain * phraseGain * sustainSettle * restHush * attackGain * gestureDecay;

    Voice voice;
    voice.soundingPitch = soundingPitch;
    voice.gain = gain;
    voice.timbreBrightness = timbreBrightness;
    voice.micro = microState;
    voice.phrase = phraseState;
    voice.justTriggered = triggeredThisSample;
    return voice;
}
} // namespace antitotem
