# AMD PMC Test Suite - Documentação de Testes

Este documento explica cada teste da suite de testes PMC para processadores AMD, incluindo objetivos, resultados esperados, como verificar baseline e estratégias para evitar regressões.

---

## 1. Visão Geral da Suite de Testes

A suite de testes AMD PMC é organizada em três componentes principais:

| Componente | Arquivo | Descrição |
|------------|---------|-----------|
| PMC Core L3 | `pmc_l3_test.c` | Testes para contadores de cache L3 (Zen+) |
| PMC Uncore DF | `pmc_df_test.c` | Testes para Data Fabric (Zen/Zen2) |
| PMC Uncore DF Zen4 | `pmc_df_zen4_test.c` | Testes para Data Fabric (Zen4/Zen5) |

### Requisitos
- FreeBSD 14.x ou superior
- Módulo `hwpmc` carregado (`kldload hwpmc`)
- Permissões de root (acesso a MSRs)
- CPU AMD com suporte a contadores de performance

---

## 2. Testes PMC Core L3 Counters

Os contadores L3 estão disponíveis em processadores AMD a partir da Família 17h (Zen).

### TC-L3-01: Detecção de Contadores L3

**Objetivo:** Verificar se o sistema detecta corretamente o suporte a contadores L3.

**O que o teste faz:**
1. Verifica se está rodando como root
2. Imprime informações da CPU (família, modelo)
3. Detecta suporte a contadores L3 via `cpu_supports_l3_counters()`

**Resultado esperado:**
- ✅ **PASS**: CPU suporta contadores L3 (Família >= 0x17)
- ⚠️ **SKIP**: CPU não suporta contadores L3 (Família < 0x17)
- ❌ **FAIL**: Erro na execução

**Como verificar baseline:**
```bash
# No primeiro teste, anote a saída:
./pmc_l3_test l3-detection
# Saída esperada:
# CPU Family: 0x17 (Zen) ou superior
# L3 Counters: Yes
```

**Evitar regressões:**
- Executar este teste antes de qualquer outro teste L3
- Se falhar, os outros testes L3 devem ser pulados automaticamente

---

### TC-L3-02: Contador de L3 Hits

**Objetivo:** Testar a contagem de acertos no cache L3.

**O que o teste faz:**
1. Aloca contador PMC para evento `L3_HIT`
2. Executa workload de acesso sequencial (alta taxa de acertos)
3. Lê e exibe o valor do contador

**Workload:**
```c
// Acesso sequencial - deve gerar altos L3 hits
for (i = 0; i < 1000000; i++) {
    array[(i * 16) % 4096] = i;  // 16KB cabe no L3
}
```

**Resultado esperado:**
- ✅ **PASS**: Contador > 0 (houve acertos no L3)
- ⚠️ **SKIP**: Evento L3_HIT não disponível
- ❌ **FAIL**: Erro na execução

**Como verificar baseline:**
```bash
# Primeiro teste - anote o valor:
./pmc_l3_test l3-hit
# Exemplo de saída:
# [RESULT] L3 Hit Count: 1234567
```

**Evitar regressões:**
- Registrar o valor baseline
- Variação aceitável: ±20% (depende da frequência da CPU)
- Se valor cair >50%, possível regressão no driver hwpmc

---

### TC-L3-03: Contador de L3 Misses

**Objetivo:** Testar a contagem de falhas no cache L3.

**O que o teste faz:**
1. Aloca contador PMC para evento `L3_MISS`
2. Executa workload de acesso com stride grande (gera cache misses)
3. Lê e exibe o valor do contador

**Workload:**
```c
// Stride de 256 elementos - causa L3 misses
for (i = 0; i < 1000000; i++) {
    array[(i * 256) % 1048576] = i;  // 4MB - maior que L3
}
```

**Resultado esperado:**
- ✅ **PASS**: Contador > 0 (houve misses no L3)
- ⚠️ **SKIP**: Evento L3_MISS não disponível
- ❌ **FAIL**: Erro na execução

**Como verificar baseline:**
```bash
./pmc_l3_test l3-miss
# Exemplo de saída:
# [RESULT] L3 Miss Count: 876543
```

**Evitar regressões:**
- Comparar com baseline registrado
- L3 misses devem ser consistentes para mesmo padrão de acesso

---

### TC-L3-04: Contador de L3 Evictions

**Objetivo:** Testar a contagem de linhas de cache evitadas do L3.

**O que o teste faz:**
1. Aloca contador PMC para evento `L3_EVICTION`
2. Executa workload que preenche e overflow o cache múltiplas vezes
3. Lê e exibe o valor do contador

**Workload:**
```c
// Preencher e overflow cache 100x
for (i = 0; i < 100; i++) {
    for (j = 0; j < 8192; j++) {
        array[j] = j;  // 32KB - preenche L3
    }
}
```

**Resultado esperado:**
- ✅ **PASS**: Contador > 0 (houve evictions)
- ⚠️ **SKIP**: Evento L3_EVICTION não disponível
- ❌ **FAIL**: Erro na execução

**Como verificar baseline:**
```bash
./pmc_l3_test l3-eviction
# [RESULT] L3 Eviction Count: 12345
```

**Evitar regressões:**
- Este teste é sensível ao tamanho do L3
- Baseline deve ser estabelecido para cada modelo de CPU

---

### TC-L3-05: Múltiplos Contadores L3

**Objetivo:** Verificar se múltiplos contadores L3 funcionam simultaneamente.

**O que o teste faz:**
1. Aloca dois contadores (L3_HIT e L3_MISS)
2. Executa workload misto (hits e misses)
3. Lê ambos os contadores

**Resultado esperado:**
- ✅ **PASS**: Pelo menos um contador > 0
- ⚠️ **SKIP**: Hardware não suporta contadores suficientes
- ❌ **FAIL**: Erro na execução

**Como verificar baseline:**
```bash
./pmc_l3_test l3-multiple
# [RESULT] L3 Hit Count: 500000
# [RESULT] L3 Miss Count: 100000
```

**Evitar regressões:**
- Verificar que ambos os contadores incrementam
- Regressão comum: um contador para de funcionar

---

## 3. Testes PMC Uncore DF (Zen/Zen2)

Os contadores Data Fabric estão disponíveis a partir da Família 15h (Bulldozer).

### TC-DF-01: Detecção de Contadores DF

**Objetivo:** Verificar suporte a contadores Data Fabric.

**O que o teste faz:**
1. Verifica se está rodando como root
2. Detecta suporte via `cpu_supports_df_counters()`
3. Imprime informações da CPU

**Resultado esperado:**
- ✅ **PASS**: CPU suporta DF counters (Família >= 0x15)
- ⚠️ **SKIP**: CPU não suporta DF counters

---

### TC-DF-02: Contador de Leituras de Memória DF

**Objetivo:** Testar contagem de leituras via Data Fabric.

**O que o teste faz:**
1. Aloca contador para `DF_MEM_READ` (ou alternativos)
2. Lê de /dev/zero e acessa memória
3. Exibe valor do contador

**Resultado esperado:**
- ✅ **PASS**: Contador > 0
- ⚠️ **SKIP**: Evento não disponível

**Baseline:**
```bash
./pmc_df_test df-reads
# [RESULT] DF Memory Read Count: 5000000
```

---

### TC-DF-03: Contador de Escritas de Memória DF

**Objetivo:** Testar contagem de escritas via Data Fabric.

**Baseline:**
```bash
./pmc_df_test df-writes
# [RESULT] DF Memory Write Count: 3000000
```

---

### TC-DF-04: Contador de Cache Snoop DF

**Objetivo:** Testar contagem de operações de snoop de cache.

**Nota:** Este evento pode requerer nível de privilégio kernel.

**Baseline:**
```bash
./pmc_df_test df-snoops
# [RESULT] DF Snoop Count: 12345
```

---

### TC-DF-05: Contadores DF Combinados

**Objetivo:** Verificar múltiplos contadores DF simultâneos.

**Baseline:**
```bash
./pmc_df_test df-combined
# [RESULT] DF Read Count: 2500000
# [RESULT] DF Write Count: 1500000
```

---

## 4. Testes PMC Uncore DF (Zen4/Zen5)

Contadores específicos para Zen4 (Raphael) e Zen5 (Granite Ridge).

### TC-Z4-01: Detecção Zen4/Zen5

**Objetivo:** Verificar se a CPU é Zen4 ou Zen5.

**O que o teste faz:**
- Verifica `cpu_is_zen4_or_later()` (Família 0x19)
- Verifica `cpu_is_zen5()` (Família 0x19, modelo >= 0x20)

---

### TC-Z4-02: Contador DF L3 Reads Zen4

**Objetivo:** Testar contador de leituras L3 específico do Zen4.

**Baseline:**
```bash
./pmc_df_zen4_test zen4-l3-reads
# [RESULT] Zen4 DF L3 Read Count: 1234567
```

---

### TC-Z4-03: Contador de Memória Zen4

**Objetivo:** Testar contador de controlador de memória Zen4.

**Baseline:**
```bash
./pmc_df_zen4_test zen4-mem-reads
# [RESULT] Zen4 DF Memory Read Count: 5000000
```

---

### TC-Z4-04: Contador NBIF (Infinity Fabric)

**Objetivo:** Testar contador de tráfego NBIF/Infinity Fabric.

**Baseline:**
```bash
./pmc_df_zen4_test zen4-nbif
# [RESULT] Zen4 NBIF Read Count: 98765
```

---

### TC-Z4-05: Contador de Probe Zen4

**Objetivo:** Testar contador de snoop probe.

**Baseline:**
```bash
./pmc_df_zen4_test zen4-probe
# [RESULT] Zen4 DF Probe Count: 54321
```

---

### TC-Z4-06: Múltiplos Contadores Zen4

**Objetivo:** Verificar múltiplos contadores simultâneos.

**Baseline:**
```bash
./pmc_df_zen4_test zen4-multiple
# [RESULT] Counter 1: 1000000
# [RESULT] Counter 2: 2000000
```

---

## 5. Como Estabelecer e Verificar Baseline

### 5.1 Primeiro Execusão (Estabelecer Baseline)

```bash
# Carregar módulo PMC
kldload hwpmc

# Executar todos os testes e salvar saída
cd tests/sys/amd/pmc
make

# Executar e salvar baseline
./pmc_l3_test all > baseline_l3.txt 2>&1
./pmc_df_test all > baseline_df.txt 2>&1
./pmc_df_zen4_test all > baseline_zen4.txt 2>&1

# Salvar em arquivo de baseline
git tag -a v1.0-baseline -m "Baseline PMC tests for $(uname -r)"
```

### 5.2 Verificar Regressões

```bash
# Executar testes novamente
./pmc_l3_test all > test_l3.txt 2>&1

# Comparar com baseline
diff baseline_l3.txt test_l3.txt
```

### 5.3 Valores Esperados por Tipo de Teste

| Tipo de Teste | Valor Esperado | Variação Aceitável |
|---------------|---------------|-------------------|
| L3 Hit | > 100000 | ±30% |
| L3 Miss | > 50000 | ±30% |
| L3 Eviction | > 1000 | ±50% |
| DF Read | > 1000000 | ±30% |
| DF Write | > 500000 | ±30% |
| NBIF | > 10000 | ±50% |

---

## 6. Estratégias para Evitar Regressões

### 6.1 Testes Automatizados

Adicione ao CI/CD (Jenkins):

```bash
# No Jenkinsfile
stage('PMC Tests') {
    steps {
        sh '''
            kldload hwpmc || true
            cd tests/sys/amd/pmc
            make test || true
        '''
    }
    post {
        always {
            junit '*.xml'
        }
    }
}
```

### 6.2 Monitoramento de Flakiness

- Flaky test: teste que passa/falha inconsistentemente
- Tracking: manter registro de falhas por execução
- Ação: se >10% de flakiness, investigar causa raiz

### 6.3 Checklist Pré-Commit

- [ ] Todos os testes passam localmente
- [ ] Baseline atualizado se necessário
- [ ] Sem mudanças em arquivos de kernel (verificar com `git diff`)
- [ ] Código compila sem warnings

### 6.4 Testes por Pull Request

| Tipo de PR | Testes Obrigatórios |
|------------|-------------------|
| Bug fix | TC-DET-* + afetados |
| Nova feature | Todos os testes |
| Mudança kernel | Todos os testes |

---

## 7. Troubleshooting

### Problema: "pmc_init failed"

**Causa:** Módulo hwpmc não carregado

**Solução:**
```bash
kldload hwpmc
kldstat | grep hwpmc
```

### Problema: "pmc_allocate failed: Invalid argument"

**Causa:** Evento PMC não suportado pelo hardware

**Solução:** O código tenta eventos alternativos automaticamente

### Problema: "Must be root"

**Causa:** Sem privilégios para acessar MSRs

**Solução:**
```bash
sudo ./pmc_l3_test all
```

### Problema: Contador sempre zero

**Causa:** Hardware não suporta o evento

**Solução:**
```bash
# Listar eventos disponíveis
pmcstat -M
```

---

## 8. Referências

- Manual do programador AMD, Volume 2: MSRs
- `hwpmc(4)` - FreeBSD man page
- FreeBSD PMC architecture: `/usr/src/sys/dev/hwpmc/`

---

*Documento criado em Fevereiro 2026*
*Autor: AMD IBS Test Team*
