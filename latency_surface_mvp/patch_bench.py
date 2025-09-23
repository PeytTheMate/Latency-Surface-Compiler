from pathlib import Path
p = Path("src/bench_main.cpp")
if not p.exists():
    print("ERROR: src/bench_main.cpp not found at", p.resolve())
    raise SystemExit(1)

text = p.read_text()

# backup
bak = p.with_suffix(".cpp.bak")
if not bak.exists():
    bak.write_text(text)
    print("Backup written to", bak)

if "hot_find_level(" in text:
    print("hot_find_level already present — no change made.")
else:
    inserted = False
    # If a proper struct exists, insert the stub after it
    idx = text.find("struct QuoteAoS")
    if idx != -1:
        end_idx = text.find("};", idx)
        if end_idx != -1:
            end_idx += 2
            stub = "\nstatic inline uint64_t hot_find_level(QuoteAoS* /*data*/, int /*n*/, float /*v*/) { (void)/*data*/; (void)/*n*/; (void)/*v*/; return 0ULL; }\n"
            text = text[:end_idx] + stub + text[end_idx:]
            inserted = True
    # If only vector line exists, replace it with struct + vector + stub
    if not inserted and "std::vector<QuoteAoS> b(N);" in text:p = Path("src/bement = (
            "struct QuoteAoS {   oat price; uint32    raise SystemExit(1)

text = vector<QuoteAoS> b(N);\n"
       
text = p.read_line uint64_t hot_find_level(Quobak = p/*data*/, int /*n*/, float /*v*/)     bak.writta*/; (void)/*n*/; (void)/*v*/; re
if "hot_find_"
        )
        tex    print("hot_find_levelctor<else:
    inserted = False
    # If a proper sd = True

    if inserted:
        p.write_te    idx = text.find("struct QuoteAoS")
    if idx != into src/bench_main.cpp")
    else:
           nt("Could not f        if end_idx != -1:
         t around QuoteAoS occurrences:            stub = text.s            text = text[:end_idx] + stub + text[end_idx:]
            inserted = True
    # If only vector line exists, replace it with struct 0, i-6)
                           inserted = T+6)
                print("\n".joi    # If only lines[n-1]}" f    if not inserted and "std::vector        break
