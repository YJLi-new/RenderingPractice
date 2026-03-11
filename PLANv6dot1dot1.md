# PLANv6.1.1 – Wide Writer Bandwidth, Mid‑Layer Injection, and Forced Memory Consumption

## Purpose

The purpose of this document is to define a concrete, code‑driven execution plan for **Phase V6.1.1** of the MemTOTAL project.  This phase is motivated by the findings in V6‑4 and V6‑5 and the critiques of PLANv6.1.  Our goal is to fairly assess the viability of an **external Writer** by systematically expanding its bandwidth, injecting memory at mid‑network layers, and forcing the backbone to consume memory.  If the external Writer cannot deliver meaningful gains under these conditions, we will have strong evidence that the architecture’s limitations are fundamental rather than contingent.


## Key Findings from V6‑4/V6‑5 and Critiques

1. **Rank‑1 Writer collapse** is now irrefutable.  Both V6‑4 and V6‑5 reported `writer_memory_slot_effective_rank ≈ 1.0` while `projected_memory_effective_rank ≈ 17–21`.  The projector is manufacturing diversity from a collapsed Writer source【994678294201948†screenshot】.
2. **C2 (support & context gating) > C0 (support only)** does not mean per‑sample gating; it is a global learned scalar after layer‑norm.  The Reader has not learned when to trust memory; it simply scales memory and context contributions【994678294201948†screenshot】.
3. **Additive layer sets** that include mid‑network layers (e.g., [0,1,2,3,4,8,14]) were the only recipes that stabilized FEVER in V6‑5.  This suggests that mid‑layer injection may be beneficial and must be prioritized.
4. **V6‑5’s stabilized recipe** (S3 + C2 + L5 + group‑wise clipping + projector LR 7.5e‑6 + acc=4 + additive layers) should become the default baseline for V6.1.  FEVER is solved and should no longer be the gating task.
5. **GSM8K and NarrativeQA remain flat**.  Even with the V6‑5 recipe, GSM8K’s `task_score_delta` is zero.  NarrativeQA became route‑live and stable for the first time, but usefulness remains zero.  Thus the Writer’s ability to encode useful long‑form information is still unproven.
6. **Reviewer critiques** emphasize that collapse is a symptom of gradient starvation and injection depth limitations, not just lack of diversity.  They advocate mid‑layer injection and bandwidth expansion before exploring new aux losses.  Forced memory consumption (e.g., masking the main prompt) is necessary to make the backbone actually use the Writer output.


## Hard Requirements from the User

1. **Do not abandon the external Writer.**  Instead, dramatically expand its memory bandwidth (more tokens, higher dimensionality).  Memory output can be very wide; the Reader will compress it later.  GPU budget: single RTX PRO 6000 with 96 GB.
2. **Use GSM8K and TriviaQA as primary benchmarks.**  FEVER is now a calibration check rather than a gate.
3. **Prioritize injection into mid‑network layers [12,13,14,15]** in Qwen‑2.5‑1.5B.  Early‑layer injection remains a baseline but is no longer the default.
4. **Start from the best V6‑4 recipe: S3 + C2 + L5.**  V6‑5’s improvements (groupwise clipping, acc=4, projector LR 7.5e‑6, additive layers) are to be integrated as defaults.
5. **V6‑5’s stabilized recipe becomes the baseline for all V6.1 phases.**  We will no longer sweep over groupwise vs. global clipping or projector LR; these are fixed at the V6‑5 optimum.


## Summary of the Architecture under Test

- **Backbone:** Qwen‑2.5‑1.5B‑Instruct (28 layers, hidden = 1536).
- **External Writer:** independent MLP/Transformer generating latent memory tokens from support items.
- **Projector:** maps each Writer token through a bottleneck to per‑layer K/V prefixes.  In V6‑5 this uses a shared `down_proj` → `bottleneck_rank=32` → `up_proj` architecture.
- **Receiver (LoRA):** micro‑LoRA adapters on `k_proj`/`v_proj` at selected layers.
- **Support Interface (S3):** multi‑item cross‑attention providing the Writer with structured evidence (one key/value per support item).
- **Context Gating (C2):** global learned scalar balancing Writer and context contributions.
- **Aux Loss (L5):** orthogonality + coverage loss encouraging diversity among memory slots.


## New Variables to Explore

We will introduce the following new variables:

1. **Writer Bandwidth (W)** – number of output slots, dimensionality, and number of layers in the Writer.  Four levels:
   - **W0 (baseline)** – 8 slots × 128 dim × 2 layers (current default).
   - **W1 (moderate)** – 16 slots × 256 dim × 2 layers; projector bottleneck doubled to 64.
   - **W2 (wide)** – 32 slots × 512 dim × 3 layers; projector bottleneck doubled again (128) and optionally per‑layer projectors.
   - **W3 (very wide)** – 64 slots × 1024 dim × 4 layers; projector bottleneck increased to 256 and possibly per‑layer projectors.
   Note: GPU memory must be monitored carefully; if W3 exceeds 96 GB GPU budget, we fall back to W2.

2. **Injection Depth (D)** – the set of layers where memory K/V prefixes are injected:
   - **D_base:** [0,1,2,3] (V6‑5 baseline).
   - **D_mid:** [12,13,14,15] (primary hypothesis).
   - **D_additive:** [0,1,2,3,4,8,14] (V6‑5 winner) as a stable baseline; injection at early + mid layers.

3. **Force‑Consumption Mechanism (F)** – methods to compel the backbone to rely on memory:
   - **F0 (control):** no forced consumption.
   - **F1 (information starvation):** mask key tokens (numbers, entities) from the backbone prompt while supplying full source to the Writer.  For GSM8K, mask numerical values; for TriviaQA, mask evidence passages.
   - **F2 (asymmetric gradient decoupling):** freeze Writer & projector (pretrained from W0 baseline), train LoRA adapters alone for 50–100 steps to encourage the receiver to learn to consume memory, then jointly train.

4. **Auxiliary Loss Variants (A)** – we keep L5 as default but test a reconstruction‑style loss:
   - **A0:** no aux loss (ablate L5 entirely).
   - **A1:** orthogonality + coverage (L5, baseline).
   - **A2:** reconstruction loss: mean-pooled Writer output must predict a bag‑of‑entities or keyword distribution extracted from source.  Provides a direct gradient signal about what to encode.


## Phases and Gates

We structure V6.1.1 into seven phases.  Each phase has specific go/no‑go gates.  If a gate fails, we re‑evaluate whether to continue the external Writer program.

### Phase 0: Oracle Injection Gate (V6.1‑0)

**Goal:** Determine if early vs. mid‑layer injection has headroom.  We bypass the Writer entirely by extracting the backbone’s own hidden states as the prefix.

1. Run the baseline V6‑5 recipe (S3+C2+L5, W0, D_base and D_mid) on GSM8K and TriviaQA.  Compare to no‑memory control.
2. Create an **oracle prefix**: run the backbone on the full support text, extract hidden states at layers [0,1,2,3] or [12,13,14,15], then inject these as deep prefixes (zero‑source).  Evaluate on the same tasks.

**Gate:** If the oracle cannot improve GSM8K or TriviaQA by at least **+0.025 absolute score** vs. control, then early‑layer injection cannot carry useful information and we must consider alternative integration methods (e.g., Perceiver cross‑attention or in‑model writer).  If the oracle improves mid‑layer but not early‑layer, we prioritize D_mid for all subsequent phases.

### Phase 1: Joint Width × Depth Screen (V6.1‑1)

**Goal:** Explore Writer bandwidth and injection depth simultaneously, using the best recipe from V6‑5.  Avoid running a full W×D×F×A matrix at once; first identify promising W and D combinations.

1. Fix F0 (no forced consumption) and A1 (orthogonality + coverage).  Use S3+C2+L5 baseline: groupwise clipping, acc=4, projector LR 7.5e‑6.
2. Run a 2×2 screen: W0, W1 × D_base, D_mid × tasks (FEVER, GSM8K, TriviaQA).  Evaluate for 500 steps.
3. Promotion criteria: `task_score_delta > 0` for GSM8K or TriviaQA, `writer_memory_slot_effective_rank > 1.1` and `source_not_collapsed = true`.  If neither W1 at D_mid nor W1 at D_base meets the gate, skip wider Writers.

**Gate:** If W1 shows no improvement over W0 in any metric, we must revisit the assumption that more bandwidth helps.  Otherwise, proceed with the best W and D.

### Phase 2: Wider Bandwidth Screen (V6.1‑2)

**Goal:** Expand Writer capacity further (W2/W3) at the best depth from Phase 1.

1. Fix depth to the winning D (`D_best` from Phase 1).  Fix F0 and A1.
2. Run W2 and optionally W3 if memory permits.  Evaluate on GSM8K and TriviaQA for 500 steps.
3. Monitor GPU memory; if W3 exceeds budget, drop to W2.  Use per‑layer projectors or increase bottleneck rank to avoid new bottlenecks.

**Gate:** Select the minimal W that improves `task_score_delta` or `writer_memory_slot_effective_rank` above Phase 1.  If W2 and W3 both collapse or yield no improvement, then Writer width alone cannot solve collapse – move on to injection architecture.

### Phase 3: Compression Bridge / Reader Re‑Opening (V6.1‑3)

**Goal:** Introduce a compression bridge (Reader) to map the wide Writer memory to a fixed number of slots, mitigating CDMI and aligning with the eventual `M_long → M_short` goal.

1. Implement `M_short` Reader: cross‑attend from a small number of queries (8 or 16) to the wide Writer output.  Reuse support_interface (S3) for the Reader to attend over Writer tokens.
2. Train only the Reader + LoRA, freeze the Writer & projector, for 50 steps; then jointly train for 450 steps.  Continue using the best W and D from Phase 2.
3. Evaluate on GSM8K and TriviaQA.  Gate: `task_score_delta > 0` and `reader_usefulness_positive`.  If the Reader improves TriviaQA or GSM8K, we keep it; otherwise, it becomes a fallback.

**Note:** This phase aligns with the original PLANv6 “V6‑7 reader reopening” but integrates it with the V6.1 bandwidth discoveries.

### Phase 4: Forced Memory Consumption (V6.1‑4)

**Goal:** Force the backbone to rely on memory using information starvation and asymmetric gradient training.

1. Use the best Writer/Reader combination from Phase 3.  Evaluate F1 (mask numbers/entities), F2 (receiver‑only warm‑up), and F0 (control).
2. For F1, design task‑aware masking: For GSM8K, remove numbers and arithmetic operators from the backbone prompt; for TriviaQA, remove the evidence passage.  The Writer sees the full information.
3. For F2, freeze the Writer & projector, train only the LoRA adapters and Reader for the first 50–100 steps to increase memory consumption; then unfreeze and continue joint training.
4. Evaluate each F variant for 500 steps.  The key metric is whether `task_score_delta` becomes > 0 on GSM8K or TriviaQA.

**Gate:** If forced consumption yields the first nonzero GSM8K or TriviaQA improvement, adopt the corresponding F.  If none works, the external Writer may be fundamentally limited.

### Phase 5: Auxiliary Loss Revisit (V6.1‑5)

**Goal:** Explore new auxiliary losses beyond diversity.  Only relevant if the Writer is alive after Phases 0–4.

1. Test A0 (no aux loss), A1 (diversity), and A2 (reconstruction).  Use the best W, D, F combination from previous phases.
2. For A2, compute a bag‑of‑entities or keyword vector for each source (e.g., using named entity extraction).  Mean‑pool the Writer outputs and predict this vector via a linear head.  Use a cross‑entropy or KL‑divergence loss scaled by λ_recon (annealed from 1 to 0.1 over training).
3. Evaluate on GSM8K and TriviaQA for 500 steps.

**Gate:** A variant is promoted if it improves `writer_task_supervision_live` (task gradients reaching the Writer) and `task_score_delta`.  If none improve beyond baseline, the diversity loss remains as default.

### Phase 6: Confirmation and Comparison (V6.1‑6)

**Goal:** Consolidate the best combination of variables and compare against existing baselines, including MemGen and Oracle injection.

1. Re‑run FEVER, GSM8K, and TriviaQA with the final recipe, plus:
   - **MemGen‑style baseline:** implement an in‑model Weaver (LoRA on the backbone) and evaluate to gauge the benefit of external vs. in‑model Writer.
   - **Oracle injection baseline:** re‑inject backbone hidden states at the chosen depth to establish an upper bound.
2. Evaluate on multiple seeds (≥3) to measure stability.  Document metrics, including `task_score_delta`, `delta_answer_logprob`, `writer_memory_slot_effective_rank`, `source_not_collapsed`, `writer_task_supervision_live`.

**Gate:** If the final external Writer recipe does not outperform the MemGen baseline or the Oracle injection baseline on TriviaQA or GSM8K by a meaningful margin, then the external Writer architecture is likely inferior.  At that point, re‑evaluate the project’s focus.


## Time & Compute Budget

- Each phase (0–5) will run a matrix of ≈12–24 arms.  On a single RTX PRO 6000 (96 GB), 500 training steps per arm takes ~1–2 hours.  We plan for one phase per 24–36 hours.  The whole V6.1.1 program therefore requires ~10–14 days.
- We set a **kill criterion:** If Phase 0’s oracle injection shows < +0.025 improvement on GSM8K and TriviaQA at mid‑layer injection, we will halt the external Writer program and pivot to alternative architectures (e.g., in‑model Weaver or Perceiver injection).


## Summary and Rationale

This updated plan is grounded in the real failure modes observed in V6‑4/V6‑5 and the critiques from reviewers.  It prioritizes injection depth and bandwidth before exploring more sophisticated losses, acknowledges gradient starvation as the root cause of collapse, and includes forced consumption strategies to break the backbone’s dependence on its own prompt.  By introducing an oracle‑based gating experiment, we prevent wasted cycles on an architecture that may be intrinsically limited.  By integrating a Reader/compression bridge and a reconstruction auxiliary, we open avenues for semantically richer memory without abandoning the external Writer.

We will treat **TriviaQA** (factual recall) and **GSM8K** (complex reasoning) as co‑primary tasks.  FEVER remains a calibration benchmark but will no longer gate architectural decisions.  All experiments will use the V6‑5 optimized training recipe as the default (S3+C2+L5, groupwise clipping, acc=4, projector LR 7.5e‑6, additive layers) unless explicitly superseded.

With this roadmap, we give the external Writer its fairest chance.  If it still fails to produce meaningful improvements after these steps, we will have strong justification to pivot to in‑model memory mechanisms.
