#!/usr/bin/env bash
# Compila e abre sempre a versão atual do Objeto Sonoro.
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="/tmp/antitotem-simple-sequencer-app"
# JUCE numa pasta neutra do workspace (RASGO/third_party), na mesma versão
# da CI (9.0.3), e não mais dentro do Rasgo Synth: regra RASGO de
# instrumentos autônomos (GOVERNANCA §7.0, 7 out. 2026). ANTITOTEM_JUCE_DIR
# troca o caminho.
juce_dir="${ANTITOTEM_JUCE_DIR:-$project_dir/../third_party/JUCE-9.0.3}"
app_path="$build_dir/src/app/AntitotemSimpleSequencerApp_artefacts/Release/Antitotem - Objeto Sonoro"

if [[ ! -f "$juce_dir/CMakeLists.txt" ]]; then
    echo "JUCE não encontrado em: $juce_dir" >&2
    exit 1
fi

# Release, não Debug (17 ago. 2026): underrun de ALSA recorreu (não foi
# soluço pontual). Margem medida em CPU_BASELINE.md - ~2,5x tempo real em
# Debug no dispositivo real do autor (48kHz/quantum 1024) contra ~9-10x em
# Release - contingência já mapeada, aplicada agora que o sintoma voltou.
cmake -S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DANTITOTEM_JUCE_PATH="$juce_dir"
cmake --build "$build_dir" -j2
exec "$app_path"
