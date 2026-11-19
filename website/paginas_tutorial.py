#!/usr/bin/env python3
"""Gera as páginas Tutorial e Aprender do site do Antitotem (4 línguas) a
partir do tutorial que o próprio app traz (src/app/UiLanguage.h), e põe o
mesmo menu de páginas em todas as páginas do site.

    python3 paginas_tutorial.py && python3 seo.py

Nivelamento dos sites da família RASGO (autor, 19/11/2026: "nivele pelos
mais completos"). O app tem três níveis de TUTORIAL: BÁSICO (cada parte do
painel), INTERMEDIÁRIO (exercícios de escuta, de docs/TUTORIAIS.md) e
AVANÇADO (as relações entre as partes). No site:
- Aprender = BÁSICO, o que cada parte do painel faz;
- Tutorial = INTERMEDIÁRIO e AVANÇADO, os gestos e as escutas.
O texto é lido do código, para a janela e o site dizerem o mesmo. A casca
de cada página (cabeçalho, rodapé, estilos) vem da página de instalação da
mesma língua; o bloco SEO é escrito depois pelo seo.py.
"""

import html
import pathlib
import re

AQUI = pathlib.Path(__file__).parent
FONTE = AQUI.parent / "src" / "app" / "UiLanguage.h"
LANGS = ("pt-BR", "en", "fr", "es")
SUFIXO = {"pt-BR": "", "en": "-en", "fr": "-fr", "es": "-es"}
SIGLA = {"pt-BR": "PT", "en": "EN", "fr": "FR", "es": "ES"}
ORDEM = {"en": 0, "pt-BR": 1, "fr": 2, "es": 3}   # LocalizedText: en, pt, fr, es


def arquivo(tipo, lang):
    if tipo == "conceito":
        return "index.html" if lang == "pt-BR" else lang + ".html"
    return tipo + SUFIXO[lang] + ".html"


MENU = {
    "pt-BR": ("Seções", [("conceito", "Conceito"), ("tutorial", "Tutorial"), ("aprender", "Aprender"), ("install", "Instalação"), ("baixar", "Baixar")]),
    "en": ("Sections", [("conceito", "Concept"), ("tutorial", "Tutorial"), ("aprender", "Learn"), ("install", "Installation"), ("baixar", "Download")]),
    "fr": ("Sections", [("conceito", "Concept"), ("tutorial", "Tutoriel"), ("aprender", "Apprendre"), ("install", "Installation"), ("baixar", "Télécharger")]),
    "es": ("Secciones", [("conceito", "Concepto"), ("tutorial", "Tutorial"), ("aprender", "Aprender"), ("install", "Instalación"), ("baixar", "Descargar")]),
}

TEXTOS = {
    "pt-BR": {
        "tutorial": ("Tutorial — Antitotem", "Tutorial",
                     "Gestos e escutas para tocar o Antitotem: os exercícios do nível INTERMEDIÁRIO e as relações do nível AVANÇADO do tutorial que o app traz no botão TUTORIAL.",
                     "Tutorial do Antitotem: exercícios de escuta, dois objetos se ouvindo, rotas, camadas de tempo, deriva, matéria e caos."),
        "aprender": ("Aprender — Antitotem", "Aprender",
                     "Cada parte do painel do Antitotem e o que ela faz: o nível BÁSICO do tutorial que o app traz no botão TUTORIAL.",
                     "Aprender o Antitotem: navegação, osciladores, sequência, VCF e ADSR, retorno, mixer, deriva e gravação."),
        "contents": "Sumário", "tip": "Dica", "inter": "Intermediário: gestos e escutas", "avanc": "Avançado: as relações entre as partes",
    },
    "en": {
        "tutorial": ("Tutorial — Antitotem", "Tutorial",
                     "Gestures and listenings to play Antitotem: the INTERMEDIATE exercises and the ADVANCED relationships of the tutorial the app carries under its TUTORIAL button.",
                     "Antitotem tutorial: listening exercises, two objects hearing each other, routes, time layers, drift, matter and chaos."),
        "aprender": ("Learn — Antitotem", "Learn",
                     "Every part of Antitotem's panel and what it does: the BASIC level of the tutorial the app carries under its TUTORIAL button.",
                     "Learn Antitotem: navigation, oscillators, sequence, VCF and ADSR, return, mixer, drift and recording."),
        "contents": "Contents", "tip": "Tip", "inter": "Intermediate: gestures and listenings", "avanc": "Advanced: how the parts relate",
    },
    "fr": {
        "tutorial": ("Tutoriel — Antitotem", "Tutoriel",
                     "Gestes et écoutes pour jouer Antitotem : les exercices du niveau INTERMÉDIAIRE et les relations du niveau AVANCÉ du tutoriel que l'app porte sous son bouton TUTORIAL.",
                     "Tutoriel d'Antitotem : exercices d'écoute, deux objets qui s'écoutent, routes, couches de temps, dérive, matière et chaos."),
        "aprender": ("Apprendre — Antitotem", "Apprendre",
                     "Chaque partie du panneau d'Antitotem et ce qu'elle fait : le niveau BASIQUE du tutoriel que l'app porte sous son bouton TUTORIAL.",
                     "Apprendre Antitotem : navigation, oscillateurs, séquence, VCF et ADSR, retour, mixeur, dérive et enregistrement."),
        "contents": "Sommaire", "tip": "Astuce", "inter": "Intermédiaire : gestes et écoutes", "avanc": "Avancé : les relations entre les parties",
    },
    "es": {
        "tutorial": ("Tutorial — Antitotem", "Tutorial",
                     "Gestos y escuchas para tocar Antitotem: los ejercicios del nivel INTERMEDIO y las relaciones del nivel AVANZADO del tutorial que la app lleva en su botón TUTORIAL.",
                     "Tutorial de Antitotem: ejercicios de escucha, dos objetos que se escuchan, rutas, capas de tiempo, deriva, materia y caos."),
        "aprender": ("Aprender — Antitotem", "Aprender",
                     "Cada parte del panel de Antitotem y lo que hace: el nivel BÁSICO del tutorial que la app lleva en su botón TUTORIAL.",
                     "Aprender Antitotem: navegación, osciladores, secuencia, VCF y ADSR, retorno, mezclador, deriva y grabación."),
        "contents": "Sumario", "tip": "Consejo", "inter": "Intermedio: gestos y escuchas", "avanc": "Avanzado: las relaciones entre las partes",
    },
}


def ler(nome, esperado):
    fonte = FONTE.read_text(encoding="utf-8")
    inicio = fonte.index("> %s {{" % nome) + len("> %s {" % nome)
    tokens = re.findall(r'"((?:[^"\\]|\\.)*)"|([{},])', fonte[inicio:])
    pilha, raiz, anterior = [], [], False
    atual = raiz
    for literal, sinal in tokens:
        if sinal == "{":
            no = []
            atual.append(no)
            pilha.append(atual)
            atual, anterior = no, False
        elif sinal == "}":
            if not pilha:
                break
            atual, anterior = pilha.pop(), False
        elif sinal == ",":
            anterior = False
        else:
            texto = bytes(literal, "utf-8").decode("unicode_escape").encode("latin-1").decode("utf-8")
            if anterior:
                atual[-1] += texto
            else:
                atual.append(texto)
            anterior = True
    caps = [c for c in raiz[0] if len(c) in (2, 3) and all(isinstance(x, list) and len(x) == 4 for x in c)]
    if len(caps) != esperado:
        raise ValueError("%s: esperava %d capítulos, achei %d" % (nome, esperado, len(caps)))
    return caps


def paragrafos(texto):
    out = []
    for par in texto.split("\n\n"):
        linhas = [l for l in par.split("\n") if l]
        if linhas and all(re.match(r"^\d+\. ", l) for l in linhas):
            out.append('  <ol class="tut-list">\n%s\n  </ol>' % "\n".join(
                "    <li>%s</li>" % html.escape(re.sub(r"^\d+\. ", "", l), quote=False) for l in linhas))
        elif linhas and all(l.startswith(("- ", "• ")) for l in linhas):
            out.append('  <ul class="tut-list">\n%s\n  </ul>' % "\n".join(
                "    <li>%s</li>" % html.escape(l[2:], quote=False) for l in linhas))
        elif linhas:
            out.append("  <p>%s</p>" % "<br />".join(html.escape(l, quote=False) for l in linhas))
    return "\n".join(out)


def capitulos_html(caps, lang, inicio, rotulo_dica):
    i, partes = ORDEM[lang], []
    for n, cap in enumerate(caps):
        tit, corpo = cap[0], cap[1]
        dica = cap[2] if len(cap) == 3 else ["", "", "", ""]
        classe = "section" + (" section-dark" if (inicio + n) % 2 else "")
        bloco = '<section class="%s" id="cap-%d">\n  <h2>%d · %s</h2>\n%s' % (
            classe, inicio + n + 1, inicio + n + 1, html.escape(tit[i], quote=False), paragrafos(corpo[i]))
        if dica[i]:
            bloco += '\n  <p class="tut-tip"><strong>%s:</strong> %s</p>' % (rotulo_dica, html.escape(dica[i], quote=False))
        partes.append(bloco + "\n</section>")
    return partes


def sumario(grupos, lang, rotulo):
    i, partes, n = ORDEM[lang], [], 0
    for titulo_grupo, caps in grupos:
        if titulo_grupo:
            partes.append('  <p class="tut-group">%s</p>' % html.escape(titulo_grupo, quote=False))
        itens = []
        for c in caps:
            n += 1
            itens.append('    <li><a href="#cap-%d">%s</a></li>' % (n, html.escape(c[0][i], quote=False)))
        partes.append('  <ol class="tut-toc" start="%d">\n%s\n  </ol>' % (n - len(caps) + 1, "\n".join(itens)))
    return '<section class="section section-dark">\n  <h2>%s</h2>\n%s\n</section>' % (html.escape(rotulo, quote=False), "\n".join(partes))


def menu_html(lang, atual):
    rotulo, itens = MENU[lang]
    links = []
    for tipo, nome in itens:
        href = arquivo("install", lang) + "#baixar" if tipo == "baixar" else arquivo(tipo, lang)
        cur = ' aria-current="page"' if tipo == atual else ""
        links.append('      <a href="%s"%s>%s</a>' % (href, cur, nome))
    return '<nav class="page-nav" aria-label="%s">\n%s\n    </nav>' % (rotulo, "\n".join(links))


def linguas_html(lang, atual):
    rotulo = {"pt-BR": "Idioma", "en": "Language", "fr": "Langue", "es": "Idioma"}[lang]
    links = "\n".join('      <a href="%s"%s>%s</a>' % (arquivo(atual, L), ' aria-current="true"' if L == lang else "", SIGLA[L])
                      for L in LANGS)
    return '<nav class="lang-switch" aria-label="%s">\n%s\n    </nav>' % (rotulo, links)


NAV_RE = re.compile(r'<nav class="page-nav".*?</nav>', re.S)


def trocar_menu(texto, lang, atual):
    if not NAV_RE.search(texto):
        raise ValueError("sem page-nav")
    return NAV_RE.sub(lambda _: menu_html(lang, atual), texto, count=1)


def pagina(lang, tipo, corpo):
    casca = (AQUI / arquivo("install", lang)).read_text(encoding="utf-8")
    titulo, h1, lead, desc = TEXTOS[lang][tipo]
    abre = re.search(r'<main id="[^"]*">', casca).group(0)   # conteudo / content / contenu / contenido
    cabeca, resto = casca.split(abre, 1)
    rodape = resto.split("</main>", 1)[1]
    cabeca = re.sub(r"<title>.*?</title>", "<title>%s</title>" % html.escape(titulo, quote=False), cabeca, count=1)
    cabeca = re.sub(r'<meta name="description" content="[^"]*"', '<meta name="description" content="%s"' % html.escape(desc, quote=True), cabeca, count=1)
    cabeca = trocar_menu(cabeca, lang, tipo)
    cabeca = re.sub(r'<nav class="lang-switch".*?</nav>', lambda _: linguas_html(lang, tipo), cabeca, count=1, flags=re.S)
    hero = '<section class="hero" id="top">\n  <h1>%s</h1>\n  <p class="lead">%s</p>\n</section>' % (
        html.escape(h1, quote=False), html.escape(lead, quote=False))
    return cabeca + abre + "\n" + hero + "\n\n" + corpo + "\n</main>" + rodape


def main():
    basico = ler("tutorialChapters", 9)
    inter = ler("tutorialChaptersIntermediate", 8)
    avanc = ler("tutorialChaptersAdvanced", 8)
    for lang in LANGS:
        t = TEXTOS[lang]
        corpo = [sumario([("", basico)], lang, t["contents"])] + capitulos_html(basico, lang, 0, t["tip"])
        (AQUI / arquivo("aprender", lang)).write_text(pagina(lang, "aprender", "\n\n".join(corpo)), encoding="utf-8")
        corpo = [sumario([(t["inter"], inter), (t["avanc"], avanc)], lang, t["contents"])]
        corpo += capitulos_html(inter, lang, 0, t["tip"]) + capitulos_html(avanc, lang, len(inter), t["tip"])
        (AQUI / arquivo("tutorial", lang)).write_text(pagina(lang, "tutorial", "\n\n".join(corpo)), encoding="utf-8")
        for tipo in ("conceito", "install"):
            caminho = AQUI / arquivo(tipo, lang)
            caminho.write_text(trocar_menu(caminho.read_text(encoding="utf-8"), lang, tipo), encoding="utf-8")
    print("paginas_tutorial.py: 8 páginas novas, menu em 16")


if __name__ == "__main__":
    main()
