"""Read-only recommendation; never changes plugin defaults."""
import csv, pathlib, platform, sys
build=pathlib.Path(sys.argv[1])
def read(name):
    with (build/name).open(newline="",encoding="utf-8") as f:
        return list(csv.DictReader(f))
short=read("hd-unison-engine-benchmark.csv")
long=read("hd-unison-endurance.csv")
lines=["# HD unison acceleration — "+platform.system()+" "+platform.machine(),
       "", "| Mode | Scenario | Mean budget % | Worst trial p99 % | Peak ms | Overruns | Audio pass |",
       "|---|---|---:|---:|---:|---:|---:|"]
names={"0":"Reference","1":"Grouped scalar","2":"NEON/SSE2","3":"SoA 8","4":"AVX2 gather 8"}
for r in short+long:
    lines.append("| "+names[r["mode"]]+" | "+r["scenario"]+" | "+ " | ".join(f'{float(r[k]):.3f}' for k in ("mean_budget_pct","p99_budget_pct","max_ms"))+" | "+r["overruns"]+" | "+r["quality_pass"]+" |")
lines+=["","Short tests: two trials; peak, overruns and p99 use worst/all trials. Endurance: 60 rendered seconds per available mode. No real-time claim if overruns are nonzero.",""]
for mode in ("3","4"):
    rows=[r for r in short+long if r["mode"]==mode]
    if not rows:
        lines.append(names[mode]+": unavailable on this runner.")
        continue
    refs={r["scenario"]:r for r in short+long if r["mode"]=="0"}
    def ok(r):
        ref=refs[r["scenario"]]
        return (r["quality_pass"]=="1" and float(r["mean_ms"])<=float(ref["mean_ms"])*1.03
                and float(r["p99_ms"])<=float(ref["p99_ms"])*1.03
                and int(r["overruns"])<=int(ref["overruns"]))
    end=next(r for r in rows if "endurance" in r["scenario"])
    ref=refs[end["scenario"]]
    eligible=all(ok(r) for r in rows) and float(end["mean_ms"])<=float(ref["mean_ms"])*.95 and int(end["overruns"])==0
    lines.append(names[mode]+(": eligible for further hardware validation." if eligible else ": keep disabled; performance or deadline criteria not met."))
lines.append("Production mode remains scalar (0) on all architectures.")
report="\n".join(lines)+"\n"
(build/"hd-unison-decision.md").write_text(report,encoding="utf-8")
print(report)
