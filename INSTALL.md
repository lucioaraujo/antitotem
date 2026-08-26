# Antitotem — Installation Guide / Guia de Instalação

Two languages, same content: [🇬🇧 English](#english) below, [🇧🇷 Português](#português) further down.

---

## English

### Status

Antitotem is a prototype under active investigation (`v0.1.0`). Linux, Windows and macOS
installers are available from the
[antitotem-v0.1.0 release](https://github.com/lucioaraujo/antitotem/releases/tag/v0.1.0)
(the repository is private, so this requires a GitHub account with access). This guide
also covers building from source and building/installing the `.deb` package yourself; see
[Windows/macOS](#windowsmacos) below for that platform's current validation status.

### Requirements

| | |
|---|---|
| OS | Linux (tested on Debian/Ubuntu, amd64) |
| Build tools | CMake ≥ 3.22, a C++20 compiler (GCC or Clang) |
| Dependency | A local checkout of [JUCE](https://github.com/juce-framework/JUCE) |
| Hardware | An audio device (ALSA/PipeWire) |

Runtime libraries needed to *run* the built app or install the `.deb` (already declared
in the package's `Depends`, install manually if building/running without the package):

```
libasound2, libfontconfig1, libfreetype6, libstdc++6, libc6,
libx11-6, libxext6, libxss1, libgio-2.0-0
```

The last four (`libx11-6`, `libxext6`, `libxss1`, `libgio-2.0-0`) are loaded by JUCE via
`dlopen()` at runtime rather than linked directly — without them the process starts but
the window never opens, with no error message. If that happens, install them first.

### Option A — build and run directly

Simplest path for immediate use. From the repository root:

```bash
./run_antitotem.sh
```

This configures a Release build in `/tmp/antitotem-simple-sequencer-app`, builds it, and
launches the binary. **The script currently hardcodes an absolute JUCE path from the
author's own machine** (`juce_dir` near the top of `run_antitotem.sh`) — on any other
machine, edit that line to point at your own JUCE checkout before running it. This is a
known portability gap, not yet fixed (see `docs/TAREFAS.md`); Option B/C below take
`ANTITOTEM_JUCE_PATH` as a normal CMake variable instead and work anywhere.

### Option B — build the `.deb` package

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DANTITOTEM_JUCE_PATH=/path/to/JUCE
cmake --build build --target AntitotemSimpleSequencerApp -j"$(nproc)"
cd build
cpack -G DEB
sudo dpkg -i antitotem-0.1.0-Linux.deb
sudo apt-get install -f   # resolves any missing dependency automatically
```

After installing, run `antitotem` from a terminal, or look for "Antitotem - Objeto
Sonoro" in your desktop's application menu (a `.desktop` entry is installed to
`/usr/share/applications/`).

To remove it later: `sudo dpkg -r antitotem`.

### Option C — manual build without packaging

If you just want the binary without a system-wide install:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DANTITOTEM_JUCE_PATH=/path/to/JUCE
cmake --build build --target AntitotemSimpleSequencerApp -j"$(nproc)"
"build/src/app/AntitotemSimpleSequencerApp_artefacts/Release/Antitotem - Objeto Sonoro"
```

### Building the core tests (no JUCE required)

The DSP core (`SimpleSequencer`, `DualObjectEngine`, `MelodicInterpreter`) has no JUCE
dependency and builds independently:

```bash
cmake -S . -B build-tests -DANTITOTEM_BUILD_APP=OFF -DANTITOTEM_BUILD_TESTS=ON
cmake --build build-tests -j"$(nproc)"
ctest --test-dir build-tests --output-on-failure
```

### Troubleshooting

- **Window never opens, no error**: missing `libx11-6`/`libxext6`/`libxss1`/
  `libgio-2.0-0` — see [Requirements](#requirements) above.
- **`ANTITOTEM_JUCE_PATH` warning during CMake configure**: point it at a directory
  containing JUCE's own `CMakeLists.txt` (the root of a JUCE source checkout).
- **ALSA `underrun occurred` messages / no sound**: check that no other JUCE audio app
  is holding the same audio device open at the same time — running two standalone JUCE
  audio apps simultaneously is a common cause of underruns unrelated to Antitotem itself.
- **Recordings and MIDI/MusicXML exports**: land in `~/Music/Antitotem Objeto Sonoro/`
  by default, or wherever `ANTITOTEM_RECORDINGS_DIR` points if that environment variable
  is set.

### Windows/macOS

Download the NSIS `.exe` (Windows) or DragNDrop `.dmg` (macOS) from the
[antitotem-v0.1.0 release](https://github.com/lucioaraujo/antitotem/releases/tag/v0.1.0).
Both are built and tested via
[GitHub Actions](https://github.com/lucioaraujo/antitotem/actions/workflows/package.yml)
on real Windows/macOS runners hosted by GitHub — no Windows or macOS machine is needed to
build these. The installers are produced successfully, but **haven't been opened or tested
on real Windows/macOS hardware yet**. If you have a Windows or macOS machine and want to
help validate this, download the release asset above and report back whether it installs
and opens.

### License

AGPLv3 or later. Full text in [`LICENSE`](LICENSE); third-party sources and their
licenses in [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md).

---

## Português

### Estado

Antitotem é um protótipo em investigação ativa (`v0.1.0`). Instaladores pra Linux,
Windows e macOS estão disponíveis na
[release antitotem-v0.1.0](https://github.com/lucioaraujo/antitotem/releases/tag/v0.1.0)
(o repositório é privado, então precisa de conta GitHub com acesso). Este guia também
cobre compilar a partir do código-fonte e gerar/instalar o pacote `.deb` você mesmo; ver
[Windows/macOS](#windowsmacos-1) abaixo pro estado de validação atual dessas plataformas.

### Requisitos

| | |
|---|---|
| Sistema | Linux (testado em Debian/Ubuntu, amd64) |
| Ferramentas de build | CMake ≥ 3.22, compilador C++20 (GCC ou Clang) |
| Dependência | Checkout local do [JUCE](https://github.com/juce-framework/JUCE) |
| Hardware | Dispositivo de áudio (ALSA/PipeWire) |

Bibliotecas de runtime necessárias pra *rodar* o app compilado ou instalar o `.deb` (já
declaradas no `Depends` do pacote; instale manualmente se compilar/rodar sem o pacote):

```
libasound2, libfontconfig1, libfreetype6, libstdc++6, libc6,
libx11-6, libxext6, libxss1, libgio-2.0-0
```

As últimas quatro (`libx11-6`, `libxext6`, `libxss1`, `libgio-2.0-0`) são carregadas
pelo JUCE via `dlopen()` em tempo de execução, não linkadas diretamente — sem elas o
processo inicia, mas a janela nunca abre, sem mensagem de erro nenhuma. Se isso
acontecer, instale-as primeiro.

### Opção A — compilar e rodar direto

Caminho mais simples para uso imediato. A partir da raiz do repositório:

```bash
./run_antitotem.sh
```

Isso configura um build Release em `/tmp/antitotem-simple-sequencer-app`, compila, e
abre o binário. **O script hoje tem um caminho absoluto do JUCE fixo na máquina do
autor** (`juce_dir`, perto do topo de `run_antitotem.sh`) — em qualquer outra máquina,
edite essa linha apontando pro seu próprio checkout do JUCE antes de rodar. É uma
lacuna de portabilidade conhecida, ainda não corrigida (ver `docs/TAREFAS.md`); as
Opções B/C abaixo recebem `ANTITOTEM_JUCE_PATH` como variável normal do CMake e
funcionam em qualquer lugar.

### Opção B — gerar o pacote `.deb`

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DANTITOTEM_JUCE_PATH=/caminho/para/JUCE
cmake --build build --target AntitotemSimpleSequencerApp -j"$(nproc)"
cd build
cpack -G DEB
sudo dpkg -i antitotem-0.1.0-Linux.deb
sudo apt-get install -f   # resolve automaticamente qualquer dependência faltando
```

Depois de instalado, rode `antitotem` no terminal, ou procure "Antitotem - Objeto
Sonoro" no menu de aplicativos do seu ambiente gráfico (uma entrada `.desktop` é
instalada em `/usr/share/applications/`).

Pra remover depois: `sudo dpkg -r antitotem`.

### Opção C — build manual sem empacotamento

Se você só quer o binário sem instalação no sistema:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DANTITOTEM_JUCE_PATH=/caminho/para/JUCE
cmake --build build --target AntitotemSimpleSequencerApp -j"$(nproc)"
"build/src/app/AntitotemSimpleSequencerApp_artefacts/Release/Antitotem - Objeto Sonoro"
```

### Compilando os testes do core (sem precisar de JUCE)

O núcleo DSP (`SimpleSequencer`, `DualObjectEngine`, `MelodicInterpreter`) não depende
de JUCE e compila de forma independente:

```bash
cmake -S . -B build-tests -DANTITOTEM_BUILD_APP=OFF -DANTITOTEM_BUILD_TESTS=ON
cmake --build build-tests -j"$(nproc)"
ctest --test-dir build-tests --output-on-failure
```

### Solução de problemas

- **Janela nunca abre, sem erro nenhum**: falta `libx11-6`/`libxext6`/`libxss1`/
  `libgio-2.0-0` — ver [Requisitos](#requisitos) acima.
- **Aviso `ANTITOTEM_JUCE_PATH` ao configurar o CMake**: aponte pra um diretório que
  contenha o próprio `CMakeLists.txt` do JUCE (raiz de um checkout do código-fonte).
- **Mensagens `underrun occurred` do ALSA / sem som**: confira se nenhum outro app de
  áudio JUCE está com o mesmo dispositivo de áudio aberto ao mesmo tempo — rodar dois
  apps standalone JUCE de áudio simultaneamente é uma causa comum de underrun sem
  relação com o próprio Antitotem.
- **Gravações e exportações MIDI/MusicXML**: caem em
  `~/Music/Antitotem Objeto Sonoro/` por padrão, ou onde a variável de ambiente
  `ANTITOTEM_RECORDINGS_DIR` apontar, se estiver definida.

### Windows/macOS

Baixe o `.exe` NSIS (Windows) ou o `.dmg` DragNDrop (macOS) na
[release antitotem-v0.1.0](https://github.com/lucioaraujo/antitotem/releases/tag/v0.1.0).
Os dois são gerados e testados via
[GitHub Actions](https://github.com/lucioaraujo/antitotem/actions/workflows/package.yml)
em runners Windows/macOS reais hospedados pelo GitHub — não precisa de máquina Windows
nem macOS pra gerar esses builds. Os instaladores são produzidos com sucesso, mas
**ainda não foram abertos nem testados num Windows/macOS de verdade**. Se você tem uma
máquina Windows ou macOS e quer ajudar a validar isso, baixe o asset da release acima e
relate se instala e abre.

### Licença

AGPLv3 ou posterior. Texto completo em [`LICENSE`](LICENSE); fontes de terceiros e
suas licenças em [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md).
