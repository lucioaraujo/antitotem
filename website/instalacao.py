#!/usr/bin/env python3
"""Rewrites the Download, Minimum requirements and Installation sections of
the four install pages (install.html, install-en/fr/es.html) for a version.

    python3 instalacao.py 0.1.3

Why a script: three sections × four languages with the version written into
every link. By hand, one page ends up pointing at the old release and nobody
notices, because nobody reads all four languages. Same reason as Rasgo
Modular's estado.py. Follows the RASGO multiplatform distribution standard
(RASGO_DOCUMENTATION/PADRAO_DISTRIBUICAO_MULTIPLATAFORMA.md): six packages,
first-launch warnings explained, what was measured kept apart from what
comes from the build, the Linux path for each distribution.

The three sections are located by their own markup (the `downloads`
section, the section holding `req-grid`, the `section-dark` holding the
steps) and are then wrapped in INSTALACAO:* comments, so later runs replace
exactly the same regions.
"""
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = "https://github.com/lucioaraujo/antitotem"
VCREDIST = "https://aka.ms/vs/17/release/vc_redist.x64.exe"

PKG = {
    "deb": "antitotem-{v}-Linux.deb",
    "appimage": "antitotem-{v}-linux-x86_64.AppImage",
    "targz": "antitotem-{v}-linux-x86_64.tar.gz",
    "exe": "antitotem-{v}-win64.exe",
    "zip": "antitotem-{v}-win64.zip",
    "dmg": "antitotem-{v}-Darwin.dmg",
}

T = {
    "install.html": dict(
        anchor="baixar", download="Baixar", req="Requisitos mínimos", inst="Instalação",
        intro="Versão v{v} — escolha seu sistema; o download começa direto:",
        cards=[("Linux", "pacote .deb (Ubuntu, Debian, Mint)", "deb"),
               ("Linux", "AppImage (qualquer distribuição)", "appimage"),
               ("Linux", ".tar.gz (sem root)", "targz"),
               ("Windows", "instalador .exe", "exe"),
               ("Windows", ".zip portátil (sem instalar)", "zip"),
               ("macOS", "imagem .dmg (Intel e Apple Silicon)", "dmg")],
        cta="Baixar →",
        note=("Se, na primeira abertura, o Windows ou o macOS disser que o programa não pôde ser verificado "
              "ou que pode conter software malicioso (ou mostrar uma mensagem parecida), não apague o arquivo: "
              "siga os passos de <a href=\"#instalacao\">Instalação</a>, logo abaixo, para liberá-lo. Todas as "
              "versões, inclusive as de teste: <a href=\"{repo}/releases\">página de releases</a>."),
        sys_h="Sistema",
        sys=("<strong>Windows</strong> 10 ou 11, 64 bits · <strong>macOS</strong> 10.13+ (Intel) "
             "ou 11+ (Apple Silicon), um só pacote Universal 2 · <strong>Linux</strong> x86-64 "
             "com glibc 2.35+ (distribuições de 2022 em diante)."),
        pc_h="Computador (medido)",
        pc=("Num Intel Core i5-6500 (2015, 4 núcleos, 3,2 GHz): o app inteiro usa cerca de 60% "
            "de um núcleo; o motor de som sozinho, 10–11% a 44,1/48 kHz, no pior caso. Memória: "
            "cerca de 45 MB. Qualquer computador de 64 bits com dois núcleos e 4 GB basta; "
            "máquinas mais fracas não foram testadas."),
        build_h="Para compilar",
        build=["CMake ≥ 3.22", "Compilador C++20 (GCC/Clang)",
               "Checkout local do <a href=\"https://github.com/juce-framework/JUCE\">JUCE</a>",
               "Dispositivo de áudio (ALSA/PipeWire)"],
        deb_h="O que o <code>.deb</code> pede",
        deb=["<code>libasound2</code>, <code>libfontconfig1</code>, <code>libfreetype6</code>",
             "<code>libx11-6</code>, <code>libxext6</code>, <code>libxss1</code>",
             "GLib (<code>libglib2.0-0</code>), <code>libstdc++6</code>, <code>libc6</code>"],
        steps=[
            ("Windows",
             "<ol><li>Abra o <code>.exe</code>. Aparece a janela azul <strong>\"O Windows protegeu o "
             "computador\"</strong>: clique em <strong>\"Mais informações\"</strong> e depois em "
             "<strong>\"Executar assim mesmo\"</strong>.</li><li>Se pedir permissão de administrador, "
             "clique em <strong>\"Sim\"</strong>.</li><li>\"Avançar\" até o fim; a última tela já traz "
             "<strong>\"Executar o Antitotem\"</strong> marcado. Depois ele fica no Menu Iniciar e na "
             "área de trabalho.</li></ol><p>Sem instalar: descompacte o <code>.zip</code> e abra "
             "<code>Antitotem - Objeto Sonoro.exe</code>. Se você ainda tem a v0.1.0 e ela não abre, "
             "instale o <a href=\"{vc}\">Visual C++ Redistributable</a> da Microsoft — a partir da "
             "v0.1.1 isso não é mais necessário. Se o atalho do Menu Iniciar da v0.1.0 ou v0.1.1 não abre o "
             "programa, instale a versão mais nova por cima. Se aparecerem várias versões instaladas, desinstale as "
             "antigas em Configurações → Aplicativos; a partir da v0.1.3, cada versão nova substitui a "
             "anterior sozinha.</p>"),
            ("macOS",
             "<ol><li>Abra o <code>.dmg</code> e arraste o Antitotem para <strong>Aplicativos</strong>.</li>"
             "<li>Abra o app. O macOS bloqueia o app dizendo que a Apple não pôde confirmar que ele está livre de software malicioso — não é defeito. Clique em "
             "<strong>\"OK\"</strong> (não em \"Mover para o Lixo\").</li><li>Em <strong>Ajustes do "
             "Sistema → Privacidade e Segurança</strong>, clique em <strong>\"Abrir Mesmo Assim\"</strong>, "
             "confirme com a senha e clique em <strong>\"Abrir\"</strong>. Só na primeira vez. No macOS 14 "
             "ou anterior: botão direito no app → Abrir → Abrir.</li></ol><p>Se o botão não aparecer ou o bloqueio continuar, abra o Terminal (Aplicativos → Utilitários), cole <code>xattr -dr com.apple.quarantine \"/Applications/Antitotem - Objeto Sonoro.app\"</code> e aperte Enter; depois abra o app normalmente. Isso também resolve o \"está danificado\" da v0.1.0.</p><p>Se ainda assim não abrir, "
             "escreva para <a href=\"mailto:rasgo.instruments@gmail.com\">rasgo.instruments@gmail.com</a> com a versão do macOS, o modelo do Mac e "
             "a mensagem que aparece.</p>"),
            ("Linux",
             "<ul><li><strong>Ubuntu 22.04+, Debian 12+, Mint 21+:</strong> dois cliques no "
             "<code>.deb</code>, ou <code>sudo apt install ./antitotem-{v}-Linux.deb</code>.</li>"
             "<li><strong>Fedora, Arch, openSUSE e outras:</strong> o <strong>AppImage</strong> (marque "
             "como executável e abra) ou o <strong>.tar.gz</strong> (descompacte e rode "
             "<code>./install.sh</code>, que instala no seu usuário, sem root).</li></ul>"
             "<p>Os três pacotes Linux são testados na abertura em Debian 12, Ubuntu 24.04, Fedora e Arch "
             "antes de cada versão.</p>"),
        ],
        inst_note=("Guia completo, com requisitos e solução de problemas, no "
                   "<a href=\"{repo}/blob/main/INSTALL.md#instalar-passo-a-passo\">INSTALL.md</a>."),
    ),
    "install-en.html": dict(
        anchor="download", download="Download", req="Minimum requirements", inst="Installation",
        intro="Version v{v} — pick your system; the download starts right away:",
        cards=[("Linux", ".deb package (Ubuntu, Debian, Mint)", "deb"),
               ("Linux", "AppImage (any distribution)", "appimage"),
               ("Linux", ".tar.gz (no root)", "targz"),
               ("Windows", ".exe installer", "exe"),
               ("Windows", "portable .zip (no install)", "zip"),
               ("macOS", ".dmg image (Intel and Apple Silicon)", "dmg")],
        cta="Download →",
        note=("If, on first launch, Windows or macOS says the program could not be verified or may contain "
              "malicious software (or shows a similar message), do not delete it: follow the steps in "
              "<a href=\"#installation\">Installation</a> below to allow it. All versions, including test "
              "builds: <a href=\"{repo}/releases\">releases page</a>."),
        sys_h="System",
        sys=("<strong>Windows</strong> 10 or 11, 64-bit · <strong>macOS</strong> 10.13+ (Intel) or "
             "11+ (Apple Silicon), one Universal 2 package · <strong>Linux</strong> x86-64 with "
             "glibc 2.35+ (2022+ distributions)."),
        pc_h="Computer (measured)",
        pc=("On an Intel Core i5-6500 (2015, 4 cores, 3.2 GHz): the whole app uses about 60% of one "
            "core; the sound engine alone 10–11% at 44.1/48 kHz, worst case. Memory: about 45 MB. "
            "Any 64-bit computer with two cores and 4 GB is enough; weaker machines have not been "
            "tested."),
        build_h="To build",
        build=["CMake ≥ 3.22", "C++20 compiler (GCC/Clang)",
               "A local checkout of <a href=\"https://github.com/juce-framework/JUCE\">JUCE</a>",
               "An audio device (ALSA/PipeWire)"],
        deb_h="What the <code>.deb</code> needs",
        deb=["<code>libasound2</code>, <code>libfontconfig1</code>, <code>libfreetype6</code>",
             "<code>libx11-6</code>, <code>libxext6</code>, <code>libxss1</code>",
             "GLib (<code>libglib2.0-0</code>), <code>libstdc++6</code>, <code>libc6</code>"],
        steps=[
            ("Windows",
             "<ol><li>Open the <code>.exe</code>. A blue <strong>\"Windows protected your PC\"</strong> box "
             "appears: click <strong>\"More info\"</strong>, then <strong>\"Run anyway\"</strong>.</li>"
             "<li>If asked for administrator permission, click <strong>\"Yes\"</strong>.</li><li>\"Next\" to "
             "the end; the last page has <strong>\"Run Antitotem\"</strong> already ticked. It is then in "
             "the Start menu and on the desktop.</li></ol><p>Without installing: unzip the "
             "<code>.zip</code> and open <code>Antitotem - Objeto Sonoro.exe</code>. If you still have "
             "v0.1.0 and it does not open, install Microsoft's <a href=\"{vc}\">Visual C++ "
             "Redistributable</a> — no longer needed from v0.1.1 on. If the Start menu shortcut of v0.1.0 or "
             "v0.1.1 does not open the program, install the newest version over it. If several versions are installed, "
             "uninstall the old ones in Settings → Apps; from v0.1.3 on, each new version replaces the "
             "previous one by itself.</p>"),
            ("macOS",
             "<ol><li>Open the <code>.dmg</code> and drag Antitotem into <strong>Applications</strong>.</li>"
             "<li>Open the app. macOS blocks it, saying Apple could not confirm it is free of malicious software — not a defect. Click <strong>\"OK\"</strong> "
             "(not \"Move to Trash\").</li><li>In <strong>System Settings → Privacy & Security</strong>, "
             "click <strong>\"Open Anyway\"</strong>, confirm with your password and click "
             "<strong>\"Open\"</strong>. Only the first time. On macOS 14 or earlier: right-click the app → "
             "Open → Open.</li></ol><p>If the button does not appear or the block remains, open Terminal (Applications → Utilities), paste <code>xattr -dr com.apple.quarantine \"/Applications/Antitotem - Objeto Sonoro.app\"</code> and press Enter; then open the app normally. This also fixes v0.1.0's \"is damaged\".</p><p>If it still does not open, write to "
             "<a href=\"mailto:rasgo.instruments@gmail.com\">rasgo.instruments@gmail.com</a> with your macOS version, your Mac model and the message "
             "you see.</p>"),
            ("Linux",
             "<ul><li><strong>Ubuntu 22.04+, Debian 12+, Mint 21+:</strong> double-click the "
             "<code>.deb</code>, or <code>sudo apt install ./antitotem-{v}-Linux.deb</code>.</li>"
             "<li><strong>Fedora, Arch, openSUSE and others:</strong> the <strong>AppImage</strong> (make "
             "it executable and open it) or the <strong>.tar.gz</strong> (unpack and run "
             "<code>./install.sh</code>, which installs for your user, no root).</li></ul>"
             "<p>All three Linux packages are launch-tested on Debian 12, Ubuntu 24.04, Fedora and Arch "
             "before every version.</p>"),
        ],
        inst_note=("Full guide, with requirements and troubleshooting, in "
                   "<a href=\"{repo}/blob/main/INSTALL.md#installing-step-by-step\">INSTALL.md</a>."),
    ),
    "install-fr.html": dict(
        anchor="telecharger", download="Télécharger", req="Prérequis minimaux", inst="Installation",
        intro="Version v{v} — choisissez votre système ; le téléchargement démarre directement :",
        cards=[("Linux", "paquet .deb (Ubuntu, Debian, Mint)", "deb"),
               ("Linux", "AppImage (toute distribution)", "appimage"),
               ("Linux", ".tar.gz (sans root)", "targz"),
               ("Windows", "installateur .exe", "exe"),
               ("Windows", ".zip portable (sans installer)", "zip"),
               ("macOS", "image .dmg (Intel et Apple Silicon)", "dmg")],
        cta="Télécharger →",
        note=("Si, au premier lancement, Windows ou macOS indique que le programme n’a pas pu être vérifié "
              "ou qu’il pourrait contenir un logiciel malveillant (ou affiche un message semblable), ne le "
              "supprimez pas : suivez les étapes de <a href=\"#installation\">Installation</a> ci-dessous "
              "pour l’autoriser. Toutes les versions, y compris de test : "
              "<a href=\"{repo}/releases\">page des releases</a>."),
        sys_h="Système",
        sys=("<strong>Windows</strong> 10 ou 11, 64 bits · <strong>macOS</strong> 10.13+ (Intel) ou "
             "11+ (Apple Silicon), un seul paquet Universal 2 · <strong>Linux</strong> x86-64 avec "
             "glibc 2.35+ (distributions de 2022 et après)."),
        pc_h="Ordinateur (mesuré)",
        pc=("Sur un Intel Core i5-6500 (2015, 4 cœurs, 3,2 GHz) : l’application entière utilise "
            "environ 60 % d’un cœur ; le moteur sonore seul 10–11 % à 44,1/48 kHz, au pire. "
            "Mémoire : environ 45 Mo. Tout ordinateur 64 bits à deux cœurs et 4 Go suffit ; les "
            "machines plus faibles n’ont pas été testées."),
        build_h="Pour compiler",
        build=["CMake ≥ 3.22", "Compilateur C++20 (GCC/Clang)",
               "Un checkout local de <a href=\"https://github.com/juce-framework/JUCE\">JUCE</a>",
               "Un périphérique audio (ALSA/PipeWire)"],
        deb_h="Ce que demande le <code>.deb</code>",
        deb=["<code>libasound2</code>, <code>libfontconfig1</code>, <code>libfreetype6</code>",
             "<code>libx11-6</code>, <code>libxext6</code>, <code>libxss1</code>",
             "GLib (<code>libglib2.0-0</code>), <code>libstdc++6</code>, <code>libc6</code>"],
        steps=[
            ("Windows",
             "<ol><li>Ouvrez le <code>.exe</code>. Une fenêtre bleue <strong>« Windows a protégé votre "
             "ordinateur »</strong> apparaît : cliquez sur <strong>« Informations complémentaires »</strong>, "
             "puis <strong>« Exécuter quand même »</strong>.</li><li>Si l’autorisation d’administrateur est "
             "demandée, cliquez sur <strong>« Oui »</strong>.</li><li>« Suivant » jusqu’au bout ; la dernière "
             "page propose déjà <strong>« Lancer Antitotem »</strong>. Il est ensuite dans le menu Démarrer "
             "et sur le bureau.</li></ol><p>Sans installer : décompressez le <code>.zip</code> et ouvrez "
             "<code>Antitotem - Objeto Sonoro.exe</code>. Si vous avez encore la v0.1.0 et qu’elle ne "
             "s’ouvre pas, installez le <a href=\"{vc}\">Visual C++ Redistributable</a> de Microsoft — "
             "inutile à partir de la v0.1.1. Si le raccourci du menu Démarrer de la v0.1.0 ou v0.1.1 "
             "n’ouvre pas le programme, installez la version la plus récente par-dessus. Si plusieurs versions sont "
             "installées, désinstallez les anciennes dans Paramètres → Applications ; à partir de la "
             "v0.1.3, chaque nouvelle version remplace la précédente toute seule.</p>"),
            ("macOS",
             "<ol><li>Ouvrez le <code>.dmg</code> et glissez Antitotem dans <strong>Applications</strong>.</li>"
             "<li>Ouvrez l’app. macOS la bloque en disant qu’Apple n’a pas pu confirmer qu’elle ne contient pas de logiciel malveillant — ce n’est pas un défaut. Cliquez sur "
             "<strong>« OK »</strong> (pas « Placer dans la corbeille »).</li><li>Dans <strong>Réglages "
             "Système → Confidentialité et sécurité</strong>, cliquez sur <strong>« Ouvrir quand même »</strong>, "
             "confirmez avec votre mot de passe, puis <strong>« Ouvrir »</strong>. Seulement la première fois. "
             "Sous macOS 14 ou antérieur : clic droit sur l’app → Ouvrir → Ouvrir.</li></ol><p>Si le bouton n’apparaît pas ou si le blocage persiste, ouvrez le Terminal (Applications → Utilitaires), collez <code>xattr -dr com.apple.quarantine \"/Applications/Antitotem - Objeto Sonoro.app\"</code> et appuyez sur Entrée ; ouvrez ensuite l’app normalement. Cela corrige aussi le « endommagée » de la v0.1.0.</p><p>S’il ne s’ouvre toujours pas, "
             "écrivez à <a href=\"mailto:rasgo.instruments@gmail.com\">rasgo.instruments@gmail.com</a> en indiquant la version de macOS, le modèle du "
             "Mac et le message affiché.</p>"),
            ("Linux",
             "<ul><li><strong>Ubuntu 22.04+, Debian 12+, Mint 21+ :</strong> double-cliquez sur le "
             "<code>.deb</code>, ou <code>sudo apt install ./antitotem-{v}-Linux.deb</code>.</li>"
             "<li><strong>Fedora, Arch, openSUSE et autres :</strong> l’<strong>AppImage</strong> (rendez-la "
             "exécutable et ouvrez-la) ou le <strong>.tar.gz</strong> (décompressez et lancez "
             "<code>./install.sh</code>, qui installe pour votre utilisateur, sans root).</li></ul>"
             "<p>Les trois paquets Linux sont testés au lancement sur Debian 12, Ubuntu 24.04, Fedora et "
             "Arch avant chaque version.</p>"),
        ],
        inst_note=("Guide complet, avec prérequis et dépannage, dans "
                   "<a href=\"{repo}/blob/main/INSTALL.md#installing-step-by-step\">INSTALL.md</a> "
                   "(anglais et portugais)."),
    ),
    "install-es.html": dict(
        anchor="descargar", download="Descargar", req="Requisitos mínimos", inst="Instalación",
        intro="Versión v{v} — elija su sistema; la descarga empieza directamente:",
        cards=[("Linux", "paquete .deb (Ubuntu, Debian, Mint)", "deb"),
               ("Linux", "AppImage (cualquier distribución)", "appimage"),
               ("Linux", ".tar.gz (sin root)", "targz"),
               ("Windows", "instalador .exe", "exe"),
               ("Windows", ".zip portátil (sin instalar)", "zip"),
               ("macOS", "imagen .dmg (Intel y Apple Silicon)", "dmg")],
        cta="Descargar →",
        note=("Si, en la primera apertura, Windows o macOS dice que el programa no pudo verificarse o que "
              "puede contener software malicioso (o muestra un mensaje parecido), no lo borre: siga los pasos "
              "de <a href=\"#instalacion\">Instalación</a>, más abajo, para autorizarlo. Todas las versiones, "
              "incluidas las de prueba: <a href=\"{repo}/releases\">página de releases</a>."),
        sys_h="Sistema",
        sys=("<strong>Windows</strong> 10 u 11, 64 bits · <strong>macOS</strong> 10.13+ (Intel) u "
             "11+ (Apple Silicon), un solo paquete Universal 2 · <strong>Linux</strong> x86-64 con "
             "glibc 2.35+ (distribuciones de 2022 en adelante)."),
        pc_h="Ordenador (medido)",
        pc=("En un Intel Core i5-6500 (2015, 4 núcleos, 3,2 GHz): la app entera usa cerca del 60% "
            "de un núcleo; el motor de sonido solo, 10–11% a 44,1/48 kHz, en el peor caso. Memoria: "
            "unos 45 MB. Cualquier ordenador de 64 bits con dos núcleos y 4 GB basta; máquinas más "
            "débiles no se han probado."),
        build_h="Para compilar",
        build=["CMake ≥ 3.22", "Compilador C++20 (GCC/Clang)",
               "Un checkout local de <a href=\"https://github.com/juce-framework/JUCE\">JUCE</a>",
               "Un dispositivo de audio (ALSA/PipeWire)"],
        deb_h="Lo que pide el <code>.deb</code>",
        deb=["<code>libasound2</code>, <code>libfontconfig1</code>, <code>libfreetype6</code>",
             "<code>libx11-6</code>, <code>libxext6</code>, <code>libxss1</code>",
             "GLib (<code>libglib2.0-0</code>), <code>libstdc++6</code>, <code>libc6</code>"],
        steps=[
            ("Windows",
             "<ol><li>Abra el <code>.exe</code>. Aparece la ventana azul <strong>«Windows protegió su PC»</strong>: "
             "pulse <strong>«Más información»</strong> y luego <strong>«Ejecutar de todas formas»</strong>.</li>"
             "<li>Si pide permiso de administrador, pulse <strong>«Sí»</strong>.</li><li>«Siguiente» hasta el "
             "final; la última pantalla ya trae <strong>«Ejecutar Antitotem»</strong> marcado. Después queda en "
             "el menú Inicio y en el escritorio.</li></ol><p>Sin instalar: descomprima el <code>.zip</code> y "
             "abra <code>Antitotem - Objeto Sonoro.exe</code>. Si aún tiene la v0.1.0 y no abre, instale el "
             "<a href=\"{vc}\">Visual C++ Redistributable</a> de Microsoft — desde la v0.1.1 ya no hace "
             "falta. Si el acceso directo del menú Inicio de la v0.1.0 o v0.1.1 no abre el programa, "
             "instale la versión más reciente encima. Si aparecen varias versiones instaladas, desinstale las antiguas "
             "en Configuración → Aplicaciones; desde la v0.1.3, cada versión nueva reemplaza a la "
             "anterior sola.</p>"),
            ("macOS",
             "<ol><li>Abra el <code>.dmg</code> y arrastre Antitotem a <strong>Aplicaciones</strong>.</li>"
             "<li>Abra la app. macOS la bloquea diciendo que Apple no pudo confirmar que esté libre de software malicioso — no es un defecto. Pulse "
             "<strong>«OK»</strong> (no «Trasladar a la papelera»).</li><li>En <strong>Ajustes del Sistema → "
             "Privacidad y seguridad</strong>, pulse <strong>«Abrir igualmente»</strong>, confirme con su "
             "contraseña y pulse <strong>«Abrir»</strong>. Solo la primera vez. En macOS 14 o anterior: clic "
             "derecho en la app → Abrir → Abrir.</li></ol><p>Si el botón no aparece o el bloqueo continúa, abra el Terminal (Aplicaciones → Utilidades), pegue <code>xattr -dr com.apple.quarantine \"/Applications/Antitotem - Objeto Sonoro.app\"</code> y pulse Intro; después abra la app normalmente. Esto también resuelve el «está dañada» de la v0.1.0.</p><p>Si aun así no abre, escriba "
             "a <a href=\"mailto:rasgo.instruments@gmail.com\">rasgo.instruments@gmail.com</a> indicando la versión de macOS, el modelo del Mac y el "
             "mensaje que aparece.</p>"),
            ("Linux",
             "<ul><li><strong>Ubuntu 22.04+, Debian 12+, Mint 21+:</strong> doble clic en el "
             "<code>.deb</code>, o <code>sudo apt install ./antitotem-{v}-Linux.deb</code>.</li>"
             "<li><strong>Fedora, Arch, openSUSE y otras:</strong> el <strong>AppImage</strong> (márquelo "
             "como ejecutable y ábralo) o el <strong>.tar.gz</strong> (descomprima y ejecute "
             "<code>./install.sh</code>, que instala para su usuario, sin root).</li></ul>"
             "<p>Los tres paquetes Linux se prueban al abrir en Debian 12, Ubuntu 24.04, Fedora y Arch "
             "antes de cada versión.</p>"),
        ],
        inst_note=("Guía completa, con requisitos y solución de problemas, en "
                   "<a href=\"{repo}/blob/main/INSTALL.md#installing-step-by-step\">INSTALL.md</a> "
                   "(inglés y portugués)."),
    ),
}

INST_ID = {"install.html": "instalacao", "install-en.html": "installation",
           "install-fr.html": "installation", "install-es.html": "instalacion"}


def blocks(page, v):
    t = T[page]
    f = lambda s: s.format(v=v, repo=REPO, vc=VCREDIST)
    cards = "\n".join(
        '    <a class="download-card" href="%s/releases/download/v%s/%s">\n'
        '      <span class="download-os">%s</span>\n'
        '      <span class="download-format">%s</span>\n'
        '      <span class="download-cta">%s</span>\n    </a>'
        % (REPO, v, PKG[k].format(v=v), os_, fmt, t["cta"]) for os_, fmt, k in t["cards"])
    down = ('<!-- INSTALACAO:DOWNLOAD:INICIO -->\n<section class="section downloads" id="%s">\n'
            '  <h2>%s</h2>\n  <p>%s</p>\n  <div class="download-grid">\n%s\n  </div>\n'
            '  <p class="note">%s</p>\n</section>\n<!-- INSTALACAO:DOWNLOAD:FIM -->'
            % (t["anchor"], t["download"], f(t["intro"]), cards, f(t["note"])))
    li = lambda xs: "\n".join("        <li>%s</li>" % x for x in xs)
    req = ('<!-- INSTALACAO:REQUISITOS:INICIO -->\n<section class="section">\n  <h2>%s</h2>\n'
           '  <div class="req-grid">\n'
           '    <div class="req-card">\n      <h3>%s</h3>\n      <p>%s</p>\n    </div>\n'
           '    <div class="req-card">\n      <h3>%s</h3>\n      <p>%s</p>\n    </div>\n'
           '    <div class="req-card">\n      <h3>%s</h3>\n      <ul>\n%s\n      </ul>\n    </div>\n'
           '    <div class="req-card">\n      <h3>%s</h3>\n      <ul>\n%s\n      </ul>\n    </div>\n'
           '  </div>\n</section>\n<!-- INSTALACAO:REQUISITOS:FIM -->'
           % (t["req"], t["sys_h"], t["sys"], t["pc_h"], t["pc"], t["build_h"], li(t["build"]),
              t["deb_h"], li(t["deb"])))
    steps = "\n".join(
        '  <div class="step">\n    <span class="step-index">%d</span>\n    <div class="step-body">\n'
        '      <h3>%s</h3>\n      %s\n    </div>\n  </div>' % (i + 1, h, f(b))
        for i, (h, b) in enumerate(t["steps"]))
    inst = ('<!-- INSTALACAO:PASSOS:INICIO -->\n<section class="section section-dark" id="%s">\n'
            '  <h2>%s</h2>\n\n%s\n\n  <p class="note">%s</p>\n</section>\n'
            '<!-- INSTALACAO:PASSOS:FIM -->'
            % (INST_ID[page], t["inst"], steps, f(t["inst_note"])))
    return down, req, inst


def replace_section(html, marker, finder, new):
    m = re.search(r"<!-- INSTALACAO:%s:INICIO -->.*?<!-- INSTALACAO:%s:FIM -->" % (marker, marker),
                  html, re.S)
    if m:
        return html[:m.start()] + new + html[m.end():]
    m = re.search(finder, html, re.S)
    if not m:
        raise SystemExit("section %s not found" % marker)
    return html[:m.start()] + new + html[m.end():]


def main():
    if len(sys.argv) != 2 or not re.fullmatch(r"\d+\.\d+\.\d+", sys.argv[1]):
        raise SystemExit("usage: instalacao.py X.Y.Z")
    v = sys.argv[1]
    for page in T:
        p = HERE / page
        html = p.read_text(encoding="utf-8")
        down, req, inst = blocks(page, v)
        html = replace_section(html, "DOWNLOAD", r'<section class="section downloads".*?</section>', down)
        html = replace_section(html, "REQUISITOS",
                               r'<section class="section">\s*<h2>[^<]*</h2>\s*(?:<p>.*?</p>\s*)?<div class="req-grid">.*?</section>',
                               req)
        html = replace_section(html, "PASSOS",
                               r'<section class="section section-dark">\s*<h2>[^<]*</h2>\s*<div class="step">.*?</section>',
                               inst)
        html = re.sub(r'("downloadUrl": "%s/releases/tag/v)[\d.]+(")' % re.escape(REPO), r"\g<1>%s\2" % v, html)
        p.write_text(html, encoding="utf-8")
        print("%-17s v%s" % (page, v))


if __name__ == "__main__":
    main()
