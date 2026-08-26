#pragma once

// Same four-language convention already established in NAVALHA2_JUCE
// (src/app/UiHelp.h): English is the primary/default language, with
// Portuguese, French and Spanish alternatives. Only the TUTORIAL and SOBRE
// windows are localized so far - the rest of the panel's Portuguese labels
// are a separate, much larger task (see TAREFAS.md).

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace antitotem::ui
{
enum class Language
{
    english = 0,
    portuguese,
    french,
    spanish
};

inline constexpr std::array<const char*, 4> languageCodes { "en", "pt", "fr", "es" };

inline int languageIndex(Language language) noexcept { return static_cast<int>(language); }

inline Language languageFromCode(const juce::String& code) noexcept
{
    if (code.equalsIgnoreCase("pt")) return Language::portuguese;
    if (code.equalsIgnoreCase("fr")) return Language::french;
    if (code.equalsIgnoreCase("es")) return Language::spanish;
    return Language::english;
}

inline juce::String languageCode(Language language)
{
    return languageCodes[static_cast<std::size_t>(languageIndex(language))];
}

inline juce::String languageLabel(Language language)
{
    constexpr std::array<const char*, 4> labels { "EN", "PT", "FR", "ES" };
    return labels[static_cast<std::size_t>(languageIndex(language))];
}

inline Language nextLanguage(Language language) noexcept
{
    return static_cast<Language>((languageIndex(language) + 1) % 4);
}

struct LocalizedText
{
    const char* en;
    const char* pt;
    const char* fr;
    const char* es;
};

inline juce::String text(const LocalizedText& value, Language language)
{
    const std::array<const char*, 4> alternatives { value.en, value.pt, value.fr, value.es };
    return juce::String::fromUTF8(alternatives[static_cast<std::size_t>(languageIndex(language))]);
}

// The panel's own mark/title - the first piece of the main window itself
// (not just TUTORIAL/SOBRE) to follow the four-language convention.
inline const LocalizedText mainTitle {
    "SOUND OBJECT - SYNTH", "OBJETO SONORO - SYNTH", "OBJET SONORE - SYNTH", "OBJETO SONORO - SYNTH"
};

struct TutorialChapter
{
    LocalizedText title;
    LocalizedText body;
    // Optional - defaults to empty so every chapter written before this
    // field existed keeps compiling unchanged (aggregate init only needs
    // to supply title/body; tip falls back to this default). A non-empty
    // tip renders as its own highlighted box in TutorialComponent instead
    // of sitting inline inside the body text - 18 ago. 2026, author:
    // "avance nos tutoriais, tudo bem organizado, com layout simpático e
    // didático". The FEEDBACK chapter's own inline "Tip:"/"Dica:"
    // paragraph (15 ago. 2026) is the content precedent this field
    // formalizes into its own visual slot.
    LocalizedText tip { "", "", "", "" };
};

// Control names on the panel itself (FREQ, MIX, OSC A, VCF, REC...) stay as
// printed - only the explanatory prose is translated, so a reader can always
// match a sentence back to the exact knob/button it describes.
inline const std::array<TutorialChapter, 9> tutorialChapters {{
    {
        { "NAVIGATION", "NAVEGAÇÃO", "NAVIGATION", "NAVEGACIÓN" },
        {
            "CLONE (top header) swaps the visible body between two "
            "independent voice objects sharing the same instrument - "
            "transport, clock and header stay the same, but PRINCIPAL and "
            "CLONE keep separate patches. Shift+C does the same swap from "
            "the keyboard.\n\n"
            "With a second monitor connected, the 2 MONITORS button turns "
            "on dual-monitor mode - from then on CLONE opens as its own "
            "window instead of swapping the body, so PRINCIPAL and CLONE "
            "run side by side while still sharing the same engine "
            "underneath.\n\n"
            "SOUND / SEQUENCE / MIX (top header) split the panel into "
            "three focused groups on narrower windows: SOUND holds the "
            "oscillators and ADSR/VCF; SEQUENCE holds the clock, steps, "
            "loop, feedback and drift; MIX holds MATÉRIA and the effects/"
            "routing controls. At the instrument's own reference size, "
            "1920x1080, all three show together and this toggle stays "
            "hidden - it only matters below that width.",
            "CLONE (cabeçalho, topo) alterna o corpo visível entre dois "
            "objetos de voz independentes que compartilham o mesmo "
            "instrumento - transporte, clock e cabeçalho continuam os "
            "mesmos, mas PRINCIPAL e CLONE guardam patches separados. "
            "Shift+C faz a mesma troca pelo teclado.\n\n"
            "Com um segundo monitor conectado, o botão 2 MONITORES liga o "
            "modo dois monitores - a partir daí CLONE abre como janela "
            "própria em vez de trocar o corpo, e PRINCIPAL e CLONE passam "
            "a rodar lado a lado, ainda compartilhando o mesmo motor por "
            "baixo.\n\n"
            "SOM / SEQUÊNCIA / MIX (cabeçalho, topo) dividem o painel em "
            "três grupos focados em janelas mais estreitas: SOM reúne os "
            "osciladores e o ADSR/VCF; SEQUÊNCIA reúne clock, passos, "
            "loop, retorno e deriva; MIX reúne MATÉRIA e os controles de "
            "efeitos/roteamento. No tamanho de referência do instrumento, "
            "1920x1080, os três aparecem juntos e esse alternador fica "
            "escondido - só importa abaixo dessa largura.",
            "CLONE (en-tête, en haut) fait basculer le corps visible "
            "entre deux objets de voix indépendants qui partagent le "
            "même instrument - transport, horloge et en-tête restent les "
            "mêmes, mais PRINCIPAL et CLONE gardent des patches séparés. "
            "Shift+C fait la même bascule au clavier.\n\n"
            "Avec un second écran connecté, le bouton 2 MONITEURS active "
            "le mode deux écrans - dès lors CLONE s'ouvre dans sa propre "
            "fenêtre au lieu de basculer le corps, et PRINCIPAL et CLONE "
            "tournent côte à côte tout en partageant le même moteur.\n\n"
            "SON / SÉQUENCE / MIX (en-tête, en haut) séparent le panneau "
            "en trois groupes ciblés sur les fenêtres plus étroites : SON "
            "réunit les oscillateurs et l'ADSR/VCF ; SÉQUENCE réunit "
            "l'horloge, les pas, la boucle, le retour et la dérive ; MIX "
            "réunit MATÉRIA et les contrôles d'effets/routage. À la "
            "taille de référence de l'instrument, 1920x1080, les trois "
            "s'affichent ensemble et ce bouton reste caché - il ne compte "
            "qu'en dessous de cette largeur.",
            "CLONE (encabezado, arriba) alterna el cuerpo visible entre "
            "dos objetos de voz independientes que comparten el mismo "
            "instrumento - transporte, reloj y encabezado siguen siendo "
            "los mismos, pero PRINCIPAL y CLONE guardan patches "
            "separados. Shift+C hace el mismo cambio desde el teclado.\n\n"
            "Con un segundo monitor conectado, el botón 2 MONITORES "
            "activa el modo dos monitores - desde entonces CLONE se abre "
            "como ventana propia en vez de alternar el cuerpo, y "
            "PRINCIPAL y CLONE pasan a correr lado a lado, todavía "
            "compartiendo el mismo motor por debajo.\n\n"
            "SONIDO / SECUENCIA / MIX (encabezado, arriba) dividen el "
            "panel en tres grupos enfocados en ventanas más angostas: "
            "SONIDO reúne los osciladores y el ADSR/VCF; SECUENCIA reúne "
            "el reloj, los pasos, el loop, el retorno y la deriva; MIX "
            "reúne MATÉRIA y los controles de efectos/enrutamiento. En el "
            "tamaño de referencia del instrumento, 1920x1080, los tres "
            "aparecen juntos y este alternador queda oculto - solo "
            "importa por debajo de ese ancho."
        },
        {
            "Start with CLONE muted and PRINCIPAL alone - learn how one "
            "voice behaves before bringing the second one in. Unmuting "
            "CLONE too early makes it hard to tell which object is doing "
            "what.",
            "Comece com o CLONE mutado e o PRINCIPAL sozinho - aprenda "
            "como uma voz se comporta antes de trazer a segunda. "
            "Desmutar o CLONE cedo demais dificulta saber qual objeto "
            "está fazendo o quê.",
            "Commencez avec CLONE en sourdine et PRINCIPAL seul - "
            "apprenez comment une voix se comporte avant d'amener la "
            "seconde. Réactiver CLONE trop tôt rend difficile de savoir "
            "quel objet fait quoi.",
            "Empieza con CLONE muteado y PRINCIPAL solo - aprende cómo "
            "se comporta una voz antes de traer la segunda. Desmutear "
            "CLONE demasiado pronto dificulta saber qué objeto está "
            "haciendo qué."
        }
    },
    {
        { "START", "COMEÇAR", "POUR COMMENCER", "EMPEZAR" },
        {
            "PLAY starts the time flow; STOP applies a short, safe fade "
            "(it never cuts the sound abruptly); RESET restarts the "
            "scanner.\n\n"
            "ENERGIA raises the whole engine's internal voltage "
            "(brighter and louder); MASTER is the final output gain.\n\n"
            "Any knob or slider with an amber thumb is a module that "
            "contributes nothing to the sound yet - it starts at zero and "
            "is silent until raised. Turn it up to hear that module join "
            "in; the highlight disappears once it does.",
            "PLAY inicia o fluxo temporal; STOP faz uma queda curta e "
            "segura (nunca corta o som abruptamente); RESET reinicia o "
            "scanner.\n\n"
            "ENERGIA sobe a tensão interna de todo o motor (mais brilho "
            "e volume); MASTER é o ganho final.\n\n"
            "Todo knob ou slider com o cursor âmbar é um módulo que ainda "
            "não contribui em nada para o som - começa zerado e fica "
            "silencioso até ser levantado. Suba-o para ouvir esse módulo "
            "entrar; o destaque desaparece assim que isso acontece.",
            "PLAY démarre le flux temporel ; STOP applique une chute courte "
            "et sûre (le son n'est jamais coupé brutalement) ; RESET "
            "redémarre le scanner.\n\n"
            "ENERGIA augmente la tension interne de tout le moteur (plus "
            "de brillance et de volume) ; MASTER est le gain de sortie "
            "final.\n\n"
            "Tout bouton ou curseur au repère ambré est un module qui ne "
            "contribue encore en rien au son - il démarre à zéro et reste "
            "silencieux tant qu'il n'est pas monté. Augmentez-le pour "
            "entendre ce module entrer ; le repère disparaît dès que c'est "
            "fait.",
            "PLAY inicia el flujo temporal; STOP aplica una caída corta y "
            "segura (nunca corta el sonido de golpe); RESET reinicia el "
            "scanner.\n\n"
            "ENERGIA sube la tensión interna de todo el motor (más "
            "brillo y volumen); MASTER es la ganancia final.\n\n"
            "Cualquier knob o control deslizante con el cursor ámbar es un "
            "módulo que todavía no contribuye en nada al sonido - empieza "
            "en cero y permanece silencioso hasta que se sube. Súbelo para "
            "escuchar ese módulo entrar; el destaque desaparece en cuanto "
            "eso ocurre."
        },
        {
            "Press PLAY, then raise ENERGIA and MASTER together, slowly "
            "- that alone is the fastest way to hear the whole engine "
            "wake up, instead of hunting for which amber-highlighted "
            "knob is the silent one.",
            "Aperte PLAY, depois suba ENERGIA e MASTER juntos, devagar - "
            "só isso já é o jeito mais rápido de ouvir o motor inteiro "
            "acordar, em vez de caçar qual knob com destaque âmbar está "
            "mudo.",
            "Appuyez sur PLAY, puis montez ENERGIA et MASTER ensemble, "
            "lentement - c'est déjà la façon la plus rapide d'entendre "
            "tout le moteur s'éveiller, plutôt que de chercher quel "
            "bouton au repère ambré est silencieux.",
            "Presiona PLAY, luego sube ENERGIA y MASTER juntos, "
            "despacio - eso solo ya es la forma más rápida de escuchar "
            "todo el motor despertar, en vez de buscar cuál knob con "
            "destaque ámbar está mudo."
        }
    },
    {
        { "OSCILLATORS", "OSCILADORES", "OSCILLATEURS", "OSCILADORES" },
        {
            "Five columns - OSC A, OSC B, OSC C, OSC 4, OSC 5 - each with the "
            "same six controls (FREQ, MIX, SHAPE, AXIS X, AXIS Y, AXIS Z), "
            "but different behaviour behind them.\n\n"
            "OSC A/B/C are continuous voices (sine/triangle/saw/square via "
            "SHAPE); FREQ ranges 0.125x to 4x. The 40106/8038/4069UB switch "
            "above the columns changes the shared colouring character of "
            "these three together.\n\n"
            "OSC 4 (4093 . 4020, sub/clock) is a deep-divided relaxation "
            "source. FREQ goes from 0.03125x to 4x - at the low end it reads "
            "as a divided clock pulse, not a note; SHAPE sets the pulse's "
            "asymmetry. It starts at MIX 0 (silent).\n\n"
            "OSC 5 (4046 . LM13600, heterodyne) does not add - it multiplies "
            "its own wave by OSC A's raw waveform (real ring modulation). "
            "FREQ near 1x gives a slow beat against OSC A; moving away from "
            "1x opens a more inharmonic, heterodyne spectrum. It also starts "
            "at MIX 0 (silent).\n\n"
            "AXIS Y (proximity) blends each oscillator with a duller, "
            "filtered version of itself - 0 is dry, raising it moves that "
            "voice further into material. AXIS Z (orbit) lets an "
            "oscillator drift slightly in pitch and slowly circulate its "
            "own pan position over several seconds instead of sitting "
            "still - 0 leaves it exactly where EIXO X puts it. Both start "
            "at 0 (no change from before these controls existed).",
            "Cinco colunas - OSC A, OSC B, OSC C, OSC 4, OSC 5 - cada uma com "
            "os mesmos 6 controles (FREQ, MIX, FORMA, EIXO X, EIXO Y, EIXO "
            "Z), mas com comportamento diferente por trás.\n\n"
            "OSC A/B/C são vozes contínuas (senoide/triangular/serra/"
            "quadrada via FORMA); FREQ vai de 0.125x a 4x. O seletor "
            "40106/8038/4069UB acima das colunas muda o caráter de "
            "coloração compartilhado dessas três juntas.\n\n"
            "OSC 4 (4093 . 4020, sub/clock) é um relaxador dividido fundo. "
            "FREQ vai de 0.03125x a 4x - nas razões baixas soa como pulso de "
            "clock dividido, não uma nota; FORMA controla a assimetria do "
            "pulso. Começa em MIX 0 (mudo).\n\n"
            "OSC 5 (4046 . LM13600, heteródino) não soma - multiplica sua "
            "própria onda pela onda crua do OSC A (ring modulation real). "
            "FREQ perto de 1x dá batimento lento contra OSC A; afastando de "
            "1x abre um espectro mais inarmônico e heteródino. Também "
            "começa em MIX 0 (mudo).\n\n"
            "EIXO Y (proximidade) mistura cada oscilador com uma versão "
            "mais abafada e filtrada de si mesmo - 0 é seco, subir move "
            "essa voz mais pra dentro da matéria. EIXO Z (órbita) deixa um "
            "oscilador derivar de leve em afinação e circular sua própria "
            "posição de pan ao longo de vários segundos, em vez de ficar "
            "parado - 0 deixa exatamente onde EIXO X coloca. Os dois "
            "começam em 0 (sem mudança em relação a antes desses "
            "controles existirem).",
            "Cinq colonnes - OSC A, OSC B, OSC C, OSC 4, OSC 5 - chacune avec "
            "les six mêmes contrôles (FREQ, MIX, FORME, AXE X, AXE Y, "
            "AXE Z), mais un comportement différent derrière.\n\n"
            "OSC A/B/C sont des voix continues (sinus/triangle/dent de "
            "scie/carré via FORME) ; FREQ va de 0.125x à 4x. Le "
            "sélecteur 40106/8038/4069UB au-dessus des colonnes change le "
            "caractère de coloration partagé de ces trois-là ensemble.\n\n"
            "OSC 4 (4093 . 4020, sous-horloge) est une source de relaxation "
            "profondément divisée. FREQ va de 0.03125x à 4x - aux ratios "
            "bas, on entend une impulsion d'horloge divisée, pas une note ; "
            "FORME règle l'asymétrie de l'impulsion. Il démarre à MIX 0 "
            "(silencieux).\n\n"
            "OSC 5 (4046 . LM13600, hétérodyne) n'additionne pas - il "
            "multiplie sa propre onde par l'onde brute de OSC A (vraie "
            "modulation en anneau). FREQ proche de 1x donne un battement "
            "lent contre OSC A ; s'éloigner de 1x ouvre un spectre plus "
            "inharmonique et hétérodyne. Il démarre aussi à MIX 0 "
            "(silencieux).\n\n"
            "AXE Y (proximité) mélange chaque oscillateur avec une "
            "version plus sourde et filtrée de lui-même - 0 est sec, "
            "l'augmenter déplace cette voix plus loin dans la matière. "
            "AXE Z (orbite) laisse un oscillateur dériver légèrement en "
            "hauteur et faire circuler lentement sa propre position de "
            "panoramique sur plusieurs secondes, au lieu de rester "
            "immobile - 0 le laisse exactement là où EIXO X le place. Les "
            "deux démarrent à 0 (aucun changement par rapport à avant "
            "l'existence de ces contrôles).",
            "Cinco columnas - OSC A, OSC B, OSC C, OSC 4, OSC 5 - cada una "
            "con los mismos 6 controles (FREQ, MIX, FORMA, EJE X, EJE Y, "
            "EJE Z), pero con comportamiento distinto detrás.\n\n"
            "OSC A/B/C son voces continuas (seno/triángulo/sierra/cuadrada "
            "vía FORMA); FREQ va de 0.125x a 4x. El selector "
            "40106/8038/4069UB encima de las columnas cambia el carácter de "
            "coloración compartido de esas tres juntas.\n\n"
            "OSC 4 (4093 . 4020, sub-reloj) es una fuente de relajación "
            "dividida en profundidad. FREQ va de 0.03125x a 4x - en las "
            "razones bajas suena como un pulso de reloj dividido, no una "
            "nota; FORMA controla la asimetría del pulso. Empieza en MIX 0 "
            "(mudo).\n\n"
            "OSC 5 (4046 . LM13600, heterodino) no suma - multiplica su "
            "propia onda por la onda cruda de OSC A (ring modulation real). "
            "FREQ cerca de 1x da un batido lento contra OSC A; alejarse de "
            "1x abre un espectro más inarmónico y heterodino. También "
            "empieza en MIX 0 (mudo).\n\n"
            "EJE Y (proximidad) mezcla cada oscilador con una versión "
            "más apagada y filtrada de sí mismo - 0 es seco, subirlo "
            "mueve esa voz más adentro de la materia. EJE Z (órbita) "
            "deja que un oscilador derive levemente en afinación y "
            "circule su propia posición de panorama a lo largo de varios "
            "segundos, en vez de quedarse quieto - 0 lo deja exactamente "
            "donde EIXO X lo pone. Ambos empiezan en 0 (sin cambio "
            "respecto a antes de que estos controles existieran)."
        },
        {
            "Start with only OSC A active (everything else at MIX 0) "
            "and learn its own FREQ/SHAPE by ear before bringing in "
            "OSC 4 or OSC 5 one at a time - stacking all five from the "
            "start makes it hard to tell which oscillator you are "
            "actually hearing.",
            "Comece só com OSC A ativo (o resto em MIX 0) e aprenda "
            "FREQ/FORMA dele de ouvido antes de trazer OSC 4 ou OSC 5 "
            "um de cada vez - empilhar os cinco desde o início dificulta "
            "saber qual oscilador você está realmente ouvindo.",
            "Commencez avec seulement OSC A actif (le reste à MIX 0) et "
            "apprenez son propre FREQ/FORME à l'oreille avant d'ajouter "
            "OSC 4 ou OSC 5 un à la fois - empiler les cinq dès le "
            "départ rend difficile de savoir quel oscillateur vous "
            "entendez réellement.",
            "Empieza solo con OSC A activo (el resto en MIX 0) y "
            "aprende su propio FREQ/FORMA de oído antes de traer OSC 4 "
            "u OSC 5 uno a la vez - apilar los cinco desde el inicio "
            "dificulta saber qué oscilador estás escuchando en "
            "realidad."
        }
    },
    {
        { "SEQUENCE", "SEQUÊNCIA", "SÉQUENCE", "SECUENCIA" },
        {
            "16 steps, each with CV (pitch/voltage), AMP (level), FX (send "
            "to effects) and MUTE.\n\n"
            "FIM DO LOOP sets how many of the 16 steps play before "
            "repeating.\n\n"
            "PERCURSO picks the scanner's direction: FWD (always forward), "
            "REV (always backward), ALT (alternates each pass) or MEM "
            "(addresses chosen from internal memory, never repeating the "
            "previous step - deterministic, not audio noise).\n\n"
            "SUBDIVISÃO changes the clock's rhythmic feel (straight, "
            "tuplets, swing, or glitch).",
            "16 steps, cada um com CV (altura/tensão), AMP (nível), FX "
            "(envio aos efeitos) e MUTE.\n\n"
            "FIM DO LOOP define quantos dos 16 steps tocam antes de "
            "repetir.\n\n"
            "PERCURSO escolhe a direção do scanner: FWD (sempre em "
            "frente), REV (sempre para trás), ALT (alterna a cada volta) "
            "ou MEM (endereços escolhidos por memória interna, sem "
            "repetir o passo anterior - determinístico, não é ruído de "
            "áudio).\n\n"
            "SUBDIVISÃO muda o sabor rítmico do clock (reto, tuplets, "
            "swing, ou glitch).",
            "16 pas, chacun avec CV (hauteur/tension), AMP (niveau), FX "
            "(envoi aux effets) et MUTE.\n\n"
            "FIM DO LOOP règle combien des 16 pas jouent avant de "
            "répéter.\n\n"
            "PERCURSO choisit la direction du scanner : FWD (toujours en "
            "avant), REV (toujours en arrière), ALT (alterne à chaque "
            "tour) ou MEM (adresses choisies par une mémoire interne, "
            "sans jamais répéter le pas précédent - déterministe, pas du "
            "bruit audio).\n\n"
            "SUBDIVISÃO change la sensation rythmique de l'horloge (droit, "
            "triolets, swing, ou glitch).",
            "16 pasos, cada uno con CV (altura/tensión), AMP (nivel), FX "
            "(envío a los efectos) y MUTE.\n\n"
            "FIM DO LOOP define cuántos de los 16 pasos suenan antes de "
            "repetir.\n\n"
            "PERCURSO elige la dirección del scanner: FWD (siempre hacia "
            "adelante), REV (siempre hacia atrás), ALT (alterna en cada "
            "vuelta) o MEM (direcciones elegidas por memoria interna, sin "
            "repetir el paso anterior - determinístico, no es ruido de "
            "audio).\n\n"
            "SUBDIVISÃO cambia la sensación rítmica del reloj (recto, "
            "tresillos, swing, o glitch)."
        },
        {
            "Set FIM DO LOOP to a small number first (4 or 8) so you can "
            "hear the whole loop repeat quickly, then switch PERCURSO "
            "from FWD to MEM - the difference between a predictable "
            "cycle and a deterministic path that never repeats its "
            "previous step is much clearer once you already know what "
            "the loop sounds like straight.",
            "Deixe FIM DO LOOP num número pequeno primeiro (4 ou 8) pra "
            "ouvir o loop inteiro repetir rápido, depois troque PERCURSO "
            "de FWD pra MEM - a diferença entre um ciclo previsível e um "
            "percurso determinístico que nunca repete o passo anterior "
            "fica bem mais clara quando você já conhece o loop reto.",
            "Mettez d'abord FIM DO LOOP sur un petit nombre (4 ou 8) "
            "pour entendre toute la boucle se répéter rapidement, puis "
            "passez PERCURSO de FWD à MEM - la différence entre un "
            "cycle prévisible et un parcours déterministe qui ne répète "
            "jamais le pas précédent est bien plus claire une fois que "
            "vous connaissez déjà la boucle droite.",
            "Pon primero FIM DO LOOP en un número pequeño (4 u 8) para "
            "oír todo el loop repetirse rápido, luego cambia PERCURSO "
            "de FWD a MEM - la diferencia entre un ciclo predecible y "
            "un recorrido determinístico que nunca repite el paso "
            "anterior queda mucho más clara cuando ya conoces el loop "
            "recto."
        }
    },
    {
        { "VCF / ADSR", "VCF / ADSR", "VCF / ADSR", "VCF / ADSR" },
        {
            "VCF: FREQ sets the cutoff, RES adds resonant emphasis right "
            "at that cutoff, and CV sets how deep the scanner's own "
            "step-by-step modulation reaches the filter - at CV 0 the "
            "cutoff sits still, a higher CV lets each step push it up "
            "or down.\n\n"
            "The mode button cycles four classic responses: LPF cuts "
            "everything above FREQ, HPF cuts everything below it, BPF "
            "keeps only a band around FREQ, and NOTCH removes a narrow "
            "band around FREQ instead of keeping it. VCF is the "
            "instrument's clean filter - a second, saturating filter "
            "(MATÉRIA) sits further down the chain, covered in "
            "AVANÇADO.\n\n"
            "ADSR: ATT/DEC/SUS/REL shape the amplitude envelope of "
            "every step - ATT is how long it takes to reach full level "
            "once a step starts, DEC how long it then takes to fall to "
            "the SUS level, SUS the level it holds while the step "
            "stays active, and REL how long it takes to fade once the "
            "step ends. This envelope is the instrument's own VCA: "
            "every oscillator's sum passes through it before anything "
            "else touches the signal.",
            "VCF: FREQ define o corte, RES acrescenta ênfase ressonante "
            "bem nesse corte, e CV define o quão fundo a modulação do "
            "scanner, passo a passo, alcança o filtro - com CV em 0 o "
            "corte fica parado, um CV mais alto deixa cada step "
            "empurrá-lo pra cima ou pra baixo.\n\n"
            "O botão de modo cicla quatro respostas clássicas: LPF "
            "corta tudo acima de FREQ, HPF corta tudo abaixo, BPF "
            "mantém só uma faixa ao redor de FREQ, e NOTCH remove uma "
            "faixa estreita ao redor de FREQ em vez de mantê-la. VCF é "
            "o filtro limpo do instrumento - um segundo filtro, "
            "saturador (MATÉRIA), fica mais adiante na cadeia, coberto "
            "no AVANÇADO.\n\n"
            "ADSR: ATT/DEC/SUS/REL moldam a envolvente de amplitude de "
            "cada step - ATT é quanto tempo leva pra alcançar o nível "
            "máximo assim que um step começa, DEC quanto tempo leva "
            "depois pra cair até o nível de SUS, SUS o nível que ele "
            "mantém enquanto o step continua ativo, e REL quanto tempo "
            "leva pra apagar quando o step termina. Essa envolvente é o "
            "próprio VCA do instrumento: a soma de todos os osciladores "
            "passa por ela antes de qualquer outra coisa tocar o sinal.",
            "VCF : FREQ règle la coupure, RES ajoute une emphase "
            "résonante juste à cette coupure, et CV règle à quel point "
            "la modulation du scanner, pas à pas, atteint le filtre - à "
            "CV 0 la coupure reste immobile, un CV plus élevé laisse "
            "chaque pas la pousser vers le haut ou le bas.\n\n"
            "Le bouton de mode fait défiler quatre réponses classiques "
            ": LPF coupe tout au-dessus de FREQ, HPF coupe tout en "
            "dessous, BPF ne garde qu'une bande autour de FREQ, et "
            "NOTCH retire une bande étroite autour de FREQ au lieu de "
            "la garder. VCF est le filtre propre de l'instrument - un "
            "second filtre, saturant (MATÉRIA), se trouve plus loin "
            "dans la chaîne, couvert dans AVANÇADO.\n\n"
            "ADSR : ATT/DEC/SUS/REL façonnent l'enveloppe d'amplitude "
            "de chaque pas - ATT est le temps pour atteindre le niveau "
            "plein dès qu'un pas commence, DEC le temps pour ensuite "
            "retomber au niveau SUS, SUS le niveau maintenu tant que le "
            "pas reste actif, et REL le temps pour s'éteindre une fois "
            "le pas terminé. Cette enveloppe est le VCA propre de "
            "l'instrument : la somme de tous les oscillateurs la "
            "traverse avant que quoi que ce soit d'autre ne touche le "
            "signal.",
            "VCF: FREQ define el corte, RES añade énfasis resonante "
            "justo en ese corte, y CV define cuán profundo llega al "
            "filtro la modulación del scanner, paso a paso - con CV en "
            "0 el corte queda quieto, un CV más alto deja que cada "
            "paso lo empuje hacia arriba o hacia abajo.\n\n"
            "El botón de modo recorre cuatro respuestas clásicas: LPF "
            "corta todo por encima de FREQ, HPF corta todo por debajo, "
            "BPF mantiene solo una banda alrededor de FREQ, y NOTCH "
            "elimina una banda estrecha alrededor de FREQ en vez de "
            "conservarla. VCF es el filtro limpio del instrumento - un "
            "segundo filtro, saturador (MATÉRIA), está más adelante en "
            "la cadena, cubierto en AVANÇADO.\n\n"
            "ADSR: ATT/DEC/SUS/REL moldean la envolvente de amplitud "
            "de cada paso - ATT es cuánto tarda en alcanzar el nivel "
            "máximo apenas empieza un paso, DEC cuánto tarda después "
            "en caer hasta el nivel de SUS, SUS el nivel que mantiene "
            "mientras el paso sigue activo, y REL cuánto tarda en "
            "apagarse cuando el paso termina. Esa envolvente es el "
            "propio VCA del instrumento: la suma de todos los "
            "osciladores pasa por ella antes de que cualquier otra "
            "cosa toque la señal."
        },
        {
            "Raise RES with CV at 0 first to hear the filter's own "
            "character alone, then bring CV up to hear the scanner's "
            "own modulation reach it - cycling LPF/BPF/HPF/NOTCH on the "
            "exact same sound is the fastest way to learn what each "
            "mode actually removes.",
            "Suba RES com CV em 0 primeiro pra ouvir o caráter do "
            "filtro sozinho, depois suba CV pra ouvir a modulação do "
            "scanner alcançá-lo - ciclar entre LPF/BPF/HPF/NOTCH no "
            "mesmo som exato é o jeito mais rápido de aprender o que "
            "cada modo realmente remove.",
            "Montez RES avec CV à 0 d'abord pour entendre le caractère "
            "du filtre seul, puis montez CV pour entendre la "
            "modulation du scanner l'atteindre - faire défiler LPF/BPF/"
            "HPF/NOTCH sur exactement le même son est la façon la plus "
            "rapide d'apprendre ce que chaque mode retire vraiment.",
            "Sube RES con CV en 0 primero para oír el carácter del "
            "filtro solo, luego sube CV para oír la modulación del "
            "scanner alcanzarlo - recorrer LPF/BPF/HPF/NOTCH sobre el "
            "mismo sonido exacto es la forma más rápida de aprender qué "
            "quita realmente cada modo."
        }
    },
    {
        { "FEEDBACK", "RETORNO", "RETOUR", "RETORNO" },
        {
            "Up to 6 feedback ports (FB/DIODE/CAP/PULSE/TRANS/REFLUX) can be "
            "active at once - each shapes the voice's own last output "
            "sample (or its slow internal capacitor charge) differently "
            "before feeding it back in. FB is the raw sample - sustains/"
            "reinforces the current timbre. DIODE half-wave-rectifies it - "
            "doubles the perceived pitch, rougher harmonics. CAP feeds "
            "back the slow capacitor average instead of the instant "
            "sample - smoother, memory-like sustain. PULSE collapses it "
            "to a bare +/-1 square - a clock/gate-like return, more "
            "rhythmic than tonal. TRANS blends the instant sample with "
            "the capacitor and soft-clips the mix - warmer, compressed. "
            "REFLUX subtracts the capacitor from the instant sample, "
            "isolating the fast/transient part - sharpens attacks, the "
            "opposite of CAP. With more than one port active they are "
            "averaged, not summed - two ports blend character, they do "
            "not double loudness. FB GAIN scales this combined return "
            "(plus any incoming signal from the other object) into OSC "
            "A's own pitch and a touch of stereo colour - it works even "
            "with no PRINCIPAL/CLONE routing at all, as long as a port "
            "is on.\n\n"
            "NOISE has 6 colours (white, pink, brown, blue, violet, "
            "bit-reduced) and its own MIX. S&H (sample & hold) creates "
            "unstable voltage steps - RATE sets sampling speed, MIX its "
            "presence in the signal.\n\n"
            "The MODULAÇÃO row carries the LFO (RATE, sine/triangle/"
            "pulse SHAPE) and RING (ring modulation).\n\n"
            "ESPAÇO/FASE carries REVERB, PHASER and FLANGER (each with "
            "MIX, and RATE/DEPTH where relevant), plus the comb/"
            "resonator's own RES MIX, RES ALTURA (its pitch) and RES "
            "CORPO (how long it rings) - a distinct pitched resonance, "
            "not more ambience.\n\n"
            "PRINCIPAL and CLONE (OBJETO 5) feed each other's live output "
            "into their own external feedback input, shaped by the same "
            "DIRETO/DIODO/CAP/PULSO vocabulary as PORTAS DE FEEDBACK above "
            "- PRINCIPAL→CLONE and CLONE→PRINCIPAL set how strongly. "
            "AUX→PRINCIPAL and AUX→CLONE add a second, quieter channel: "
            "two hidden oscillators, each one listening to the other "
            "object's loudness and drifting with it, sent in instead of "
            "the direct signal. All four controls and both route rows "
            "only appear in the CLONE window's own CONEXÃO ENTRE OBJETOS "
            "section, even though they affect both objects equally. Open "
            "CLONE at least once to see what that routing is actually "
            "set to.",
            "Até 6 portas de retorno (FB/DIODE/CAP/PULSE/TRANS/REFLUX) "
            "podem ficar ativas ao mesmo tempo - cada uma molda de um "
            "jeito diferente a última amostra de saída da própria voz (ou "
            "sua carga lenta interna, o capacitor) antes de realimentá-la. "
            "FB é a amostra crua - sustenta/reforça o timbre atual. DIODE "
            "retifica em meia-onda - dobra a frequência percebida, "
            "harmônicos mais ásperos. CAP realimenta a média lenta do "
            "capacitor em vez da amostra instantânea - sustain mais "
            "suave, tipo memória. PULSE reduz tudo a um +/-1 quadrado "
            "puro - retorno tipo clock/gate, mais rítmico que tonal. "
            "TRANS mistura a amostra instantânea com o capacitor e satura "
            "suave a mistura - mais quente, comprimido. REFLUX subtrai o "
            "capacitor da amostra instantânea, isolando só a parte "
            "rápida/transiente - realça ataques, o oposto do CAP. Com "
            "mais de uma porta ativa, elas são somadas por MÉDIA, não "
            "empilhadas - duas portas misturam caráter, não dobram "
            "volume. FB GAIN dosa esse retorno combinado (mais qualquer "
            "sinal vindo do outro objeto) na própria afinação do OSC A e "
            "num toque de coloração estéreo - funciona mesmo sem nenhuma "
            "rota PRINCIPAL/CLONE, desde que uma porta esteja ligada.\n\n"
            "NOISE tem 6 cores (branco, rosa, marrom, azul, violeta, "
            "bit-reduzido) e um MIX próprio. S&H (sample & hold) cria "
            "degraus de tensão instáveis - RATE define a velocidade das "
            "amostras, MIX a presença no sinal.\n\n"
            "A fileira MODULAÇÃO traz o LFO (RATE, FORMA seno/triângulo/"
            "pulso) e o RING (ring modulation).\n\n"
            "ESPAÇO/FASE traz REVERB, PHASER e FLANGER (cada um com MIX, "
            "e RATE/DEPTH quando aplicável), além do RES MIX, RES ALTURA "
            "(altura da ressonância) e RES CORPO (quanto tempo ela soa) "
            "do comb/resonador - uma ressonância com altura própria, "
            "distinta de mais ambiência.\n\n"
            "PRINCIPAL e CLONE (OBJETO 5) reintroduzem a saída ao vivo um "
            "do outro na própria entrada de retorno externo, moldada pelo "
            "mesmo vocabulário DIRETO/DIODO/CAP/PULSO das PORTAS DE "
            "FEEDBACK acima - PRINCIPAL→CLONE e CLONE→PRINCIPAL dosam a "
            "intensidade disso. AUX→PRINCIPAL e AUX→CLONE somam um "
            "segundo canal, mais discreto: dois osciladores escondidos, "
            "cada um escutando o volume do outro objeto e derivando junto "
            "com ele, enviados em vez do sinal direto. Os quatro "
            "controles e as duas fileiras de rota só aparecem na própria "
            "seção CONEXÃO ENTRE OBJETOS da janela CLONE, mesmo afetando "
            "os dois objetos igualmente. Abra o CLONE ao menos uma vez "
            "para ver como essa rota está configurada.",
            "Jusqu'à 6 ports de retour (FB/DIODE/CAP/PULSE/TRANS/REFLUX) "
            "peuvent être actifs en même temps - chacun façonne "
            "différemment le dernier échantillon de sortie de la voix (ou "
            "sa charge lente interne, le capacitor) avant de le "
            "réinjecter. FB est l'échantillon brut - soutient/renforce le "
            "timbre actuel. DIODE le redresse en demi-onde - double la "
            "hauteur perçue, harmoniques plus rêches. CAP réinjecte la "
            "moyenne lente du capacitor au lieu de l'échantillon "
            "instantané - sustain plus doux, façon mémoire. PULSE le "
            "réduit à un simple carré +/-1 - retour type horloge/gate, "
            "plus rythmique que tonal. TRANS mélange l'échantillon "
            "instantané avec le capacitor et sature doucement le mélange "
            "- plus chaud, compressé. REFLUX soustrait le capacitor de "
            "l'échantillon instantané, isolant la partie rapide/"
            "transitoire - accentue les attaques, l'inverse de CAP. Avec "
            "plus d'un port actif, ils sont MOYENNÉS, pas additionnés - "
            "deux ports mélangent leur caractère sans doubler le volume. "
            "FB GAIN dose ce retour combiné (plus tout signal venant de "
            "l'autre objet) dans la hauteur de l'OSC A et une touche de "
            "couleur stéréo - fonctionne même sans aucune route "
            "PRINCIPAL/CLONE, tant qu'un port est actif.\n\n"
            "NOISE a 6 couleurs (blanc, rose, brun, bleu, violet, réduit en "
            "bits) et son propre MIX. S&H (sample & hold) crée des paliers "
            "de tension instables - RATE règle la vitesse "
            "d'échantillonnage, MIX sa présence dans le signal.\n\n"
            "La rangée MODULAÇÃO porte le LFO (RATE, FORME sinus/"
            "triangle/impulsion) et le RING (ring modulation).\n\n"
            "ESPAÇO/FASE porte REVERB, PHASER et FLANGER (chacun avec "
            "MIX, et RATE/DEPTH quand applicable), ainsi que RES MIX, "
            "RES ALTURA (hauteur de la résonance) et RES CORPO (durée de "
            "résonance) du comb/résonateur - une résonance avec sa "
            "propre hauteur, distincte d'une ambiance supplémentaire.\n\n"
            "PRINCIPAL et CLONE (OBJETO 5) réinjectent la sortie en direct "
            "l'un de l'autre dans leur propre entrée de retour externe, "
            "façonnée par le même vocabulaire DIRETO/DIODO/CAP/PULSO que "
            "PORTAS DE FEEDBACK ci-dessus - PRINCIPAL→CLONE et "
            "CLONE→PRINCIPAL dosent l'intensité de cela. AUX→PRINCIPAL "
            "et AUX→CLONE ajoutent un second canal, plus discret : deux "
            "oscillateurs cachés, chacun à l'écoute du volume de l'autre "
            "objet et dérivant avec lui, envoyés à la place du signal "
            "direct. Les quatre contrôles et les deux rangées de routes "
            "n'apparaissent que dans la section CONEXÃO ENTRE OBJETOS de "
            "la fenêtre CLONE elle-même, même si cela affecte les deux "
            "objets de façon égale. Ouvrez CLONE au moins une fois pour "
            "voir comment cette route est réglée.",
            "Hasta 6 puertos de retorno (FB/DIODE/CAP/PULSE/TRANS/REFLUX) "
            "pueden estar activos a la vez - cada uno moldea de forma "
            "distinta la última muestra de salida de la voz (o su carga "
            "lenta interna, el capacitor) antes de reintroducirla. FB es "
            "la muestra cruda - sostiene/refuerza el timbre actual. DIODE "
            "la rectifica en media onda - duplica la altura percibida, "
            "armónicos más ásperos. CAP reintroduce el promedio lento del "
            "capacitor en vez de la muestra instantánea - sustain más "
            "suave, tipo memoria. PULSE la reduce a un +/-1 cuadrado "
            "puro - retorno tipo reloj/gate, más rítmico que tonal. TRANS "
            "mezcla la muestra instantánea con el capacitor y satura "
            "suave la mezcla - más cálido, comprimido. REFLUX resta el "
            "capacitor de la muestra instantánea, aislando solo la parte "
            "rápida/transitoria - realza los ataques, lo opuesto de CAP. "
            "Con más de un puerto activo, se promedian, no se suman - dos "
            "puertos mezclan carácter, no duplican volumen. FB GAIN "
            "dosifica ese retorno combinado (más cualquier señal del "
            "otro objeto) en la afinación del OSC A y un toque de color "
            "estéreo - funciona incluso sin ninguna ruta PRINCIPAL/"
            "CLONE, mientras un puerto esté activo.\n\n"
            "NOISE tiene 6 colores (blanco, rosa, marrón, azul, violeta, "
            "bit-reducido) y su propio MIX. S&H (sample & hold) crea "
            "escalones de tensión inestables - RATE define la velocidad "
            "de muestreo, MIX su presencia en la señal.\n\n"
            "La fila MODULAÇÃO trae el LFO (RATE, FORMA seno/triángulo/"
            "pulso) y el RING (ring modulation).\n\n"
            "ESPAÇO/FASE trae REVERB, PHASER y FLANGER (cada uno con "
            "MIX, y RATE/DEPTH cuando aplica), además de RES MIX, RES "
            "ALTURA (altura de la resonancia) y RES CORPO (cuánto "
            "tiempo suena) del comb/resonador - una resonancia con "
            "altura propia, distinta de más ambiencia.\n\n"
            "PRINCIPAL y CLONE (OBJETO 5) reintroducen la salida en vivo "
            "uno del otro en su propia entrada de retorno externo, "
            "moldeada por el mismo vocabulario DIRETO/DIODO/CAP/PULSO de "
            "PORTAS DE FEEDBACK arriba - PRINCIPAL→CLONE y CLONE→PRINCIPAL "
            "dosifican esa intensidad. AUX→PRINCIPAL y AUX→CLONE suman un "
            "segundo canal, más discreto: dos osciladores escondidos, "
            "cada uno escuchando el volumen del otro objeto y derivando "
            "junto con él, enviados en lugar de la señal directa. Los "
            "cuatro controles y las dos filas de ruta solo aparecen en la "
            "propia sección CONEXÃO ENTRE OBJETOS de la ventana CLONE, "
            "aunque afecta a ambos objetos por igual. Abra CLONE al menos "
            "una vez para ver cómo está configurada esa ruta."
        },
        {
            "Start with only FB active and FB GAIN low, to hear the raw "
            "return before layering DIODE or TRANS on top - stacking "
            "straight from all six at once just reads as noise.",
            "Comece só com FB ativo e FB GAIN baixo, pra ouvir o retorno "
            "cru antes de empilhar DIODE ou TRANS por cima - ligar as "
            "seis de uma vez só soa como ruído.",
            "Commencez avec seulement FB actif et FB GAIN bas, pour "
            "entendre le retour brut avant d'ajouter DIODE ou TRANS "
            "par-dessus - activer les six d'un coup sonne juste comme du "
            "bruit.",
            "Empieza solo con FB activo y FB GAIN bajo, para oír el "
            "retorno crudo antes de sumar DIODE o TRANS encima - activar "
            "los seis de una vez solo suena a ruido."
        }
    },
    {
        { "MIXER", "MIXER", "MIXER", "MEZCLADOR" },
        {
            "The MIX page has 4 channels - FILTRO, RING, RUÍDO, ESPAÇO - "
            "each with gain, pan, RET (return) and the ON/M(ute)/S(olo) "
            "buttons.\n\n"
            "MEMÓRIA MIX keeps up to 4 full snapshots of the mixer: "
            "click CAPTURAR then a slot M1-M4 to save the current state "
            "into it; click a slot without CAPTURAR armed to recall it. "
            "PRINCIPAL and CLONE each have their own separate mixer and "
            "their own 4 memory slots - capturing one never touches the "
            "other's.\n\n"
            "These 4 channels are listening points along a single chain, "
            "not 4 independent sources: RING feeds FILTRO, and FILTRO "
            "feeds ESPAÇO. Turning a channel's ON/gain off only removes "
            "its own contribution to the mix - it does not stop the "
            "signal from reaching the next channel downstream. RING off "
            "with FILTRO on still lets you hear ring modulation (FILTRO "
            "processes the already-modulated signal); FILTRO off with "
            "ESPAÇO on still lets the filtered signal through (REVERB/"
            "PHASER/FLANGER/RESONATOR read it directly). RUÍDO is the "
            "one true on/off switch among the four - its own NOISE SEND "
            "knob is silenced entirely whenever this channel is off.",
            "A página MIX tem 4 canais - FILTRO, RING, RUÍDO, ESPAÇO - cada "
            "um com ganho, pan, RET (retorno) e os botões ON/M(ute)/"
            "S(olo).\n\n"
            "MEMÓRIA MIX guarda até 4 fotografias completas do mixer: "
            "clique CAPTURAR e depois um slot M1-M4 para gravar o estado "
            "atual nele; clique um slot sem CAPTURAR armado para "
            "recuperá-lo. PRINCIPAL e CLONE têm cada um seu próprio "
            "mixer separado e seus próprios 4 slots de memória - "
            "capturar em um nunca mexe no do outro.\n\n"
            "Esses 4 canais são pontos de escuta ao longo de uma única "
            "cadeia, não 4 fontes independentes: RING alimenta FILTRO, e "
            "FILTRO alimenta ESPAÇO. Desligar o ON/ganho de um canal só "
            "remove a contribuição própria dele na mixagem - não impede "
            "o sinal de chegar ao canal seguinte. RING desligado com "
            "FILTRO ligado ainda deixa ouvir a modulação em anel (o "
            "FILTRO processa o sinal já modulado); FILTRO desligado com "
            "ESPAÇO ligado ainda deixa passar o sinal filtrado (REVERB/"
            "PHASER/FLANGER/RESONATOR o leem diretamente). RUÍDO é o "
            "único interruptor de verdade entre os quatro - o próprio "
            "knob NOISE SEND fica totalmente silenciado sempre que este "
            "canal está desligado.",
            "La page MIX a 4 canaux - FILTRO, RING, RUÍDO, ESPAÇO - chacun "
            "avec gain, pan, RET (retour) et les boutons ON/M(ute)/"
            "S(olo).\n\n"
            "MEMÓRIA MIX garde jusqu'à 4 instantanés complets du mixer : "
            "cliquez CAPTURAR puis un emplacement M1-M4 pour y "
            "enregistrer l'état actuel ; cliquez un emplacement sans "
            "CAPTURAR armé pour le rappeler. PRINCIPAL et CLONE ont "
            "chacun leur propre mixer séparé et leurs propres 4 "
            "emplacements de mémoire - capturer sur l'un ne touche "
            "jamais à l'autre.\n\n"
            "Ces 4 canaux sont des points d'écoute le long d'une seule "
            "chaîne, pas 4 sources indépendantes : RING alimente FILTRO, "
            "et FILTRO alimente ESPAÇO. Désactiver le ON/gain d'un canal "
            "ne retire que sa propre contribution au mixage - cela "
            "n'empêche pas le signal d'atteindre le canal suivant. RING "
            "désactivé avec FILTRO actif laisse encore entendre la "
            "modulation en anneau (FILTRO traite le signal déjà modulé) ; "
            "FILTRO désactivé avec ESPAÇO actif laisse encore passer le "
            "signal filtré (REVERB/PHASER/FLANGER/RESONATOR le lisent "
            "directement). RUÍDO est le seul véritable interrupteur "
            "parmi les quatre - son propre bouton NOISE SEND est "
            "totalement coupé dès que ce canal est désactivé.",
            "La página MIX tiene 4 canales - FILTRO, RING, RUÍDO, ESPAÇO - "
            "cada uno con ganancia, pan, RET (retorno) y los botones "
            "ON/M(ute)/S(olo).\n\n"
            "MEMÓRIA MIX guarda hasta 4 fotografías completas del "
            "mezclador: haz clic en CAPTURAR y luego en un slot M1-M4 "
            "para grabar el estado actual en él; haz clic en un slot sin "
            "CAPTURAR armado para recuperarlo. PRINCIPAL y CLONE tienen "
            "cada uno su propio mezclador separado y sus propios 4 slots "
            "de memoria - capturar en uno nunca toca el del otro.\n\n"
            "Estos 4 canales son puntos de escucha a lo largo de una "
            "única cadena, no 4 fuentes independientes: RING alimenta "
            "FILTRO, y FILTRO alimenta ESPAÇO. Apagar el ON/ganancia de "
            "un canal solo quita su propia contribución a la mezcla - no "
            "impide que la señal llegue al siguiente canal. RING apagado "
            "con FILTRO activo todavía deja oír la modulación en anillo "
            "(FILTRO procesa la señal ya modulada); FILTRO apagado con "
            "ESPAÇO activo todavía deja pasar la señal filtrada (REVERB/"
            "PHASER/FLANGER/RESONATOR la leen directamente). RUÍDO es el "
            "único interruptor verdadero entre los cuatro - su propio "
            "knob NOISE SEND queda totalmente silenciado siempre que este "
            "canal está apagado."
        },
        {
            "Turn RING off and leave FILTRO on to hear the chain, not "
            "the switches: you will still hear ring modulation, because "
            "FILTRO is downstream of RING and only removes its OWN "
            "contribution, not what already reached it - the same test "
            "with FILTRO off and ESPAÇO on shows the same thing one "
            "step further down.",
            "Desligue RING e deixe FILTRO ligado pra ouvir a cadeia, não "
            "os interruptores: você ainda vai ouvir a modulação em anel, "
            "porque FILTRO fica depois de RING na cadeia e só remove a "
            "contribuição PRÓPRIA dele, não o que já chegou até ali - o "
            "mesmo teste com FILTRO desligado e ESPAÇO ligado mostra a "
            "mesma coisa um passo adiante.",
            "Désactivez RING et laissez FILTRO actif pour entendre la "
            "chaîne, pas les interrupteurs : vous entendrez encore la "
            "modulation en anneau, parce que FILTRO est en aval de RING "
            "et ne retire que sa PROPRE contribution, pas ce qui l'a "
            "déjà atteint - le même test avec FILTRO désactivé et "
            "ESPAÇO actif montre la même chose un cran plus loin.",
            "Apaga RING y deja FILTRO encendido para oír la cadena, no "
            "los interruptores: seguirás oyendo la modulación en "
            "anillo, porque FILTRO está después de RING en la cadena y "
            "solo quita su PROPIA contribución, no lo que ya llegó "
            "hasta ahí - la misma prueba con FILTRO apagado y ESPAÇO "
            "encendido muestra lo mismo un paso más adelante."
        }
    },
    {
        { "DRIFT", "DERIVA", "DÉRIVE", "DERIVA" },
        {
            "DERIVA captures a phrase memory (steps, effects, routes and "
            "relations between oscillators) and slowly shifts that memory "
            "over time - PROFUNDIDADE sets how strong that shift is. It is "
            "not audio randomness: it is deterministic and can be "
            "re-listened to. Green means armed and ready; red means it is "
            "actively drifting the sound right now.\n\n"
            "The L/R trace at the centre of the header is the real output, "
            "not an animation. The vertical slider beside it (GANHO Y) "
            "scales the wave's drawing up or down to fit the box comfortably "
            "- the engine's own automatic leveling already keeps typical "
            "output using a healthy share of the available headroom on its "
            "own, so GANHO Y is mainly for adjusting how large the drawing "
            "itself looks, not for compensating a naturally weak signal.\n\n"
            "The slower, flatter trace beneath it is a ~5 second volume "
            "summary, not the waveform itself - it shows things the fast "
            "trace hides: LFO, per-step dynamics, the return path's slow "
            "breathing. GANHO Y scales this trace too.",
            "DERIVA captura uma memória de frase (passos, efeitos, rotas e "
            "relações entre osciladores) e desloca essa memória lentamente "
            "ao longo do tempo - PROFUNDIDADE controla a intensidade desse "
            "deslocamento. Não é aleatoriedade de áudio: é determinístico e "
            "pode ser reescutado. Verde significa armado e pronto; vermelho "
            "significa que está desviando o som ativamente agora.\n\n"
            "O traçado L/R ao centro do cabeçalho é a saída real, não uma "
            "animação. O slider vertical ao lado (GANHO Y) amplia ou reduz "
            "o desenho da onda para caber bem na caixa - o próprio "
            "nivelamento automático do motor já mantém a saída típica "
            "usando boa parte do headroom disponível sozinho, então o "
            "GANHO Y serve principalmente para ajustar o tamanho do "
            "desenho em si, não para compensar um sinal naturalmente "
            "fraco.\n\n"
            "O traçado mais lento e achatado abaixo dele é um resumo de "
            "volume de ~5 segundos, não a forma de onda em si - mostra "
            "coisas que o traço rápido não deixa ver: LFO, dinâmica por "
            "step, a respiração lenta do retorno. GANHO Y também escala "
            "esse traçado.",
            "DERIVA capture une mémoire de phrase (pas, effets, routes et "
            "relations entre oscillateurs) et déplace lentement cette "
            "mémoire au fil du temps - PROFUNDIDADE règle l'intensité de ce "
            "déplacement. Ce n'est pas de l'aléatoire audio : c'est "
            "déterministe et peut être réécouté. Vert signifie armé et "
            "prêt ; rouge signifie qu'il fait activement dériver le son "
            "en ce moment.\n\n"
            "Le tracé L/R au centre de l'en-tête est la sortie réelle, pas "
            "une animation. Le curseur vertical à côté (GANHO Y) agrandit "
            "ou réduit le dessin de l'onde pour qu'il tienne bien dans la "
            "boîte - le nivellement automatique du moteur maintient déjà "
            "seul la sortie typique en utilisant une bonne part de la marge "
            "disponible, donc GANHO Y sert surtout à ajuster la taille du "
            "dessin lui-même, pas à compenser un signal naturellement "
            "faible.\n\n"
            "Le tracé plus lent et aplati en dessous est un résumé du "
            "volume sur ~5 secondes, pas la forme d'onde elle-même - il "
            "montre ce que le tracé rapide cache : LFO, dynamique par pas, "
            "la respiration lente du retour. GANHO Y met aussi cette "
            "courbe à l'échelle.",
            "DERIVA captura una memoria de frase (pasos, efectos, rutas y "
            "relaciones entre osciladores) y desplaza esa memoria "
            "lentamente con el tiempo - PROFUNDIDADE controla la "
            "intensidad de ese desplazamiento. No es aleatoriedad de "
            "audio: es determinístico y se puede volver a escuchar. Verde "
            "significa armado y listo; rojo significa que está desviando "
            "el sonido activamente ahora mismo.\n\n"
            "El trazo L/R en el centro del encabezado es la salida real, no "
            "una animación. El control vertical al lado (GANHO Y) amplía o "
            "reduce el dibujo de la onda para que quepa bien en la caja - "
            "el propio nivelado automático del motor ya mantiene la salida "
            "típica usando buena parte del margen disponible por sí solo, "
            "así que GANHO Y sirve principalmente para ajustar el tamaño "
            "del dibujo en sí, no para compensar una señal naturalmente "
            "débil.\n\n"
            "El trazo más lento y plano debajo de él es un resumen de "
            "volumen de ~5 segundos, no la forma de onda en sí - muestra "
            "cosas que el trazo rápido no deja ver: LFO, dinámica por "
            "step, la respiración lenta del retorno. GANHO Y también "
            "escala este trazo."
        },
        {
            "Capture a phrase with DERIVA on a sound you already know "
            "well, then just watch the L/R trace and listen while it "
            "drifts - the slow ~5s summary trace makes the shift easier "
            "to follow than the fast one, since it filters out the "
            "moment-to-moment waveform and shows only the larger, "
            "actually-drifting shape.",
            "Capture uma frase com DERIVA num som que você já conhece "
            "bem, depois só observe o traçado L/R e escute enquanto ele "
            "deriva - o traçado lento de resumo (~5s) facilita mais "
            "acompanhar o deslocamento que o rápido, porque filtra o "
            "vaivém instantâneo da onda e mostra só a forma maior que "
            "está de fato derivando.",
            "Capturez une phrase avec DERIVA sur un son que vous "
            "connaissez déjà bien, puis observez juste le tracé L/R et "
            "écoutez pendant qu'il dérive - le tracé lent de résumé "
            "(~5s) facilite le suivi du déplacement par rapport au "
            "rapide, car il filtre le va-et-vient instantané de l'onde "
            "et ne montre que la forme plus large qui dérive vraiment.",
            "Captura una frase con DERIVA en un sonido que ya conoces "
            "bien, luego solo observa el trazo L/R y escucha mientras "
            "deriva - el trazo lento de resumen (~5s) facilita seguir "
            "el desplazamiento más que el rápido, porque filtra el "
            "vaivén instantáneo de la onda y muestra solo la forma más "
            "grande que realmente está derivando."
        }
    },
    {
        { "RECORDING", "GRAVAÇÃO", "ENREGISTREMENT", "GRABACIÓN" },
        {
            "Pick a duration (1, 2, 3 or 5 min) - REC fires on its own as soon "
            "as you click a duration, with a live countdown. REC also works "
            "as a manual button (starts/stops the same recording); its "
            "colour only turns red while actually recording. The file is "
            "24-bit stereo WAV.\n\n"
            "PULSO, POROSA and HETERÓDINA each apply a coherent set of "
            "parameters at once (dry gates/pulses; porous memory with S&H "
            "and capacitive returns; beating and ring modulation).\n\n"
            "RND 16 perturbs CV/AMP/FX across the 16 steps within safe "
            "bounds, without touching the rest of the patch.\n\n"
            "ÓRBITA is the first of these to actually turn on AXIS Y/Z on "
            "all 5 oscillators - slow, spacious, breathing, the opposite of "
            "PULSO. PÊNDULO puts the comb/resonator (RES MIX/ALTURA/CORPO) "
            "in the centre instead of the background, with PERCURSO forced "
            "to pendulum - a struck, ringing body, alternating step by "
            "step instead of sustaining.",
            "Escolha uma duração (1, 2, 3 ou 5 min) - o REC dispara sozinho ao "
            "clicar numa duração, com contagem regressiva ao vivo. REC "
            "também funciona como botão manual (liga/desliga a mesma "
            "gravação); a cor só fica vermelha enquanto grava de verdade. O "
            "arquivo sai em WAV 24-bit estéreo.\n\n"
            "PULSO, POROSA e HETERÓDINA aplicam de uma vez um conjunto "
            "coerente de parâmetros (gates/pulsos secos; memória porosa "
            "com S&H e retornos capacitivos; batimentos e ring "
            "modulation).\n\n"
            "RND 16 perturba CV/AMP/FX dos 16 steps dentro de limites "
            "seguros, sem tocar no resto do patch.\n\n"
            "ÓRBITA é a primeira delas a de fato ligar EIXO Y/Z nos 5 "
            "osciladores - lenta, espacial, respirante, o oposto do PULSO. "
            "PÊNDULO coloca o comb/resonador (RES MIX/ALTURA/CORPO) no "
            "centro em vez de acessório, com PERCURSO forçado em pêndulo - "
            "um corpo golpeado e ressoante, alternando passo a passo em "
            "vez de sustentar.",
            "Choisissez une durée (1, 2, 3 ou 5 min) - REC se déclenche seul "
            "dès que vous cliquez une durée, avec un compte à rebours en "
            "direct. REC fonctionne aussi comme bouton manuel (démarre/"
            "arrête le même enregistrement) ; sa couleur ne devient rouge "
            "que pendant l'enregistrement réel. Le fichier sort en WAV "
            "stéréo 24 bits.\n\n"
            "PULSO, POROSA et HETERÓDINA appliquent chacun d'un coup un "
            "ensemble cohérent de paramètres (gates/impulsions sèches ; "
            "mémoire poreuse avec S&H et retours capacitifs ; battements "
            "et ring modulation).\n\n"
            "RND 16 perturbe CV/AMP/FX des 16 pas dans des limites "
            "sûres, sans toucher au reste du patch.\n\n"
            "ÓRBITA est la première à vraiment activer AXE Y/Z sur les 5 "
            "oscillateurs - lente, spatiale, respirante, l'opposé de "
            "PULSO. PÊNDULO met le comb/résonateur (RES MIX/ALTURA/CORPO) "
            "au centre plutôt qu'en accessoire, avec PERCURSO forcé en "
            "pendule - un corps frappé et résonnant, alternant pas à pas "
            "plutôt que de soutenir.",
            "Elige una duración (1, 2, 3 o 5 min) - REC se dispara solo al "
            "hacer clic en una duración, con cuenta regresiva en vivo. REC "
            "también funciona como botón manual (inicia/detiene la misma "
            "grabación); su color solo se pone rojo mientras graba de "
            "verdad. El archivo sale en WAV estéreo de 24 bits.\n\n"
            "PULSO, POROSA y HETERÓDINA aplican de una vez un conjunto "
            "coherente de parámetros (gates/pulsos secos; memoria porosa "
            "con S&H y retornos capacitivos; batidos y ring "
            "modulation).\n\n"
            "RND 16 perturba CV/AMP/FX de los 16 pasos dentro de límites "
            "seguros, sin tocar el resto del patch.\n\n"
            "ÓRBITA es la primera en realmente activar EJE Y/Z en los 5 "
            "osciladores - lenta, espacial, respirante, lo opuesto de "
            "PULSO. PÊNDULO pone el comb/resonador (RES MIX/ALTURA/CORPO) "
            "en el centro en vez de accesorio, con PERCURSO forzado en "
            "péndulo - un cuerpo golpeado y resonante, alternando paso a "
            "paso en vez de sostener."
        },
        {
            "Try RND 16 a few times in a row on the same patch before "
            "reaching for a whole variation preset (PULSO/POROSA/"
            "HETERÓDINA/ÓRBITA/PÊNDULO) - it perturbs only CV/AMP/FX "
            "within safe bounds, so you can hear how far a small nudge "
            "already takes the sound before committing to a bigger, "
            "coherent change.",
            "Tente RND 16 algumas vezes seguidas no mesmo patch antes de "
            "recorrer a um preset de variação inteiro (PULSO/POROSA/"
            "HETERÓDINA/ÓRBITA/PÊNDULO) - ele perturba só CV/AMP/FX "
            "dentro de limites seguros, então dá pra ouvir até onde um "
            "empurrão pequeno já leva o som antes de se comprometer com "
            "uma mudança maior e coerente.",
            "Essayez RND 16 plusieurs fois de suite sur le même patch "
            "avant de passer à tout un preset de variation (PULSO/"
            "POROSA/HETERÓDINA/ÓRBITA/PÊNDULO) - il ne perturbe que CV/"
            "AMP/FX dans des limites sûres, ce qui permet d'entendre "
            "jusqu'où une petite poussée mène déjà le son avant de "
            "s'engager dans un changement plus grand et cohérent.",
            "Prueba RND 16 varias veces seguidas en el mismo patch "
            "antes de recurrir a un preset de variación entero (PULSO/"
            "POROSA/HETERÓDINA/ÓRBITA/PÊNDULO) - solo perturba CV/AMP/"
            "FX dentro de límites seguros, así que puedes oír hasta "
            "dónde ya lleva el sonido un pequeño empujón antes de "
            "comprometerte con un cambio más grande y coherente."
        }
    }
}};

// Second level, added 18 ago. 2026 (author: "sim, em todo o instrumento" -
// answering the recommendation to structure BÁSICO/INTERMEDIÁRIO/AVANÇADO
// as three things that already existed separately, given a place each).
// This one ports `docs/TUTORIAIS.md`'s own gesture-and-listen exercises
// (previously PT-only, reachable only as a README link) into the same
// 4-language TutorialChapter shape as BÁSICO - `body` holds the gesture
// steps, `tip` holds the closing reflective question instead of a
// practical hint (same highlighted box, different kind of takeaway - "the
// one thing to hold onto while you listen" fits a question just as well
// as a tip). Exercise 5 of TUTORIAIS.md ("Variações iniciais") isn't
// ported here - it has no gesture/question shape, it's a note about the
// VARIAÇÃO presets, which the RECORDING AND VARIATIONS chapter (BÁSICO)
// already covers.
inline const std::array<TutorialChapter, 8> tutorialChaptersIntermediate {{
    {
        { "FIRST PULSE", "PRIMEIRO PULSO", "PREMIÈRE IMPULSION", "PRIMER PULSO" },
        {
            "PORTAS DE FEEDBACK don't just repeat a sample - each one "
            "reshapes it differently before sending it back, and PULSE "
            "is the starkest of the six, collapsing everything to a "
            "bare square.\n\n"
            "Choose 40106 and keep PULSE open among the PORTAS. Adjust "
            "the first four CVs and set RETORNO's own return routes on "
            "steps 3, 5 and 8. Record a short take, then play it back "
            "and compare it to what you remember hearing live.",
            "As PORTAS DE FEEDBACK não só repetem uma amostra - cada "
            "uma a remodela de um jeito diferente antes de devolvê-la, "
            "e PULSE é a mais crua das seis, reduzindo tudo a um "
            "quadrado puro.\n\n"
            "Escolha 40106 e mantenha PULSE aberto nas PORTAS. Ajuste "
            "as quatro primeiras CVs e o retorno nos passos 3, 5 e 8. "
            "Grave uma tomada curta, depois escute de volta e compare "
            "com o que você lembra de ter ouvido ao vivo.",
            "Les PORTAS DE FEEDBACK ne se contentent pas de répéter un "
            "échantillon - chacune le remodèle différemment avant de "
            "le renvoyer, et PULSE est la plus brute des six, "
            "réduisant tout à un simple carré.\n\n"
            "Choisissez 40106 et gardez PULSE ouvert parmi les PORTAS. "
            "Ajustez les quatre premières CV et le retour aux pas 3, 5 "
            "et 8. Enregistrez une courte prise, puis réécoutez-la et "
            "comparez-la à ce dont vous vous souvenez avoir entendu en "
            "direct.",
            "Las PORTAS DE FEEDBACK no solo repiten una muestra - cada "
            "una la remodela de forma distinta antes de devolverla, y "
            "PULSE es la más cruda de las seis, reduciendo todo a un "
            "cuadrado puro.\n\n"
            "Elige 40106 y mantén PULSE abierto entre las PORTAS. "
            "Ajusta las cuatro primeras CV y el retorno en los pasos "
            "3, 5 y 8. Graba una toma corta, luego escúchala de vuelta "
            "y compárala con lo que recuerdas haber oído en vivo."
        },
        {
            "When does the return stop being repetition and become "
            "form? Listen for the moment a pattern you expected to "
            "just repeat starts sounding like it's building toward "
            "something.",
            "Quando o retorno deixa de ser repetição e vira forma? "
            "Preste atenção no momento em que um padrão que parecia só "
            "repetir começa a soar como se estivesse construindo pra "
            "algum lugar.",
            "Quand le retour cesse-t-il d'être une répétition pour "
            "devenir une forme ? Guettez le moment où un motif censé "
            "simplement se répéter commence à sonner comme s'il "
            "construisait vers quelque chose.",
            "¿Cuándo el retorno deja de ser repetición y se vuelve "
            "forma? Presta atención al momento en que un patrón que "
            "parecía solo repetirse empieza a sonar como si estuviera "
            "construyendo hacia algo."
        }
    },
    {
        { "STEP THAT HOLDS", "DEGRAU QUE RETÉM", "PALIER QUI RETIENT", "ESCALÓN QUE RETIENE" },
        {
            "S&H turns a continuously wandering NOISE signal into a "
            "staircase of held values - each one only changes when "
            "the sample clock ticks, not continuously.\n\n"
            "Open NOISE at low volume - S&H (Sample & Hold) turns it "
            "into held values. Open CAP and REFLUX; keep FX high on "
            "only two steps. Slow CLOCK down and let it cross both "
            "eight-step banks, listening for where the held values "
            "line up with step boundaries and where they drift across "
            "them.",
            "O S&H transforma um sinal de NOISE em deriva contínua "
            "numa escada de valores mantidos - cada um só muda quando "
            "o clock de amostragem bate, não continuamente.\n\n"
            "Abra NOISE em volume baixo - o S&H (Sample & Hold) o "
            "converte em valores mantidos. Abra CAP e REFLUX; deixe FX "
            "alto só em duas etapas. Diminua CLOCK e atravesse as duas "
            "bancas de oito passos, prestando atenção em onde os "
            "valores mantidos coincidem com as bordas dos passos e "
            "onde eles derivam por cima.",
            "S&H transforme un signal NOISE en dérive continue en un "
            "escalier de valeurs maintenues - chacune ne change que "
            "lorsque l'horloge d'échantillonnage bat, pas en "
            "continu.\n\n"
            "Ouvrez NOISE à faible volume - le S&H (Sample & Hold) le "
            "transforme en valeurs maintenues. Ouvrez CAP et REFLUX ; "
            "laissez FX haut sur seulement deux pas. Ralentissez CLOCK "
            "et traversez les deux banques de huit pas, en remarquant "
            "où les valeurs maintenues coïncident avec les limites des "
            "pas et où elles dérivent par-dessus.",
            "El S&H transforma una señal de NOISE que deriva "
            "continuamente en una escalera de valores mantenidos - "
            "cada uno solo cambia cuando el reloj de muestreo late, no "
            "de forma continua.\n\n"
            "Abre NOISE en volumen bajo - el S&H (Sample & Hold) lo "
            "convierte en valores mantenidos. Abre CAP y REFLUX; deja "
            "FX alto solo en dos etapas. Baja CLOCK y atraviesa los "
            "dos bancos de ocho pasos, fijándote en dónde los valores "
            "mantenidos coinciden con los bordes de los pasos y dónde "
            "derivan por encima."
        },
        {
            "Does the memory hold the sound, or change what returns? "
            "Pay attention to whether the same held value ever comes "
            "back exactly, or whether CAP and REFLUX keep nudging it "
            "somewhere new.",
            "A memória segura o som ou altera aquilo que retorna? "
            "Repare se o mesmo valor mantido volta exatamente igual "
            "alguma vez, ou se CAP e REFLUX ficam empurrando ele pra "
            "outro lugar.",
            "La mémoire retient-elle le son, ou change-t-elle ce qui "
            "revient ? Remarquez si la même valeur maintenue revient "
            "un jour exactement identique, ou si CAP et REFLUX "
            "continuent de la pousser ailleurs.",
            "¿La memoria sostiene el sonido o altera lo que regresa? "
            "Fíjate si el mismo valor mantenido regresa alguna vez "
            "exactamente igual, o si CAP y REFLUX lo siguen empujando "
            "a otro lugar."
        }
    },
    {
        { "SPACE PER STEP", "ESPAÇO POR ETAPA", "ESPACE PAR ÉTAPE", "ESPACIO POR ETAPA" },
        {
            "REVERB/PHASER/FLANGER are gated per step by FX, not "
            "applied globally - so an effect can live entirely in the "
            "quiet spaces between loud steps rather than on the steps "
            "themselves.\n\n"
            "Make one step loud via AMP and others nearly silent. Keep "
            "FX high on the silent steps. Open REVERB, PHASER and "
            "FLANGER gradually, listening for whether the effect "
            "colours the loud step or blooms only in the silence "
            "around it.",
            "REVERB/PHASER/FLANGER são controlados por passo via FX, "
            "não aplicados globalmente - então um efeito pode viver "
            "inteiro nos espaços quietos entre passos altos, em vez de "
            "nos próprios passos.\n\n"
            "Faça uma etapa forte por AMP e outras quase silenciosas. "
            "Deixe FX alto nos passos silenciosos. Abra REVERB, PHASER "
            "e FLANGER gradualmente, prestando atenção se o efeito "
            "colore o passo alto ou floresce só no silêncio ao redor.",
            "REVERB/PHASER/FLANGER sont contrôlés par pas via FX, pas "
            "appliqués globalement - un effet peut donc vivre "
            "entièrement dans les espaces silencieux entre les pas "
            "forts plutôt que sur les pas eux-mêmes.\n\n"
            "Rendez un pas fort via AMP et d'autres presque silencieux. "
            "Gardez FX haut sur les pas silencieux. Ouvrez REVERB, "
            "PHASER et FLANGER progressivement, en écoutant si l'effet "
            "colore le pas fort ou s'épanouit seulement dans le "
            "silence autour.",
            "REVERB/PHASER/FLANGER se controlan por paso mediante FX, "
            "no se aplican globalmente - así que un efecto puede vivir "
            "entero en los espacios silenciosos entre pasos fuertes en "
            "vez de en los propios pasos.\n\n"
            "Haz una etapa fuerte por AMP y otras casi silenciosas. "
            "Deja FX alto en los pasos silenciosos. Abre REVERB, "
            "PHASER y FLANGER gradualmente, fijándote si el efecto "
            "colorea el paso fuerte o florece solo en el silencio "
            "alrededor."
        },
        {
            "Does the effect belong to the voice, or to the gaps "
            "between voices? Try muting the loud step entirely and see "
            "if the effect alone still reads as music.",
            "O efeito pertence à voz ou às lacunas entre vozes? Tente "
            "silenciar o passo alto por completo e veja se o efeito "
            "sozinho ainda soa como música.",
            "L'effet appartient-il à la voix, ou aux vides entre les "
            "voix ? Essayez de couper complètement le pas fort et "
            "voyez si l'effet seul sonne encore comme de la musique.",
            "¿El efecto pertenece a la voz o a los huecos entre voces? "
            "Intenta silenciar el paso fuerte por completo y mira si "
            "el efecto solo aún suena como música."
        }
    },
    {
        { "INFILTRATION PORTS", "PORTAS DE INFILTRAÇÃO", "PORTES D'INFILTRATION", "PUERTAS DE INFILTRACIÓN" },
        {
            "All six PORTAS DE FEEDBACK can run at once, but they "
            "average together rather than stack in volume - so opening "
            "one at a time is the only way to hear each one's own "
            "character before they blend.\n\n"
            "Open FB, DIODE, CAP, TRANS, PULSE and REFLUX, one at a "
            "time. Wait a few cycles between each gesture, letting the "
            "previous one fully settle before the next one joins.",
            "As seis PORTAS DE FEEDBACK podem rodar juntas, mas elas "
            "se combinam por MÉDIA, não se empilham em volume - então "
            "abrir uma de cada vez é o único jeito de ouvir o caráter "
            "próprio de cada uma antes de se misturarem.\n\n"
            "Abra FB, DIODE, CAP, TRANS, PULSE e REFLUX, uma de cada "
            "vez. Espere alguns ciclos entre cada gesto, deixando a "
            "anterior se assentar por completo antes da próxima "
            "entrar.",
            "Les six PORTAS DE FEEDBACK peuvent tourner ensemble, mais "
            "elles se combinent par MOYENNE, pas en s'additionnant en "
            "volume - donc les ouvrir une à la fois est le seul moyen "
            "d'entendre le caractère propre de chacune avant qu'elles "
            "se mélangent.\n\n"
            "Ouvrez FB, DIODE, CAP, TRANS, PULSE et REFLUX, un à la "
            "fois. Attendez quelques cycles entre chaque geste, en "
            "laissant le précédent bien s'installer avant que le "
            "suivant n'entre.",
            "Las seis PORTAS DE FEEDBACK pueden correr juntas, pero se "
            "combinan por PROMEDIO, no se apilan en volumen - así que "
            "abrirlas una a la vez es la única forma de oír el "
            "carácter propio de cada una antes de que se mezclen.\n\n"
            "Abre FB, DIODE, CAP, TRANS, PULSE y REFLUX, uno a la vez. "
            "Espera algunos ciclos entre cada gesto, dejando que el "
            "anterior se asiente por completo antes de que entre el "
            "siguiente."
        },
        {
            "Which of the six changes the timbre the most - and which "
            "one you almost cannot hear alone? Notice whether the "
            "quiet one still changes something even when you can't "
            "quite name what.",
            "Qual das seis muda mais o timbre - e qual você quase não "
            "escuta sozinha? Repare se a mais quieta ainda muda alguma "
            "coisa mesmo quando você não consegue nomear bem o quê.",
            "Laquelle des six change le plus le timbre - et laquelle "
            "vous entendez à peine seule ? Remarquez si la plus "
            "discrète change quand même quelque chose, même quand vous "
            "ne savez pas trop nommer quoi.",
            "¿Cuál de las seis cambia más el timbre - y cuál casi no "
            "escuchas sola? Fíjate si la más silenciosa igual cambia "
            "algo aunque no logres nombrar bien qué."
        }
    },
    {
        { "INITIAL VARIATIONS", "VARIAÇÕES INICIAIS", "VARIATIONS INITIALES", "VARIACIONES INICIALES" },
        {
            "RND 16 perturbs CV/AMP/FX across the 16 steps within safe "
            "bounds - a narrow, controlled kind of chance, not a full "
            "reshuffle of the patch.\n\n"
            "Build a short loop you like. Press RND 16 once and listen "
            "before touching anything else. Press it three more times, "
            "listening after each one, and try to name what stayed the "
            "same each time versus what actually moved.",
            "RND 16 perturba CV/AMP/FX nos 16 passos dentro de limites "
            "seguros - um tipo de acaso estreito e controlado, não um "
            "reembaralhamento completo do patch.\n\n"
            "Construa um loop curto de que goste. Aperte RND 16 uma "
            "vez e escute antes de tocar em mais nada. Aperte mais "
            "três vezes, escutando depois de cada uma, e tente nomear "
            "o que ficou igual cada vez e o que realmente se moveu.",
            "RND 16 perturbe CV/AMP/FX sur les 16 pas dans des limites "
            "sûres - un hasard étroit et contrôlé, pas un rebrassage "
            "complet du patch.\n\n"
            "Construisez une courte boucle que vous aimez. Appuyez sur "
            "RND 16 une fois et écoutez avant de toucher à autre "
            "chose. Appuyez trois fois de plus, en écoutant après "
            "chacune, et essayez de nommer ce qui reste identique à "
            "chaque fois et ce qui bouge vraiment.",
            "RND 16 perturba CV/AMP/FX en los 16 pasos dentro de "
            "límites seguros - un tipo de azar estrecho y controlado, "
            "no una reorganización completa del patch.\n\n"
            "Construye un loop corto que te guste. Presiona RND 16 una "
            "vez y escucha antes de tocar cualquier otra cosa. "
            "Presiona tres veces más, escuchando después de cada una, "
            "e intenta nombrar qué se mantiene igual cada vez y qué "
            "realmente se mueve."
        },
        {
            "Where is the line between the same patch and a new one? "
            "At some point across the four presses, decide for "
            "yourself which one crossed it - and try to say why.",
            "Onde fica a linha entre o mesmo patch e um novo? Em algum "
            "ponto entre os quatro apertos, decida por si mesmo qual "
            "deles cruzou essa linha - e tente dizer por quê.",
            "Où se situe la ligne entre le même patch et un nouveau ? "
            "À un moment donné parmi les quatre pressions, décidez "
            "vous-même laquelle l'a franchie - et essayez de dire "
            "pourquoi.",
            "¿Dónde está la línea entre el mismo patch y uno nuevo? En "
            "algún punto entre las cuatro pulsaciones, decide tú "
            "mismo cuál la cruzó - e intenta decir por qué."
        }
    },
    {
        { "TWO OBJECTS LISTENING", "DOIS OBJETOS SE OUVINDO", "DEUX OBJETS QUI S'ÉCOUTENT", "DOS OBJETOS ESCUCHÁNDOSE" },
        {
            "Muting an object's own output and its influence on its "
            "sibling are two separate controls - one silences what "
            "you hear, the other doesn't touch what it's sending.\n\n"
            "Open CLONE. Mute PRINCIPAL's own output in MIXER OBJETOS. "
            "Raise AUX→CLONE slowly while CLONE plays alone, and "
            "listen for PRINCIPAL's own character bleeding into "
            "CLONE's sound even though PRINCIPAL itself is silent.",
            "Silenciar a saída própria de um objeto e sua influência "
            "sobre o irmão são dois controles separados - um silencia "
            "o que você ouve, o outro não toca no que está sendo "
            "enviado.\n\n"
            "Abra CLONE. Silencie a saída própria de PRINCIPAL em "
            "MIXER OBJETOS. Suba AUX→CLONE devagar enquanto CLONE toca "
            "sozinho, e escute o caráter próprio de PRINCIPAL vazando "
            "no som de CLONE mesmo com PRINCIPAL em silêncio.",
            "Couper la sortie propre d'un objet et son influence sur "
            "son frère sont deux contrôles séparés - l'un coupe ce que "
            "vous entendez, l'autre ne touche pas à ce qui est "
            "envoyé.\n\n"
            "Ouvrez CLONE. Coupez la sortie propre de PRINCIPAL dans "
            "MIXER OBJETOS. Montez AUX→CLONE lentement pendant que "
            "CLONE joue seul, et écoutez le caractère propre de "
            "PRINCIPAL s'infiltrer dans le son de CLONE alors même que "
            "PRINCIPAL est silencieux.",
            "Silenciar la salida propia de un objeto y su influencia "
            "sobre su hermano son dos controles separados - uno "
            "silencia lo que oyes, el otro no toca lo que se está "
            "enviando.\n\n"
            "Abre CLONE. Silencia la salida propia de PRINCIPAL en "
            "MIXER OBJETOS. Sube AUX→CLONE despacio mientras CLONE "
            "toca solo, y escucha el carácter propio de PRINCIPAL "
            "filtrándose en el sonido de CLONE aunque PRINCIPAL esté "
            "en silencio."
        },
        {
            "Can you hear PRINCIPAL if you cannot hear PRINCIPAL? Try "
            "answering before reading the AVANÇADO chapter that "
            "explains exactly why.",
            "Dá pra ouvir PRINCIPAL se você não consegue ouvir "
            "PRINCIPAL? Tente responder antes de ler o capítulo do "
            "AVANÇADO que explica exatamente por quê.",
            "Peut-on entendre PRINCIPAL si on ne peut pas entendre "
            "PRINCIPAL ? Essayez de répondre avant de lire le chapitre "
            "d'AVANÇADO qui explique exactement pourquoi.",
            "¿Puedes oír a PRINCIPAL si no puedes oír a PRINCIPAL? "
            "Intenta responder antes de leer el capítulo de AVANÇADO "
            "que explica exactamente por qué."
        }
    },
    {
        { "MEMORY COMPARE", "COMPARAR MEMÓRIA", "COMPARER LA MÉMOIRE", "COMPARAR MEMORIA" },
        {
            "MEMÓRIA MIX doesn't just recall a single gain value - it "
            "swaps the whole mixer state at once: every channel's "
            "gain, pan and RET, in one click.\n\n"
            "Shape a mix you like on FILTRO/RING/RUÍDO/ESPAÇO. Press "
            "CAPTURAR then M1. Change the mix completely, capture it "
            "into M2. Alternate M1/M2 while the loop keeps playing, "
            "paying attention to whether the jump feels instant or "
            "like the mix physically moves there.",
            "MEMÓRIA MIX não recupera só um valor de ganho - ela troca "
            "o estado inteiro do mixer de uma vez: o ganho, pan e RET "
            "de cada canal, num clique só.\n\n"
            "Molde uma mixagem de que goste em FILTRO/RING/RUÍDO/"
            "ESPAÇO. Aperte CAPTURAR e depois M1. Mude a mixagem por "
            "completo, capture em M2. Alterne M1/M2 com o loop tocando, "
            "prestando atenção se o salto parece instantâneo ou como "
            "se a mixagem se movesse fisicamente até lá.",
            "MEMÓRIA MIX ne rappelle pas juste une valeur de gain - "
            "elle échange tout l'état du mixer d'un coup : le gain, le "
            "pan et le RET de chaque canal, en un clic.\n\n"
            "Façonnez un mixage que vous aimez sur FILTRO/RING/RUÍDO/"
            "ESPAÇO. Appuyez sur CAPTURAR puis M1. Changez le mixage "
            "complètement, capturez-le dans M2. Alternez M1/M2 pendant "
            "que la boucle continue de jouer, en remarquant si le saut "
            "semble instantané ou comme si le mixage s'y déplaçait "
            "physiquement.",
            "MEMÓRIA MIX no recupera solo un valor de ganancia - "
            "intercambia todo el estado del mezclador de una vez: la "
            "ganancia, pan y RET de cada canal, en un clic.\n\n"
            "Da forma a una mezcla que te guste en FILTRO/RING/RUÍDO/"
            "ESPAÇO. Presiona CAPTURAR y luego M1. Cambia la mezcla "
            "por completo, captúrala en M2. Alterna M1/M2 mientras el "
            "loop sigue sonando, fijándote si el salto parece "
            "instantáneo o como si la mezcla se moviera físicamente "
            "hasta allí."
        },
        {
            "Does the jump between them sound like two mixes, or one "
            "mix with two moods? Try alternating faster and faster "
            "until the two start to feel like a single, unstable "
            "third thing.",
            "O salto entre elas soa como duas mixagens, ou uma "
            "mixagem com dois humores? Tente alternar cada vez mais "
            "rápido até as duas começarem a parecer uma terceira "
            "coisa instável só.",
            "Le saut entre elles sonne-t-il comme deux mixages, ou un "
            "seul mixage avec deux humeurs ? Essayez d'alterner de "
            "plus en plus vite jusqu'à ce que les deux commencent à "
            "ressembler à une troisième chose instable, unique.",
            "¿El salto entre ellas suena a dos mezclas, o a una mezcla "
            "con dos estados de ánimo? Intenta alternar cada vez más "
            "rápido hasta que ambas empiecen a sentirse como una "
            "tercera cosa inestable, única."
        }
    },
    {
        { "DRIFTING ROUTE", "ROTA QUE DERIVA", "ROUTE QUI DÉRIVE", "RUTA QUE DERIVA" },
        {
            "DERIVA can drift the CONEXÃO ENTRE OBJETOS routes "
            "themselves, not just CV/AMP/FX values - the topology "
            "between PRINCIPAL and CLONE can shift on its own while "
            "it's armed.\n\n"
            "Open CONEXÃO ENTRE OBJETOS and note where PRINCIPAL→CLONE "
            "sits. Arm DERIVA with PROFUNDIDADE around the middle. "
            "Check the same knob again after a few minutes, without "
            "touching it, and watch whether it's still animating or "
            "has settled somewhere new.",
            "A DERIVA pode derivar as próprias rotas de CONEXÃO ENTRE "
            "OBJETOS, não só valores de CV/AMP/FX - a topologia entre "
            "PRINCIPAL e CLONE pode mudar sozinha enquanto ela está "
            "armada.\n\n"
            "Abra CONEXÃO ENTRE OBJETOS e anote onde PRINCIPAL→CLONE "
            "está. Arme a DERIVA com PROFUNDIDADE na metade. Confira o "
            "mesmo knob de novo depois de alguns minutos, sem tocá-lo, "
            "e veja se ele ainda está se movendo ou se assentou em "
            "outro lugar.",
            "DERIVA peut faire dériver les routes de CONEXÃO ENTRE "
            "OBJETOS elles-mêmes, pas seulement les valeurs CV/AMP/FX "
            "- la topologie entre PRINCIPAL et CLONE peut changer "
            "d'elle-même tant qu'elle est armée.\n\n"
            "Ouvrez CONEXÃO ENTRE OBJETOS et notez où se trouve "
            "PRINCIPAL→CLONE. Armez DERIVA avec PROFUNDIDADE au "
            "milieu. Revérifiez le même bouton après quelques minutes, "
            "sans y toucher, et observez s'il bouge encore ou s'il "
            "s'est stabilisé ailleurs.",
            "DERIVA puede derivar las propias rutas de CONEXÃO ENTRE "
            "OBJETOS, no solo valores de CV/AMP/FX - la topología "
            "entre PRINCIPAL y CLONE puede cambiar sola mientras está "
            "armada.\n\n"
            "Abre CONEXÃO ENTRE OBJETOS y anota dónde está PRINCIPAL→"
            "CLONE. Arma DERIVA con PROFUNDIDADE a la mitad. Revisa el "
            "mismo knob de nuevo tras unos minutos, sin tocarlo, y "
            "observa si sigue moviéndose o se asentó en otro lugar."
        },
        {
            "If nobody moved the knob, what moved it? Try to "
            "describe, in your own words, what kind of 'nobody' is "
            "actually doing the moving.",
            "Se ninguém mexeu no knob, o que o moveu? Tente descrever, "
            "com suas próprias palavras, que tipo de 'ninguém' está de "
            "fato fazendo esse movimento.",
            "Si personne n'a touché le bouton, qu'est-ce qui l'a "
            "déplacé ? Essayez de décrire, avec vos propres mots, quel "
            "genre de « personne » est réellement à l'origine de ce "
            "mouvement.",
            "Si nadie movió el knob, ¿qué lo movió? Intenta describir, "
            "con tus propias palabras, qué clase de 'nadie' está de "
            "hecho haciendo ese movimiento."
        }
    }
}};

// Third level, same date/author decision as tutorialChaptersIntermediate
// above. Where BÁSICO explains one control/section at a time, this one
// explains relationships between sections that already have their own
// controls - the "topologia de roteamento" third of
// `docs/FLUXO_DE_SINAL.md`, in the same accessible tone as every other
// chapter (referencing real panel names, not the diagram's own arrows/
// boxes). Only 2 chapters so far - AVANÇADO's own scope ("todos os
// objetos", per the author) is not finished; see TAREFAS.md.
inline const std::array<TutorialChapter, 8> tutorialChaptersAdvanced {{
    {
        { "AUDIO FLOW", "FLUXO DE ÁUDIO", "FLUX AUDIO", "FLUJO DE AUDIO" },
        {
            "Fixed order every step: the 5 oscillators sum first, "
            "then ADSR scales that sum at once (the instrument's own "
            "VCA).\n\n"
            "NOISE is injected next, then RING (using the LFO as "
            "carrier), then VCF/MAT, then PHASER/FLANGER/REVERB/"
            "RESONATOR inside ESPAÇO.\n\n"
            "Each of those four effects inside ESPAÇO is gated by the "
            "current step's own FX value, not by a fixed on/off - a "
            "step with FX near 0 passes through almost untouched even "
            "while every other step is drenched in it, so FX behaves "
            "like a per-step door rather than a global switch.\n\n"
            "Only at the end does each MIXER channel's gain/pan/RET "
            "set the final balance - the MIXER's channels are "
            "listening points along this chain, not separate "
            "sources.\n\n"
            "That's why muting RING can still leave a ring-modulated "
            "character audible through FILTRO, and the same logic "
            "goes one step further: turning FILTRO off with ESPAÇO on "
            "still lets the already-filtered signal through, because "
            "REVERB/PHASER/FLANGER/RESONATOR read directly from what "
            "reached them, not from what the MIXER decided to keep.",
            "Ordem fixa em todo step: os 5 osciladores somam primeiro, "
            "depois o ADSR escala essa soma de uma vez (o próprio VCA "
            "do instrumento).\n\n"
            "O NOISE é injetado em seguida, depois o RING (usando o "
            "LFO como portadora), depois VCF/MAT, depois PHASER/"
            "FLANGER/REVERB/RESONATOR dentro de ESPAÇO.\n\n"
            "Cada um desses quatro efeitos dentro de ESPAÇO é "
            "controlado pelo valor de FX do step atual, não por um "
            "liga/desliga fixo - um step com FX perto de 0 passa quase "
            "intocado mesmo com todos os outros passos encharcados "
            "nele, então FX funciona mais como uma porta por passo do "
            "que um interruptor geral.\n\n"
            "Só no fim o ganho/pan/RET de cada canal do MIXER decide o "
            "equilíbrio final - os canais do MIXER são pontos de "
            "escuta ao longo da cadeia, não fontes separadas.\n\n"
            "Por isso silenciar RING ainda pode deixar passar caráter "
            "modulado em anel pelo FILTRO, e a mesma lógica vai um "
            "passo além: desligar FILTRO com ESPAÇO ligado ainda deixa "
            "passar o sinal já filtrado, porque REVERB/PHASER/FLANGER/"
            "RESONATOR leem direto o que chegou até eles, não o que o "
            "MIXER decidiu manter.",
            "Ordre fixe à chaque pas : les 5 oscillateurs s'additionnent "
            "d'abord, puis ADSR met à l'échelle cette somme d'un coup "
            "(le VCA propre de l'instrument).\n\n"
            "NOISE est injecté ensuite, puis RING (LFO comme "
            "porteuse), puis VCF/MAT, puis PHASER/FLANGER/REVERB/"
            "RESONATOR dans ESPAÇO.\n\n"
            "Chacun de ces quatre effets dans ESPAÇO est contrôlé par "
            "la valeur FX propre au pas actuel, pas par un interrupteur "
            "fixe - un pas avec un FX proche de 0 passe presque intact "
            "même si tous les autres pas en sont trempés, donc FX agit "
            "plus comme une porte par pas qu'un interrupteur global.\n\n"
            "Ce n'est qu'à la fin que le gain/pan/RET de chaque canal "
            "du MIXER fixe l'équilibre final - les canaux du MIXER "
            "sont des points d'écoute le long de la chaîne, pas des "
            "sources séparées.\n\n"
            "D'où le fait que couper RING peut encore laisser un "
            "caractère modulé en anneau via FILTRO, et la même logique "
            "va un cran plus loin : désactiver FILTRO avec ESPAÇO actif "
            "laisse encore passer le signal déjà filtré, car REVERB/"
            "PHASER/FLANGER/RESONATOR lisent directement ce qui les a "
            "atteints, pas ce que le MIXER a décidé de garder.",
            "Orden fijo en cada paso: los 5 osciladores se suman "
            "primero, luego ADSR escala esa suma de una vez (el VCA "
            "propio del instrumento).\n\n"
            "NOISE se inyecta después, luego RING (con el LFO como "
            "portadora), luego VCF/MAT, luego PHASER/FLANGER/REVERB/"
            "RESONATOR dentro de ESPAÇO.\n\n"
            "Cada uno de esos cuatro efectos dentro de ESPAÇO está "
            "controlado por el valor de FX del paso actual, no por un "
            "encendido/apagado fijo - un paso con FX cerca de 0 pasa "
            "casi intacto aunque todos los demás pasos estén "
            "empapados en él, así que FX funciona más como una puerta "
            "por paso que un interruptor general.\n\n"
            "Solo al final el ganancia/pan/RET de cada canal del MIXER "
            "decide el equilibrio final - los canales del MIXER son "
            "puntos de escucha a lo largo de la cadena, no fuentes "
            "separadas.\n\n"
            "Por eso silenciar RING aún puede dejar pasar carácter "
            "modulado en anillo por FILTRO, y la misma lógica va un "
            "paso más allá: apagar FILTRO con ESPAÇO encendido "
            "todavía deja pasar la señal ya filtrada, porque REVERB/"
            "PHASER/FLANGER/RESONATOR leen directamente lo que llegó "
            "hasta ellos, no lo que el MIXER decidió mantener."
        },
        {
            "Mute every MIXER channel except FILTRO - you can still "
            "hear character from RING or NOISE if they were active, "
            "because mute only removes each channel's OWN direct "
            "contribution, not what already passed through it "
            "downstream.",
            "Silencie todos os canais do MIXER exceto FILTRO - ainda "
            "dá pra ouvir caráter de RING ou NOISE se estavam ativos, "
            "porque o mute só remove a contribuição PRÓPRIA de cada "
            "canal, não o que já passou por ele adiante.",
            "Coupez tous les canaux du MIXER sauf FILTRO - vous "
            "entendrez encore le caractère de RING ou NOISE s'ils "
            "étaient actifs, car le mute ne retire que la contribution "
            "PROPRE de chaque canal, pas ce qui l'a déjà traversé en "
            "aval.",
            "Silencia todos los canales del MIXER excepto FILTRO - "
            "aún puedes oír carácter de RING o NOISE si estaban "
            "activos, porque el mute solo quita la contribución "
            "PROPIA de cada canal, no lo que ya pasó por él más "
            "adelante."
        }
    },
    {
        { "OBJECT ROUTING", "ROTAS ENTRE OBJETOS", "ROUTES ENTRE OBJETS", "RUTAS ENTRE OBJETOS" },
        {
            "PRINCIPAL and CLONE aren't parallel copies - each one's "
            "output feeds the other's feedback input, one sample "
            "later, through CONEXÃO ENTRE OBJETOS (DIRETO/DIODO/CAP/"
            "PULSO, same vocabulary as PORTAS DE FEEDBACK, now running "
            "between objects).\n\n"
            "The same shaping logic applies here as inside a single "
            "object's own feedback: DIRETO passes the raw sample, "
            "DIODO rectifies it, CAP uses the slow capacitor average, "
            "PULSO collapses it to a bare square. Nothing stops an "
            "object's own internal PORTAS DE FEEDBACK and its CONEXÃO "
            "ENTRE OBJETOS route from running at the same time - one "
            "shapes the voice from within, the other reintroduces its "
            "sibling's voice from outside, and both compound in the "
            "same signal.\n\n"
            "AUX→PRINCIPAL and AUX→CLONE add a quieter path on top: "
            "two hidden oscillators each drifting with the other "
            "object's loudness, sent in instead of the direct "
            "signal.\n\n"
            "Muting an object's own output (MIXER OBJETOS) does NOT "
            "silence its influence through these routes - they're "
            "separate controls, so an object can keep shaping its "
            "sibling even while you can't hear it directly.\n\n"
            "DERIVA, when armed, can drift the routes themselves too, "
            "not just CV/AMP/FX - the topology itself can change on "
            "its own.",
            "PRINCIPAL e CLONE não são cópias paralelas - a saída de "
            "cada um alimenta a entrada de retorno do outro, com um "
            "sample de atraso, pela CONEXÃO ENTRE OBJETOS (DIRETO/"
            "DIODO/CAP/PULSO, mesmo vocabulário das PORTAS DE "
            "FEEDBACK, agora entre objetos).\n\n"
            "A mesma lógica de moldagem se aplica aqui como dentro do "
            "retorno interno de um só objeto: DIRETO passa a amostra "
            "crua, DIODO a retifica, CAP usa a média lenta do "
            "capacitor, PULSO a reduz a um quadrado puro. Nada impede "
            "que as PORTAS DE FEEDBACK internas de um objeto e sua "
            "rota de CONEXÃO ENTRE OBJETOS rodem ao mesmo tempo - uma "
            "molda a voz por dentro, a outra reintroduz a voz do "
            "irmão por fora, e as duas se somam no mesmo sinal.\n\n"
            "AUX→PRINCIPAL e AUX→CLONE somam um caminho mais discreto "
            "por cima: dois osciladores escondidos, cada um derivando "
            "com o volume do outro objeto, enviados em vez do sinal "
            "direto.\n\n"
            "Silenciar a saída de um objeto (MIXER OBJETOS) NÃO "
            "silencia sua influência por essas rotas - são controles "
            "separados, então um objeto pode continuar moldando o "
            "irmão mesmo enquanto você não o escuta diretamente.\n\n"
            "A DERIVA, quando armada, também pode derivar as próprias "
            "rotas, não só CV/AMP/FX - a própria topologia pode mudar "
            "sozinha.",
            "PRINCIPAL et CLONE ne sont pas des copies parallèles - la "
            "sortie de chacun alimente l'entrée de retour de l'autre, "
            "avec un sample de retard, via CONEXÃO ENTRE OBJETOS "
            "(DIRETO/DIODO/CAP/PULSO, même vocabulaire que PORTAS DE "
            "FEEDBACK, désormais entre objets).\n\n"
            "La même logique de façonnage s'applique ici que dans le "
            "retour interne propre d'un seul objet : DIRETO transmet "
            "l'échantillon brut, DIODO le redresse, CAP utilise la "
            "moyenne lente du capacitor, PULSO le réduit à un simple "
            "carré. Rien n'empêche les PORTAS DE FEEDBACK internes "
            "d'un objet et sa route de CONEXÃO ENTRE OBJETOS de "
            "fonctionner en même temps - l'une façonne la voix de "
            "l'intérieur, l'autre réintroduit la voix de son frère de "
            "l'extérieur, et les deux s'additionnent dans le même "
            "signal.\n\n"
            "AUX→PRINCIPAL et AUX→CLONE ajoutent un chemin plus "
            "discret par-dessus : deux oscillateurs cachés dérivant "
            "chacun avec le volume de l'autre objet, envoyés à la "
            "place du signal direct.\n\n"
            "Couper la sortie propre d'un objet (MIXER OBJETOS) ne "
            "coupe PAS son influence via ces routes - ce sont des "
            "contrôles séparés, donc un objet peut continuer à "
            "façonner son frère même quand vous ne l'entendez pas "
            "directement.\n\n"
            "DERIVA, une fois armée, peut aussi faire dériver les "
            "routes elles-mêmes, pas seulement CV/AMP/FX - la "
            "topologie elle-même peut changer d'elle-même.",
            "PRINCIPAL y CLONE no son copias paralelas - la salida de "
            "cada uno alimenta la entrada de retorno del otro, con un "
            "sample de retraso, por CONEXÃO ENTRE OBJETOS (DIRETO/"
            "DIODO/CAP/PULSO, mismo vocabulario que PORTAS DE "
            "FEEDBACK, ahora entre objetos).\n\n"
            "La misma lógica de moldeado se aplica aquí que en el "
            "retorno interno propio de un solo objeto: DIRETO pasa la "
            "muestra cruda, DIODO la rectifica, CAP usa el promedio "
            "lento del capacitor, PULSO la reduce a un cuadrado puro. "
            "Nada impide que las PORTAS DE FEEDBACK internas de un "
            "objeto y su ruta de CONEXÃO ENTRE OBJETOS corran al "
            "mismo tiempo - una moldea la voz por dentro, la otra "
            "reintroduce la voz de su hermano por fuera, y ambas se "
            "suman en la misma señal.\n\n"
            "AUX→PRINCIPAL y AUX→CLONE suman un camino más discreto "
            "encima: dos osciladores escondidos derivando cada uno "
            "con el volumen del otro objeto, enviados en lugar de la "
            "señal directa.\n\n"
            "Silenciar la salida propia de un objeto (MIXER OBJETOS) "
            "NO silencia su influencia por esas rutas - son controles "
            "separados, así que un objeto puede seguir moldeando a su "
            "hermano incluso cuando no lo escuchas directamente.\n\n"
            "DERIVA, cuando está armada, también puede derivar las "
            "propias rutas, no solo CV/AMP/FX - la propia topología "
            "puede cambiar sola."
        },
        {
            "Open CLONE, mute PRINCIPAL's own output in MIXER OBJETOS, "
            "and raise PRINCIPAL→CLONE - you will still hear "
            "PRINCIPAL's influence in CLONE's sound, proof the route "
            "and the mute answer two different questions.",
            "Abra CLONE, silencie a saída própria de PRINCIPAL em "
            "MIXER OBJETOS, e suba PRINCIPAL→CLONE - você ainda vai "
            "ouvir a influência de PRINCIPAL no som de CLONE, prova de "
            "que a rota e o mute respondem a duas perguntas "
            "diferentes.",
            "Ouvrez CLONE, coupez la sortie propre de PRINCIPAL dans "
            "MIXER OBJETOS, et montez PRINCIPAL→CLONE - vous "
            "entendrez encore l'influence de PRINCIPAL dans le son de "
            "CLONE, preuve que la route et le mute répondent à deux "
            "questions différentes.",
            "Abre CLONE, silencia la salida propia de PRINCIPAL en "
            "MIXER OBJETOS, y sube PRINCIPAL→CLONE - todavía "
            "escucharás la influencia de PRINCIPAL en el sonido de "
            "CLONE, prueba de que la ruta y el mute responden a dos "
            "preguntas diferentes."
        }
    },
    {
        { "TIMING LAYERS", "CAMADAS DE TEMPO", "COUCHES DE TEMPS", "CAPAS DE TIEMPO" },
        {
            "SUBDIVISÃO and PERCURSO are independent axes, not one "
            "setting.\n\n"
            "SUBDIVISÃO shapes how the clock itself feels (straight, "
            "tuplets, swing, or glitch) - the pulse's texture.\n\n"
            "GROOVE layers its own small long-short alternation on top "
            "of whichever SUBDIVISÃO feel is active - even SWG's own "
            "fixed shuffle gets deepened by it, since the two stack "
            "rather than replace each other.\n\n"
            "PERCURSO decides step order (FWD/REV/ALT/MEM) - the "
            "address, not the pulse's shape. The same MEM path sounds "
            "different under a straight clock than under a glitch "
            "one.\n\n"
            "A third layer sits above both: the master CLOCK rate "
            "itself, which simply speeds up or slows down everything "
            "already described without touching either axis - the "
            "same SUBDIVISÃO feel and the same PERCURSO path, just "
            "replayed faster or slower.\n\n"
            "FIM DO LOOP sits outside all three: it only decides how "
            "many of the 16 steps get visited before the whole thing "
            "wraps, whichever axis and whichever rate are currently in "
            "play.",
            "SUBDIVISÃO e PERCURSO são eixos independentes, não um "
            "único ajuste.\n\n"
            "SUBDIVISÃO molda como o próprio clock soa (reto, tuplets, "
            "swing, ou glitch) - a textura do pulso.\n\n"
            "GROOVE aplica sua própria alternância longo-curto por cima "
            "de qualquer sabor de SUBDIVISÃO ativo - até o shuffle fixo "
            "de SWG fica mais fundo com ele, já que os dois se somam em "
            "vez de se substituir.\n\n"
            "PERCURSO decide a ordem dos passos (FWD/REV/ALT/MEM) - o "
            "endereço, não a forma do pulso. O mesmo percurso MEM soa "
            "diferente sob um clock reto e sob um clock glitch.\n\n"
            "Uma terceira camada fica acima das duas: o próprio ritmo "
            "do CLOCK mestre, que só acelera ou desacelera tudo que já "
            "foi descrito sem tocar em nenhum dos eixos - o mesmo sabor "
            "de SUBDIVISÃO e o mesmo percurso, só que tocados mais "
            "rápido ou mais devagar.\n\n"
            "FIM DO LOOP fica fora dos três: só decide quantos dos 16 "
            "passos são visitados antes de tudo dar a volta, seja qual "
            "for o eixo e o ritmo em jogo no momento.",
            "SUBDIVISÃO et PERCURSO sont des axes indépendants, pas "
            "un seul réglage.\n\n"
            "SUBDIVISÃO façonne la sensation de l'horloge elle-"
            "même (droit, triolets, swing, ou glitch) - la texture de "
            "l'impulsion.\n\n"
            "GROOVE superpose sa propre alternance longue-courte à "
            "n'importe quelle sensation de SUBDIVISÃO active - même le "
            "shuffle fixe de SWG s'en trouve approfondi, les deux "
            "s'additionnant au lieu de se remplacer.\n\n"
            "PERCURSO décide l'ordre des pas (FWD/REV/ALT/MEM) - "
            "l'adresse, pas la forme de l'impulsion. Le même parcours "
            "MEM sonne différemment sous une horloge droite et sous "
            "une horloge glitch.\n\n"
            "Une troisième couche se trouve au-dessus des deux : le "
            "rythme propre de l'horloge CLOCK maîtresse, qui accélère "
            "ou ralentit simplement tout ce qui a déjà été décrit sans "
            "toucher à aucun des deux axes - la même sensation SUBDIVISÃO "
            "et le même parcours, juste rejoués plus vite ou "
            "plus lentement.\n\n"
            "FIM DO LOOP se trouve en dehors des trois : il ne décide "
            "que combien des 16 pas sont visités avant que tout ne "
            "reboucle, quel que soit l'axe et le rythme en jeu à ce "
            "moment.",
            "SUBDIVISÃO y PERCURSO son ejes independientes, no un "
            "solo ajuste.\n\n"
            "SUBDIVISÃO moldea cómo suena el propio reloj (recto, "
            "tresillos, swing, o glitch) - la textura del pulso.\n\n"
            "GROOVE superpone su propia alternancia larga-corta sobre "
            "cualquier sensación de SUBDIVISÃO activa - hasta el "
            "shuffle fijo de SWG se profundiza con él, ya que los dos "
            "se suman en vez de reemplazarse.\n\n"
            "PERCURSO decide el orden de los pasos (FWD/REV/ALT/MEM) - "
            "la dirección, no la forma del pulso. El mismo recorrido "
            "MEM suena distinto bajo un reloj recto y bajo uno "
            "glitch.\n\n"
            "Una tercera capa está por encima de las dos: el propio "
            "ritmo del CLOCK maestro, que solo acelera o frena todo lo "
            "ya descrito sin tocar ninguno de los ejes - la misma "
            "sensación de SUBDIVISÃO y el mismo recorrido, solo que "
            "reproducidos más rápido o más despacio.\n\n"
            "FIM DO LOOP queda fuera de los tres: solo decide cuántos "
            "de los 16 pasos se visitan antes de que todo dé la "
            "vuelta, sea cual sea el eje y el ritmo en juego en ese "
            "momento."
        },
        {
            "Keep PERCURSO on MEM and cycle only through SUBDIVISÃO - "
            "the step order never changes, only its feel. "
            "Then try the reverse: freeze SUBDIVISÃO and cycle "
            "PERCURSO instead, and notice the pulse itself never "
            "wavers.",
            "Deixe PERCURSO em MEM e cicle só SUBDIVISÃO - a ordem "
            "dos passos nunca muda, só o sabor. Depois tente o "
            "inverso: congele SUBDIVISÃO e cicle PERCURSO, e note "
            "que o pulso em si nunca vacila.",
            "Laissez PERCURSO sur MEM et ne faites défiler que SUBDIVISÃO - "
            "l'ordre des pas ne change jamais, seulement sa "
            "sensation. Puis essayez l'inverse : figez SUBDIVISÃO "
            "et faites défiler PERCURSO, et remarquez que l'impulsion "
            "elle-même ne vacille jamais.",
            "Deja PERCURSO en MEM y recorre solo SUBDIVISÃO - el "
            "orden de los pasos nunca cambia, solo su sensación. Luego "
            "prueba lo inverso: congela SUBDIVISÃO y recorre "
            "PERCURSO, y nota que el pulso en sí nunca vacila."
        }
    },
    {
        { "VARIATION PRESETS", "PRESETS DE VARIAÇÃO", "PRESETS DE VARIATION", "PRESETS DE VARIACIÓN" },
        {
            "PULSO, POROSA, HETERÓDINA, ÓRBITA and PÊNDULO each set a "
            "coherent bundle of parameters at once - not a single "
            "knob.\n\n"
            "RND 16 only nudges CV/AMP/FX per step; the five presets go "
            "further, touching routing, S&H, capacitive returns, AXIS "
            "Y/Z or PERCURSO itself.\n\n"
            "PÊNDULO is the clearest example of a preset reaching into "
            "a different layer entirely: it doesn't just raise RES "
            "MIX/ALTURA/CORPO, it also forces PERCURSO to pendulum, "
            "overriding whatever step order was already set. ÓRBITA "
            "reaches just as far in a different direction, turning on "
            "AXIS Y/Z on all 5 oscillators at once - a relation "
            "between oscillators that RND 16 never touches.\n\n"
            "That's why RND 16 keeps a patch's identity while a preset "
            "can change it entirely - different layers of the same "
            "instrument.",
            "PULSO, POROSA, HETERÓDINA, ÓRBITA e PÊNDULO ajustam de uma "
            "vez um conjunto coerente de parâmetros - não um knob só.\n"
            "\n"
            "RND 16 só cutuca CV/AMP/FX por passo; os cinco presets vão "
            "além, tocando roteamento, S&H, retornos capacitivos, EIXO "
            "Y/Z ou o próprio PERCURSO.\n\n"
            "PÊNDULO é o exemplo mais claro de um preset alcançando "
            "uma camada totalmente diferente: não só sobe RES MIX/"
            "ALTURA/CORPO, também força PERCURSO pro modo pêndulo, "
            "sobrepondo qualquer ordem de passos já ajustada. ÓRBITA "
            "alcança tão longe quanto, só que numa direção diferente, "
            "ligando EIXO Y/Z nos 5 osciladores de uma vez - uma "
            "relação entre osciladores que RND 16 nunca toca.\n\n"
            "Por isso RND 16 mantém a identidade do patch enquanto um "
            "preset pode mudá-la por completo - camadas diferentes do "
            "mesmo instrumento.",
            "PULSO, POROSA, HETERÓDINA, ÓRBITA et PÊNDULO règlent "
            "chacun d'un coup un ensemble cohérent de paramètres - pas "
            "un seul bouton.\n\n"
            "RND 16 ne fait que pousser légèrement CV/AMP/FX par pas ; "
            "les cinq presets vont plus loin, touchant le routage, "
            "S&H, les retours capacitifs, AXE Y/Z ou PERCURSO lui-"
            "même.\n\n"
            "PÊNDULO est l'exemple le plus clair d'un preset qui "
            "atteint une couche entièrement différente : il ne fait "
            "pas que monter RES MIX/ALTURA/CORPO, il force aussi "
            "PERCURSO en mode pendule, écrasant l'ordre des pas déjà "
            "réglé. ÓRBITA va tout aussi loin dans une autre direction, "
            "activant AXE Y/Z sur les 5 oscillateurs à la fois - une "
            "relation entre oscillateurs que RND 16 ne touche jamais."
            "\n\n"
            "C'est pourquoi RND 16 garde l'identité du patch tandis "
            "qu'un preset peut la changer entièrement - des couches "
            "différentes du même instrument.",
            "PULSO, POROSA, HETERÓDINA, ÓRBITA y PÊNDULO ajustan de una "
            "vez un conjunto coherente de parámetros - no un solo "
            "knob.\n\n"
            "RND 16 solo empuja levemente CV/AMP/FX por paso; los "
            "cinco presets van más allá, tocando el ruteo, S&H, "
            "retornos capacitivos, EJE Y/Z o el propio PERCURSO.\n\n"
            "PÊNDULO es el ejemplo más claro de un preset que alcanza "
            "una capa completamente distinta: no solo sube RES MIX/"
            "ALTURA/CORPO, también fuerza PERCURSO al modo péndulo, "
            "sobreponiéndose a cualquier orden de pasos ya ajustado. "
            "ÓRBITA alcanza igual de lejos en otra dirección, "
            "encendiendo EJE Y/Z en los 5 osciladores a la vez - una "
            "relación entre osciladores que RND 16 nunca toca.\n\n"
            "Por eso RND 16 mantiene la identidad del patch mientras "
            "un preset puede cambiarla por completo - capas distintas "
            "del mismo instrumento."
        },
        {
            "Try RND 16 four times, then one preset once - compare how "
            "far each one actually moved the patch. Then try PÊNDULO "
            "specifically and watch PERCURSO switch on its own - proof "
            "a preset can reach past sound into structure.",
            "Tente RND 16 quatro vezes, depois um preset uma vez - "
            "compare o quanto cada um realmente moveu o patch. Depois "
            "tente PÊNDULO especificamente e observe PERCURSO mudar "
            "sozinho - prova de que um preset pode alcançar além do "
            "som, até a estrutura.",
            "Essayez RND 16 quatre fois, puis un preset une fois - "
            "comparez à quel point chacun a réellement déplacé le "
            "patch. Puis essayez PÊNDULO en particulier et observez "
            "PERCURSO changer tout seul - preuve qu'un preset peut "
            "aller au-delà du son, jusqu'à la structure.",
            "Prueba RND 16 cuatro veces, luego un preset una vez - "
            "compara cuánto movió realmente el patch cada uno. Luego "
            "prueba PÊNDULO específicamente y observa cómo PERCURSO "
            "cambia solo - prueba de que un preset puede llegar más "
            "allá del sonido, hasta la estructura."
        }
    },
    {
        { "WHAT DERIVA CAPTURES", "O QUE A DERIVA CAPTURA", "CE QUE DERIVA CAPTURE", "LO QUE CAPTURA DERIVA" },
        {
            "DERIVA's captured memory holds four things at once: the "
            "16 steps' own CV/AMP/FX, the effect sends, the CONEXÃO "
            "ENTRE OBJETOS routes, and the relations between "
            "oscillators (AXIS Y/Z).\n\n"
            "PROFUNDIDADE doesn't randomize any of these - it "
            "interpolates the whole captured memory toward a slowly-"
            "shifting target, sample by sample.\n\n"
            "The four don't drift independently of each other either - "
            "they're captured and interpolated together, as one "
            "memory, not four separate timers. A phrase where the "
            "routes drift noticeably while the steps barely move just "
            "means the target happened to sit close to the current "
            "steps and far from the current routes, not that some "
            "parts are more 'droppable' than others.\n\n"
            "That's why the same capture always drifts the same "
            "deterministic way if replayed from the same start.",
            "A memória capturada pela DERIVA guarda quatro coisas de "
            "uma vez: o CV/AMP/FX dos 16 passos, os envios de efeito, "
            "as rotas de CONEXÃO ENTRE OBJETOS, e as relações entre "
            "osciladores (EIXO Y/Z).\n\n"
            "PROFUNDIDADE não sorteia nada disso - ela interpola a "
            "memória capturada inteira em direção a um alvo que se "
            "desloca devagar, amostra por amostra.\n\n"
            "As quatro também não derivam independentes umas das "
            "outras - são capturadas e interpoladas juntas, como uma "
            "memória só, não quatro cronômetros separados. Uma frase "
            "em que as rotas derivam bastante enquanto os passos quase "
            "não se movem só significa que o alvo calhou de ficar "
            "perto dos passos atuais e longe das rotas atuais, não que "
            "algumas partes sejam mais 'soltas' que outras.\n\n"
            "Por isso a mesma captura sempre deriva do mesmo jeito "
            "determinístico se replayada a partir do mesmo início.",
            "La mémoire capturée par DERIVA contient quatre choses à "
            "la fois : le CV/AMP/FX propre des 16 pas, les envois "
            "d'effets, les routes de CONEXÃO ENTRE OBJETOS, et les "
            "relations entre oscillateurs (AXE Y/Z).\n\n"
            "PROFUNDIDADE ne tire rien de tout cela au hasard - elle "
            "interpole toute la mémoire capturée vers une cible qui se "
            "déplace lentement, échantillon par échantillon.\n\n"
            "Les quatre ne dérivent pas non plus indépendamment les "
            "unes des autres - elles sont capturées et interpolées "
            "ensemble, comme une seule mémoire, pas quatre minuteries "
            "séparées. Une phrase où les routes dérivent nettement "
            "tandis que les pas bougent à peine signifie juste que la "
            "cible se trouvait proche des pas actuels et loin des "
            "routes actuelles, pas que certaines parties seraient plus "
            "« lâches » que d'autres.\n\n"
            "C'est pourquoi la même capture dérive toujours de la même "
            "façon déterministe si elle est rejouée depuis le même "
            "point de départ.",
            "La memoria capturada por DERIVA guarda cuatro cosas a la "
            "vez: el CV/AMP/FX propio de los 16 pasos, los envíos de "
            "efecto, las rutas de CONEXÃO ENTRE OBJETOS, y las "
            "relaciones entre osciladores (EJE Y/Z).\n\n"
            "PROFUNDIDADE no sortea nada de esto - interpola toda la "
            "memoria capturada hacia un objetivo que se desplaza "
            "despacio, muestra por muestra.\n\n"
            "Las cuatro tampoco derivan independientes unas de otras - "
            "se capturan e interpolan juntas, como una sola memoria, "
            "no cuatro cronómetros separados. Una frase donde las "
            "rutas derivan bastante mientras los pasos casi no se "
            "mueven solo significa que el objetivo quedó cerca de los "
            "pasos actuales y lejos de las rutas actuales, no que "
            "algunas partes sean más 'sueltas' que otras.\n\n"
            "Por eso la misma captura siempre deriva de la misma "
            "forma determinística si se reproduce desde el mismo "
            "inicio."
        },
        {
            "Capture the same phrase twice in a row without changing "
            "anything - both captures will drift identically, since "
            "the randomness people hear in DERIVA lives entirely in "
            "when you choose to capture, not in the drift itself.",
            "Capture a mesma frase duas vezes seguidas sem mudar nada "
            "- as duas capturas vão derivar de forma idêntica, já que "
            "a aleatoriedade que as pessoas ouvem na DERIVA mora "
            "inteira no momento em que você escolhe capturar, não na "
            "deriva em si.",
            "Capturez la même phrase deux fois de suite sans rien "
            "changer - les deux captures dériveront de façon "
            "identique, car le hasard que l'on entend dans DERIVA se "
            "trouve entièrement dans le moment où l'on choisit de "
            "capturer, pas dans la dérive elle-même.",
            "Captura la misma frase dos veces seguidas sin cambiar "
            "nada - ambas capturas derivarán de forma idéntica, ya que "
            "la aleatoriedad que la gente oye en DERIVA vive por "
            "completo en el momento en que eliges capturar, no en la "
            "deriva en sí."
        }
    },
    {
        { "RING'S DOUBLE LIFE", "A DUPLA VIDA DO RING", "LA DOUBLE VIE DU RING", "LA DOBLE VIDA DEL RING" },
        {
            "Two different ring-modulation paths exist in the same "
            "instrument.\n\n"
            "OSC 5 (4046 . LM13600) multiplies its own wave by OSC A's "
            "raw waveform, right at the oscillator stage - a "
            "heterodyne character baked into the voice itself.\n\n"
            "RING, in MODULAÇÃO, multiplies the whole downstream sum "
            "by the LFO, further along the chain, after ADSR and "
            "NOISE already joined in. Both are ring modulation - only "
            "their place in the chain differs.\n\n"
            "OSC 5's own FREQ decides its flavour on its own: near 1x "
            "it beats slowly against OSC A, moving away opens a more "
            "inharmonic spectrum - independent of whether MODULAÇÃO's "
            "RING is on at all. Engage both at once and the two "
            "multiplications stack: OSC 5 already reshapes the voice "
            "before it even reaches ADSR, then RING reshapes the whole "
            "downstream sum again - a ring-modulated signal getting "
            "ring-modulated a second time, not the same modulation "
            "doubled.",
            "Dois caminhos diferentes de ring modulation existem no "
            "mesmo instrumento.\n\n"
            "OSC 5 (4046 . LM13600) multiplica sua própria onda pela "
            "onda crua do OSC A, direto no estágio do oscilador - um "
            "caráter heteródino embutido na própria voz.\n\n"
            "O RING, em MODULAÇÃO, multiplica toda a soma já formada "
            "pelo LFO, mais adiante na cadeia, depois que ADSR e NOISE "
            "já entraram. Os dois são ring modulation - só o lugar na "
            "cadeia muda.\n\n"
            "O próprio FREQ do OSC 5 decide seu sabor sozinho: perto "
            "de 1x bate devagar contra o OSC A, afastando abre um "
            "espectro mais inarmônico - independente de o RING de "
            "MODULAÇÃO estar ligado ou não. Ligue os dois ao mesmo "
            "tempo e as duas multiplicações se empilham: o OSC 5 já "
            "remodela a voz antes mesmo de chegar ao ADSR, depois o "
            "RING remodela a soma inteira de novo - um sinal já "
            "modulado em anel sendo modulado em anel de novo, não a "
            "mesma modulação dobrada.",
            "Deux chemins de ring modulation différents existent dans "
            "le même instrument.\n\n"
            "OSC 5 (4046 . LM13600) multiplie sa propre onde par "
            "l'onde brute de OSC A, directement au niveau de "
            "l'oscillateur - un caractère hétérodyne intégré à la "
            "voix elle-même.\n\n"
            "RING, dans MODULAÇÃO, multiplie toute la somme déjà "
            "formée par le LFO, plus loin dans la chaîne, après que "
            "ADSR et NOISE ont déjà rejoint. Les deux sont de la ring "
            "modulation - seul leur emplacement dans la chaîne "
            "diffère.\n\n"
            "Le propre FREQ de OSC 5 décide seul de sa couleur : "
            "proche de 1x il bat lentement contre OSC A, s'en éloigner "
            "ouvre un spectre plus inharmonique - indépendamment du "
            "fait que RING de MODULAÇÃO soit actif ou non. Activez les "
            "deux à la fois et les deux multiplications s'empilent : "
            "OSC 5 remodèle déjà la voix avant même qu'elle n'atteigne "
            "ADSR, puis RING remodèle toute la somme en aval encore "
            "une fois - un signal déjà modulé en anneau se faisant "
            "moduler en anneau une seconde fois, pas la même "
            "modulation doublée.",
            "Existen dos caminos de ring modulation distintos en el "
            "mismo instrumento.\n\n"
            "OSC 5 (4046 . LM13600) multiplica su propia onda por la "
            "onda cruda de OSC A, justo en la etapa del oscilador - "
            "un carácter heterodino integrado en la propia voz.\n\n"
            "RING, en MODULAÇÃO, multiplica toda la suma ya formada "
            "por el LFO, más adelante en la cadena, después de que "
            "ADSR y NOISE ya se sumaron. Ambos son ring modulation - "
            "solo cambia su lugar en la cadena.\n\n"
            "El propio FREQ de OSC 5 decide su sabor solo: cerca de "
            "1x late despacio contra OSC A, alejarse abre un espectro "
            "más inarmónico - independiente de que el RING de "
            "MODULAÇÃO esté encendido o no. Activa ambos a la vez y "
            "las dos multiplicaciones se apilan: OSC 5 ya remodela la "
            "voz antes incluso de llegar al ADSR, luego RING remodela "
            "toda la suma otra vez - una señal ya modulada en anillo "
            "siendo modulada en anillo una segunda vez, no la misma "
            "modulación duplicada."
        },
        {
            "Mute OSC 5 and only then engage RING - the character you "
            "hear now comes from a different point in the chain. Then "
            "bring OSC 5 back in on top of it and listen for the "
            "stack, not just a louder version of the same thing.",
            "Silencie o OSC 5 e só então ligue o RING - o caráter que "
            "você ouve agora vem de um ponto diferente da cadeia. "
            "Depois traga o OSC 5 de volta por cima e escute o "
            "empilhamento, não só uma versão mais alta da mesma "
            "coisa.",
            "Coupez OSC 5 et n'activez RING qu'ensuite - le caractère "
            "que vous entendez maintenant vient d'un point différent "
            "de la chaîne. Puis ramenez OSC 5 par-dessus et écoutez "
            "l'empilement, pas juste une version plus forte de la même "
            "chose.",
            "Silencia OSC 5 y solo entonces activa RING - el carácter "
            "que oyes ahora viene de un punto distinto de la cadena. "
            "Luego trae OSC 5 de vuelta por encima y escucha el "
            "apilamiento, no solo una versión más fuerte de lo mismo."
        }
    },
    {
        { "MATÉRIA", "MATÉRIA", "MATÉRIA", "MATÉRIA" },
        {
            "MATÉRIA (CUTOFF/RESONANCE/DRIVE/ASYMMETRY, plus its own "
            "MIX) is a second, separate filter from VCF - not a "
            "duplicate.\n\n"
            "Where VCF is a clean filter, MATÉRIA puts an asymmetric "
            "saturator right inside its own resonance feedback path: "
            "positive and negative swings get different drive, so "
            "RESONANCE/DRIVE colour and destabilise the sound the way "
            "a loaded analogue stage would.\n\n"
            "CUTOFF works the same way as VCF's FREQ - it sets where "
            "the response centres - but here it also decides where "
            "that saturating resonance itself lives in the spectrum, "
            "so moving it changes not just tone but where the "
            "instability sits.\n\n"
            "ASYMMETRY sets how lopsided that saturation is - 0 clips "
            "both directions the same; 1 drives the negative side "
            "noticeably less, MATÉRIA's own uneven character. MIX "
            "blends it in, same as every other effect.",
            "MATÉRIA (CUTOFF/RESONANCE/DRIVE/ASYMMETRY, mais seu "
            "próprio MIX) é um segundo filtro, separado do VCF - não "
            "uma duplicata.\n\n"
            "Enquanto o VCF é um filtro limpo, MATÉRIA põe um "
            "saturador assimétrico direto dentro do próprio caminho de "
            "realimentação da ressonância: oscilações positivas e "
            "negativas recebem drive diferente, então RESONANCE/DRIVE "
            "colorem e desestabilizam o som como um estágio analógico "
            "carregado faria.\n\n"
            "CUTOFF funciona igual ao FREQ do VCF - define onde a "
            "resposta se centra - mas aqui também decide onde essa "
            "ressonância saturada mora no espectro, então movê-lo "
            "muda não só o tom, mas onde a instabilidade fica.\n\n"
            "ASYMMETRY define o quão desigual é essa saturação - 0 "
            "satura os dois lados igual; 1 dirige o lado negativo "
            "visivelmente menos, o caráter desigual próprio da "
            "MATÉRIA. MIX mistura, igual todo outro efeito.",
            "MATÉRIA (CUTOFF/RESONANCE/DRIVE/ASYMMETRY, plus son "
            "propre MIX) est un second filtre, séparé de VCF - pas un "
            "doublon.\n\n"
            "Là où VCF est un filtre propre, MATÉRIA place un "
            "saturateur asymétrique directement dans son propre "
            "chemin de réaction de résonance : les excursions "
            "positives et négatives reçoivent un drive différent, "
            "donc RESONANCE/DRIVE colorent et déstabilisent le son "
            "comme le ferait un étage analogique chargé.\n\n"
            "CUTOFF fonctionne comme FREQ de VCF - il règle où la "
            "réponse se centre - mais ici il décide aussi où vit "
            "cette résonance saturée dans le spectre, donc le "
            "déplacer change non seulement le ton mais l'endroit où "
            "se trouve l'instabilité.\n\n"
            "ASYMMETRY règle à quel point cette saturation est "
            "déséquilibrée - 0 sature les deux sens pareil ; 1 pousse "
            "le côté négatif nettement moins, le caractère inégal "
            "propre à MATÉRIA. MIX le mélange, comme tout autre effet.",
            "MATÉRIA (CUTOFF/RESONANCE/DRIVE/ASYMMETRY, más su propio "
            "MIX) es un segundo filtro, separado de VCF - no un "
            "duplicado.\n\n"
            "Mientras VCF es un filtro limpio, MATÉRIA pone un "
            "saturador asimétrico justo dentro de su propio camino de "
            "realimentación de resonancia: las oscilaciones positivas "
            "y negativas reciben drive distinto, así que RESONANCE/"
            "DRIVE colorean y desestabilizan el sonido como lo haría "
            "una etapa analógica cargada.\n\n"
            "CUTOFF funciona igual que FREQ del VCF - define dónde se "
            "centra la respuesta - pero aquí también decide dónde "
            "vive esa resonancia saturada en el espectro, así que "
            "moverlo cambia no solo el tono sino dónde queda la "
            "inestabilidad.\n\n"
            "ASYMMETRY controla cuán desigual es esa saturación - 0 "
            "satura ambos lados igual; 1 impulsa el lado negativo "
            "notablemente menos, el carácter desigual propio de "
            "MATÉRIA. MIX lo mezcla, igual que cualquier otro efecto."
        },
        {
            "Push RESONANCE and DRIVE up together with ASYMMETRY at "
            "1 - that lopsided, unstable edge is MATÉRIA's own "
            "signature, something VCF can't reach. Then sweep CUTOFF "
            "slowly and listen to the instability itself travel, not "
            "just the tone.",
            "Suba RESONANCE e DRIVE juntos com ASYMMETRY em 1 - essa "
            "borda desigual e instável é a assinatura própria da "
            "MATÉRIA, algo que o VCF não alcança. Depois varra CUTOFF "
            "devagar e escute a própria instabilidade viajar, não só "
            "o tom.",
            "Montez RESONANCE et DRIVE ensemble avec ASYMMETRY à 1 - "
            "ce bord déséquilibré et instable est la signature propre "
            "de MATÉRIA, que VCF n'atteint pas. Puis balayez CUTOFF "
            "lentement et écoutez l'instabilité elle-même se "
            "déplacer, pas seulement le ton.",
            "Sube RESONANCE y DRIVE juntos con ASYMMETRY en 1 - ese "
            "borde desigual e inestable es la firma propia de "
            "MATÉRIA, algo que VCF no alcanza. Luego barre CUTOFF "
            "despacio y escucha la propia inestabilidad viajar, no "
            "solo el tono."
        }
    },
    {
        { "CAOS / VAGA", "CAOS / VAGA", "CAOS / VAGA", "CAOS / VAGA" },
        {
            "CAOS and VAGA are two of the six FORMA LFO shapes, but "
            "they aren't periodic waveforms like SEN/TRI/PUL - each is "
            "its own generator.\n\n"
            "CAOS is a chaotic oscillator settling between two states: "
            "DRIVE sets how hard it's pulled toward them and the size "
            "of its periodic random kick; DAMPING sets how much energy "
            "it loses per step. Low DAMPING with high DRIVE swings "
            "unpredictably between the two; high DAMPING settles it "
            "into one.\n\n"
            "VAGA is a smoothed random walk instead: DEPTH sets how "
            "far a fresh target can land from centre, RATE sets how "
            "often it retargets. FRZ holds either one exactly still at "
            "its current value - no effect on SEN/TRI/PUL/STEP.\n\n"
            "Because CAOS and VAGA feed into the same LFO slot as "
            "SEN/TRI/PUL, whatever consumes that LFO downstream - "
            "RING's own carrier being the clearest example - inherits "
            "their non-periodic behaviour too. A ring-modulated signal "
            "carried by CAOS doesn't just get a chaotic amplitude on "
            "top, it gets a genuinely non-repeating spectrum, since "
            "the carrier itself never completes a cycle to repeat "
            "from.",
            "CAOS e VAGA são duas das seis formas de FORMA LFO, mas "
            "não são ondas periódicas como SEN/TRI/PUL - cada uma é "
            "seu próprio gerador.\n\n"
            "CAOS é um oscilador caótico que se assenta entre dois "
            "estados: DRIVE define a força de atração para eles e o "
            "tamanho do seu chute aleatório periódico; DAMPING define "
            "quanta energia ele perde a cada passo. DAMPING baixo com "
            "DRIVE alto oscila de forma imprevisível entre os dois; "
            "DAMPING alto o assenta em um só.\n\n"
            "VAGA é uma caminhada aleatória suavizada: DEPTH define "
            "até onde um novo alvo pode cair do centro, RATE define "
            "com que frequência ele mira de novo. FRZ segura qualquer "
            "um dos dois exatamente parado no valor atual - sem efeito "
            "em SEN/TRI/PUL/STEP.\n\n"
            "Como CAOS e VAGA entram no mesmo lugar do LFO que SEN/"
            "TRI/PUL, o que consumir esse LFO adiante - o próprio RING "
            "como portadora é o exemplo mais claro - herda também esse "
            "comportamento não periódico. Um sinal modulado em anel "
            "carregado por CAOS não ganha só uma amplitude caótica por "
            "cima, ganha um espectro genuinamente não repetitivo, já "
            "que a própria portadora nunca completa um ciclo pra "
            "repetir.",
            "CAOS et VAGA sont deux des six formes de FORMA LFO, mais "
            "ce ne sont pas des ondes périodiques comme SEN/TRI/PUL - "
            "chacune est son propre générateur.\n\n"
            "CAOS est un oscillateur chaotique qui se stabilise entre "
            "deux états : DRIVE règle la force d'attraction vers eux "
            "et la taille de sa secousse aléatoire périodique ; "
            "DAMPING règle l'énergie perdue à chaque pas. Un DAMPING "
            "faible avec un DRIVE élevé oscille de façon imprévisible "
            "entre les deux ; un DAMPING élevé le stabilise dans un "
            "seul.\n\n"
            "VAGA est une marche aléatoire lissée à la place : DEPTH "
            "règle jusqu'où une nouvelle cible peut tomber par rapport "
            "au centre, RATE règle la fréquence des nouvelles cibles. "
            "FRZ fige l'un ou l'autre exactement à sa valeur actuelle "
            "- sans effet sur SEN/TRI/PUL/STEP.\n\n"
            "Comme CAOS et VAGA occupent le même emplacement de LFO "
            "que SEN/TRI/PUL, tout ce qui consomme ce LFO en aval - la "
            "porteuse propre de RING étant l'exemple le plus clair - "
            "hérite aussi de leur comportement non périodique. Un "
            "signal modulé en anneau porté par CAOS ne reçoit pas "
            "seulement une amplitude chaotique par-dessus, il reçoit "
            "un spectre véritablement non répétitif, puisque la "
            "porteuse elle-même ne termine jamais de cycle pour se "
            "répéter.",
            "CAOS y VAGA son dos de las seis formas de FORMA LFO, pero "
            "no son ondas periódicas como SEN/TRI/PUL - cada una es "
            "su propio generador.\n\n"
            "CAOS es un oscilador caótico que se asienta entre dos "
            "estados: DRIVE fija la fuerza de atracción hacia ellos y "
            "el tamaño de su empujón aleatorio periódico; DAMPING fija "
            "cuánta energía pierde en cada paso. DAMPING bajo con "
            "DRIVE alto oscila de forma impredecible entre los dos; "
            "DAMPING alto lo asienta en uno solo.\n\n"
            "VAGA es una caminata aleatoria suavizada: DEPTH fija "
            "hasta dónde puede caer un nuevo objetivo respecto al "
            "centro, RATE fija cada cuánto apunta de nuevo. FRZ "
            "mantiene a cualquiera de los dos exactamente quieto en su "
            "valor actual - sin efecto en SEN/TRI/PUL/STEP.\n\n"
            "Como CAOS y VAGA ocupan el mismo lugar del LFO que SEN/"
            "TRI/PUL, lo que consuma ese LFO más adelante - la propia "
            "portadora de RING es el ejemplo más claro - también "
            "hereda ese comportamiento no periódico. Una señal "
            "modulada en anillo llevada por CAOS no gana solo una "
            "amplitud caótica encima, gana un espectro genuinamente "
            "no repetitivo, ya que la propia portadora nunca completa "
            "un ciclo para repetirse."
        },
        {
            "Select CAOS, set DAMPING low and DRIVE high, then press "
            "FRZ mid-swing - the value holds exactly where it was, "
            "proof FRZ freezes the trajectory, not just the sound. "
            "Release FRZ and it resumes exactly from that frozen "
            "point, not from a fresh kick.",
            "Selecione CAOS, deixe DAMPING baixo e DRIVE alto, depois "
            "aperte FRZ no meio de uma oscilação - o valor segura "
            "exatamente onde estava, prova de que FRZ congela a "
            "trajetória, não só silencia. Solte o FRZ e ele retoma "
            "exatamente daquele ponto congelado, não de um chute novo.",
            "Sélectionnez CAOS, réglez DAMPING bas et DRIVE haut, puis "
            "appuyez sur FRZ en pleine oscillation - la valeur se fige "
            "exactement là où elle était, preuve que FRZ gèle la "
            "trajectoire, pas seulement le son. Relâchez FRZ et il "
            "reprend exactement depuis ce point figé, pas depuis une "
            "nouvelle secousse.",
            "Selecciona CAOS, pon DAMPING bajo y DRIVE alto, luego "
            "presiona FRZ a mitad de una oscilación - el valor se "
            "mantiene exactamente donde estaba, prueba de que FRZ "
            "congela la trayectoria, no solo la silencia. Suelta FRZ "
            "y retoma exactamente desde ese punto congelado, no desde "
            "un nuevo empujón."
        }
    }
}};

struct AboutContent
{
    LocalizedText title;
    LocalizedText body;
};

inline const AboutContent aboutContent {
    { "ANTITOTEM — SOUND OBJECT", "ANTITOTEM — OBJETO SONORO", "ANTITOTEM — OBJET SONORE", "ANTITOTEM — OBJETO SONORO" },
    {
        "Antitotem - Objeto Sonoro - v0.1\n\n"
        "Authorial digital instrument in JUCE/C++, critically inspired by "
        "CMOS, Lunetta and circuit-bending practices.\n\n"
        "License: GNU AGPLv3.\n\n"
        "Chip names indicate studied concepts - they are not copies of "
        "schematics. Sources and credits are in "
        "ANTITOTEM/CREDITS_AND_SOURCES.md.",
        "Antitotem - Objeto Sonoro - v0.1\n\n"
        "Instrumento digital autoral em JUCE/C++, inspirado criticamente em "
        "práticas CMOS, Lunetta e circuit bending.\n\n"
        "Licença: GNU AGPLv3.\n\n"
        "Os nomes de CIs indicam conceitos estudados - não são cópias de "
        "esquemas. As fontes e créditos estão em "
        "ANTITOTEM/CREDITS_AND_SOURCES.md.",
        "Antitotem - Objeto Sonoro - v0.1\n\n"
        "Instrument numérique autoral en JUCE/C++, inspiré de façon "
        "critique par les pratiques CMOS, Lunetta et circuit-bending.\n\n"
        "Licence : GNU AGPLv3.\n\n"
        "Les noms de circuits intégrés indiquent des concepts étudiés - ce "
        "ne sont pas des copies de schémas. Les sources et crédits sont "
        "dans ANTITOTEM/CREDITS_AND_SOURCES.md.",
        "Antitotem - Objeto Sonoro - v0.1\n\n"
        "Instrumento digital de autor en JUCE/C++, inspirado críticamente "
        "en prácticas CMOS, Lunetta y circuit bending.\n\n"
        "Licencia: GNU AGPLv3.\n\n"
        "Los nombres de chips indican conceptos estudiados - no son copias "
        "de esquemas. Las fuentes y créditos están en "
        "ANTITOTEM/CREDITS_AND_SOURCES.md."
    }
};
// Main panel tooltips - the "much larger task" flagged in docs/TAREFAS.md
// when only TUTORIAL/SOBRE were localized. Control names that already
// appear printed on the panel itself (FREQ, MIX, EIXO X/Y/Z, FORMA, CV,
// RES, ATT/DEC/SUS/REL, LFO, RING, NOISE, REVERB, PHASER, FLANGER, RET, X,
// FX, PRINCIPAL/CLONE, TUTORIAL/SOBRE, GANHO Y, CAPTURAR, PERCURSO, VCF,
// LPF/BPF/HPF/NOTCH...) stay untranslated inside the tooltip prose too,
// same rule tutorialChapters already follows - only the explanatory text
// around them is translated, so a tooltip's wording always matches the
// exact printed label it explains.
namespace tooltip
{
inline const LocalizedText openTutorial {
    "Opens a detailed listening/patching roadmap, module by module, in EN/PT/FR/ES.",
    "Abre um roteiro detalhado de escuta e patch, módulo por módulo, em EN/PT/FR/ES.",
    "Ouvre un guide d'écoute et de patch détaillé, module par module, en EN/PT/FR/ES.",
    "Abre una guía detallada de escucha y parcheo, módulo por módulo, en EN/PT/FR/ES."
};
inline const LocalizedText energy {
    "Changes the digital supply: timing, the relationship between oscillators and the wave's body.",
    "Altera a alimentação digital: tempo, relação entre os osciladores e corpo da onda.",
    "Modifie l'alimentation numérique : le tempo, la relation entre les oscillateurs et le corps de l'onde.",
    "Cambia la alimentación digital: el tiempo, la relación entre los osciladores y el cuerpo de la onda."
};
inline const LocalizedText cloneToggle {
    "Switches between PRINCIPAL and CLONE in this window (or opens CLONE on the second monitor, if 2-monitor mode is active) - a second, complete instance of the engine, coupled to the main object through configurable return routes.",
    "Alterna entre PRINCIPAL e CLONE nesta janela (ou abre o CLONE no segundo monitor, se o modo 2 monitores estiver ativo) — uma segunda instância completa do motor, acoplada ao objeto principal por rotas de retorno configuráveis.",
    "Bascule entre PRINCIPAL et CLONE dans cette fenêtre (ou ouvre CLONE sur le second moniteur, si le mode 2 moniteurs est actif) - une seconde instance complète du moteur, couplée à l'objet principal par des routes de retour configurables.",
    "Alterna entre PRINCIPAL y CLONE en esta ventana (o abre CLONE en el segundo monitor, si el modo de 2 monitores está activo) - una segunda instancia completa del motor, acoplada al objeto principal mediante rutas de retorno configurables."
};
inline const LocalizedText sampleHold {
    "Samples and holds the noise in steps.",
    "Amostra e sustenta o ruído em degraus.",
    "Échantillonne et maintient le bruit par paliers.",
    "Muestrea y sostiene el ruido en escalones."
};
inline const LocalizedText noiseBreath {
    "Adds a breathing layer of noise on top of NOISE MIX, rising and "
    "easing with each step's own gate - its own timing, not the note's.",
    "Adiciona uma camada de ruído que respira em cima do NOISE MIX, "
    "subindo e cedendo com o gate de cada passo - tempo próprio, "
    "diferente do da nota.",
    "Ajoute une couche de bruit qui respire par-dessus le NOISE MIX, "
    "montant et cédant avec la porte de chaque pas - son propre tempo, "
    "distinct de celui de la note.",
    "Añade una capa de ruido que respira sobre el NOISE MIX, subiendo y "
    "cediendo con la puerta de cada paso - tiempo propio, distinto del "
    "de la nota."
};
inline const LocalizedText stepAmp {
    "AMP: this step's dynamics.",
    "AMP: dinâmica desta etapa.",
    "AMP : la dynamique de ce pas.",
    "AMP: la dinámica de este paso."
};
inline const LocalizedText mixCapture {
    "Arms the capture: the next slot you click saves the current mixer instead of recalling it.",
    "Arma a gravação: o próximo slot clicado guarda o mixer atual em vez de recuperá-lo.",
    "Arme la capture : le prochain emplacement cliqué enregistre le mixer actuel au lieu de le rappeler.",
    "Arma la captura: el próximo slot que hagas clic guarda el mezclador actual en vez de recuperarlo."
};
// One per PORTAS DE FEEDBACK button (FB/DIODE/CAP/PULSE/TRANS/REFLUX,
// same order as feedbackNames/connectionNames in Main.cpp) - author,
// live: "cada botão de feedback deve ser explicado a função em learn
// (tá genérico)", replacing the single shared feedbackDoor text below.
// Kept to 4 lines or less in the LEARN box (author: "os conteúdos do
// learn na caixa learn não devem ultrapassar 4 linhas").
inline const std::array<LocalizedText, 6> feedbackDoorTips {{
    { "The raw sample - sustains the current timbre.",
      "A amostra crua - sustenta o timbre atual.",
      "L'échantillon brut - soutient le timbre actuel.",
      "La muestra cruda - sostiene el timbre actual." },
    { "Half-wave rectifies the sample - doubles the perceived pitch.",
      "Retifica a amostra em meia-onda - dobra a altura percebida.",
      "Redresse l'échantillon en demi-onde - double la hauteur perçue.",
      "Rectifica la muestra en media onda - duplica la altura percibida." },
    { "Feeds back the capacitor's slow average, not the instant sample - smoother sustain.",
      "Realimenta a média lenta do capacitor, não a amostra instantânea - sustain mais suave.",
      "Réinjecte la moyenne lente du capacitor, pas l'échantillon instantané - sustain plus doux.",
      "Reintroduce el promedio lento del capacitor, no la muestra instantánea - sustain más suave." },
    { "Collapses the return to a bare square - rhythmic, not tonal.",
      "Reduz o retorno a um quadrado puro - rítmico, não tonal.",
      "Réduit le retour à un simple carré - rythmique, pas tonal.",
      "Reduce el retorno a un cuadrado puro - rítmico, no tonal." },
    { "Blends the sample with the capacitor and softly saturates it - warmer.",
      "Mistura a amostra com o capacitor e satura suave - mais quente.",
      "Mélange l'échantillon avec le capacitor et sature doucement - plus chaud.",
      "Mezcla la muestra con el capacitor y satura suave - más cálido." },
    { "Subtracts the capacitor from the sample, isolating the transient - opposite of CAP.",
      "Subtrai o capacitor da amostra, isolando o transiente - oposto do CAP.",
      "Soustrait le capacitor de l'échantillon, isolant le transitoire - l'inverse de CAP.",
      "Resta el capacitor de la muestra, aislando el transitorio - opuesto de CAP." }
}};
inline const LocalizedText deriveButton {
    "Activates a generative memory: at every phrase return, the object retains and shifts CV, dynamics, sends and feedback routes.",
    "Ativa uma memória gerativa: a cada retorno de frase, o objeto retém e desloca CV, dinâmica, envios e rotas de feedback.",
    "Active une mémoire générative : à chaque retour de phrase, l'objet retient et déplace CV, dynamique, envois et routes de retour.",
    "Activa una memoria generativa: en cada retorno de frase, el objeto retiene y desplaza CV, dinámica, envíos y rutas de retorno."
};
inline const LocalizedText derivationLayer {
    "Toggles this drift layer on/off (independent from the others, like the VCF mode buttons): A = steps/topology/oscillator ratios, B = ADSR/LFO/NOISE/GROOVE/filter, C = pans/METRIC/SUBDIVISION/NOISE COLOR.",
    "Liga/desliga essa camada de deriva (independente das outras, como os botões de modo do VCF): A = passos/topologia/razão dos osciladores, B = ADSR/LFO/NOISE/GROOVE/filtro, C = pans/MÉTRICA/SUBDIVISÃO/NOISE COR.",
    "Active/désactive cette couche de dérive (indépendante des autres, comme les boutons de mode du VCF) : A = pas/topologie/rapport des oscillateurs, B = ADSR/LFO/NOISE/GROOVE/filtre, C = pans/MÉTRIQUE/SUBDIVISION/COULEUR DE BRUIT.",
    "Activa/desactiva esta capa de deriva (independiente de las otras, como los botones de modo del VCF): A = pasos/topología/razón de los osciladores, B = ADSR/LFO/NOISE/GROOVE/filtro, C = pans/MÉTRICA/SUBDIVISIÓN/COLOR DE RUIDO."
};
inline const LocalizedText derivationParticipation {
    "Toggles whether this section participates in DRIFT (both A/B/C and AUTO modes). On by default; turning it off freezes just this section, independent of the other titles.",
    "Liga/desliga se esta seção participa da DERIVA (nos modos A/B/C e AUTO). Ligado por padrão; desligar congela só esta seção, independente dos outros títulos.",
    "Active/désactive si cette section participe à la DÉRIVE (modes A/B/C et AUTO). Activé par défaut ; désactiver ne fige que cette section, indépendamment des autres titres.",
    "Activa/desactiva si esta sección participa en la DERIVA (modos A/B/C y AUTO). Activado por defecto; desactivarlo congela solo esta sección, independiente de los demás títulos."
};
inline const LocalizedText envelopeAttack {
    "ATT: the energy's rise time.",
    "ATT: tempo de subida da energia.",
    "ATT : le temps de montée de l'énergie.",
    "ATT: el tiempo de subida de la energía."
};
inline const LocalizedText mixSlotRecall {
    "Click to recall this slot; with CAPTURAR armed, saves the current mixer into it instead.",
    "Clique para recuperar este slot; com CAPTURAR armado, guarda o mixer atual nele.",
    "Cliquez pour rappeler cet emplacement ; avec CAPTURAR armé, y enregistre le mixer actuel à la place.",
    "Haz clic para recuperar este slot; con CAPTURAR armado, guarda el mezclador actual en él."
};
inline const LocalizedText monitorModeToggle {
    "With 2 monitors: chooses whether CLONE opens filling the second monitor, or near the main window on the same monitor. Only takes effect after CLONE is opened for the first time.",
    "Com 2 monitores: escolhe se o CLONE abre preenchendo o segundo monitor, ou perto da janela principal no mesmo monitor. Só faz efeito depois que o CLONE for aberto pela primeira vez.",
    "Avec 2 moniteurs : choisit si CLONE s'ouvre en remplissant le second moniteur, ou près de la fenêtre principale sur le même moniteur. Ne prend effet qu'après la première ouverture de CLONE.",
    "Con 2 monitores: elige si CLONE se abre llenando el segundo monitor, o cerca de la ventana principal en el mismo monitor. Solo tiene efecto después de que CLONE se abra por primera vez."
};
inline const LocalizedText openAbout {
    "Credits, version, license and principles of the sound object, in EN/PT/FR/ES.",
    "Créditos, versão, licença e princípios do objeto sonoro, em EN/PT/FR/ES.",
    "Crédits, version, licence et principes de l'objet sonore, en EN/PT/FR/ES.",
    "Créditos, versión, licencia y principios del objeto sonoro, en EN/PT/FR/ES."
};
inline const LocalizedText learnPanelIdle {
    "Hover or focus a control to see its explanation here.",
    "Passe o mouse ou foque um controle para ver a explicação aqui.",
    "Survolez ou focalisez un contrôle pour voir son explication ici.",
    "Pasa el mouse o enfoca un control para ver su explicación aquí."
};
inline const LocalizedText objectMixPrincipal {
    "PRINCIPAL's own volume in the final mix - M/S work like the "
    "mixer's own, without stopping its transport or its influence on "
    "CLONE through the feedback ports.",
    "Volume do PRINCIPAL na mixagem final - M/S funcionam como os do "
    "mixer, sem parar o transporte dele nem sua influência no CLONE "
    "pelas portas de feedback.",
    "Volume du PRINCIPAL dans le mixage final - M/S fonctionnent comme "
    "ceux du mixer, sans arrêter son transport ni son influence sur "
    "CLONE via les ports de feedback.",
    "Volumen del PRINCIPAL en la mezcla final - M/S funcionan como los "
    "del mixer, sin detener su transporte ni su influencia en CLONE por "
    "los puertos de feedback."
};
inline const LocalizedText objectMixClone {
    "CLONE's own volume in the final mix - M/S work like the mixer's "
    "own, without stopping its transport or its influence on PRINCIPAL "
    "through the feedback ports.",
    "Volume do CLONE na mixagem final - M/S funcionam como os do "
    "mixer, sem parar o transporte dele nem sua influência no PRINCIPAL "
    "pelas portas de feedback.",
    "Volume du CLONE dans le mixage final - M/S fonctionnent comme ceux "
    "du mixer, sans arrêter son transport ni son influence sur "
    "PRINCIPAL via les ports de feedback.",
    "Volumen del CLONE en la mezcla final - M/S funcionan como los del "
    "mixer, sin detener su transporte ni su influencia en PRINCIPAL por "
    "los puertos de feedback."
};
// EXCITAÇÃO (20 ago. 2026, author: "algo generativo, não performático
// ... que inventasse melodias a partir de estimulos e acontecimentos dos
// fluxos do que acontece nos controles que já existem" / "entendo o
// treremin como um outro objeto") - a third, generative object next to
// PRINCIPAL/CLONE, not a manually-played instrument. Gain-only, no mute:
// 0 already means off.
inline const LocalizedText excitationAmount {
    "EXCITAÇÃO - a generative voice that listens to PRINCIPAL/CLONE's own activity and invents pitches from it. 0 = off; higher values react more easily and play louder.",
    "EXCITAÇÃO - uma voz generativa que escuta a atividade do PRINCIPAL/CLONE e inventa alturas a partir dela. 0 = desligada; valores maiores reagem com mais facilidade e tocam mais alto.",
    "EXCITAÇÃO - une voix générative qui écoute l'activité de PRINCIPAL/CLONE et en invente des hauteurs. 0 = éteinte ; des valeurs plus hautes réagissent plus facilement et jouent plus fort.",
    "EXCITAÇÃO - una voz generativa que escucha la actividad de PRINCIPAL/CLONE e inventa alturas a partir de ella. 0 = apagada; valores más altos reaccionan más fácil y suenan más fuerte."
};
inline const LocalizedText filterCvDepth {
    "CV: depth of the scanner's CV over the filter.",
    "CV: profundidade da CV do scanner sobre o filtro.",
    "CV : profondeur de la CV du scanner sur le filtre.",
    "CV: profundidad de la CV del scanner sobre el filtro."
};
inline const LocalizedText envelopeDecay {
    "DEC: fall to the sustained level.",
    "DEC: queda até o nível sustentado.",
    "DEC : chute jusqu'au niveau soutenu.",
    "DEC: caída hasta el nivel sostenido."
};
inline const LocalizedText metricButton {
    "Sets the sequence's grouping and accent.",
    "Define agrupamento e acento da sequência.",
    "Règle le groupement et l'accent de la séquence.",
    "Define la agrupación y el acento de la secuencia."
};
inline const LocalizedText feedbackGain {
    "Dosage of the internal feedback; the output stage keeps the technical limit.",
    "Dosagem da realimentação interna; o estágio de saída mantém o limite técnico.",
    "Dosage de la réinjection interne ; l'étage de sortie maintient la limite technique.",
    "Dosis de la realimentación interna; la etapa de salida mantiene el límite técnico."
};
inline const LocalizedText axisXGeneric {
    "AXIS X: this oscillator's position between left and right.",
    "EIXO X: posição deste oscilador entre esquerda e direita.",
    "AXE X : la position de cet oscillateur entre gauche et droite.",
    "EJE X: la posición de este oscilador entre izquierda y derecha."
};
inline const LocalizedText axisYGeneric {
    "AXIS Y: this oscillator's proximity - blends its sound with a filtered/duller version of itself.",
    "EIXO Y: proximidade deste oscilador - mistura seu som com uma versão filtrada/abafada de si mesmo.",
    "AXE Y : la proximité de cet oscillateur - mélange son son avec une version filtrée/sourde de lui-même.",
    "EJE Y: la proximidad de este oscilador - mezcla su sonido con una versión filtrada/apagada de sí mismo."
};
inline const LocalizedText axisYShort {
    "AXIS Y: proximity/materiality - blends with a filtered/duller version of itself.",
    "EIXO Y: proximidade/materialidade - mistura com uma versão filtrada/abafada de si mesmo.",
    "AXE Y : proximité/matérialité - se mélange avec une version filtrée/sourde de lui-même.",
    "EJE Y: proximidad/materialidad - se mezcla con una versión filtrada/apagada de sí mismo."
};
inline const LocalizedText axisZGeneric {
    "AXIS Z: lets this oscillator drift slightly in tuning and circulate around its X position.",
    "EIXO Z: deixa este oscilador derivar de leve em afinação e circular ao redor da sua posição X.",
    "AXE Z : laisse cet oscillateur dériver légèrement en accord et circuler autour de sa position X.",
    "EJE Z: deja que este oscilador derive levemente en afinación y circule alrededor de su posición X."
};
inline const LocalizedText axisZShort {
    "AXIS Z: orbit/pitch - drifts slightly in tuning and circulates around the X position.",
    "EIXO Z: órbita/altura - deriva de leve em afinação e circula ao redor da posição X.",
    "AXE Z : orbite/hauteur - dérive légèrement en accord et circule autour de la position X.",
    "EJE Z: órbita/altura - deriva levemente en afinación y circula alrededor de la posición X."
};
inline const LocalizedText loopEnd {
    "Loop end: there is always exactly one active return step.",
    "Fim do loop: sempre há uma única etapa de retorno ativa.",
    "Fin de boucle : il y a toujours une seule étape de retour active.",
    "Fin del loop: siempre hay un único paso de retorno activo."
};
inline const LocalizedText flanger {
    "FLANGER: short modulated delay. Depends on the step's FX send.",
    "FLANGER: atraso curto modulado. Depende do envio FX do step.",
    "FLANGER : retard court modulé. Dépend de l'envoi FX du pas.",
    "FLANGER: retardo corto modulado. Depende del envío FX del paso."
};
inline const LocalizedText osc4Freq {
    "FREQ: 4020 division down to near the fundamental (0.03125x to 4x) - at the lowest ratios it sounds like a divided clock pulse, not a note.",
    "FREQ: divisão do 4020 até quase o fundamental (0.03125x a 4x) - nas razões mais baixas soa como um pulso de clock dividido, não uma nota.",
    "FREQ : division du 4020 jusque près du fondamental (0.03125x à 4x) - aux ratios les plus bas, cela sonne comme une impulsion d'horloge divisée, pas une note.",
    "FREQ: división del 4020 hasta casi el fundamental (0.03125x a 4x) - en las razones más bajas suena como un pulso de reloj dividido, no una nota."
};
inline const LocalizedText filterCutoff {
    "FREQ: the filter's cutoff frequency.",
    "FREQ: frequência de corte do filtro.",
    "FREQ : la fréquence de coupure du filtre.",
    "FREQ: la frecuencia de corte del filtro."
};
inline const LocalizedText osc5Freq {
    "FREQ: the 4046's phase detector pulls this osc toward OSC A; the further this ratio moves from 1x, the wider the heterodyne beat against A.",
    "FREQ: o detector de fase do 4046 puxa este osc em direção ao OSC A; quanto mais essa razão se afasta de 1x, mais largo o batimento heteródino contra A.",
    "FREQ : le détecteur de phase du 4046 tire cet osc vers OSC A ; plus ce ratio s'éloigne de 1x, plus le battement hétérodyne contre A s'élargit.",
    "FREQ: el detector de fase del 4046 atrae este osc hacia OSC A; cuanto más se aleja esta razón de 1x, más amplio el batido heterodino contra A."
};
inline const LocalizedText oscFreqGeneric {
    "FREQ: this oscillator's frequency ratio (0.125x to 4x).",
    "FREQ: relação de frequência deste oscilador (0.125x a 4x).",
    "FREQ : le ratio de fréquence de cet oscillateur (0.125x à 4x).",
    "FREQ: la relación de frecuencia de este oscilador (0.125x a 4x)."
};
inline const LocalizedText channelGain {
    "Channel gain.",
    "Ganho do canal.",
    "Gain du canal.",
    "Ganancia del canal."
};
// Clarifies that FILTER/RING sit in series in the signal chain (RING feeds
// FILTER, FILTER feeds ESPAÇO) - turning a channel's own ON/gain off here
// removes only its own contribution to the mix, not its processing further
// down the chain (author, live, 18 ago. 2026: after finding RING OFF still
// audible through FILTER, and FILTER OFF still audible through ESPAÇO -
// "faça isso" after being asked whether to document or change the routing).
inline const LocalizedText filterChannelSeries {
    "ON/gain here only removes FILTER's own contribution to the mix - the filtered signal keeps feeding ESPAÇO (REVERB/PHASER/FLANGER/RESONATOR) even with this channel off.",
    "ON/ganho aqui só remove a contribuição própria do FILTRO na mixagem - o sinal filtrado continua alimentando o ESPAÇO (REVERB/PHASER/FLANGER/RESONATOR) mesmo com este canal desligado.",
    "ON/gain ici ne retire que la contribution propre du FILTER dans le mixage - le signal filtré continue d'alimenter ESPAÇO (REVERB/PHASER/FLANGER/RESONATOR) même si ce canal est désactivé.",
    "ON/ganancia aquí solo quita la contribución propia del FILTER en la mezcla - la señal filtrada sigue alimentando ESPAÇO (REVERB/PHASER/FLANGER/RESONATOR) aunque este canal esté apagado."
};
inline const LocalizedText ringChannelSeries {
    "ON/gain here only removes RING's own contribution to the mix - ring modulation keeps shaping the signal that reaches FILTER even with this channel off.",
    "ON/ganho aqui só remove a contribuição própria do RING na mixagem - a modulação em anel continua atuando sobre o sinal que chega ao FILTRO mesmo com este canal desligado.",
    "ON/gain ici ne retire que la contribution propre du RING dans le mixage - la modulation en anneau continue d'agir sur le signal qui arrive au FILTER même si ce canal est désactivé.",
    "ON/ganancia aquí solo quita la contribución propia del RING en la mezcla - la modulación en anillo sigue actuando sobre la señal que llega al FILTER aunque este canal esté apagado."
};
inline const LocalizedText scopeGain {
    "GANHO Y: amplitude of the waveform drawing on the oscilloscope.",
    "GANHO Y: amplitude do desenho da onda no osciloscópio.",
    "GANHO Y : amplitude du tracé de l'onde sur l'oscilloscope.",
    "GANHO Y: amplitud del dibujo de la onda en el osciloscopio."
};
inline const LocalizedText recordDurationPrefix {
    "Records for ", "Grava por ", "Enregistre pendant ", "Graba durante "
};
inline const LocalizedText recordDurationSuffix {
    " minute(s), stops on its own at the end.",
    " minuto(s) e para sozinho ao final.",
    " minute(s) et s'arrête seul à la fin.",
    " minuto(s) y se detiene solo al final."
};
inline const LocalizedText languageSwitch {
    "Language for TUTORIAL and SOBRE: EN (default), PT, FR, ES.",
    "Idioma do TUTORIAL e do SOBRE: EN (padrão), PT, FR, ES.",
    "Langue du TUTORIAL et du SOBRE : EN (par défaut), PT, FR, ES.",
    "Idioma del TUTORIAL y del SOBRE: EN (predeterminado), PT, FR, ES."
};
// Main panel section labels/titles (not tooltips - the always-visible
// juce::Label text via configureLabel()). Author, 15 ago. 2026: "faça em
// ingles e deixe como padrão, se já fez em portugues mantenha para a
// lingua pt, as outras linguas faça mas se houver palavras compridas
// simplifique a palavra" - English becomes the default/reference text
// (a step beyond tooltips, which already reference EIXO X/Y/Z and FORMA
// as literal printed tokens - EN now translates those to AXIS X/Y/Z and
// SHAPE too, so labels/tooltips/tutorial stay internally consistent in
// EN; PT is unchanged everywhere). PRINCIPAL/CLONE and other proper
// names (ANTITOTEM, JUCE, author's own name) stay identical in every
// language, matching the rest of this file. FR/ES favour a shorter word
// or drop a connector ("DE"/"ENTRE"/"BETWEEN") where the literal
// translation would run noticeably longer than the PT original, since
// this panel's pixel budget was tuned around PT/EN-length text.
} // namespace tooltip
namespace label
{
inline const LocalizedText axisX {
    "AXIS X", "EIXO X", "AXE X", "EJE X"
};
inline const LocalizedText axisY {
    "AXIS Y", "EIXO Y", "AXE Y", "EJE Y"
};
inline const LocalizedText axisZ {
    "AXIS Z", "EIXO Z", "AXE Z", "EJE Z"
};
// Truncated to <=4 letters (18 ago. 2026, author: "máximo 4 letras para
// cada título de knob dos osciladores") - matches FREQ/MIX, the other two
// oscillator knob captions, both already 4 letters or fewer and hardcoded
// (not localized) a few hundred lines up in Main.cpp. Was full "SHAPE"/
// "FORMA"/"FORME"/"FORMA" (5 letters each).
inline const LocalizedText shape {
    "SHP", "FORM", "FORM", "FORM"
};
inline const LocalizedText lfoShape {
    "LFO SHAPE", "FORMA LFO", "FORME LFO", "FORMA LFO"
};
inline const LocalizedText path {
    "PATH", "PERCURSO", "TRAJET", "RUTA"
};
// Was "METER"/"MÉTRICA"/"MESURE"/"COMPÁS" (20 ago. 2026, author: "acho
// que mesure Métrica deve ser mudado para acento (ou acentuada)") - the
// object only ever adds a volume accent on top of the same fixed clock,
// never a real time-signature/meter change (see TAREFAS.md), so the old
// name implied a musical distinction that wasn't there.
inline const LocalizedText meter {
    "ACCENT", "ACENTO", "ACCENT", "ACENTO"
};
// Was "PULSE"/"PULSO" (20 ago. 2026, author: "mudar pulso para
// subdivisão (isso que estamos fazendo não é pulso é subdivisão)") -
// the pulse/beat itself is what CLOCK's own rate controls; this row
// changes how that steady beat divides into smaller units (straight,
// triplet, quintuplet, swing...), which is subdivision, not the pulse.
inline const LocalizedText pulse {
    "SUBDIVISION", "SUBDIVISÃO", "SUBDIVISION", "SUBDIVISIÓN"
};
// International/recognizable term, left unstranslated in all 4 - same
// treatment as GLITCH/SWG (20 ago. 2026, "precisa colocar o título no
// slider").
inline const LocalizedText groove {
    "GROOVE", "GROOVE", "GROOVE", "GROOVE"
};
inline const LocalizedText drift {
    "DRIFT", "DERIVA", "DÉRIVE", "DERIVA"
};
inline const LocalizedText driftDepthLabel {
    "DRIFT · DEPTH", "DERIVA · PROFUNDIDADE", "DÉRIVE · PROFONDEUR", "DERIVA · PROFUNDIDAD"
};
inline const LocalizedText variation {
    "VARIATION", "VARIAÇÃO", "VARIATION", "VARIACIÓN"
};
inline const LocalizedText activeRoutes {
    "ACTIVE ROUTES", "ROTAS ATIVAS", "ROUTES ACTIVES", "RUTAS ACTIVAS"
};
// Umbrella header for the whole 6-column rails band (CAOS/ESPAÇO-FASE/
// ROTAS ATIVAS×3/MATÉRIA), 18 ago. 2026 - author noticed every other
// section of the panel (5 OSC, VCF, ADSR, MODULAÇÃO, FORMA LFO, NOISE,
// ENERGIA) has its own title but this one didn't. Deliberately generic,
// not "filter parameters" or similar - the columns are heterogeneous
// (CAOS is modulation, ESPAÇO/FASE is effects, only MATÉRIA is actually
// filter-related), so a narrower name would misrepresent most of it.
inline const LocalizedText parametersRail {
    "PARAMETERS", "PARÂMETROS", "PARAMÈTRES", "PARÁMETROS"
};
// Header for MaterialFilter's CUTOFF/RESONANCE/DRIVE/ASYMMETRY column,
// joining the ROTAS ATIVAS rail (17 ago. 2026) - short on purpose, this
// column is narrower than its neighbours.
inline const LocalizedText materialRail {
    "MATTER", "MATÉRIA", "MATIÈRE", "MATERIA"
};
// Header for CAOS/VAGA's own DRIVE/DAMPING/DEPTH column (18 ago. 2026),
// joining the ROTAS ATIVAS rail in MODULAÇÃO's old slot - "CAOS" not
// "CAOS/VAGA": short on purpose, matching MATÉRIA's own convention, and
// ChaosField is the more prominent of the two modules it covers.
inline const LocalizedText chaosRail {
    "CHAOS", "CAOS", "CHAOS", "CAOS"
};
inline const LocalizedText feedbackPorts {
    "FEEDBACK PORTS · 0–6 ACTIVE", "PORTAS DE FEEDBACK · 0–6 ATIVAS",
    "PORTES RETOUR · 0–6 ACTIVES", "PUERTAS RETORNO · 0–6 ACTIVAS"
};
inline const LocalizedText spacePhase {
    "SPACE / PHASE", "ESPAÇO / FASE", "ESPACE / PHASE", "ESPACIO / FASE"
};
// Shortened from "VCF · MULTIMODO" (18 ago. 2026, author: "o titulo
// VCF-MULTIMODO e o ADSR estão muito próximos... Só VCF") - the 4 mode
// buttons (LPF/BPF/HPF/NCH) right below already make the multimode
// nature obvious, so the word was redundant, not just long.
inline const LocalizedText vcfMultimode {
    "VCF", "VCF", "VCF", "VCF"
};
inline const LocalizedText coreHeader {
    "CORE: 40106 > 4040 > 4051", "NÚCLEO: 40106 > 4040 > 4051",
    "NOYAU : 40106 > 4040 > 4051", "NÚCLEO: 40106 > 4040 > 4051"
};
inline const LocalizedText footerCredit {
    "© LÚCIO ARAÚJO / ANTITOTEM 2013 · JUCE 2026 · v0.1 · AGPLv3 LICENSE",
    "© LÚCIO ARAÚJO / ANTITOTEM 2013 · JUCE 2026 · v0.1 · LICENÇA AGPLv3",
    "© LÚCIO ARAÚJO / ANTITOTEM 2013 · JUCE 2026 · v0.1 · LICENCE AGPLv3",
    "© LÚCIO ARAÚJO / ANTITOTEM 2013 · JUCE 2026 · v0.1 · LICENCIA AGPLv3"
};
inline const LocalizedText cloneHeading {
    "CLONE — COUPLED TO THE PRINCIPAL OBJECT", "CLONE — ACOPLADO AO OBJETO PRINCIPAL",
    "CLONE — COUPLÉ À L'OBJET PRINCIPAL", "CLONE — ACOPLADO AL OBJETO PRINCIPAL"
};
inline const LocalizedText mixMemory {
    "MIX MEMORY", "MEMÓRIA MIX", "MÉMOIRE MIX", "MEMORIA MIX"
};
inline const LocalizedText objectMix {
    "OBJECT MIXER", "MIXER OBJETOS", "MIXER OBJETS", "MIXER OBJETOS"
};
inline const LocalizedText modePrincipal {
    "MODE: PRINCIPAL", "MODO: PRINCIPAL", "MODE : PRINCIPAL", "MODO: PRINCIPAL"
};
inline const LocalizedText modeClone {
    "MODE: CLONE", "MODO: CLONE", "MODE : CLONE", "MODO: CLONE"
};
inline const LocalizedText modulation {
    "MODULATION", "MODULAÇÃO", "MODULATION", "MODULACIÓN"
};
inline const LocalizedText objectConnection {
    "OBJECT CONNECTION", "CONEXÃO ENTRE OBJETOS", "CONNEXION OBJETS", "CONEXIÓN OBJETOS"
};
inline const LocalizedText recControls {
    "REC CONTROLS", "CONTROLES DE GRAVAÇÃO", "CONTRÔLES REC", "CONTROLES REC"
};
inline const LocalizedText routeToClone {
    "ROUTE PRINCIPAL→CLONE", "ROTA PRINCIPAL→CLONE",
    "ROUTE PRINCIPAL→CLONE", "RUTA PRINCIPAL→CLONE"
};
inline const LocalizedText routeToPrincipal {
    "ROUTE CLONE→PRINCIPAL", "ROTA CLONE→PRINCIPAL",
    "ROUTE CLONE→PRINCIPAL", "RUTA CLONE→PRINCIPAL"
};
inline const LocalizedText energy {
    "ENERGY", "ENERGIA", "ÉNERGIE", "ENERGÍA"
};
inline const LocalizedText resPitch {
    "RES PITCH", "RES ALTURA", "RES HAUT.", "RES ALTURA"
};
inline const LocalizedText resBody {
    "RES BODY", "RES CORPO", "RES CORPS", "RES CUERPO"
};
inline const std::array<LocalizedText, 4> mixerChannelNames {{
    { "FILTER", "FILTRO", "FILTRE", "FILTRO" },
    { "RING", "RING", "RING", "RING" },
    { "NOISE", "NOISE", "NOISE", "NOISE" },
    { "SPACE", "ESPAÇO", "ESPACE", "ESPACIO" }
}};
// Split prefix/suffix, not a fixed "...1 ATIVO" string: the active step
// count is a live juce::String concatenation (loopLabel.setText()),
// same pattern as tooltip::recordDurationPrefix/Suffix above.
inline const LocalizedText loopEndPrefix {
    "LOOP END · ", "FIM DO LOOP · ", "FIN BOUCLE · ", "FIN LOOP · "
};
inline const LocalizedText loopEndSuffix {
    " ACTIVE", " ATIVO", " ACTIVE", " ACTIVA"
};
// Shortened from "5 OSC — FREQ / MIX / FORMA / EIXO X" (18 ago. 2026,
// same reasoning as vcfMultimode above, author's own idea while fixing
// that one - each of those 4 words already labels its own knob under
// every oscillator, so repeating them in the section header was
// redundant, not just long.
inline const LocalizedText oscHeaderTitle {
    "5 OSC", "5 OSC",
    "5 OSC", "5 OSC"
};
} // namespace label
namespace tooltip
{
inline const LocalizedText lfo {
    "LFO: modulation speed, from slow to the audible range.",
    "LFO: velocidade da modulação, do lento à faixa audível.",
    "LFO : vitesse de la modulation, du lent jusqu'à la plage audible.",
    "LFO: velocidad de la modulación, de lo lento hasta el rango audible."
};
inline const LocalizedText mixGeneric {
    "MIX: this oscillator's level in the voice.",
    "MIX: nível deste oscilador na voz.",
    "MIX : le niveau de cet oscillateur dans la voix.",
    "MIX: el nivel de este oscilador en la voz."
};
inline const LocalizedText mixRingProduct {
    "MIX: this oscillator's level in the voice (ring-modulation product with OSC A).",
    "MIX: nível deste oscilador na voz (produto de ring modulation com OSC A).",
    "MIX : le niveau de cet oscillateur dans la voix (produit de ring modulation avec OSC A).",
    "MIX: el nivel de este oscilador en la voz (producto de ring modulation con OSC A)."
};
inline const LocalizedText noiseMod {
    "NOISE: white noise before the filter.",
    "NOISE: ruído branco antes do filtro.",
    "NOISE : bruit blanc avant le filtre.",
    "NOISE: ruido blanco antes del filtro."
};
inline const LocalizedText randomizeSteps {
    "Perturbs CV, dynamics and FX send across the 16 steps within safe bounds.",
    "Perturba CV, dinâmica e envio FX dos 16 steps dentro de limites seguros.",
    "Perturbe CV, dynamique et envoi FX des 16 pas dans des limites sûres.",
    "Perturba CV, dinámica y envío FX de los 16 pasos dentro de límites seguros."
};
inline const LocalizedText phaser {
    "PHASER: moving four-stage network. Depends on the step's FX send.",
    "PHASER: rede móvel de quatro estágios. Depende do envio FX do step.",
    "PHASER : réseau mobile à quatre étages. Dépend de l'envoi FX du pas.",
    "PHASER: red móvil de cuatro etapas. Depende del envío FX del paso."
};
inline const LocalizedText driftDepth {
    "How much the memory can shift steps, OSC relationships, space and routes.",
    "Quanto a memória pode deslocar passos, relações dos OSCs, espaço e rotas.",
    "Combien la mémoire peut déplacer les pas, les relations des OSC, l'espace et les routes.",
    "Cuánto puede desplazar la memoria los pasos, las relaciones de los OSC, el espacio y las rutas."
};
inline const LocalizedText driftDepthPhrase {
    "How much the memory can shift steps, OSC relationships, space and routes. The intensity also transforms between phrases.",
    "Quanto a memória pode deslocar passos, relações dos OSCs, espaço e rotas. A intensidade também se transforma entre frases.",
    "Combien la mémoire peut déplacer les pas, les relations des OSC, l'espace et les routes. L'intensité se transforme aussi entre les phrases.",
    "Cuánto puede desplazar la memoria los pasos, las relaciones de los OSC, el espacio y las rutas. La intensidad también se transforma entre frases."
};
inline const LocalizedText envelopeRelease {
    "REL: release time, including after STOP.",
    "REL: tempo de desligamento, inclusive após STOP.",
    "REL : temps de relâchement, y compris après STOP.",
    "REL: tiempo de apagado, incluso después de STOP."
};
inline const LocalizedText filterResonance {
    "RES: the filter's feedback/resonance.",
    "RES: realimentação/ressonância do filtro.",
    "RES : la réinjection/résonance du filtre.",
    "RES: la realimentación/resonancia del filtro."
};
inline const LocalizedText channelReturn {
    "RET: this material's send back to the voice.",
    "RET: envio desta matéria de volta à voz.",
    "RET : l'envoi de cette matière retour vers la voix.",
    "RET: el envío de esta materia de vuelta a la voz."
};
inline const LocalizedText reverb {
    "REVERB: short memories across multiple taps. Depends on the step's FX send.",
    "REVERB: memórias curtas em múltiplos taps. Depende do envio FX do step.",
    "REVERB : mémoires courtes sur plusieurs taps. Dépend de l'envoi FX du pas.",
    "REVERB: memorias cortas en múltiples taps. Depende del envío FX del paso."
};
inline const LocalizedText ring {
    "RING: continuous multiplication of the voice by the LFO.",
    "RING: multiplicação contínua da voz pelo LFO.",
    "RING : multiplication continue de la voix par le LFO.",
    "RING: multiplicación continua de la voz por el LFO."
};
inline const LocalizedText filterMode {
    "Selects the same VCF route's projection: LPF, BPF, HPF or NOTCH.",
    "Seleciona a projeção da mesma rota VCF: LPF, BPF, HPF ou NOTCH.",
    "Sélectionne la projection de la même route VCF : LPF, BPF, HPF ou NOTCH.",
    "Selecciona la proyección de la misma ruta VCF: LPF, BPF, HPF o NOTCH."
};
inline const LocalizedText materialFilterMix {
    "MAT MIX: an asymmetric saturator stage right after the VCF. 0 leaves the VCF untouched; scales continuously up to fully driven.",
    "MAT MIX: um estágio de saturação assimétrica logo depois do VCF. 0 deixa o VCF intocado; escalona continuamente até totalmente carregado.",
    "MAT MIX : un étage de saturation asymétrique juste après le VCF. 0 laisse le VCF intact ; s'intensifie en continu jusqu'au maximum.",
    "MAT MIX: una etapa de saturación asimétrica justo después del VCF. 0 deja el VCF intacto; escala continuamente hasta el máximo."
};
inline const LocalizedText lfoFreeze {
    "FREEZE: holds CHAOS/WANDER exactly still at its current value. Has no effect on SEN/TRI/PUL/STEP.",
    "FREEZE: segura CAOS/VAGA exatamente parados no valor atual. Sem efeito em SEN/TRI/PUL/STEP.",
    "FREEZE : fige CHAOS/WANDER exactement à leur valeur actuelle. Sans effet sur SEN/TRI/PUL/STEP.",
    "FREEZE: mantiene CHAOS/WANDER exactamente quietos en su valor actual. Sin efecto en SEN/TRI/PUL/STEP."
};
inline const LocalizedText envelopeSustain {
    "SUS: level while the step stays open.",
    "SUS: nível enquanto a etapa permanece aberta.",
    "SUS : niveau tant que le pas reste ouvert.",
    "SUS: nivel mientras el paso permanece abierto."
};
inline const LocalizedText variationHeterodyne {
    "Variation: beats, ring modulation and mixed returns.",
    "Variação: batimentos, ring modulation e retornos mistos.",
    "Variation : battements, ring modulation et retours mixtes.",
    "Variación: batidos, ring modulation y retornos mixtos."
};
inline const LocalizedText variationPendulum {
    "Variation: comb/resonator in the foreground, PERCURSO set to pendulum.",
    "Variação: comb/resonador em primeiro plano, PERCURSO em pêndulo.",
    "Variation : comb/résonateur au premier plan, PERCURSO en pendule.",
    "Variación: comb/resonador en primer plano, PERCURSO en péndulo."
};
inline const LocalizedText variationOrbit {
    "Variation: AXIS Y/Z active on all 5 oscillators - slow drift and spatial circulation.",
    "Variação: EIXO Y/Z ativos nos 5 osciladores - deriva lenta e circulação espacial.",
    "Variation : AXE Y/Z actifs sur les 5 oscillateurs - dérive lente et circulation spatiale.",
    "Variación: EJE Y/Z activos en los 5 osciladores - deriva lenta y circulación espacial."
};
inline const LocalizedText variationPorous {
    "Variation: porous memory, S&H and capacitive returns.",
    "Variação: memória porosa, S&H e retornos capacitivos.",
    "Variation : mémoire poreuse, S&H et retours capacitifs.",
    "Variación: memoria porosa, S&H y retornos capacitivos."
};
inline const LocalizedText variationPulse {
    "Variation: eight gates, pulses and little ambience.",
    "Variação: oito gates, pulsos e pouca ambiência.",
    "Variation : huit gates, impulsions et peu d'ambiance.",
    "Variación: ocho gates, pulsos y poca ambiencia."
};
inline const LocalizedText channelPan {
    "X: the channel's lateral position.",
    "X: posição lateral do canal.",
    "X : la position latérale du canal.",
    "X: la posición lateral del canal."
};
inline const LocalizedText stepFx {
    "FX: this step's send to reverb, phaser and flanger.",
    "FX: envio desta etapa para reverb, phaser e flanger.",
    "FX : l'envoi de ce pas vers reverb, phaser et flanger.",
    "FX: el envío de este paso a reverb, phaser y flanger."
};
inline const LocalizedText stepMute {
    "Mutes this step without erasing its controls.",
    "Silencia esta etapa sem apagar seus controles.",
    "Coupe ce pas sans effacer ses réglages.",
    "Silencia este paso sin borrar sus controles."
};
inline const std::array<LocalizedText, 8> clockFeelTips {{
    { "Straight pulse.", "Pulso reto.", "Impulsion droite.", "Pulso recto." },
    { "Triplets: three steps in the space of two.", "Tercinas: três passos no espaço de dois.",
      "Triolets : trois pas dans l'espace de deux.", "Tresillos: tres pasos en el espacio de dos." },
    { "Quintuplets: five steps in the space of four.", "Quintinas: cinco passos no espaço de quatro.",
      "Quintolets : cinq pas dans l'espace de quatre.", "Quintillos: cinco pasos en el espacio de cuatro." },
    { "Swing: a long-short shuffle, not a flat tuplet speed (a plain sextuplet ratio would sound identical to triplet).",
      "Swing: um shuffle longo-curto, não uma fração fixa (uma sextina simples soaria idêntica à tercina).",
      "Swing : un shuffle long-court, pas une fraction fixe (une sextolet simple sonnerait comme un triolet).",
      "Swing: un shuffle largo-corto, no una fracción fija (una sextillo simple sonaría idéntico al tresillo)." },
    { "Septuplets: seven steps in the space of four.", "Septinas: sete passos no espaço de quatro.",
      "Septolets : sept pas dans l'espace de quatre.", "Septillos: siete pasos en el espacio de cuatro." },
    { "Nonuplets: nine steps in the space of eight.", "Noninas: nove passos no espaço de oito.",
      "Nonolets : neuf pas dans l'espace de huit.", "Nonillos: nueve pasos en el espacio de ocho." },
    { "Undecuplets: eleven steps in the space of eight.", "Onzinas: onze passos no espaço de oito.",
      "Onzolets : onze pas dans l'espace de huit.", "Oncillos: once pasos en el espacio de ocho." },
    { "Glitch: repeatable rhythmic instability.", "Glitch: instabilidade rítmica repetível.",
      "Glitch : instabilité rythmique reproductible.", "Glitch: inestabilidad rítmica repetible." }
}};
inline const std::array<LocalizedText, 4> scannerTips {{
    { "Forward scanner: 1 to the loop's end.", "Scanner em frente: 1 até o fim do loop.",
      "Scanner en avant : 1 jusqu'à la fin de la boucle.", "Scanner hacia adelante: 1 hasta el fin del loop." },
    { "Reverse scanner: loop's end down to 1.", "Scanner reverso: fim do loop até 1.",
      "Scanner en arrière : de la fin de la boucle jusqu'à 1.", "Scanner reverso: del fin del loop hasta 1." },
    { "Alternating scanner: back-and-forth between the loop's edges.",
      "Scanner alternado: vai-e-volta entre as bordas do loop.",
      "Scanner alterné : va-et-vient entre les bords de la boucle.",
      "Scanner alternado: va y viene entre los bordes del loop." },
    { "Memory address: pseudo-random path, deterministic and without immediate repetition.",
      "Endereço por memória: percurso pseudoaleatório, determinístico e sem repetição imediata.",
      "Adresse par mémoire : parcours pseudo-aléatoire, déterministe et sans répétition immédiate.",
      "Dirección por memoria: recorrido pseudoaleatorio, determinístico y sin repetición inmediata." }
}};
inline const std::array<LocalizedText, 6> lfoShapeTips {{
    { "Sine: continuous, circular variation.", "Seno: variação contínua e circular.",
      "Sinus : variation continue et circulaire.", "Seno: variación continua y circular." },
    { "Triangle: linear rise and fall.", "Triângulo: subida e descida lineares.",
      "Triangle : montée et descente linéaires.", "Triángulo: subida y bajada lineales." },
    { "Pulse: abrupt switching, useful for gates and cuts.", "Pulso: comutação abrupta, útil para gates e cortes.",
      "Impulsion : commutation abrupte, utile pour les gates et les coupures.",
      "Pulso: conmutación abrupta, útil para gates y cortes." },
    { "Chaos: double-well oscillator, settles into or swings between two states.",
      "Caos: oscilador de dois poços, se assenta ou alterna entre dois estados.",
      "Chaos : oscillateur à double puits, se stabilise ou oscille entre deux états.",
      "Caos: oscilador de dos pozos, se asienta o alterna entre dos estados." },
    { "Wander: a smoothed random walk, continuous and non-repeating.",
      "Vaga: uma caminhada aleatória suavizada, contínua e não-repetitiva.",
      "Vagabonde : une marche aléatoire lissée, continue et non répétitive.",
      "Vaga: una caminata aleatoria suavizada, continua y no repetitiva." },
    { "Step: sample & hold, a fresh random value once per cycle.",
      "Degrau: sample & hold, um novo valor aleatório a cada ciclo.",
      "Palier : échantillonnage-blocage, une nouvelle valeur aléatoire par cycle.",
      "Escalón: sample & hold, un nuevo valor aleatorio por ciclo." }
}};
inline const std::array<LocalizedText, 3> coreTips {{
    { "Threshold and pulse", "Limiar e pulso", "Seuil et impulsion", "Umbral y pulso" },
    { "Continuous shapes", "Formas contínuas", "Formes continues", "Formas continuas" },
    { "Drift and gain", "Deriva e ganho", "Dérive et gain", "Deriva y ganancia" }
}};
inline const std::array<LocalizedText, 6> noiseColourNames {{
    { "White noise", "Ruído branco", "Bruit blanc", "Ruido blanco" },
    { "Pink noise", "Ruído rosa", "Bruit rose", "Ruido rosa" },
    { "Brown noise", "Ruído marrom", "Bruit marron", "Ruido marrón" },
    { "Blue noise", "Ruído azul", "Bruit bleu", "Ruido azul" },
    { "Violet noise", "Ruído violeta", "Bruit violet", "Ruido violeta" },
    { "Bit noise", "Ruído bit", "Bruit bit", "Ruido bit" }
}};
// The 16 PARÂMETROS/ROTAS ATIVAS sliders (detailControls in Main.cpp),
// same order as detailNames there - S&H RATE/RVB RET/PHS RATE/PHS PROF/
// FLG RATE/FLG PROF/RES MIX/RES ALTURA/RES CORPO/CUTOFF/RESON/DRIVE/
// ASYM (MATÉRIA)/DRIVE/DAMPING (CAOS)/DEPTH (VAGA). None of these had a
// tooltip before 18 ago. 2026 (author: "crie os conteudos tooltip para
// todo os itens que não tem ainda", found via LEARN itself showing
// stale text on them - "vários knobs não tem conteúdo no learn").
inline const std::array<LocalizedText, 16> detailControlTips {{
    { "How often sample & hold resamples the noise.",
      "Com que frequência o sample & hold reamostra o ruído.",
      "À quelle fréquence le sample & hold ré-échantillonne le bruit.",
      "Con qué frecuencia el sample & hold vuelve a muestrear el ruido." },
    { "Reverb's own feedback - how much of its tail returns.",
      "Feedback próprio do reverb - quanto da cauda retorna.",
      "Feedback propre à la reverb - la part de sa traîne qui revient.",
      "Feedback propio del reverb - cuánto de su cola regresa." },
    { "Phaser's own sweep speed.", "Velocidade de varredura própria do phaser.",
      "Vitesse de balayage propre au phaser.", "Velocidad de barrido propia del phaser." },
    { "Phaser's own sweep depth.", "Profundidade da varredura do phaser.",
      "Profondeur du balayage du phaser.", "Profundidad del barrido del phaser." },
    { "Flanger's own sweep speed.", "Velocidade de varredura própria do flanger.",
      "Vitesse de balayage propre au flanger.", "Velocidad de barrido propia del flanger." },
    { "Flanger's own sweep depth.", "Profundidade da varredura do flanger.",
      "Profondeur du balayage du flanger.", "Profundidad del barrido del flanger." },
    { "How present the comb/resonator's own ring is in the mix.",
      "O quanto o anel do comb/ressonador está presente na mixagem.",
      "La présence de l'anneau du comb/résonateur dans le mixage.",
      "Cuánto presente está el anillo del comb/resonador en la mezcla." },
    { "The comb/resonator's own pitch - a real note.",
      "A altura própria do comb/ressonador - uma nota real.",
      "La hauteur propre du comb/résonateur - une vraie note.",
      "La altura propia del comb/resonador - una nota real." },
    { "How long the resonator keeps ringing.",
      "Por quanto tempo o ressonador continua soando.",
      "Combien de temps le résonateur continue de sonner.",
      "Cuánto tiempo sigue sonando el resonador." },
    { "MATÉRIA's own cutoff - like VCF's FREQ.",
      "O corte próprio da MATÉRIA - como o FREQ do VCF.",
      "La coupure propre de MATÉRIA - comme FREQ pour VCF.",
      "El corte propio de MATÉRIA - como FREQ del VCF." },
    { "MATÉRIA's own resonance - drives its saturator.",
      "A ressonância própria da MATÉRIA - alimenta o saturador.",
      "La résonance propre de MATÉRIA - alimente son saturateur.",
      "La resonancia propia de MATÉRIA - alimenta su saturador." },
    { "How hard MATÉRIA's own saturator is driven.",
      "O quanto o saturador próprio da MATÉRIA é levado.",
      "À quel point le saturateur propre de MATÉRIA est poussé.",
      "Cuánto se empuja el saturador propio de MATÉRIA." },
    { "How lopsided MATÉRIA's saturation is.",
      "O quão desigual é a saturação da MATÉRIA.",
      "À quel point la saturation de MATÉRIA est déséquilibrée.",
      "Cuán desigual es la saturación de MATÉRIA." },
    { "CAOS's own pull toward its two states.",
      "A força de atração própria do CAOS entre os dois estados.",
      "La force d'attraction propre de CAOS vers ses deux états.",
      "La fuerza de atracción propia de CAOS hacia sus dos estados." },
    { "How much energy CAOS loses per step.",
      "Quanta energia o CAOS perde a cada passo.",
      "L'énergie perdue par CAOS à chaque pas.",
      "Cuánta energía pierde CAOS en cada paso." },
    { "How far VAGA's own target can land from centre.",
      "Até onde o alvo próprio da VAGA pode cair do centro.",
      "Jusqu'où la cible propre de VAGA peut tomber du centre.",
      "Hasta dónde puede caer el objetivo propio de VAGA del centro." }
}};
// Remaining gaps found live via LEARN itself (author: "explicar no
// learn o osciloscópio, o log, o clock, etc" / "conexão entre objetos
// nada reconhece no learn" / "botoes m e s do mixer") - kept to 4 lines
// or less each in the LEARN box.
inline const LocalizedText scopeTrace {
    "Live L/R waveform of the real output, not an animation.",
    "Forma de onda L/R ao vivo da saída real, não uma animação.",
    "Forme d'onde L/R en direct de la sortie réelle, pas une animation.",
    "Forma de onda L/R en vivo de la salida real, no una animación."
};
inline const LocalizedText activityLog {
    "A rolling history of recent events.",
    "Um histórico rolante dos eventos recentes.",
    "Un historique déroulant des événements récents.",
    "Un historial rodante de los eventos recientes."
};
inline const LocalizedText clockRateKnob {
    "The master clock rate - how fast the sequencer advances through its steps.",
    "O ritmo do clock mestre - a velocidade com que o sequenciador avança pelos passos.",
    "Le rythme de l'horloge maîtresse - la vitesse à laquelle le séquenceur avance dans ses pas.",
    "El ritmo del reloj maestro - la velocidad con la que el secuenciador avanza por los pasos."
};
// Was gated to only SWG (20 ago. 2026) - author: "deixa o swing somente
// enquanto botão, e utilise esse slide atual do swing para o groove" -
// this became a general long-short modifier layered on every SUBDIVISÃO
// feel, not exclusive to SWG anymore.
inline const LocalizedText grooveAmount {
    "GROOVE - a long-short feel layered on every SUBDIVISÃO option (0 = none, 1 = strongest).",
    "GROOVE - uma sensação longo-curto sobre qualquer opção de SUBDIVISÃO (0 = nenhuma, 1 = mais forte).",
    "GROOVE - une sensation long-court sur toute option de SUBDIVISÃO (0 = aucune, 1 = la plus forte).",
    "GROOVE - una sensación largo-corto sobre cualquier opción de SUBDIVISÃO (0 = ninguna, 1 = la más fuerte)."
};
inline const LocalizedText gainToClone {
    "How much of PRINCIPAL's own output feeds back into CLONE.",
    "O quanto da própria saída de PRINCIPAL realimenta o CLONE.",
    "La part de la sortie propre de PRINCIPAL qui se réinjecte dans CLONE.",
    "Cuánto de la propia salida de PRINCIPAL se realimenta en CLONE."
};
inline const LocalizedText gainToPrincipal {
    "How much of CLONE's own output feeds back into PRINCIPAL.",
    "O quanto da própria saída de CLONE realimenta o PRINCIPAL.",
    "La part de la sortie propre de CLONE qui se réinjecte dans PRINCIPAL.",
    "Cuánto de la propia salida de CLONE se realimenta en PRINCIPAL."
};
inline const LocalizedText auxToPrincipal {
    "A quieter path into PRINCIPAL - a hidden oscillator drifting with CLONE's loudness.",
    "Um caminho mais discreto até PRINCIPAL - um oscilador derivando com o volume do CLONE.",
    "Un chemin plus discret vers PRINCIPAL - un oscillateur dérivant avec le volume de CLONE.",
    "Un camino más discreto hacia PRINCIPAL - un oscilador derivando con el volumen de CLONE."
};
inline const LocalizedText auxToCloneObject {
    "A quieter path into CLONE - a hidden oscillator drifting with PRINCIPAL's loudness.",
    "Um caminho mais discreto até CLONE - um oscilador derivando com o volume do PRINCIPAL.",
    "Un chemin plus discret vers CLONE - un oscillateur dérivant avec le volume de PRINCIPAL.",
    "Un camino más discreto hacia CLONE - un oscilador derivando con el volumen de PRINCIPAL."
};
// DIRETO/DIODO/CAP/PULSO between objects - same 4-way vocabulary as the
// internal PORTAS DE FEEDBACK (feedbackDoorTips above), just reused for
// both route rows (PRINCIPAL→CLONE and CLONE→PRINCIPAL): the row's own
// heading above it already says which direction, so one shared 4-entry
// array covers both instead of duplicating the same four descriptions.
inline const std::array<LocalizedText, 4> objectRouteTips {{
    { "Sends the raw sample along this route.",
      "Envia a amostra crua por esta rota.",
      "Envoie l'échantillon brut sur cette route.",
      "Envía la muestra cruda por esta ruta." },
    { "Half-wave rectifies the signal along this route.",
      "Retifica o sinal em meia-onda nesta rota.",
      "Redresse le signal en demi-onde sur cette route.",
      "Rectifica la señal en media onda en esta ruta." },
    { "Sends the capacitor's slow average, not the instant sample.",
      "Envia a média lenta do capacitor, não a amostra instantânea.",
      "Envoie la moyenne lente du capacitor, pas l'échantillon instantané.",
      "Envía el promedio lento del capacitor, no la muestra instantánea." },
    { "Collapses the signal to a bare square along this route.",
      "Reduz o sinal a um quadrado puro nesta rota.",
      "Réduit le signal à un simple carré sur cette route.",
      "Reduce la señal a un cuadrado puro en esta ruta." }
}};
inline const LocalizedText noiseChannelSeries {
    "Turns RUÍDO on/off - NOISE SEND goes silent when this is off.",
    "Liga/desliga RUÍDO - NOISE SEND fica mudo quando isso está desligado.",
    "Active/désactive RUÍDO - NOISE SEND devient muet quand c'est désactivé.",
    "Activa/desactiva RUÍDO - NOISE SEND queda mudo cuando esto está apagado."
};
inline const LocalizedText spaceChannelSeries {
    "Turns ESPAÇO on/off - REVERB/PHASER/FLANGER/RESONATOR pass through it.",
    "Liga/desliga ESPAÇO - REVERB/PHASER/FLANGER/RESONATOR passam por ele.",
    "Active/désactive ESPAÇO - REVERB/PHASER/FLANGER/RESONATOR y passent.",
    "Activa/desactiva ESPAÇO - REVERB/PHASER/FLANGER/RESONATOR pasan por él."
};
inline const LocalizedText mixChannelMute {
    "Removes only this channel's own contribution to the mix.",
    "Remove só a contribuição própria deste canal na mixagem.",
    "Retire seulement la contribution propre de ce canal au mixage.",
    "Quita solo la contribución propia de este canal en la mezcla."
};
inline const LocalizedText mixChannelSolo {
    "Mutes every other channel, keeping only this one's own contribution audible.",
    "Silencia todos os outros canais, deixando audível só a contribuição própria deste.",
    "Coupe tous les autres canaux, ne laissant audible que la contribution propre de celui-ci.",
    "Silencia todos los demás canales, dejando audible solo la contribución propia de este."
};
// Section/object titles, not individual controls - author, live: "os
// títulos támbém precisam de dicas de learn (titulos de knobs, titulos
// de objetos)". Kept to 4 lines or less like everything else in the
// LEARN box.
inline const LocalizedText voiceHeaderTip {
    "Five oscillator columns - each shaping its own FREQ/MIX/FORM and spatial behaviour.",
    "Cinco colunas de osciladores - cada uma moldando seu próprio FREQ/MIX/FORMA e comportamento espacial.",
    "Cinq colonnes d'oscillateurs - chacune façonnant son propre FREQ/MIX/FORME et comportement spatial.",
    "Cinco columnas de osciladores - cada una moldeando su propio FREQ/MIX/FORMA y comportamiento espacial."
};
inline const LocalizedText vcfHeaderTip {
    "The clean multimode filter - LPF/BPF/HPF/NOTCH, shared by the whole oscillator sum.",
    "O filtro multimodo limpo - LPF/BPF/HPF/NOTCH, compartilhado pela soma inteira dos osciladores.",
    "Le filtre multimode propre - LPF/BPF/HPF/NOTCH, partagé par toute la somme des oscillateurs.",
    "El filtro multimodo limpio - LPF/BPF/HPF/NOTCH, compartido por toda la suma de los osciladores."
};
inline const LocalizedText adsrHeaderTip {
    "The instrument's own VCA - shapes the amplitude envelope of every step.",
    "O próprio VCA do instrumento - molda a envolvente de amplitude de cada step.",
    "Le VCA propre de l'instrument - façonne l'enveloppe d'amplitude de chaque pas.",
    "El propio VCA del instrumento - moldea la envolvente de amplitud de cada paso."
};
inline const LocalizedText stepsHeaderTip {
    "The 16-step sequence - each step's own CV/AMP/FX/MUTE.",
    "A sequência de 16 steps - o próprio CV/AMP/FX/MUTE de cada um.",
    "La séquence de 16 pas - le propre CV/AMP/FX/MUTE de chacun.",
    "La secuencia de 16 pasos - el propio CV/AMP/FX/MUTE de cada uno."
};
inline const LocalizedText noiseHeaderTip {
    "Six noise colours plus S&H, injected right after ADSR.",
    "Seis cores de ruído mais S&H, injetadas logo após o ADSR.",
    "Six couleurs de bruit plus S&H, injectées juste après ADSR.",
    "Seis colores de ruido más S&H, inyectadas justo después del ADSR."
};
inline const LocalizedText modulationHeaderTip {
    "The shared LFO (FORMA) and RING - read by whatever taps them downstream.",
    "O LFO compartilhado (FORMA) e o RING - lidos por quem os capta adiante na cadeia.",
    "Le LFO partagé (FORMA) et RING - lus par ce qui les capte en aval.",
    "El LFO compartido (FORMA) y RING - leídos por lo que los capta más adelante."
};
inline const LocalizedText effectsHeaderTip {
    "REVERB, PHASER, FLANGER and the comb/resonator - the space stage of the chain.",
    "REVERB, PHASER, FLANGER e o comb/ressonador - o estágio de espaço da cadeia.",
    "REVERB, PHASER, FLANGER et le comb/résonateur - l'étage d'espace de la chaîne.",
    "REVERB, PHASER, FLANGER y el comb/resonador - la etapa de espacio de la cadena."
};
inline const LocalizedText detailHeaderTip {
    "Extra parameters for the effects and modulation above, gated by FX per step.",
    "Parâmetros extras dos efeitos e modulação acima, controlados por FX a cada step.",
    "Paramètres supplémentaires des effets et de la modulation ci-dessus, contrôlés par FX à chaque pas.",
    "Parámetros extra de los efectos y modulación de arriba, controlados por FX en cada paso."
};
inline const LocalizedText parametersHeaderTip {
    "This object's own extra parameters - effects, MATÉRIA and CAOS/VAGA.",
    "Os parâmetros extras próprios deste objeto - efeitos, MATÉRIA e CAOS/VAGA.",
    "Les paramètres supplémentaires propres à cet objet - effets, MATÉRIA et CAOS/VAGA.",
    "Los parámetros extra propios de este objeto - efectos, MATÉRIA y CAOS/VAGA."
};
inline const LocalizedText materialHeaderTip {
    "MATÉRIA's own extra parameters - a second, saturating filter, separate from VCF.",
    "Os parâmetros extras da própria MATÉRIA - um segundo filtro, saturador, separado do VCF.",
    "Les paramètres supplémentaires propres à MATÉRIA - un second filtre, saturant, séparé de VCF.",
    "Los parámetros extra propios de MATÉRIA - un segundo filtro, saturador, separado de VCF."
};
inline const LocalizedText chaosHeaderTip {
    "CAOS and VAGA's own extra parameters - two non-periodic FORMA LFO shapes.",
    "Os parâmetros extras próprios de CAOS e VAGA - duas formas não periódicas do FORMA LFO.",
    "Les paramètres supplémentaires propres à CAOS et VAGA - deux formes non périodiques de FORMA LFO.",
    "Los parámetros extra propios de CAOS y VAGA - dos formas no periódicas de FORMA LFO."
};
inline const LocalizedText mixMemoryHeaderTip {
    "Up to 4 full mixer snapshots - CAPTURAR then a slot to save, click a slot to recall.",
    "Até 4 fotografias completas do mixer - CAPTURAR e depois um slot pra salvar, clique pra recuperar.",
    "Jusqu'à 4 instantanés complets du mixer - CAPTURAR puis un emplacement pour enregistrer, cliquez pour rappeler.",
    "Hasta 4 fotografías completas del mezclador - CAPTURAR y luego un slot para guardar, clic para recuperar."
};
inline const LocalizedText objectMixHeaderTip {
    "PRINCIPAL and CLONE's own volume and mute in the final mix.",
    "O volume e o mute próprios de PRINCIPAL e CLONE na mixagem final.",
    "Le volume et le mute propres à PRINCIPAL et CLONE dans le mixage final.",
    "El volumen y el mute propios de PRINCIPAL y CLONE en la mezcla final."
};
inline const LocalizedText objectConnectionHeaderTip {
    "How PRINCIPAL and CLONE feed back into each other.",
    "Como PRINCIPAL e CLONE se realimentam um ao outro.",
    "Comment PRINCIPAL et CLONE se réinjectent l'un dans l'autre.",
    "Cómo PRINCIPAL y CLONE se realimentan uno al otro."
};
inline const LocalizedText recordDurationsHeaderTip {
    "Pick a duration to start REC automatically, with a live countdown.",
    "Escolha uma duração para o REC disparar sozinho, com contagem regressiva ao vivo.",
    "Choisissez une durée pour que REC se déclenche seul, avec un compte à rebours en direct.",
    "Elige una duración para que REC se dispare solo, con cuenta regresiva en vivo."
};
inline const LocalizedText feedbackHeaderTip {
    "Up to 6 ports shaping the voice's own last sample before feeding it back in.",
    "Até 6 portas moldando a última amostra da própria voz antes de realimentá-la.",
    "Jusqu'à 6 ports façonnant le dernier échantillon propre de la voix avant de le réinjecter.",
    "Hasta 6 puertos moldeando la última muestra propia de la voz antes de reintroducirla."
};
inline const LocalizedText loopHeaderTip {
    "How many of the 16 steps play before the sequence repeats.",
    "Quantos dos 16 steps tocam antes da sequência repetir.",
    "Combien des 16 pas jouent avant que la séquence ne se répète.",
    "Cuántos de los 16 pasos suenan antes de que la secuencia se repita."
};
inline const LocalizedText variationHeaderTip {
    "Six coherent presets, each retuning a bundle of parameters at once.",
    "Seis presets coerentes, cada um reajustando um conjunto de parâmetros de uma vez.",
    "Six presets cohérents, chacun réajustant un ensemble de paramètres d'un coup.",
    "Seis presets coherentes, cada uno reajustando un conjunto de parámetros de una vez."
};
inline const LocalizedText deriveHeaderTip {
    "How strongly DERIVA shifts the captured memory over time.",
    "O quanto a DERIVA desloca a memória capturada ao longo do tempo.",
    "À quel point DERIVA déplace la mémoire capturée avec le temps.",
    "Cuánto DERIVA desplaza la memoria capturada con el tiempo."
};
// Updated 20 ago. 2026 - 8 feels now (straight, 5 real tuplets, SWG,
// glitch), not the old 4; see the array-size comments in Main.cpp's own
// updateTemporal()/syncTemporal() for the full history.
inline const LocalizedText temporalHeaderTip {
    "The clock's own rhythmic feel - straight, a tuplet, swing or glitch.",
    "O sabor rítmico próprio do clock - reto, um tuplet, swing ou glitch.",
    "La sensation rythmique propre de l'horloge - droit, un tuplet, swing ou glitch.",
    "La sensación rítmica propia del reloj - recto, un tuplet, swing o glitch."
};
// Was "Works with PULSO to shape the clock's own rhythmic feel" (20 ago.
// 2026) - factually wrong, same conflation as the tutorial's old "PULSO/
// MÉTRICA" phrasing: ACENTO never touches timing/feel, that's PULSO alone
// (see samplesPerStep() vs the metricAccent gain multiplier in
// SimpleSequencer.cpp - two independent mechanisms).
inline const LocalizedText metricHeaderTip {
    "Adds a volume accent every N steps - independent of SUBDIVISÃO's timing feel.",
    "Adiciona um acento de volume a cada N passos - independente do sabor rítmico da SUBDIVISÃO.",
    "Ajoute un accent de volume tous les N pas - indépendant de la sensation rythmique de SUBDIVISÃO.",
    "Añade un acento de volumen cada N pasos - independiente de la sensación rítmica de SUBDIVISÃO."
};
inline const LocalizedText scannerHeaderTip {
    "The scanner's own direction - forward, reverse, alternating or memory.",
    "A direção própria do scanner - em frente, reverso, alternado ou memória.",
    "La direction propre du scanner - en avant, inversé, alterné ou mémoire.",
    "La dirección propia del scanner - hacia adelante, reversa, alternada o memoria."
};
} // namespace tooltip
// Button captions (not tooltips/labels - the actual juce::Button text).
// Author, 15 ago. 2026, after the label pass above: "precisa traduzir os
// botões também (ex. deriva, pulso, porosa, pendulo, direto, etc)". Same
// rules as label:: - EN is the reference, PT unchanged, FR/ES shortened
// where the literal translation runs long. Short technical abbreviations
// already shared across languages (FB/DIODE/CAP/PULSE/TRANS/REFLUX, FWD/
// REV/ALT/MEM, ON/M/S, RND 16, M1-M4, TUTORIAL, PLAY/STOP/RESET/REC, MIX)
// were already English/international and needed no entry here.
namespace button
{
inline const LocalizedText variationPulse {
    "PULSE", "PULSO", "PULSE", "PULSO"
};
inline const LocalizedText variationPorous {
    "POROUS", "POROSA", "POREUSE", "POROSA"
};
inline const LocalizedText variationHeterodyne {
    "HETERODYNE", "HETERÓDINA", "HÉTÉRODYNE", "HETERODINA"
};
inline const LocalizedText variationOrbit {
    "ORBIT", "ÓRBITA", "ORBITE", "ÓRBITA"
};
inline const LocalizedText variationPendulum {
    "PENDULUM", "PÊNDULO", "PENDULE", "PÉNDULO"
};
inline const LocalizedText capture {
    "CAPTURE", "CAPTURAR", "CAPTURER", "CAPTURAR"
};
inline const LocalizedText routeDirect {
    "DIRECT", "DIRETO", "DIRECT", "DIRECTO"
};
inline const LocalizedText routeDiode {
    "DIODE", "DIODO", "DIODE", "DIODO"
};
inline const LocalizedText routePulse {
    "PULSE", "PULSO", "PULSE", "PULSO"
};
inline const LocalizedText soundPage {
    "SOUND", "SOM", "SON", "SONIDO"
};
inline const LocalizedText sequencePage {
    "SEQUENCE", "SEQUÊNCIA", "SÉQUENCE", "SECUENCIA"
};
inline const LocalizedText about {
    "ABOUT", "SOBRE", "INFOS", "ACERCA"
};
inline const LocalizedText oneMonitor {
    "1 MONITOR", "1 MONITOR", "1 MONITEUR", "1 MONITOR"
};
inline const LocalizedText twoMonitors {
    "2 MONITORS", "2 MONITORES", "2 MONITEURS", "2 MONITORES"
};
} // namespace button
// LOG box - a separate, persistent header (author, 15 ago. 2026: "percebi
// também que não há título para o objeto log") plus the box's own opening
// diagnostic lines and every appendLog() entry. PRINCIPAL/CLONE, SCANNER,
// FEEDBACK stay literal like everywhere else in this file.
namespace logText
{
inline const LocalizedText title {
    "LOG / SOUND OBJECT", "LOG / OBJETO SONORO", "LOG / OBJET SONORE", "LOG / OBJETO SONORO"
};
inline const LocalizedText diagnosticLines {
    "UTF-8: explicit route active\nFONT: stable monospace\nAUDIO: waiting for PLAY",
    "UTF-8: rota explícita ativa\nFONTE: Monospace estável\nÁUDIO: aguardando PLAY",
    "UTF-8 : route explicite active\nPOLICE : monospace stable\nAUDIO : en attente de PLAY",
    "UTF-8: ruta explícita activa\nFUENTE: monoespaciada estable\nAUDIO: esperando PLAY"
};
inline const LocalizedText rnd16 {
    "RND 16 · CV, dynamics and FX reshuffled", "RND 16 · CV, dinâmica e FX reorganizados",
    "RND 16 · CV, dynamique et FX réorganisés", "RND 16 · CV, dinámica y FX reorganizados"
};
inline const LocalizedText driftCaptured {
    "DRIFT · phrase memory captured", "DERIVA · memória de frase capturada",
    "DÉRIVE · mémoire de phrase capturée", "DERIVA · memoria de frase capturada"
};
inline const LocalizedText driftResting {
    "DRIFT · memory at rest", "DERIVA · memória em repouso",
    "DÉRIVE · mémoire au repos", "DERIVA · memoria en reposo"
};
inline const LocalizedText mixMemoryPrefix {
    "MIX MEMORY · slot ", "MEMÓRIA MIX · slot ", "MÉMOIRE MIX · emplacement ", "MEMORIA MIX · slot "
};
inline const LocalizedText mixMemoryCapturedSuffix {
    " captured", " capturado", " capturé", " capturado"
};
inline const LocalizedText mixMemoryRecalledSuffix {
    " recalled", " recuperado", " rappelé", " recuperado"
};
inline const LocalizedText playLog {
    "PLAY · time flow running", "PLAY · fluxo temporal em curso",
    "PLAY · flux temporel en cours", "PLAY · flujo temporal en curso"
};
inline const LocalizedText stopLog {
    "STOP · safe fade applied", "STOP · queda segura aplicada",
    "STOP · chute sécurisée appliquée", "STOP · caída segura aplicada"
};
inline const LocalizedText recCancelledByStop {
    "REC · cancelled by transport STOP · step 1 never arrived",
    "REC · cancelado pelo STOP do transporte · step 1 nunca chegou",
    "REC · annulé par le STOP du transport · step 1 jamais atteint",
    "REC · cancelado por el STOP del transporte · step 1 nunca llegó"
};
inline const LocalizedText recFinishedByStop {
    "REC · finished by transport STOP · loop couldn't complete",
    "REC · finalizado pelo STOP do transporte · loop não pôde terminar",
    "REC · terminé par le STOP du transport · boucle incomplète",
    "REC · finalizado por el STOP del transporte · loop no pudo terminar"
};
inline const LocalizedText resetLog {
    "RESET · scanner restarted", "RESET · scanner reiniciado",
    "RESET · scanner redémarré", "RESET · scanner reiniciado"
};
inline const LocalizedText recArmedWaiting {
    "REC · armed · waiting for step 1 of the PRINCIPAL sequencer",
    "REC · armado · aguardando step 1 do sequenciador principal",
    "REC · armé · en attente du step 1 du séquenceur PRINCIPAL",
    "REC · armado · esperando el step 1 del secuenciador PRINCIPAL"
};
inline const LocalizedText recStopRequested {
    "REC · stop requested · waiting for the PRINCIPAL loop to end",
    "REC · parada solicitada · aguardando fim do loop principal",
    "REC · arrêt demandé · en attente de la fin de la boucle PRINCIPAL",
    "REC · parada solicitada · esperando el fin del loop PRINCIPAL"
};
inline const LocalizedText recCancelledBeforeStart {
    "REC · cancelled before starting · step 1 never arrived",
    "REC · cancelado antes do início · step 1 nunca chegou",
    "REC · annulé avant le départ · step 1 jamais atteint",
    "REC · cancelado antes de empezar · step 1 nunca llegó"
};
inline const LocalizedText recArmedPrefix {
    "REC · armed (", "REC · armado (", "REC · armé (", "REC · armado ("
};
inline const LocalizedText recArmedSuffix {
    " min) · waiting for step 1", " min) · aguardando step 1",
    " min) · en attente du step 1", " min) · esperando el step 1"
};
inline const LocalizedText driftPhrasePrefix {
    "DRIFT · phrase ", "DERIVA · frase ", "DÉRIVE · phrase ", "DERIVA · frase "
};
inline const LocalizedText routeSuffix {
    " · route 0x", " · rota 0x", " · route 0x", " · ruta 0x"
};
inline const LocalizedText variationPrefix {
    "VARIATION · ", "VARIAÇÃO · ", "VARIATION · ", "VARIACIÓN · "
};
inline const LocalizedText feedbackPrefix {
    "FEEDBACK · route 0x", "FEEDBACK · rota 0x", "FEEDBACK · route 0x", "FEEDBACK · ruta 0x"
};
inline const LocalizedText loopPrefix {
    "LOOP · ", "LOOP · ", "LOOP · ", "LOOP · "
};
inline const LocalizedText loopStepsSuffix {
    " steps", " passos", " pas", " pasos"
};
inline const LocalizedText recStartedAtStep1 {
    "REC · actually started at step 1 of the PRINCIPAL sequencer",
    "REC · início real no step 1 do sequenciador principal",
    "REC · démarrage réel au step 1 du séquenceur PRINCIPAL",
    "REC · inicio real en el step 1 del secuenciador PRINCIPAL"
};
inline const LocalizedText recFinishedAtLoopEndPrefix {
    "REC · finished at the PRINCIPAL loop's end (LOOP END · ",
    "REC · finalizado no fim do loop principal (FIM DO LOOP · ",
    "REC · terminé à la fin de la boucle PRINCIPAL (FIN BOUCLE · ",
    "REC · finalizado al fin del loop PRINCIPAL (FIN LOOP · "
};
inline const LocalizedText recFinishedAtLoopEndSuffix {
    " ACTIVE) · file saved", " ATIVO) · arquivo salvo",
    " ACTIVE) · fichier enregistré", " ACTIVA) · archivo guardado"
};
// The `flow` status line (above the log box, distinct from it) shows the
// same class of REC/DERIVA status prose as logText:: above, just as a
// single always-current line instead of a scrolling history. Author kept
// finding more of these after the log box pass (each message in this
// section spot-checked live: "Controles Rec", "RES ALTURA; RES CORPO"...).
inline const LocalizedText recCancelledByStopShort {
    "REC · cancelled by STOP (was waiting for step 1)",
    "REC · cancelada pelo STOP (aguardava step 1)",
    "REC · annulé par STOP (attendait le step 1)",
    "REC · cancelado por STOP (esperaba el step 1)"
};
inline const LocalizedText recFinishedByStopShort {
    "REC · finished by STOP (loop couldn't complete)",
    "REC · finalizada pelo STOP (loop não pôde terminar)",
    "REC · terminé par STOP (boucle incomplète)",
    "REC · finalizado por STOP (loop no pudo terminar)"
};
inline const LocalizedText recWaitingStep1 {
    "REC · waiting for step 1 of the PRINCIPAL sequencer",
    "REC · aguardando step 1 do sequenciador principal",
    "REC · en attente du step 1 du séquenceur PRINCIPAL",
    "REC · esperando el step 1 del secuenciador PRINCIPAL"
};
inline const LocalizedText recFinishingAtLoopEnd {
    "REC · finishing at the end of the current loop",
    "REC · finalizando no fim do loop atual",
    "REC · fin à la fin de la boucle actuelle",
    "REC · terminando al fin del loop actual"
};
inline const LocalizedText recFinishingAtLoopEndEllipsis {
    "REC · finishing at the end of the current loop…",
    "REC · finalizando no fim do loop atual…",
    "REC · fin à la fin de la boucle actuelle…",
    "REC · terminando al fin del loop actual…"
};
inline const LocalizedText recCancelledRecording {
    "REC · recording cancelled (was waiting for step 1)",
    "REC · gravação cancelada (aguardava step 1)",
    "REC · enregistrement annulé (attendait le step 1)",
    "REC · grabación cancelada (esperaba el step 1)"
};
inline const LocalizedText driftMemoryCaptured {
    "DRIFT: MEMORY CAPTURED · WAITING FOR PHRASE RETURN",
    "DERIVA: MEMÓRIA CAPTURADA · AGUARDANDO O RETORNO DA FRASE",
    "DÉRIVE : MÉMOIRE CAPTURÉE · EN ATTENTE DU RETOUR DE PHRASE",
    "DERIVA: MEMORIA CAPTURADA · ESPERANDO EL RETORNO DE FRASE"
};
inline const std::array<LocalizedText, 4> recPhases {{
    { "GERMINATION", "GERMINAÇÃO", "GERMINATION", "GERMINACIÓN" },
    { "INFILTRATION", "INFILTRAÇÃO", "INFILTRATION", "INFILTRACIÓN" },
    { "DENSIFICATION", "ADENSAMENTO", "DENSIFICATION", "DENSIFICACIÓN" },
    { "CODA", "CODA", "CODA", "CODA" }
}};
inline const LocalizedText driftPhraseHeaderPrefix {
    "DRIFT: PHRASE ", "DERIVA: FRASE ", "DÉRIVE : PHRASE ", "DERIVA: FRASE "
};
inline const LocalizedText driftPhraseHeaderMid {
    " · INT ", " · INT ", " · INT ", " · INT "
};
inline const LocalizedText driftPhraseHeaderRoute {
    "% · ROUTE ", "% · ROTA ", "% · ROUTE ", "% · RUTA "
};
inline const LocalizedText recArmedTrackPrefix {
    "REC · ", "REC · ", "REC · ", "REC · "
};
inline const LocalizedText recArmedTrackMid {
    " · TRACK ", " · FAIXA ", " · PISTE ", " · PISTA "
};
inline const LocalizedText recFinishedSimple {
    "REC · recording finished at the end of the loop · file saved",
    "REC · gravação finalizada no fim do loop · arquivo salvo",
    "REC · enregistrement terminé à la fin de la boucle · fichier enregistré",
    "REC · grabación finalizada al fin del loop · archivo guardado"
};
inline const LocalizedText recCountdownSuffix {
    " remaining", " restantes", " restant", " restantes"
};
} // namespace logText
} // namespace antitotem::ui
