async function loadBuildInfo() {
  const el = document.getElementById("build-info");
  const debMeta = document.getElementById("deb-meta");
  const binMeta = document.getElementById("bin-meta");

  try {
    const res = await fetch("downloads/build.json", { cache: "no-store" });
    if (!res.ok) {
      throw new Error("missing build.json");
    }
    const info = await res.json();
    const version = info.version || "latest";
    const date = info.built_at || "";
    const sha = info.commit ? String(info.commit).slice(0, 7) : "";

    el.textContent = [
      version ? `v${version}` : null,
      date ? `built ${date}` : null,
      sha ? `commit ${sha}` : null,
    ]
      .filter(Boolean)
      .join(" · ");

    if (debMeta && info.deb_name) {
      debMeta.textContent = `${info.deb_name} · amd64`;
    }
    if (binMeta) {
      binMeta.textContent = `Linux x86_64 · Qt 6 · v${version}`;
    }

    const deb = document.getElementById("btn-deb");
    const bin = document.getElementById("btn-bin");
    if (deb && info.deb_name) {
      deb.setAttribute("download", info.deb_name);
    }
    if (bin) {
      bin.setAttribute("download", `spaste-${version}-linux-x86_64`);
    }
  } catch (_) {
    el.textContent = "Latest build from main";
  }
}

loadBuildInfo();
