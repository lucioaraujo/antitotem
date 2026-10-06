# Antitotem — Installation Guide / Guia de Instalação

Two languages, same content: [🇬🇧 English](#english) below, [🇧🇷 Português](#português) further down.
**To install, start with [Installing, step by step](#installing-step-by-step) / [Instalar, passo a passo](#instalar-passo-a-passo).**

---

## English

### Installing, step by step

Packages for every system are on the
[releases page](https://github.com/lucioaraujo/antitotem/releases) (the
newest one is at the top). The security warnings below **are expected**: they do not mean anything is
wrong, and they only appear the first time.

#### Windows

1. Download `antitotem-<version>-win64.exe`.
2. Open it. Windows shows a blue **"Windows protected your PC"** box: click
   **"More info"**, then **"Run anyway"**.
3. If Windows asks for administrator permission, click **"Yes"**.
4. Click **"Next"** to the end; from v0.1.1 on, the last page has **"Run
   Antitotem - Objeto Sonoro"** already ticked. It is then in the **Start
   menu** and on the desktop.

**Without installing** (from v0.1.1): unzip `antitotem-<version>-win64.zip`
and open `Antitotem - Objeto Sonoro.exe`.

**v0.1.0 and nothing happens**, or an error about `VCRUNTIME140.dll` or
`MSVCP140.dll`: that version silently required the "Microsoft Visual C++
Redistributable". Install it from
<https://aka.ms/vs/17/release/vc_redist.x64.exe> and open Antitotem again.
From v0.1.1 on, everything is inside the `.exe`.

#### macOS

1. Download `antitotem-<version>-Darwin.dmg`. From v0.1.1 on, it works on
   both Intel and Apple Silicon Macs. v0.1.0 was Apple Silicon only.
2. Open it and **drag Antitotem into Applications**.
3. Open it. The first time, macOS **blocks** the app with a message saying
   **Apple could not confirm it is free of malicious software**. In French,
   for example: *« Antitotem ne peut pas être ouvert. Apple n'a pas pu confirmer
   que Antitotem ne contenait pas de logiciel malveillant. »*
   A similar message may appear instead, such as \"cannot verify the
   developer\" or \"is damaged and can't be opened\". In every case, do the
   same:
   - This is **not a defect or a virus**.
   - Click **"OK"** or **"Done"**, **not** "Move to Trash".
4. Allow the app in one of two ways. You only need to do this once.
   - **In System Settings:** go to **System Settings → Privacy & Security**,
     scroll to the security section, click **"Open Anyway"**, confirm with
     your password or Touch ID, and click **"Open"**. On macOS 14 or
     earlier: right-click the app in Finder → **Open → Open**.
   - **In Terminal**, if the button does not appear or the block remains:
     1. Open **Terminal**. It is in Applications → Utilities, or press
        Cmd + Space and type "Terminal".
     2. Paste this line and press **Enter**:

        ```sh
        xattr -dr com.apple.quarantine "/Applications/Antitotem - Objeto Sonoro.app"
        ```

        If nothing is printed after Enter, it worked.
     3. Close Terminal and open the app normally.

     The command only removes the "quarantine mark" macOS puts on every
     downloaded file. It does not change the app.
#### Linux

- **Ubuntu 22.04+, Debian 12+, Mint 21+:** double-click the `.deb`, or run
  `sudo apt install ./antitotem-<version>-Linux.deb`. apt installs the
  dependencies.
- **Fedora, Arch, openSUSE and other distributions** (from v0.1.1): use
  the **AppImage**. Make it executable (Properties → allow executing, or
  `chmod +x`) and open it. Or use the **`.tar.gz`**: unpack it and run
  `./install.sh`, which installs for your user only, without root
  (`./install.sh --remove` undoes it).
- **The window never opens, with no error:** the X11 libraries the app
  loads at run time are missing (see "Requirements to build" below).

### Minimum requirements to run

| | |
|---|---|
| **Windows** | Windows 10 or 11, 64-bit |
| **macOS** | Intel: macOS 10.13 or later. Apple Silicon: macOS 11 or later (Universal 2 from v0.1.1) |
| **Linux** | x86-64 with glibc 2.35 or newer (2022+ distributions): `.deb` for the Debian family; AppImage or `.tar.gz` for the rest (launch-tested on Debian 12, Ubuntu 24.04, Fedora and Arch) |
| **Processor** | 64-bit, two cores or more. **Measured** on an Intel Core i5-6500 (2015, 4 cores, 3.2 GHz): the whole app uses about 60% of one core; the sound engine alone 10–11% at 44.1/48 kHz (worst case, both objects fully patched — `CPU_BASELINE.md`) |
| **Memory** | about 45 MB in use (measured); any computer with 4 GB is enough |
| **Audio** | any system audio output |

The operating-system minimums come from the build. The processor and memory
figures were measured on the author's machine on 6 Oct. 2026; weaker
machines have not been tested.

### Status

Antitotem is a prototype under active investigation. Packages for Linux, Windows and
macOS are on the [releases page](https://github.com/lucioaraujo/antitotem/releases)
(how to install them: [step by step](#installing-step-by-step) above). The rest of this
guide covers building from source and building the `.deb` yourself.

### Requirements to build

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

Installing: see [step by step](#installing-step-by-step) above. The Windows and macOS
packages are built by
[GitHub Actions](https://github.com/lucioaraujo/antitotem/actions/workflows/package.yml),
which since v0.1.1 also checks that the `.exe` does not depend on the Visual C++
Redistributable and that the macOS bundle is Universal 2 and sealed. They **have not been
opened by the author on real Windows/macOS machines yet**. If you have one, please report
whether it installs and opens.

### License

AGPLv3 or later. Full text in [`LICENSE`](LICENSE); third-party sources and their
licenses in [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md).

---

## Português

### Instalar, passo a passo

Os pacotes de cada sistema estão na
[página de releases](https://github.com/lucioaraujo/antitotem/releases), com
a mais nova no topo. Os avisos de segurança abaixo **são esperados**:
não indicam defeito nem vírus e só aparecem na primeira vez.

#### Windows

1. Baixe `antitotem-<versão>-win64.exe`.
2. Abra-o. O Windows mostra a janela azul **"O Windows protegeu o
   computador"**: clique em **"Mais informações"** e depois em
   **"Executar assim mesmo"**.
3. Se o Windows pedir permissão de administrador, clique em **"Sim"**.
4. Clique em **"Avançar"** até o fim. A partir da v0.1.1, a última tela já
   traz **"Executar o Antitotem - Objeto Sonoro"** marcado. Depois disso,
   ele fica no **Menu Iniciar** e na área de trabalho.

**Sem instalar** (a partir da v0.1.1): descompacte
`antitotem-<versão>-win64.zip` e abra `Antitotem - Objeto Sonoro.exe`.

**Se você tem a v0.1.0 e nada acontece ao abrir**, ou aparece um erro sobre
`VCRUNTIME140.dll` ou `MSVCP140.dll`: essa versão exigia, sem avisar, o
"Microsoft Visual C++ Redistributable". Instale-o pelo link da Microsoft
(<https://aka.ms/vs/17/release/vc_redist.x64.exe>) e abra o Antitotem de
novo. A partir da v0.1.1 isso não é mais necessário, porque tudo vai dentro
do `.exe`.

#### macOS

1. Baixe `antitotem-<versão>-Darwin.dmg`. A partir da v0.1.1 o mesmo
   arquivo serve para Mac Intel e Apple Silicon. A v0.1.0 era só para
   Apple Silicon.
2. Abra o `.dmg` e **arraste o Antitotem para a pasta Aplicativos**.
3. Abra o app. Na primeira vez, o macOS **bloqueia** o app e mostra uma
   mensagem dizendo que **a Apple não pôde confirmar que ele está livre de
   software malicioso**. Em francês, por exemplo: *« Antitotem ne peut pas être
   ouvert. Apple n'a pas pu confirmer que Antitotem ne contenait pas de logiciel
   malveillant. »*
   Também pode aparecer uma mensagem parecida, como \"não é possível
   verificar o desenvolvedor\" ou \"está danificado e não pode ser aberto\". Em
   todos esses casos, faça o mesmo:
   - Isso **não indica defeito nem vírus**.
   - Clique em **"OK"** ou **"Concluído"**. **Não** clique em "Mover para o
     Lixo".
4. Libere o app de um destes dois jeitos. Basta fazer uma vez.
   - **Pelos Ajustes:** abra **Ajustes do Sistema → Privacidade e
     Segurança**, desça até a parte de segurança e clique em **"Abrir Mesmo
     Assim"**. Confirme com sua senha ou Touch ID e clique em **"Abrir"**.
     No macOS 14 ou anterior: clique no app com o botão direito no Finder e
     escolha **Abrir → Abrir**.
   - **Pelo Terminal**, se o botão não aparecer ou o bloqueio continuar:
     1. Abra o **Terminal**. Ele fica em Aplicativos → Utilitários, ou use
        Cmd + Espaço e digite "Terminal".
     2. Cole a linha abaixo e aperte **Enter**:

        ```sh
        xattr -dr com.apple.quarantine "/Applications/Antitotem - Objeto Sonoro.app"
        ```

        Se nada aparecer depois do Enter, deu certo.
     3. Feche o Terminal e abra o app normalmente.

     O comando só retira a "marca de quarentena" que o macOS põe em todo
     arquivo baixado. Ele não altera o app.

#### Linux

- **Ubuntu 22.04+, Debian 12+ e Mint 21+:** dê dois cliques no `.deb` ou
  rode `sudo apt install ./antitotem-<versão>-Linux.deb`. O apt instala as
  dependências.
- **Fedora, Arch, openSUSE e outras distribuições** (a partir da v0.1.1):
  use o **AppImage**. Marque o arquivo como executável (propriedades →
  permitir executar, ou `chmod +x`) e abra. Ou use o **`.tar.gz`**:
  descompacte-o e rode `./install.sh`, que instala no seu usuário, sem
  root. `./install.sh --remove` desfaz a instalação.
- **A janela nunca abre e não aparece erro:** faltam as bibliotecas X11
  que o app carrega ao rodar (ver "Requisitos para compilar", mais abaixo).

### Requisitos mínimos para usar

| | |
|---|---|
| **Windows** | Windows 10 ou 11, 64 bits |
| **macOS** | Intel: macOS 10.13 ou posterior. Apple Silicon: macOS 11 ou posterior (Universal 2 a partir da v0.1.1) |
| **Linux** | x86-64 com glibc 2.35 ou mais nova (distribuições de 2022 em diante): `.deb` para a família Debian; AppImage ou `.tar.gz` para as demais (testados na abertura em Debian 12, Ubuntu 24.04, Fedora e Arch) |
| **Processador** | 64 bits, dois núcleos ou mais. **Medido** num Intel Core i5-6500 (2015, 4 núcleos, 3,2 GHz): o app inteiro usa cerca de 60% de um núcleo; o motor de som sozinho, 10–11% a 44,1/48 kHz (no pior caso, os dois objetos totalmente patchados; ver `CPU_BASELINE.md`) |
| **Memória** | cerca de 45 MB em uso (medido); qualquer computador com 4 GB basta |
| **Áudio** | qualquer saída de áudio do sistema |

O mínimo de sistema operacional vem do build. Os números de processador e
de memória foram medidos na máquina do autor em 6 out. 2026; máquinas mais
fracas não foram testadas.

### Estado

O Antitotem é um protótipo em investigação ativa. Os pacotes para Linux, Windows e
macOS estão na [página de releases](https://github.com/lucioaraujo/antitotem/releases),
e como instalá-los está no [passo a passo](#instalar-passo-a-passo) acima. O resto deste
guia trata de compilar a partir do código e de gerar o `.deb` você mesmo.

### Requisitos para compilar

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

Para instalar, veja o [passo a passo](#instalar-passo-a-passo) acima. Os pacotes de
Windows e macOS são gerados pelo
[GitHub Actions](https://github.com/lucioaraujo/antitotem/actions/workflows/package.yml).
Desde a v0.1.1, a CI também confere que o `.exe` não depende do Visual C++
Redistributable e que o pacote macOS é Universal 2 e selado. Eles **ainda não foram
abertos pelo autor num Windows ou macOS de verdade**. Se você tiver uma dessas
máquinas, conte se instala e abre.

### Licença

AGPLv3 ou posterior. Texto completo em [`LICENSE`](LICENSE); fontes de terceiros e
suas licenças em [`CREDITS_AND_SOURCES.md`](CREDITS_AND_SOURCES.md).
