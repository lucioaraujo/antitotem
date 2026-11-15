# Antitotem e o Maestruc

Registro de 15/11/2026. O Maestruc é o regente da família RASGO
(`RASGO/PROJETOS_AUTORAIS/MAESTRUC` no monorepo privado): abre os
instrumentos, mixa e grava os sons deles e manda comandos pelo protocolo
`maestruc.link/1` (OSC só na máquina local, porta 47900).

## O que o Antitotem aceita

Desligado por padrão; liga com `--maestruc` ou `ANTITOTEM_MAESTRUC=1`. Sem a
opção, nada muda.

| Do Maestruc | No Antitotem |
|---|---|
| tocar / parar | os mesmos botões PLAY e STOP (com o mesmo cuidado do REC pendente) |
| estado | tocando ou não, a seed da abertura e o BPM instantâneo do PRINCIPAL |
| mixer | o mixer de objetos: PRINCIPAL, CLONE e EXCITAÇÃO (nível e M) e o MASTER (nível), pelos mesmos controles da tela; SOLO recusado (esses canais não têm) |
| som | a mistura vai ao Maestruc pela ponte de som (memória compartilhada, Linux/Windows/macOS); enquanto ele lê, o Antitotem cala a própria saída e volta sozinho em ~0,5 s se ele some; gravação WAV/MIDI intacta |

Os comandos só valem com a senha da sessão que o Maestruc grava num arquivo
só do usuário (15/11/2026, versão 0.2.0).

**`--seed N`** (até 18 algarismos) repete uma abertura: o sorteio de abertura
(sequenciador, quinto objeto, variação e passos) agora sai de uma seed. Sem
a opção, a seed é sorteada como antes e só fica guardada para o estado; o
comportamento é o mesmo de sempre. Seed e tempo com o app aberto respondem
"capacidade ausente": a seed só entra ao abrir, e o andamento é o do
PRINCIPAL, não um relógio comum.

## Arquivos

- `src/app/MaestrucLink.h` e `src/app/PonteDeSom.h`: cópias do receptor de
  referência (canal, ponte de som, senha), com a origem e o SHA-256 anotados.
- `src/app/Main.cpp`: seed de abertura e `enableMaestrucLink()` no
  `MainComponent`; o receptor é desfeito antes de tudo no destrutor.
- `src/app/CMakeLists.txt`: módulo `juce_osc`.

## Validação (15/11/2026)

Build do `run_antitotem.sh` (`/tmp/antitotem-simple-sequencer-app`, Release):
sem avisos novos (o de `createWriterFor` já existia), 1/1 teste. Ensaio sem
som (saída nula do PipeWire, HOME temporário): o app se anunciou
(`transport`), estado com a seed 123456 de `--seed`, PLAY e STOP aceitos
(estado confirmou), seed e tempo recusados como previsto. Não ensaiado: com
som e com o autor tocando; se a mesma `--seed` repete a mesma abertura de
ouvido.
